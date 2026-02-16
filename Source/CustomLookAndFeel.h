#pragma once

#include <JuceHeader.h>
#include "Utils/Constants.h"

using namespace juce;

//Draws all sliders, combo boxes, labels, and power buttons (bypasses) in Selected Eq component and RTA power in editor
struct CustomLookAndFeelA : LookAndFeel_V4 {
    CustomLookAndFeelA();

	void drawComboBox(Graphics&, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox&) override;
    void positionComboBoxText(ComboBox& b, Label& l) override;
    PopupMenu::Options getOptionsForComboBoxPopupMenu(ComboBox&, Label&);
    void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, 
        bool hasSubMenu, const String& text, const String& shortcutKeyText, const Drawable* icon, const Colour* textColourToUse);
    void drawToggleButton(Graphics&, ToggleButton&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//Draws delete button in Selected Eq component
struct CustomLookAndFeelB : LookAndFeel_V4 {
    CustomLookAndFeelB() = default;
    ~CustomLookAndFeelB() override = default;

    void drawButtonBackground(Graphics&, Button&, const Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//Draws Pre/Post eq RTA button in editor
struct CustomLookAndFeelC : LookAndFeel_V4 {
    CustomLookAndFeelC() = default;
    ~CustomLookAndFeelC() override = default;

    void drawButtonBackground(Graphics&, Button&, const Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText(Graphics& g, TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//Draws Minimize button for Gain and SelectedEq components
struct CustomLookAndFeelD : LookAndFeel_V4 {
    CustomLookAndFeelD() = default;
    ~CustomLookAndFeelD() override = default;

    void drawButtonBackground(Graphics&, Button&, const Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText(Graphics& g, TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//Draws Help/Credits button in editor
struct CustomLookAndFeelE : LookAndFeel_V4 {
    CustomLookAndFeelE() = default;
    ~CustomLookAndFeelE() override = default;

    void drawButtonBackground(Graphics&, Button&, const Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText(Graphics& g, juce::TextButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//Draws Help and Credits window buttons
struct CustomLookAndFeelF : LookAndFeel_V4 {
    CustomLookAndFeelF();
    ~CustomLookAndFeelF() override = default;

    void drawButtonBackground(Graphics&, Button&, const Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonText(Graphics& g, TextButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void fillTextEditorBackground(Graphics&, int width, int height, TextEditor&) override;
    void drawTextEditorOutline(Graphics&, int width, int height, TextEditor&) override;
};
