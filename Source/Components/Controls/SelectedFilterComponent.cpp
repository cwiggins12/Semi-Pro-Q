
#include "SelectedFilterComponent.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomLookAndFeel.h"

//==============================================================================
/** Selected Filter component: when the draggable button is selected, all of its parameters show up here, god bless juce for slider attachments
*/
SelectedFilterComponent::SelectedFilterComponent(SemiProQAudioProcessor& p, SemiProQAudioProcessorEditor& e, int id, int type, CustomLookAndFeelB& lnfb, CustomLookAndFeelD& lnfd)
                                           : MinimizableComponent(lnfd, SELECTED_SIZE_X, SELECTED_SIZE_Y, SELECTED_TOPLEFT_X, SELECTED_TOPLEFT_Y), 
                                             audioProcessor(p), editor(e), currFilter(id), currFilterType(type) {
    auto w = getWidth() / 3;
    //setup CheapLabels
    componentLabel.setText("SELECTED FILTER");
    freqLabel.setText("FREQ");
    gainLabel.setText("GAIN");
    auto qText = (currFilterType == HIGHPASS_OCT || currFilterType == LOWPASS_OCT) ? "DB/OCTAVE" : "Q";
    qualityLabel.setText(qText);
    typeLabel.setText("TYPE");
    bypassLabel.setText("BYPASS");
    deleteLabel.setText("DELETE");

    //slider setup
    makeSlider(freqSlider, w);
    makeSlider(gainSlider, w);
    makeSlider(qualitySlider, w);

    //combo box setup
    addAndMakeVisible(typeComboBox);
    //value ids are plus 1 since item value id can't be 0
    for (int i = PEAK; i <= NOTCH; ++i) {
        typeComboBox.addItem(filterTypes[i], i + 1);
    }

    //button setups
    addAndMakeVisible(bypassButton);
    bypassButton.setClickingTogglesState(true);

    deleteButton.setLookAndFeel(&lnfb);
    addAndMakeVisible(deleteButton);
    deleteButton.onClick = [this] {
        if (currFilter >= 0) {
            editor.buttonReset(currFilter);
        }
    };
    //if saved position, update to match
    resetTopLeftProps();
    //if minimized, update to match
    applyMinimized(*this, editor.minSelected.getValue());
    addAndMakeVisible(minButton);
}

SelectedFilterComponent::~SelectedFilterComponent() {
    //remove any potential type listeners
    deleteButton.setLookAndFeel(nullptr);
    for (int i = 0; i < MAX_FILTERS; ++i) {
        audioProcessor.tree.removeParameterListener(params[TYPE + i * PARAMS_PER_FILTER], this);
    }
}

void SelectedFilterComponent::updateFilterAndSliders(int id) {
    //empty the sliders/listeners to not trigger a param change to prior eq
    audioProcessor.tree.removeParameterListener(params[TYPE + currFilter * PARAMS_PER_FILTER], this);
    freqSliderAttachment.reset();
    gainSliderAttachment.reset();
    qualitySliderAttachment.reset();
    typeBoxAttachment.reset();
    bypassButtonAttachment.reset();

    //set new eq and attach all params to sliders/listener
    currFilter = id;
    currFilterType = audioProcessor.getFilterInfo(id).type.load();

    freqSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, params[FREQ + currFilter * PARAMS_PER_FILTER], freqSlider);
    gainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, params[GAIN + currFilter * PARAMS_PER_FILTER], gainSlider);
    (currFilterType == HIGHPASS_OCT || currFilterType == LOWPASS_OCT) ? swapQualitySlider(B_WORTH, "DB/OCTAVE") : swapQualitySlider(QUALITY, "Q");
    typeBoxAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.tree, params[TYPE + currFilter * PARAMS_PER_FILTER], typeComboBox);
    bypassButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.tree, params[BYPASS + currFilter * PARAMS_PER_FILTER], bypassButton);
    deleteButton.setToggleState(false, juce::NotificationType::dontSendNotification);
    audioProcessor.tree.addParameterListener(params[TYPE + currFilter * PARAMS_PER_FILTER], this);
    repaint();
}

void SelectedFilterComponent::paint(juce::Graphics& g) {
    if (!isMin && currFilter != -1) {
        //background
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::grey);
        g.fillRoundedRectangle(bounds, CORNER_SIZE);

        //div lines
        auto third = bounds.getWidth() / 3;
        auto x = bounds.getX();
        g.setColour(juce::Colours::black);
        g.fillRect(x + third - 1, bounds.getY(), DIV_LINE_WIDTH, bounds.getHeight());
        g.fillRect(x + third + third - 1, bounds.getY(), DIV_LINE_WIDTH, bounds.getHeight());

        //labels and filter color
        componentLabel.paint(g);
        freqLabel.paint(g);
        gainLabel.paint(g);
        qualityLabel.paint(g);
        typeLabel.paint(g);
        bypassLabel.paint(g);
        deleteLabel.paint(g);
        selectedColor.setColour(editor.getColour(currFilter));
        selectedColor.paint(g);
        componentLabel.paintOutline(g);
        freqLabel.paintOutline(g);
        gainLabel.paintOutline(g);
        qualityLabel.paintOutline(g);
        typeLabel.paintOutline(g);
        bypassLabel.paintOutline(g);
        deleteLabel.paintOutline(g);

        //outline
        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(bounds, CORNER_SIZE, 2.0f);
    }
}

