
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
/** Parent editor component of all GUI elements
*/
SemiProQAudioProcessorEditor::SemiProQAudioProcessorEditor(SemiProQAudioProcessor& p) : AudioProcessorEditor(&p), audioProcessor(p), 
      selectedFilterComponent(audioProcessor, *this, -1, 0, lnfb, lnfd), responseCurveComponent(audioProcessor), analyserComponent(audioProcessor), 
      gainComponent(audioProcessor, *this, lnfd), peakComponent(audioProcessor), settingsComponent(audioProcessor, *this, lnfc, lnfd, lnfe) {
    //current aspect ratio = 16 / 9
    setSize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);

    //draggable button bounding box
    buttonBounds = getRenderArea();

    //cached bg
    backgroundImage();

    //resize and set up anaylser, peak meters, and response curve
    resizeVisualizers();
    addAndMakeVisible(analyserComponent);
    addAndMakeVisible(responseCurveComponent);
    addAndMakeVisible(peakComponent);
    audioProcessor.setCurveStatus(true);

    //get property values and link internal values
    referAndAddListener(selectedFilterValue, SELECTED_FILTER);
    referAndAddListener(minSelected, MINIMIZE_SELECTED);
    referAndAddListener(minGain, MINIMIZE_GAIN);
    referAndAddListener(minSettings, MINIMIZE_SETTINGS);
    referAndAddListener(analyserOnValue, ANALYSER_ON);
    referAndAddListener(analyserModeValue, ANALYSER_MODE);
    referAndAddListener(peakOnValue, PEAK_ON);
    referAndAddListener(peakModeValue, PEAK_MODE);

    setupDraggableButtons();

    //setup minimizable and draggable components
    setupMinimizableComp(settingsComponent, minSettings);
    setupMinimizableComp(selectedFilterComponent, minSelected);
    setupMinimizableComp(gainComponent, minGain);

    //update comps to values
    selectedFilterChanged();
    analyserOnChanged();
    analyserModeChanged();
    peakOnChanged();
    peakModeChanged();

    tooltipWindow.setOpaque(false);

    startTimerHz(TIMER_FPS);
}

SemiProQAudioProcessorEditor::~SemiProQAudioProcessorEditor() {
    //stop timer, remove weak refs, and remove listeners
    stopTimer();

    settingsComponent.setLookAndFeel(nullptr);
    selectedFilterComponent.setLookAndFeel(nullptr);
    gainComponent.setLookAndFeel(nullptr);

    selectedFilterValue.removeListener(this);
    minSettings.removeListener(this);
    minGain.removeListener(this);
    minSelected.removeListener(this);

    analyserOnValue.removeListener(this);
    analyserModeValue.removeListener(this);
    peakOnValue.removeListener(this);
    peakModeValue.removeListener(this);
}

//==============================================================================
void SemiProQAudioProcessorEditor::paint(juce::Graphics& g) {
    g.drawImage(background, getLocalBounds().toFloat());
}

//rescale background, set bounds for analyser, response curve, and peak area
void SemiProQAudioProcessorEditor::resized() {
    background = background.rescaled(getLocalBounds().getWidth(), getLocalBounds().getHeight(), juce::Graphics::mediumResamplingQuality);

    resizeVisualizers();
}

//==============================================================================
// PRIMARY FUNCTIONALITY
//makes draggable button visible on double click and initializes filter to click position, then sets as selected filter
void SemiProQAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event) {
    auto mousePos = event.getPosition();
    if (!buttonBounds.contains(mousePos))
        return;

    //finds first uninitialized filter, sets position, updates the filter's params, sets the init property in tree, update bypass, 
    //set prior and new filter for repaint, update selectedFilterValue, and set new filter button to visible
    for (int i = 0; i < MAX_FILTERS; ++i) {
        if (!audioProcessor.tree.state[props[i]]) {
            buttonArr[i]->setCentrePosition(mousePos);
            buttonArr[i]->updateParamsFromPosition();
            audioProcessor.tree.state.setProperty(props[i], true, nullptr);
            audioProcessor.updateParameter(i, BYPASS, false);
            int val = selectedFilterValue.getValue();
            if (val != -1) {
                buttonArr[selectedFilterValue.getValue()]->needsRepaint.store(true);
            }
            selectedFilterValue.setValue(i);
            buttonArr[i]->setVisible(true);
            buttonArr[i]->needsRepaint.store(true);
            return;
        }
    }
    //if more than twelve, show alert for max filters
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Maximum Filter Limit Reached", "The limit of filters is 12.");
}

//reset button on right click and run vis check for Selected Filter Component
void SemiProQAudioProcessorEditor::buttonReset(int id) {
    audioProcessor.resetEq(id);
    buttonArr[id]->setVisible(false);
    sfcVisiblityCheck();
    responseCurveComponent.repaint();
}

