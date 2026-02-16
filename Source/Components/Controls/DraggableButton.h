
#pragma once
#include <JuceHeader.h>
#include "Utils/Constants.h"

class SemiProQAudioProcessor;
class SemiProQAudioProcessorEditor;

//==============================================================================
/** Draggable buttons to select and adjust parameters without needing sliders
*/
struct DraggableButton : juce::Button, juce::AudioProcessorValueTreeState::Listener {
    DraggableButton(SemiProQAudioProcessor&, SemiProQAudioProcessorEditor&, int bandId, juce::Colour c);
    ~DraggableButton();

    //update linked position and params accordingly
    void updatePositionFromParams();
    void updateParamsFromPosition();

    std::atomic<bool> needsRepaint{ false };

private:
    //paint for button objects
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    //handle left or right click
    void mouseDown(const juce::MouseEvent& event) override;
    //drag and update parameters
    void mouseDrag(const juce::MouseEvent& event) override;
    bool hitTest(int x, int y) override;
    void updateTooltip();
    //callback from slider, automation, or button changes
    void parameterChanged(const juce::String& paramID, float newValue) override;
    //helpers for position updates
    void setCentreFromFreq(float freq);
    void setCentreFromGain(float gain);

    SemiProQAudioProcessor& audioProcessor;
    SemiProQAudioProcessorEditor& editor;
    juce::Colour circleColour;
    int associatedFilter;
    juce::ComponentDragger dragger;
    std::atomic<bool> isBypassed{ false };
    std::atomic<float> freq{ 0.0f }, gain{ 0.0f };
};
