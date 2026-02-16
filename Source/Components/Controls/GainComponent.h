
#pragma once
#include <JuceHeader.h>
#include "Components/Controls/MinimizableComponentParent.h"

class SemiProQAudioProcessor;
class SemiProQAudioProcessorEditor;
class CustomLookAndFeelD;

//==============================================================================
/** Pre and Post gain sliders, can minimize
*/
struct GainComponent : public MinimizableComponent {
    GainComponent(SemiProQAudioProcessor&, SemiProQAudioProcessorEditor&, CustomLookAndFeelD& lnfd);
    ~GainComponent() override;

private:
    void paint(juce::Graphics& g) override;
    void resized() override;
    //helpers to clean up constructor and resized
    void makeSlider(juce::Slider& slider);
    void makeResizedSection(CheapLabel& l, juce::Component& c, juce::Rectangle<int> r);
    void mouseDrag(const juce::MouseEvent& event) override;
    void resetTopLeftProps();

    SemiProQAudioProcessor& audioProcessor;
    SemiProQAudioProcessorEditor& editor;

    juce::Slider preGainSlider, postGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> postGainAttachment;
    CheapLabel preGainLabel, postGainLabel, componentLabel;
};
