#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "Components/Controls/DraggableButton.h"
#include "Components/Controls/GainComponent.h"
#include "Components/Controls/SelectedFilterComponent.h"
#include "Components/Controls/SettingsComponent.h"
#include "Components/Visualization/PeakMeterComponent.h"
#include "Components/Visualization/ResponseCurveComponent.h"
#include "Components/Visualization/SpectrumAnalyserComponent.h"
#include "Components/Dialogs/CreditsWindow.h"
#include "Components/Dialogs/HelpWindow.h"
#include "Utils/Constants.h"

//==============================================================================
/**
*/
class SemiProQAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Value::Listener, juce::Timer {
public:
    SemiProQAudioProcessorEditor(SemiProQAudioProcessor&);
    ~SemiProQAudioProcessorEditor() override;

    //resets button parameters after right clicking
    void buttonReset(int id);
    //bounds of where the buttons can be dragged or double clicked
    juce::Rectangle<int> buttonBounds;
    //public values that refer to properties in value tree
    juce::Value selectedFilterValue, minGain, minSettings, minSelected, 
                analyserOnValue, analyserModeValue, analyserSlopeValue, peakOnValue, peakModeValue;

    //called on selectedFilter change to get associated colour
    juce::Colour getColour(int i) {
        return colours[i];
    }
    //on settings button press, open these dialog boxes
    void openCreditsDialog();
    void openHelpDialog();

private:
    //construction helpers
    void setupDraggableButtons();
    void setupMinimizableComp(MinimizableComponent& c, juce::Value& v);
    void resizeVisualizers();
    void referAndAddListener(juce::Value& val, int idx);
    //component update callbacks
    void paint(juce::Graphics&) override;
    void resized() override;
    //double click to initialize a new draggable button
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    //get area without border size
    juce::Rectangle<int> getRenderArea();
    //draw and cache background image
    void backgroundImage();
    //after deleted button, check which should be visible in selected filter component, if any
    void sfcVisiblityCheck();
    //property change callback from value tree
    void valueChanged(juce::Value& value) override;
    //helpers to get changes from Settings to pass to respective component
    void selectedFilterChanged();
    void analyserOnChanged();
    void analyserModeChanged();
    void peakOnChanged();
    void peakModeChanged();
    //timer to trigger children's repaints
    void timerCallback() override;

    CustomLookAndFeelA lnfa;
    CustomLookAndFeelB lnfb;
    CustomLookAndFeelC lnfc;
    CustomLookAndFeelD lnfd;
    CustomLookAndFeelE lnfe;
    CustomLookAndFeelF lnff;

    SemiProQAudioProcessor& audioProcessor;
    PeakMeterComponent peakComponent;
    SpectrumAnalyserComponent analyserComponent;
    ResponseCurveComponent responseCurveComponent;
    juce::OwnedArray<DraggableButton> buttonArr;
    SelectedFilterComponent selectedFilterComponent;
    GainComponent gainComponent;
    SettingsComponent settingsComponent;

    std::unique_ptr<HelpWindow> helpWindow;
    std::unique_ptr<CreditsWindow> creditsWindow;

    juce::Image background;
    juce::TooltipWindow tooltipWindow{ this, TOOLTIP_DELAY_MS };

    //Draggable button & selected eq color array
    const juce::Array<juce::Colour> colours{ juce::Colours::red, juce::Colours::darkorange, juce::Colours::yellow, juce::Colours::green,
                                             juce::Colours::blue, juce::Colours::indigo, juce::Colours::violet, juce::Colours::darkgoldenrod,
                                             juce::Colours::pink, juce::Colours::olive, juce::Colours::beige, juce::Colours::crimson };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SemiProQAudioProcessorEditor)
};
