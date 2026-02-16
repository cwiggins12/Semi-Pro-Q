
#pragma once

#include <JuceHeader.h>
#include "Components/Controls/MinimizableComponentParent.h"

class SemiProQAudioProcessor;
class SemiProQAudioProcessorEditor;
class CustomLookAndFeelC;
class CustomLookAndFeelD;
class CustomLookAndFeelE;

//==============================================================================
/** houses analyser and peak settings buttons, can minimize
*/
struct SettingsComponent : public MinimizableComponent {
    SettingsComponent(SemiProQAudioProcessor&, SemiProQAudioProcessorEditor&, CustomLookAndFeelC&, CustomLookAndFeelD&, CustomLookAndFeelE&);
    ~SettingsComponent() override;

    //helper to set buttons to editor values
    void referValuesToButtons(juce::Value& v) override;

private:
    void paint(juce::Graphics& g) override;
    void resized() override;
    //helpers to clean up constructor and resized
    //void makeLabel(juce::Label& label, juce::String text);
    void makeResizedSection(CheapLabel& l, juce::Button& b1, juce::Button& b2, juce::Rectangle<int> r);
    void mouseDrag(const juce::MouseEvent& event) override;
    void resetTopLeftProps();
    void setupCreditsWindow(CustomLookAndFeelE& lnfe);
    void setupHelpWindow(CustomLookAndFeelE& lnfe);

    SemiProQAudioProcessor& audioProcessor;
    SemiProQAudioProcessorEditor& editor;

    //need slope switch button for analyser
    juce::ToggleButton analyserOnButton, peakOnButton;
    juce::TextButton analyserModeButton, peakModeButton, helpButton, creditsButton;
    CheapLabel analyserSettingsLabel, peakSettingsLabel, blankLabel, componentLabel;
};