//checks if an eq is initialized, if so makes the highest indexed initialized eq selected if no others are selected, else set visibility false
void SemiProQAudioProcessorEditor::sfcVisiblityCheck() {
    for (int i = MAX_FILTERS - 1; i >= 0; --i) {
        if (buttonArr[i]->isVisible()) {
            selectedFilterValue.setValue(i);
            return;
        }
    }
    selectedFilterComponent.setVisible(false);
    selectedFilterValue.setValue(-1);
}

//listener callback, swiches between changed funcs below to notify comps
void SemiProQAudioProcessorEditor::valueChanged(juce::Value& value) {
    //selected filter value change. This has this much logic to avoid a parameter change on the prior filter in selectedFilterComponent
    if (value.refersToSameSourceAs(selectedFilterValue)) {
        selectedFilterChanged();
    }
    //if Settings minimize button is pressed
    else if (value.refersToSameSourceAs(minSettings)) {
        settingsComponent.applyMinimized(settingsComponent, minSettings.getValue());
    }
    //if gain minimize button is pressed
    else if (value.refersToSameSourceAs(minGain)) {
        gainComponent.applyMinimized(gainComponent, minGain.getValue());
    }
    //if selected eq component minimize button is pressed
    else if (value.refersToSameSourceAs(minSelected)) {
        selectedFilterComponent.applyMinimized(selectedFilterComponent, minSelected.getValue());
    }
    //analyser on button pressed
    else if (value.refersToSameSourceAs(analyserOnValue)) {
        analyserOnChanged();
    }
    //analyser mode button pressed
    else if (value.refersToSameSourceAs(analyserModeValue)) {
        analyserModeChanged();
    }
    //peak on button pressed
    else if (value.refersToSameSourceAs(peakOnValue)) {
        peakOnChanged();
    }
    //peak mode button pressed
    else if (value.refersToSameSourceAs(peakModeValue)) {
        peakModeChanged();
    }
}

void SemiProQAudioProcessorEditor::selectedFilterChanged() {
    int id = selectedFilterValue.getValue();

    if (id > -1 && id < buttonArr.size()) {
        selectedFilterComponent.updateFilterAndSliders(id);
        selectedFilterComponent.setVisible(true);
        buttonArr[id]->repaint();
    }
    else {
        selectedFilterComponent.setVisible(false);
    }
}

void SemiProQAudioProcessorEditor::analyserOnChanged() {
    bool isOn = analyserOnValue.getValue();

    analyserComponent.setVisible(isOn);
}

void SemiProQAudioProcessorEditor::analyserModeChanged() {
    bool isPost = analyserModeValue.getValue();
    auto color = isPost ? juce::Colours::lime : juce::Colours::yellow;
    analyserComponent.lineColor = color;
}

void SemiProQAudioProcessorEditor::peakOnChanged() {
    bool isOn = peakOnValue.getValue();
    if (!isOn) {
        peakComponent.resetValues();
        peakComponent.repaint();
    }
}

void SemiProQAudioProcessorEditor::peakModeChanged() {
    bool isPost = peakModeValue.getValue();
    auto color = isPost ? juce::Colours::lime : juce::Colours::yellow;
    peakComponent.fillColor = color;
}

//controls repaints of spec, peak, draggable buttons, and res curve
void SemiProQAudioProcessorEditor::timerCallback() {
    //only draw if curve is dirty
    if (audioProcessor.getCurveStatus()) {
        responseCurveComponent.timerCallback();
        audioProcessor.setCurveStatus(false);
    }
    //only repaint if true
    for (auto& button : buttonArr) {
        if (button->needsRepaint.load()) {
            button->repaint();
            button->needsRepaint.store(false);
        }
    }
    //if on, redraw frame
    if (analyserOnValue.getValue()) {
        analyserComponent.timerCallback();
    }
    //if on, redraw frame
    if (peakOnValue.getValue()) {
        peakComponent.timerCallback();
    }
}

//===============================================================================
//CONSTRUCTION/RESIZE HELPERS
juce::Rectangle<int> SemiProQAudioProcessorEditor::getRenderArea() {
    auto bounds = getLocalBounds();
    bounds.reduce(BORDER_SPACING, BORDER_SPACING);
    bounds.removeFromRight(METER_AREA_WIDTH);
    bounds.removeFromTop(6);
    return bounds;
}

void SemiProQAudioProcessorEditor::resizeVisualizers() {
    auto area = getRenderArea();
    auto peakArea = getBounds().removeFromRight(METER_AREA_WIDTH + BORDER_SPACING);
    //uses area with the removed right section
    analyserComponent.setBounds(area);
    responseCurveComponent.setBounds(area);
    //add the bottom back to it for the peak
    peakArea.setBottom(area.getBottom() + BORDER_SPACING);
    peakArea.setRight(peakArea.getRight() + BORDER_SPACING);
    peakComponent.setBounds(peakArea);
}

