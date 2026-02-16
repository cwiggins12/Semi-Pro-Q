
#include "CustomLookAndFeel.h"
using namespace juce;

CustomLookAndFeelA::CustomLookAndFeelA() {
    setColour(Slider::rotarySliderFillColourId, Colours::limegreen);
    setColour(Slider::thumbColourId, Colours::black);

    setColour(Label::backgroundColourId, Colours::black);
    setColour(Label::textColourId, Colours::white);
    setColour(Label::outlineColourId, Colours::white);

    setColour(TextEditor::backgroundColourId, Colours::black);
    setColour(TextEditor::textColourId, Colours::white);

    setColour(ComboBox::backgroundColourId, Colours::black);
}

void CustomLookAndFeelA::drawComboBox(Graphics& g, int width, int height, bool, int, int, int, int, ComboBox& box) {
    auto area = Rectangle<float>(0, 0, (float)width, (float)height);

    g.setColour(findColour(ComboBox::backgroundColourId));
    g.fillRoundedRectangle(area, BG_CORNER_SIZE);

    g.setColour(Colours::limegreen);
    g.drawRoundedRectangle(area.reduced(1), CORNER_SIZE, OUTLINE_WIDTH);
}

void CustomLookAndFeelA::positionComboBoxText(ComboBox& box, Label& label) {
    label.setBounds(box.getLocalBounds());
    label.setJustificationType(Justification::centred);   
    label.setFont(14.0f);
    label.setColour(Label::backgroundColourId, Colours::transparentBlack);
    label.setColour(Label::outlineColourId, Colours::transparentBlack);
    label.setColour(Label::textColourId, Colours::limegreen);
}

PopupMenu::Options CustomLookAndFeelA::getOptionsForComboBoxPopupMenu(ComboBox& box, Label& label) {
    auto opts = LookAndFeel_V4::getOptionsForComboBoxPopupMenu(box, label);
    return opts.withMinimumWidth(int(box.getWidth() * 0.7f))
        .withMaximumNumColumns(1)
        .withStandardItemHeight(18);
}

void CustomLookAndFeelA::drawPopupMenuItem(Graphics& g, const Rectangle<int>& area,
    bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
    const String& text, const String& shortcutKeyText,
    const Drawable* icon, const Colour* textColourToUse) {
    String singleLineText = text.replace("\n", " ");

    LookAndFeel_V4::drawPopupMenuItem(g, area, isSeparator, isActive, isHighlighted,
        isTicked, hasSubMenu, singleLineText, shortcutKeyText, icon, textColourToUse);
}

void CustomLookAndFeelA::drawToggleButton(Graphics& g, ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    const auto bounds = button.getLocalBounds().toFloat();

    bool isBypassed = button.getToggleState();
    if (button.getComponentID() == "power") {
        isBypassed = !isBypassed;
    }
    Colour bg = isBypassed ? Colours::grey : Colours::green;
    if (shouldDrawButtonAsHighlighted) {
        bg = bg.brighter(COLOR_CHANGE);
    }
    if (shouldDrawButtonAsDown) {
        bg = bg.darker(COLOR_CHANGE);
    }

    g.setColour(bg);
    g.fillRoundedRectangle(bounds, BG_CORNER_SIZE);

    g.setColour(Colours::black);
    g.drawRoundedRectangle(bounds.reduced(1), CORNER_SIZE, OUTLINE_WIDTH);

    auto c = bounds.getCentre();
    float r = bounds.getHeight() * 0.3f;

    Path power;
    power.addCentredArc(c.x, c.y, r, r, 0.0f, MathConstants<float>::pi * 0.25f, MathConstants<float>::pi * 1.75f, true);
    power.startNewSubPath(c.x, c.y - r);
    power.lineTo(c.x, c.y);

    g.setColour(Colours::black);
    g.strokePath(power, PathStrokeType(5.0f));
}

void CustomLookAndFeelB::drawButtonBackground(Graphics& g, Button& b, const Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = b.getLocalBounds().toFloat();

    Colour base = Colours::red;
    if (shouldDrawButtonAsHighlighted) {
        base = base.brighter(COLOR_CHANGE);
    }
    if (shouldDrawButtonAsDown) {
        base = base.darker(COLOR_CHANGE);
    }

    g.setColour(base);
    g.fillRoundedRectangle(bounds, BG_CORNER_SIZE);

    g.setColour(Colours::black);
    g.drawRoundedRectangle(bounds.reduced(1), CORNER_SIZE, OUTLINE_WIDTH);

    Path cross;
    float pad = bounds.getWidth() * 0.25f;
    Point<float> p1(bounds.getX() + pad, bounds.getY() + pad);
    Point<float> p2(bounds.getRight() - pad, bounds.getBottom() - pad);
    Point<float> p3(bounds.getRight() - pad, bounds.getY() + pad);
    Point<float> p4(bounds.getX() + pad, bounds.getBottom() - pad);

    cross.startNewSubPath(p1);
    cross.lineTo(p2);
    cross.startNewSubPath(p3);
    cross.lineTo(p4);

    g.strokePath(cross, PathStrokeType(5.0f));
}

