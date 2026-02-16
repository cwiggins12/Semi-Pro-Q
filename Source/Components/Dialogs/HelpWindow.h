
#pragma once

#include <JuceHeader.h>
#include "Utils/Constants.h"
#include "CustomLookAndFeel.h"

//==============================================================================
/** Help window built when the help button is pressed
*/
struct HelpWindow : juce::Component {
    HelpWindow(CustomLookAndFeelF& lookAndFeel) : lnf(lookAndFeel) {
        setSize(HELP_WINDOW_WIDTH, HELP_WINDOW_HEIGHT);

        addAndMakeVisible(titleLabel);
        titleLabel.setText("How to Use", juce::dontSendNotification);
        titleLabel.setJustificationType(juce::Justification::centredTop);
        titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
        titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

        addAndMakeVisible(textEditor);
        textEditor.setLookAndFeel(&lnf);
        textEditor.setMultiLine(true);
        textEditor.setReadOnly(true);
        textEditor.setScrollbarsShown(false);
        textEditor.setCaretVisible(false);
        textEditor.setPopupMenuEnabled(false);
        textEditor.setFont(juce::Font(17.0f));
        textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        textEditor.setText(
            "- Double left click within grid in the majority of the window to create a peak filter at the point's frequency and gain\n"
            "\n"
            "- These filters can be clicked and dragged to any frequency or gain on the grid to update the filters parameters\n"
            "\n"
            "- Filters will be color coded circles at the point double clicked and will show up as the selected filter in the Selected Filter component\n"
            "\n"
            "- Right click on any created filter to delete the filter or use the delete button in the Selected Filter component when the filter is selected\n"
            "\n"
            "- Left click on any visible filter to select it and show all of its parameters in the Selected Filter component\n"
            "\n"
            "- The Selected Filter component allows for 8 filter types, q or dB/oct adjustment, bypassing and deleting when a filter is selected\n"
            "\n"
            "- The selected filter's color will be shown in the top left of the Selected Filter component and the filter's circle will have a white outline\n"
            "\n"
            "- Spectrum analyser and peak meter(s) initialize as on and post-eq. The buttons to change these Settings are in the Settings component\n"
            "\n"
            "- All of these settings, component positions, minimized states, and filter parameters are saved and loaded on window open per instance\n"
            "\n"
            "- If any channel clips, the meter will show a notification as a red rectangle. Clicking the meter will remove the notification\n"
            "\n"
            "- The spectrum analyser has a 4.5 dB/oct tilt applied to it to allow for better visualization of the high end\n"
            "\n"
            "- The spectrum analyser is meant to line up with the dB of the peak filter + 12dB roughly. It does not correlate to the filter decibels\n"
            "\n"
            "- The PRE and POST knobs in the bottom right are gain knobs pre and post the filters you are applying\n"
            "\n"
            "- The Gain, Settings, and Selected Filter components are all minimizable by using the button in the top right of the component\n"
            "\n"
            "- The same components are also all movable by clicking and dragging anywhere on the component that isn't housing a slider\n"
        );

        closeButton.setLookAndFeel(&lnf);
        addAndMakeVisible(closeButton);
        closeButton.setButtonText("OK");
        closeButton.onClick = [this]() {
            setVisible(false);
            if (auto* parent = getParentComponent()) {
                parent->removeChildComponent(this);
            }
            };
    }

    ~HelpWindow() {
        closeButton.setLookAndFeel(nullptr);
        textEditor.setLookAndFeel(nullptr);
    }

private:
    void paint(juce::Graphics& g) {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(bounds, CORNER_SIZE);

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(bounds.reduced(1.0f), CORNER_SIZE, 2.0f);
    }

    void resized() {
        auto bounds = getLocalBounds().reduced(BORDER_SPACING);
        titleLabel.setBounds(bounds.removeFromTop(LABEL_HEIGHT * 2));
        closeButton.setBounds(bounds.removeFromBottom(LABEL_HEIGHT * 2).reduced(getWidth() / 3, 0));
        bounds.removeFromBottom(BORDER_SPACING);
        textEditor.setBounds(bounds);
    }

    CustomLookAndFeelF& lnf;
    juce::Label titleLabel;
    juce::TextEditor textEditor;
    juce::TextButton closeButton;
};