void SemiProQAudioProcessorEditor::referAndAddListener(juce::Value& val, int idx) {
    val.referTo(audioProcessor.tree.state.getPropertyAsValue(props[idx], nullptr));
    val.addListener(this);
}

void SemiProQAudioProcessorEditor::setupDraggableButtons() {
    //build button array and check if we are coming from load then set accordingly
    for (int i = 0; i < MAX_FILTERS; ++i) {
        buttonArr.add(new DraggableButton(audioProcessor, *this, i, colours[i]));
        addChildComponent(buttonArr[i]);
    }
    for (int i = 0; i < MAX_FILTERS; ++i) {
        if (audioProcessor.tree.state[props[i]]) {
            buttonArr[i]->updatePositionFromParams();
            buttonArr[i]->setVisible(true);
        }
        else {
            buttonArr[i]->setVisible(false);
        }
    }
}

void SemiProQAudioProcessorEditor::setupMinimizableComp(MinimizableComponent& c, juce::Value& v) {
    c.setLookAndFeel(&lnfa);
    addAndMakeVisible(c);
    c.referValuesToButtons(v);
    c.applyMinimized(c, v.getValue());
}

void SemiProQAudioProcessorEditor::backgroundImage() {
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    Graphics g(background);
    StringArray freqs{ "20", "30", "40", "50", "100", "200", "300", "400",
                       "500", "1K", "2K", "3K", "4K", "5K", "10K", "20K" };
    //PRECOMPUTED WITH logRange<float>
    Array<float> normX{ 0, 0.0586971, 0.100343, 0.132647, 0.23299, 0.333333, 0.39203, 0.433677,
                        0.46598, 0.566323, 0.666667, 0.725364, 0.76701, 0.799313, 0.899657, .999999 };
    Array<float> xs;
    Array<float> ys;
    StringArray gain{ "-72", "-60", "-48", "-36","-24", "-12", " 0", "+12", "+24" };
    auto renderArea = getRenderArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    auto height = renderArea.getHeight();

    //get x pixels from normX
    for (auto norm : normX) {
        xs.add(left + width * norm);
    }

    //draw freq lines 
    g.setColour(Colours::dimgrey);
    for (auto x : xs) {
        g.drawVerticalLine(x, top, bottom);
    }

    //get y pixels from ys then draw line
    for (int i = MIN_DB; i < MAX_DB + 12; i += 12) {
        int y = jmap(i, (int)MIN_DB, (int)MAX_DB, bottom, top);
        ys.add(y);
        g.setColour(i == 0.0f ? Colours::limegreen : Colours::darkgrey);
        g.drawHorizontalLine(y, left, right);
    }

    //write freq numbers
    g.setColour(Colours::lightgrey);
    g.setFont(BG_FONT_SIZE);
    for (int i = 0; i < freqs.size(); ++i) {
        auto f = freqs[i];
        auto x = xs[i];
        auto textWidth = g.getCurrentFont().getStringWidth(f);
        Rectangle<int> r;
        r.setSize(textWidth, BG_FONT_SIZE);
        r.setCentre(x - 2, ys[8] - 7);
        g.drawFittedText(f, r, juce::Justification::centred, 1);
    }

    //write dB numbers
    auto n = ys.size() - 1;
    auto textWidth = g.getCurrentFont().getStringWidth(gain[n]);
    ys.set(n, ys[n] + 4);
    for (int i = 0; i < ys.size(); ++i) {
        Rectangle<int> r;
        r.setSize(textWidth, BG_FONT_SIZE);
        r.setX(right + 2);
        r.setY(ys[i] - 5);
        g.setColour(i == 6 ? Colours::limegreen : Colours::lightgrey);
        g.drawFittedText(gain[i], r, juce::Justification::centred, 1);
    }

    //rectangle around render area
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(left, top, width, height, CORNER_SIZE, 1.0f);

    peakComponent.setYs(ys);
    peakComponent.resized();
}

void SemiProQAudioProcessorEditor::openHelpDialog() {
    helpWindow.reset(new HelpWindow(lnff));
    addAndMakeVisible(helpWindow.get());
    helpWindow->centreWithSize(HELP_WINDOW_WIDTH, HELP_WINDOW_HEIGHT);
    helpWindow->toFront(true);
}

void SemiProQAudioProcessorEditor::openCreditsDialog() {
    creditsWindow.reset(new CreditsWindow(lnff));
    addAndMakeVisible(creditsWindow.get());
    creditsWindow->centreWithSize(CREDIT_WINDOW_WIDTH, CREDIT_WINDOW_HEIGHT);
    creditsWindow->toFront(true);
}