void SelectedFilterComponent::resized() {
    auto bounds = getLocalBounds();
    if (isMin) {
        //clear all bounds but min button
        freqLabel.setBounds(0, 0, 0, 0);
        freqSlider.setBounds(0, 0, 0, 0);
        qualityLabel.setBounds(0, 0, 0, 0);
        qualitySlider.setBounds(0, 0, 0, 0);
        gainLabel.setBounds(0, 0, 0, 0);
        gainSlider.setBounds(0, 0, 0, 0);
        typeLabel.setBounds(0, 0, 0, 0);
        typeComboBox.setBounds(0, 0, 0, 0);
        deleteLabel.setBounds(0, 0, 0, 0);
        deleteButton.setBounds(0, 0, 0, 0);
        bypassLabel.setBounds(0, 0, 0, 0);
        bypassButton.setBounds(0, 0, 0, 0);
        componentLabel.setBounds(0, 0, 0, 0);
        selectedColor.setBounds(0, 0, 0, 0);
        minButton.setTopLeftPosition(bounds.getX(), bounds.getY());
    }
    else {
        //splits width into thirds and height into halves
        bounds.reduce(1, 1);
        auto minBounds = bounds.removeFromTop(MINIMIZE_BUTTON_DIM);
        componentLabel.setBounds(minBounds);
        selectedColor.setBounds(minBounds.removeFromLeft(MINIMIZE_BUTTON_DIM));
        minButton.setBounds(minBounds.removeFromRight(MINIMIZE_BUTTON_DIM));

        auto w = bounds.getWidth() / 3;
        auto top = bounds.removeFromTop(bounds.getHeight() / 2);
        //helpers for fader row
        makeResizedSection(freqLabel, freqSlider, top.removeFromLeft(w));
        makeResizedSection(qualityLabel, qualitySlider, top.removeFromRight(w));
        makeResizedSection(gainLabel, gainSlider, top);
        //helpers for button/combo box row
        makeResizedButtonSection(typeLabel, typeComboBox, bounds.removeFromLeft(w));
        makeResizedButtonSection(deleteLabel, deleteButton, bounds.removeFromRight(w));
        makeResizedButtonSection(bypassLabel, bypassButton, bounds);
    }
}

void SelectedFilterComponent::parameterChanged(const juce::String& paramID, float newValue) {
    //swap q slider if type changes from q to dB/oct 
    if ((newValue == HIGHPASS_OCT || newValue == LOWPASS_OCT) && currFilterType != HIGHPASS_OCT && currFilterType != LOWPASS_OCT) {
        swapQualitySlider(B_WORTH, "DB/OCTAVE");
        //sets q value to 1.0f, update takes a 0-1, so it looks confusing. 
        audioProcessor.updateParameter(currFilter, QUALITY, 0.1f);
    }
    else if (newValue != HIGHPASS_OCT && newValue != LOWPASS_OCT && (currFilterType == HIGHPASS_OCT || currFilterType == LOWPASS_OCT)) {
        swapQualitySlider(QUALITY, "Q");
        //reset db/oct value
        audioProcessor.updateParameter(currFilter, B_WORTH, 0);
    }
    currFilterType = newValue;
}

void SelectedFilterComponent::makeSlider(juce::Slider& slider, int w) {
    //slider construction
    addAndMakeVisible(slider);
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, w, TEXTBOX_HEIGHT);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void SelectedFilterComponent::swapQualitySlider(int type, juce::String text) {
    qualitySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, params[type + currFilter * PARAMS_PER_FILTER], qualitySlider);
    qualityLabel.setText(text);
}

void SelectedFilterComponent::makeResizedSection(CheapLabel& l, juce::Component& c, juce::Rectangle<int> r) {
    l.setBounds(r.removeFromTop(LABEL_HEIGHT));
    c.setBounds(r);
}

void SelectedFilterComponent::makeResizedButtonSection(CheapLabel& l, juce::Component& c, juce::Rectangle<int> r) {
    l.setBounds(r.removeFromTop(LABEL_HEIGHT));
    c.setBounds(makeSquare(r.reduced(PARAM_BUTTON_SPACING)));
}

void SelectedFilterComponent::mouseDrag(const juce::MouseEvent& event) {
    MinimizableComponent::mouseDrag(event);
    audioProcessor.tree.state.setProperty(props[SELECTED_X], topLeft.x, nullptr);
    audioProcessor.tree.state.setProperty(props[SELECTED_Y], topLeft.y, nullptr);
}

void SelectedFilterComponent::resetTopLeftProps() {
    if (audioProcessor.tree.state.hasProperty(props[SELECTED_X])) {
        topLeft.x = audioProcessor.tree.state[props[SELECTED_X]];
    }
    else {
        audioProcessor.tree.state.setProperty(props[SELECTED_X], SELECTED_TOPLEFT_X, nullptr);
    }
    if (audioProcessor.tree.state.hasProperty(props[SELECTED_Y])) {
        topLeft.y = audioProcessor.tree.state[props[SELECTED_Y]];
    }
    else {
        audioProcessor.tree.state.setProperty(props[SELECTED_Y], SELECTED_TOPLEFT_Y, nullptr);
    }
}