void CustomLookAndFeelC::drawButtonBackground(Graphics& g, Button& b, const Colour&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = b.getLocalBounds().toFloat();

    bool isPost = b.getToggleState();

    Colour base = isPost ? Colours::limegreen : Colours::yellow;

    if (shouldDrawButtonAsHighlighted) {
        base = base.brighter(COLOR_CHANGE);
    }
    if (shouldDrawButtonAsDown) {
        base = base.darker(COLOR_CHANGE);
    }

    g.setColour(base);
    g.fillRoundedRectangle(bounds, BG_CORNER_SIZE);

    g.setColour(Colours::black);
    g.drawRoundedRectangle(bounds.reduced(1), CORNER_SIZE, OUTLINE_WIDTH);
}

void CustomLookAndFeelC::drawButtonText(Graphics& g, TextButton& button, bool, bool) {
    auto bounds = button.getLocalBounds();
    bool isPost = button.getToggleState();

    String text = isPost ? "POST" : "PRE";

    g.setColour(Colours::black);
    g.setFont(Font(16.0f, Font::bold));
    g.drawFittedText(text, bounds, Justification::centred, 1);
}

void CustomLookAndFeelD::drawButtonBackground(Graphics& g, Button& b, const Colour&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = b.getLocalBounds().toFloat();

    Colour base = Colours::grey;

    if (shouldDrawButtonAsHighlighted) {
        base = base.brighter(COLOR_CHANGE);
    }
    if (shouldDrawButtonAsDown) {
        base = base.darker(COLOR_CHANGE);
    }

    g.setColour(base);
    g.fillRoundedRectangle(bounds, BG_CORNER_SIZE);

    g.setColour(Colours::white);
    g.drawRoundedRectangle(bounds, CORNER_SIZE, OUTLINE_WIDTH);
}

void CustomLookAndFeelD::drawButtonText(Graphics& g, TextButton& button, bool, bool) {
    auto bounds = button.getLocalBounds();
    bool isMinimized = button.getToggleState();

    String text = isMinimized ? "+" : "-";

    g.setColour(Colours::black);
    g.setFont(Font(16.0f, Font::bold));
    g.drawFittedText(text, bounds, Justification::centred, 1);
}

void CustomLookAndFeelE::drawButtonBackground(Graphics& g, Button& b, const Colour&,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = b.getLocalBounds().toFloat();

    Colour base = Colours::green;

    if (shouldDrawButtonAsHighlighted) {
        base = base.brighter(COLOR_CHANGE);
    }
    if (shouldDrawButtonAsDown) {
        base = base.darker(COLOR_CHANGE);
    }

    g.setColour(base);
    g.fillRoundedRectangle(bounds, BG_CORNER_SIZE);

    g.setColour(Colours::black);
    g.drawRoundedRectangle(bounds.reduced(1), CORNER_SIZE, OUTLINE_WIDTH);
}

void CustomLookAndFeelE::drawButtonText(Graphics& g, TextButton& button, bool, bool) {
    auto bounds = button.getLocalBounds();
    g.setColour(Colours::black);
    g.setFont(Font(14.0f, Font::bold));
    g.drawFittedText(button.getButtonText(), bounds, Justification::centred, 1);
}

CustomLookAndFeelF::CustomLookAndFeelF() {
    setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    setColour(juce::TextEditor::textColourId, juce::Colours::white);
    setColour(juce::TextEditor::outlineColourId, juce::Colours::grey);
    setColour(juce::Label::textColourId, juce::Colours::white);
}

void CustomLookAndFeelF::drawButtonBackground(Graphics& g, Button& b, const Colour&,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = b.getLocalBounds().toFloat();

    Colour base = juce::Colours::black;

    if (shouldDrawButtonAsHighlighted) {
        base = base.brighter(COLOR_CHANGE);
    }
    if (shouldDrawButtonAsDown) {
        base = base.darker(COLOR_CHANGE);
    }

    g.setColour(base);
    g.fillRoundedRectangle(bounds, BG_CORNER_SIZE);

    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(bounds.reduced(1), CORNER_SIZE, OUTLINE_WIDTH);
}

void CustomLookAndFeelF::drawButtonText(Graphics& g, TextButton& button, bool, bool) {
    auto bounds = button.getLocalBounds();
    String text = button.getButtonText();
    Font font(22.0f, Font::bold);
    g.setFont(font);

    g.setColour(juce::Colours::limegreen);
    g.drawFittedText(text, bounds, juce::Justification::centred, 1);
}

void CustomLookAndFeelF::fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& t) {
    juce::Rectangle<float> bounds(0, 0, width, height);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds, CORNER_SIZE);
}
void CustomLookAndFeelF::drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& t) {
    juce::Rectangle<float> bounds(0, 0, width, height);
    g.setColour(Colours::darkgreen);
    g.drawRoundedRectangle(bounds.reduced(1), CORNER_SIZE, OUTLINE_WIDTH + 1.0f);
}
