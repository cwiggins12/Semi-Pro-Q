
#pragma once

#include <JuceHeader.h>
#include "Utils/Constants.h"
#include "Components/Controls/MinimizableComponentParent.h"

class SemiProQAudioProcessor;
class SemiProQAudioProcessorEditor;
class CustomLookAndFeelB;
class CustomLookAndFeelD;

//color box in top left corner of component to match selected filter draggable button color
struct ColorIndicator {
public:
    void setColour(juce::Colour c) { colour = c; }

    void setBounds(int x, int y, int w, int h) { bounds = { x, y, w, h }; }
    void setBounds(juce::Rectangle<int> b) { bounds = b; }

    void paint(juce::Graphics& g) {
        g.setColour(colour);
        g.fillRect(bounds);
    }

private:
    juce::Colour colour;
    juce::Rectangle<int> bounds;
};

//==============================================================================
/** Display's selected filter parameters as sliders and buttons
*/
struct SelectedFilterComponent : public MinimizableComponent, juce::AudioProcessorValueTreeState::Listener {
    SelectedFilterComponent(SemiProQAudioProcessor&, SemiProQAudioProcessorEditor&, int id, int type, CustomLookAndFeelB& lnfb, CustomLookAndFeelD& lnfd);
    ~SelectedFilterComponent() override;

    //updates component objects on eq selection change
    void updateFilterAndSliders(int id);

private:
    //component update callbacks
    void paint(juce::Graphics& g) override;
    void resized() override;
    //callback for currEq type change, to swap q and db/oct
    void parameterChanged(const juce::String& paramID, float newValue) override;
    //used on buttons to make it square based on area
    juce::Rectangle<int> makeSquare(juce::Rectangle<int> area) {
        int side = std::min(area.getWidth(), area.getHeight());
        return area.withSizeKeepingCentre(side, side);
    }
    //construction and painting helpers
    void makeLabel(juce::Label& label, juce::String text);
    void makeSlider(juce::Slider& slider, int w);
    void makeResizedSection(CheapLabel& l, juce::Component& c, juce::Rectangle<int> r);
    void makeResizedButtonSection(CheapLabel& l, juce::Component& c, juce::Rectangle<int> r);
    void swapQualitySlider(int type, juce::String text);
    void resetTopLeftProps();
    //overide of parent virtual to update position properties to save/load
    void mouseDrag(const juce::MouseEvent& event) override;

    SemiProQAudioProcessor& audioProcessor;
    SemiProQAudioProcessorEditor& editor;

    int currFilter, currFilterType;

    juce::Slider freqSlider, gainSlider, qualitySlider;
    juce::ComboBox typeComboBox;
    juce::ToggleButton bypassButton;
    juce::TextButton deleteButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> qualitySliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeBoxAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassButtonAttachment;

    ColorIndicator selectedColor;
    CheapLabel freqLabel, gainLabel, qualityLabel, typeLabel, bypassLabel, deleteLabel, componentLabel;
};
