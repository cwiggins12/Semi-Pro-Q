
#pragma once

#include <JuceHeader.h>
#include "Utils/Constants.h"
#include "CustomLookAndFeel.h"

//==============================================================================
/** Credits window built when the credits button is pressed
*/
struct CreditsWindow : juce::Component {
    CreditsWindow(CustomLookAndFeelF& lookAndFeel) : lnf(lookAndFeel) {
        setSize(CREDIT_WINDOW_WIDTH, CREDIT_WINDOW_HEIGHT);

        addAndMakeVisible(titleLabel);
        titleLabel.setText("Credits and Contact", juce::dontSendNotification);
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
        textEditor.setFont(juce::Font(24.0f));
        textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        textEditor.setText(
            "Created by: Cody Wiggins - codywigginsdev@gmail.com\n\nSource Code Repository: https://github.com/cwiggins12/TwelveBandEq\n\nWeekly updates about development here: https://codywigginsdev.neocities.org/\n\nThanks for using my plugin! Happy mixing :)"
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

    ~CreditsWindow() {
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
