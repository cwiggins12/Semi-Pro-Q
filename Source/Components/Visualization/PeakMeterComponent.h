
#pragma once

#include <JuceHeader.h>
#include "Utils/Constants.h"

class SemiProQAudioProcessor;

//==============================================================================
/** Peak metering logic and painting
*/
//stereo version
struct PeakMeterComponent : juce::Component {
    PeakMeterComponent(SemiProQAudioProcessor&);
    ~PeakMeterComponent();

    void timerCallback();
    void resetValues();
    void resized() override;
    void setYs(juce::Array<float>& newYs);

    juce::Colour fillColor;
    juce::SmoothedValue<float> currLeftPeak, currRightPeak;
    juce::Array<float> ys;

private:
    void paint(juce::Graphics& g) override;
    void drawFilledMeter(juce::Graphics& g, float peak, juce::Rectangle<int>& r);
    void drawOutlineRect(juce::Graphics& g, juce::Rectangle<int> r);
    void drawLabelText(juce::Graphics& g);
    void drawHoldLine(juce::Graphics& g, float hold, juce::Rectangle<int> line, juce::Rectangle<int>& r);
    void drawClipNotifier(juce::Graphics& g, juce::Rectangle<int>& r);
    void drawHoldText(juce::Graphics& g, float hold, juce::Rectangle<int>& r);
    void mouseDown(const juce::MouseEvent& event) override;
    float getSmoothedValue(juce::SmoothedValue<float>& value);
    void resizeMeter(juce::Rectangle<int>& m, juce::Rectangle<int>& h, juce::Rectangle<int>& c, juce::Rectangle<int> bounds);
    SemiProQAudioProcessor& audioProcessor;

    int channels = 1;
    juce::Rectangle<int> leftRect, rightRect;
    float leftPeakHold = 0.0f, rightPeakHold = 0.0f;
    int leftPeakHoldCounter = 0, rightPeakHoldCounter = 0;
    juce::Rectangle<int> leftHoldRect, rightHoldRect, leftClipRect, rightClipRect;
    bool leftClipNotifier = false, rightClipNotifier = false;
    const juce::StringArray dBs{ "-96", "-84", "-72", "-60", "-48", "-36", "-24", "-12", "0" };
};

////mono version
//struct PeakMeterComponent : juce::Component {
//    PeakMeterComponent(SemiProQAudioProcessor&);
//    ~PeakMeterComponent();
//
//    void timerCallback();
//    void resetValues();
//    void resized() override;
//    void setYs(juce::Array<float>& newYs);
//
//    juce::Colour fillColor;
//    juce::SmoothedValue<float> currLeftPeak;
//    juce::Array<float> ys;
//
//private:
//    void paint(juce::Graphics& g) override;
//    void drawFilledMeter(juce::Graphics& g, float peak, juce::Rectangle<int>& r);
//    void drawOutlineRect(juce::Graphics& g, juce::Rectangle<int> r);
//    void drawLabelText(juce::Graphics& g);
//    void drawHoldLine(juce::Graphics& g, float hold, juce::Rectangle<int> line, juce::Rectangle<int>& r);
//    void drawClipNotifier(juce::Graphics& g, juce::Rectangle<int>& r);
//    void drawHoldText(juce::Graphics& g, float hold, juce::Rectangle<int>& r);
//    void mouseDown(const juce::MouseEvent& event) override;
//    float getSmoothedValue(juce::SmoothedValue<float>& value);
//    void resizeMeter(juce::Rectangle<int>& m, juce::Rectangle<int>& h, juce::Rectangle<int>& c, juce::Rectangle<int> bounds);
//    SemiProQAudioProcessor& audioProcessor;
//
//    int channels = 1;
//    juce::Rectangle<int> leftRect, leftHoldRect, leftClipRect;
//    float leftPeakHold = 0.0f;
//    int leftPeakHoldCounter = 0;
//    bool leftClipNotifier = false;
//    const juce::StringArray dBs{ "-96", "-84", "-72", "-60", "-48", "-36", "-24", "-12", "0" };
//};
