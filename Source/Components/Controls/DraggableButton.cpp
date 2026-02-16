
#include "DraggableButton.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
/** Custom Draggable buttons to work as eq thumbs across the response curve
*/
DraggableButton::DraggableButton(SemiProQAudioProcessor& p, SemiProQAudioProcessorEditor& e, int filterId, juce::Colour c) :
                                 Button(juce::String()), audioProcessor(p), editor(e), associatedFilter(filterId), circleColour(c) {
    setSize(DRAG_BUTTON_DIM, DRAG_BUTTON_DIM);
    audioProcessor.tree.addParameterListener(params[FREQ + filterId * PARAMS_PER_FILTER], this);
    audioProcessor.tree.addParameterListener(params[GAIN + filterId * PARAMS_PER_FILTER], this);
    audioProcessor.tree.addParameterListener(params[BYPASS + filterId * PARAMS_PER_FILTER], this);
    freq.store(*audioProcessor.tree.getRawParameterValue(params[FREQ + filterId * PARAMS_PER_FILTER]));
    gain.store(*audioProcessor.tree.getRawParameterValue(params[GAIN + filterId * PARAMS_PER_FILTER]));
    isBypassed.store(*audioProcessor.tree.getRawParameterValue(params[BYPASS + filterId * PARAMS_PER_FILTER]));
}

DraggableButton::~DraggableButton() {
    audioProcessor.tree.removeParameterListener(params[FREQ + associatedFilter * PARAMS_PER_FILTER], this);
    audioProcessor.tree.removeParameterListener(params[GAIN + associatedFilter * PARAMS_PER_FILTER], this);
    audioProcessor.tree.removeParameterListener(params[BYPASS + associatedFilter * PARAMS_PER_FILTER], this);
}

void DraggableButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    //set center from cached values
    setCentreFromFreq(freq.load());
    setCentreFromGain(gain.load());
    //fill ellipse with color based on bypassed state
    g.setColour(isBypassed ? juce::Colours::grey : circleColour);
    auto bounds = getLocalBounds().toFloat();
    g.fillEllipse(bounds);
    //if selected, draw white outline
    if ((int)(editor.selectedFilterValue.getValue()) == associatedFilter) {
        g.setColour(juce::Colours::white);
        g.drawEllipse(bounds.reduced(1.0f), 2.0f);
    }
}

//may want to add a small buffer to make it more forgiving
bool DraggableButton::hitTest(int x, int y) {
    auto centre = getLocalBounds().getCentre();
    auto radius = getLocalBounds().getWidth() / 2.0f;
    return centre.getDistanceFrom({ x, y }) <= radius;
}

//only change on gain, freq or bypass here
void DraggableButton::parameterChanged(const juce::String& paramID, float newValue) {
    if (paramID == params[FREQ + associatedFilter * PARAMS_PER_FILTER]) {
        freq.store(newValue);
    }
    else if (paramID == params[GAIN + associatedFilter * PARAMS_PER_FILTER]) {
        gain.store(newValue);
    }
    else if (paramID == params[BYPASS + associatedFilter * PARAMS_PER_FILTER]) {
        isBypassed = (newValue >= 0.5f);
    }
    updateTooltip();
    needsRepaint.store(true);
}

//select on left, delete on right
void DraggableButton::mouseDown(const juce::MouseEvent& event) {
    //if left click, select this eq
    if (event.mods.isLeftButtonDown()) {
        editor.selectedFilterValue = associatedFilter;
        needsRepaint.store(true);
    }
    //if right click, delete button and reset filter
    else if (event.mods.isRightButtonDown()) {
        editor.buttonReset(associatedFilter);
    }
    dragger.startDraggingComponent(this, event);
}

//drag to position in limit of given by editor, set position and update params from position
void DraggableButton::mouseDrag(const juce::MouseEvent& event) {
    dragger.dragComponent(this, event, nullptr);
    //drag and keep in button bounds
    auto x = juce::jlimit(editor.buttonBounds.getX(), editor.buttonBounds.getRight(), getX());
    auto y = juce::jlimit(editor.buttonBounds.getY(), editor.buttonBounds.getBottom(), getY());
    //set button center, then update params
    setCentrePosition(x, y);
    updateParamsFromPosition();
}

void DraggableButton::updateParamsFromPosition() {
    auto centre = getBounds().getCentre();
    //get pixel space 0-1, convert with freq range to hz
    float xNorm = (centre.getX() - editor.buttonBounds.getX()) / float(editor.buttonBounds.getWidth());
    float freq = freqRange.convertFrom0to1(xNorm);
    //get pixel space, gain is linear so no more need to convert, just invert
    float yNorm = (centre.getY() - editor.buttonBounds.getY()) / float(editor.buttonBounds.getHeight());
    //converrt freq back to 0-1, invert gain, then update tooltip
    audioProcessor.updateParameter(associatedFilter, FREQ, freqRange.convertTo0to1(freq));
    audioProcessor.updateParameter(associatedFilter, GAIN, 1.0f - yNorm);
    updateTooltip();
}

void DraggableButton::updatePositionFromParams() {
    //uses cached values
    setCentreFromFreq(freq.load());
    setCentreFromGain(gain.load());
    needsRepaint.store(true);
}

void DraggableButton::setCentreFromFreq(float freq) {
    //get center using logRange 0-1 value
    int x = editor.buttonBounds.getX() + freqRange.convertTo0to1(freq) * editor.buttonBounds.getWidth();
    setCentrePosition(x, getBounds().getCentreY());
}

void DraggableButton::setCentreFromGain(float gain) {
    //maps gain to 0-1
    int y = juce::jmap(gain, MIN_DB, MAX_DB, float(editor.buttonBounds.getBottom()), float(editor.buttonBounds.getY()));
    setCentrePosition(getBounds().getCentreX(), y);
}

void DraggableButton::updateTooltip() {
    //use cached vals to build tooltip
    juce::String tip = formatFrequency(freq.load()) + ", " + formatGain(gain.load());
    setTooltip(tip);
}
