
#include "GainComponent.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomLookAndFeel.h"

//==============================================================================
/** component that houses pre and post gains.
*/
GainComponent::GainComponent(SemiProQAudioProcessor& p, SemiProQAudioProcessorEditor& e, CustomLookAndFeelD& lnfd) : 
                             MinimizableComponent(lnfd, GAIN_SIZE_X, GAIN_SIZE_Y, GAIN_TOPLEFT_X, GAIN_TOPLEFT_Y), audioProcessor(p), editor(e) {
    componentLabel.setText("GAIN");
    preGainLabel.setText("PRE");
    postGainLabel.setText("POST");

    makeSlider(preGainSlider);
    preGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, params[PREGAIN], preGainSlider);
    makeSlider(postGainSlider);
    postGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.tree, params[POSTGAIN], postGainSlider);
    
    resetTopLeftProps();
    applyMinimized(*this, editor.minGain.getValue());
    addAndMakeVisible(minButton);
}

GainComponent::~GainComponent() {}

void GainComponent::paint(juce::Graphics& g) {
    if (!isMin) {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::grey);
        g.fillRoundedRectangle(bounds, CORNER_SIZE);

        auto midX = bounds.getX() + (bounds.getWidth() / 2);
        g.setColour(juce::Colours::black);
        g.fillRect(midX, bounds.getY(), DIV_LINE_WIDTH, bounds.getHeight());

        componentLabel.paintAll(g);
        preGainLabel.paintAll(g);
        postGainLabel.paintAll(g);

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(bounds, CORNER_SIZE, 2.0f);
    }
}

void GainComponent::resized() {
    auto bounds = getLocalBounds();
    if (isMin) {
        preGainLabel.setBounds(0, 0, 0, 0);
        preGainSlider.setBounds(0, 0, 0, 0);
        postGainLabel.setBounds(0, 0, 0, 0);
        postGainSlider.setBounds(0, 0, 0, 0);
        componentLabel.setBounds(0, 0, 0, 0);
        minButton.setTopLeftPosition(bounds.getX(), bounds.getY());
    }
    else {
        bounds.reduce(1, 1);
        auto minBounds = bounds.removeFromTop(MINIMIZE_BUTTON_DIM);
        componentLabel.setBounds(minBounds);
        minButton.setBounds(minBounds.removeFromRight(MINIMIZE_BUTTON_DIM));

        makeResizedSection(preGainLabel, preGainSlider, bounds.removeFromLeft(getWidth() / 2));
        makeResizedSection(postGainLabel, postGainSlider, bounds);
    }
}

void GainComponent::makeSlider(juce::Slider& slider) {
    addAndMakeVisible(slider);
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, TEXTBOX_WIDTH, TEXTBOX_HEIGHT);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void GainComponent::makeResizedSection(CheapLabel& l, juce::Component& c, juce::Rectangle<int> r) {
    l.setBounds(r.removeFromTop(LABEL_HEIGHT));
    c.setBounds(r);
}

void GainComponent::mouseDrag(const juce::MouseEvent& event) {
    MinimizableComponent::mouseDrag(event);
    audioProcessor.tree.state.setProperty(props[GAIN_X], topLeft.x, nullptr);
    audioProcessor.tree.state.setProperty(props[GAIN_Y], topLeft.y, nullptr);
}

void GainComponent::resetTopLeftProps() {
    if (audioProcessor.tree.state.hasProperty(props[GAIN_X])) {
        topLeft.x = audioProcessor.tree.state[props[GAIN_X]];
    }
    else {
        audioProcessor.tree.state.setProperty(props[GAIN_X], GAIN_TOPLEFT_X, nullptr);
    }
    if (audioProcessor.tree.state.hasProperty(props[GAIN_Y])) {
        topLeft.y = audioProcessor.tree.state[props[GAIN_Y]];
    }
    else {
        audioProcessor.tree.state.setProperty(props[GAIN_Y], GAIN_TOPLEFT_Y, nullptr);
    }
}
