
#include "SettingsComponent.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomLookAndFeel.h"

//==============================================================================
SettingsComponent::SettingsComponent(SemiProQAudioProcessor& p, SemiProQAudioProcessorEditor& e, CustomLookAndFeelC& lnfc, CustomLookAndFeelD& lnfd, CustomLookAndFeelE& lnfe)
                                   : MinimizableComponent(lnfd, SETTINGS_SIZE_X, SETTINGS_SIZE_Y, SETTINGS_TOPLEFT_X, SETTINGS_TOPLEFT_Y),audioProcessor(p), editor(e) {
    analyserSettingsLabel.setText("ANALYSER");
    peakSettingsLabel.setText("PEAK");
    componentLabel.setText("SETTINGS");
    blankLabel.setText("");

    analyserOnButton.setComponentID("power");
    addAndMakeVisible(analyserOnButton);

    analyserModeButton.setClickingTogglesState(true);
    analyserModeButton.setLookAndFeel(&lnfc);
    addAndMakeVisible(analyserModeButton);

    peakOnButton.setComponentID("power");
    addAndMakeVisible(peakOnButton);

    peakModeButton.setClickingTogglesState(true);
    peakModeButton.setLookAndFeel(&lnfc);
    addAndMakeVisible(peakModeButton);

    resetTopLeftProps();
    applyMinimized(*this, editor.minSettings.getValue());
    addAndMakeVisible(minButton);

    setupCreditsWindow(lnfe);
    setupHelpWindow(lnfe);
}

SettingsComponent::~SettingsComponent() {
    analyserModeButton.setLookAndFeel(nullptr);
    peakModeButton.setLookAndFeel(nullptr);
    helpButton.setLookAndFeel(nullptr);
    creditsButton.setLookAndFeel(nullptr);
}

void SettingsComponent::paint(juce::Graphics& g) {
    if (!isMin) {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::grey);
        g.fillRoundedRectangle(bounds, CORNER_SIZE);
        auto midX = bounds.getX() + (bounds.getWidth() / 3);
        g.setColour(juce::Colours::black);
        g.fillRect(midX, bounds.getY(), DIV_LINE_WIDTH, bounds.getHeight());
        g.fillRect(midX + bounds.getWidth() / 3 - 1, bounds.getY(), DIV_LINE_WIDTH, bounds.getHeight());

        analyserSettingsLabel.paintAll(g);
        peakSettingsLabel.paintAll(g);
        componentLabel.paintAll(g);
        blankLabel.paintAll(g);

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(bounds, CORNER_SIZE, 2.0f);
    }
}

void SettingsComponent::resized() {
    auto bounds = getLocalBounds();
    if (isMin) {
        analyserSettingsLabel.setBounds(0, 0, 0, 0);
        peakSettingsLabel.setBounds(0, 0, 0, 0);
        componentLabel.setBounds(0, 0, 0, 0);
        blankLabel.setBounds(0, 0, 0, 0);
        analyserOnButton.setBounds(0, 0, 0, 0);
        analyserModeButton.setBounds(0, 0, 0, 0);
        peakOnButton.setBounds(0, 0, 0, 0);
        peakModeButton.setBounds(0, 0, 0, 0);
        creditsButton.setBounds(0, 0, 0, 0);
        helpButton.setBounds(0, 0, 0, 0);
        minButton.setTopLeftPosition(bounds.getX(), bounds.getY());
    }
    else {
        bounds.reduce(1, 1);
        auto minBounds = bounds.removeFromTop(MINIMIZE_BUTTON_DIM);
        componentLabel.setBounds(minBounds);
        minButton.setBounds(minBounds.removeFromRight(MINIMIZE_BUTTON_DIM));

        makeResizedSection(analyserSettingsLabel, analyserOnButton, analyserModeButton, bounds.removeFromLeft(getWidth() / 3));
        makeResizedSection(blankLabel, helpButton, creditsButton, bounds.removeFromRight(getWidth() / 3));
        makeResizedSection(peakSettingsLabel, peakOnButton, peakModeButton, bounds);
    }
}

void SettingsComponent::referValuesToButtons(juce::Value& v) {
    MinimizableComponent::referValuesToButtons(v);
    analyserOnButton.getToggleStateValue().referTo(editor.analyserOnValue);
    analyserModeButton.getToggleStateValue().referTo(editor.analyserModeValue);
    peakOnButton.getToggleStateValue().referTo(editor.peakOnValue);
    peakModeButton.getToggleStateValue().referTo(editor.peakModeValue);
}

void SettingsComponent::makeResizedSection(CheapLabel& l, juce::Button& b1, juce::Button& b2, juce::Rectangle<int> r) {
    l.setBounds(r.removeFromTop(LABEL_HEIGHT));
    auto r1 = r.removeFromTop(r.getHeight() / 2);
    r1.reduce(BUTTON_SPACING, BUTTON_SPACING);
    r.reduce(BUTTON_SPACING, BUTTON_SPACING);
    b1.setBounds(r1);
    b2.setBounds(r);
}

void SettingsComponent::mouseDrag(const juce::MouseEvent& event) {
    MinimizableComponent::mouseDrag(event);
    audioProcessor.tree.state.setProperty(props[SETTINGS_X], topLeft.x, nullptr);
    audioProcessor.tree.state.setProperty(props[SETTINGS_Y], topLeft.y, nullptr);
}

void SettingsComponent::resetTopLeftProps() {
    if (audioProcessor.tree.state.hasProperty(props[SETTINGS_X])) {
        topLeft.x = audioProcessor.tree.state[props[SETTINGS_X]];
    }
    else {
        audioProcessor.tree.state.setProperty(props[SETTINGS_X], SETTINGS_TOPLEFT_X, nullptr);
    }
    if (audioProcessor.tree.state.hasProperty(props[SETTINGS_Y])) {
        topLeft.y = audioProcessor.tree.state[props[SETTINGS_Y]];
    }
    else {
        audioProcessor.tree.state.setProperty(props[SETTINGS_Y], SETTINGS_TOPLEFT_Y, nullptr);
    }
}

void SettingsComponent::setupCreditsWindow(CustomLookAndFeelE& lnfe) {
    creditsButton.setButtonText("CREDITS");
    creditsButton.setLookAndFeel(&lnfe);
    addAndMakeVisible(creditsButton);
    creditsButton.onClick = [this]() {
        editor.openCreditsDialog();
    };
}

void SettingsComponent::setupHelpWindow(CustomLookAndFeelE& lnfe) {
    helpButton.setButtonText("HELP");
    helpButton.setLookAndFeel(&lnfe);
    addAndMakeVisible(helpButton);
    helpButton.onClick = [this]() {
        editor.openHelpDialog();
    };
}
