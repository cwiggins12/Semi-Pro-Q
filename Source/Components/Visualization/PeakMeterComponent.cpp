
#include "PeakMeterComponent.h"
#include "PluginProcessor.h"

//==============================================================================
PeakMeterComponent::PeakMeterComponent(SemiProQAudioProcessor& p) : audioProcessor(p) {
    channels = 2;
    fillColor = juce::Colours::lime;

    //setup smooth values
    currLeftPeak.reset(TIMER_FPS, PEAK_DECAY_TIME);
    currLeftPeak.setCurrentAndTargetValue(0.0f);
    if (channels > 1) {
        currRightPeak.reset(TIMER_FPS, PEAK_DECAY_TIME);
        currRightPeak.setCurrentAndTargetValue(0.0f);
    }
}

PeakMeterComponent::~PeakMeterComponent() {}

void PeakMeterComponent::timerCallback() {
    //destructive read
    float leftPeakValue = audioProcessor.leftPeak.read();
    //Replace peak
    currLeftPeak.setTargetValue(leftPeakValue);
    //repalce hold if larger
    if (leftPeakValue > leftPeakHold) {
        leftPeakHold = leftPeakValue;
        leftPeakHoldCounter = PEAK_HOLD_FRAMES;
    }
    //estimated check for clipping
    if (leftPeakValue >= EST_CLIP_BARRIER) {
        leftClipNotifier = true;
    }
    //count down hold or linear drop after hold frames run out
    (leftPeakHoldCounter > 0.0f) ? leftPeakHoldCounter-- : leftPeakHold = leftPeakHold * 0.975f;

    //do the same for right
    if (channels > 1) {
        float rightPeakValue = audioProcessor.rightPeak.read();
        currRightPeak.setTargetValue(rightPeakValue);
        if (rightPeakValue > rightPeakHold) {
            rightPeakHold = rightPeakValue;
            rightPeakHoldCounter = PEAK_HOLD_FRAMES;
        }
        if (rightPeakValue >= EST_CLIP_BARRIER) {
            rightClipNotifier = true;
        }
        (rightPeakHoldCounter > 0) ? rightPeakHoldCounter-- : rightPeakHold = rightPeakHold * 0.975f;
    }
    repaint();
}

void PeakMeterComponent::paint(juce::Graphics& g) {
    //left meter draw functions
    drawFilledMeter(g, getSmoothedValue(currLeftPeak), leftRect);
    drawOutlineRect(g, leftRect);
    drawHoldLine(g, leftPeakHold, leftHoldRect, leftRect);
    if (leftClipNotifier) {
        drawClipNotifier(g, leftClipRect);
    }
    drawHoldText(g, leftPeakHold, leftRect);
    //right meter draw functions
    if (channels > 1) {
        drawFilledMeter(g, getSmoothedValue(currRightPeak), rightRect);
        drawOutlineRect(g, rightRect);
        drawHoldLine(g, rightPeakHold, rightHoldRect, rightRect);
        if (rightClipNotifier) {
            drawClipNotifier(g, rightClipRect);
        }
        drawHoldText(g, rightPeakHold, rightRect);
    }
    drawLabelText(g);
}

void PeakMeterComponent::drawFilledMeter(juce::Graphics& g, float peak, juce::Rectangle<int>& r) {
    //map based on dB, clamp, set top to percent of the meter from map, fill with fillColor
    float peak_dB = juce::Decibels::gainToDecibels(peak, MIN_ANALYSIS_DB);
    float fill = juce::jmap(peak_dB, MIN_ANALYSIS_DB, 0.0f, 0.0f, 1.0f);
    fill = juce::jlimit(0.0f, 1.0f, fill);
    int h = r.getHeight();
    int newH = static_cast<int>(h * fill);
    g.setColour(fillColor);
    g.fillRect(r.getX(), r.getY() + h - newH, r.getWidth(), newH);
}

void PeakMeterComponent::drawOutlineRect(juce::Graphics& g, juce::Rectangle<int> r) {
    //draws outline rect of the meter, expands by 1, then draws the lines of each 12 dB
    int x1 = r.getX();
    int x2 = x1 + r.getWidth() - 5;
    r.expand(1, 1);
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(r);
    for (int i = 1; i < 8; ++i) {
        int y = ys[i];
        g.fillRect(x1, y, 5, 1);
        g.fillRect(x2, y, 5, 1);
    }
}

void PeakMeterComponent::drawLabelText(juce::Graphics& g) {
    //draw to the right of the right most channel
    auto w = (channels == 2) ? rightRect.getRight() : leftRect.getRight();
    w += 4;
    g.setColour(juce::Colours::lightgrey);
    g.setFont(BG_FONT_SIZE);
    juce::Rectangle<int> textR;
    for (int i = 0; i < 9; ++i) {
        textR.setBounds(w, ys[i] - 3, 14, 6);
        g.drawFittedText(dBs[i], textR, juce::Justification::centred, 1);
    }
    textR.setBounds(w, ys[8] - 3, 14, 6);
    g.setColour(juce::Colours::green);
    g.drawFittedText(dBs[8], textR, juce::Justification::centred, 1);
}

void PeakMeterComponent::drawHoldLine(juce::Graphics& g, float hold, juce::Rectangle<int> line, juce::Rectangle<int>& r) {
    //map based on dB, clamp, move peak line based on given float, then draw it
    float hold_dB = juce::Decibels::gainToDecibels(hold, MIN_ANALYSIS_DB);
    float lineY = juce::jmap(hold_dB, MIN_ANALYSIS_DB, 0.0f, 0.0f, 1.0f);
    lineY = juce::jlimit(0.0f, 1.0f, lineY);
    int h = r.getHeight();
    line.setY(r.getY() + h - static_cast<int>(h * lineY));
    g.setColour(juce::Colours::white);
    g.fillRect(line);
}

void PeakMeterComponent::drawClipNotifier(juce::Graphics& g, juce::Rectangle<int>& r) {
    g.setColour(juce::Colours::red);
    g.fillRect(r);
}

void PeakMeterComponent::drawHoldText(juce::Graphics& g, float hold, juce::Rectangle<int>& r) {
    g.setColour(juce::Colours::lightgrey);
    g.setFont(12);
    juce::String s = juce::String::formatted("%.1f", juce::Decibels::gainToDecibels(hold, MIN_ANALYSIS_DB));
    juce::Rectangle<int> textR;
    textR.setSize(METER_WIDTH + 2, 10);
    textR.setCentre(r.getX() - 1 + r.getWidth() / 2, r.getY() - 14);
    g.drawFittedText(s, textR, juce::Justification::centred, 1);
    textR.expand(2, 2);
    g.drawRect(textR);
}

void PeakMeterComponent::resized() {
    auto bounds = getLocalBounds();
    //the 8 accounts for the text outside of the bg area
    bounds.removeFromLeft(BORDER_SPACING + METER_SPACING * 2);
    //this is 0 dB in the bg
    bounds.setTop(ys[8]);
    //remove border but leave room for outline
    bounds.removeFromBottom(BORDER_SPACING - 1);
    //for each meter, set meter area, then set clip and hold based on that meter 
    if (channels > 1) { 
        resizeMeter(leftRect, leftClipRect, leftHoldRect, bounds.removeFromLeft(METER_WIDTH));
        bounds.removeFromLeft(METER_SPACING); 
        resizeMeter(rightRect, rightClipRect, rightHoldRect, bounds.removeFromLeft(METER_WIDTH)); 
    } 
    else { 
        auto trimTo = (bounds.getWidth() - METER_WIDTH) / 2;
        resizeMeter(leftRect, leftClipRect, leftHoldRect, bounds.withTrimmedLeft(trimTo).withTrimmedRight(trimTo)); 
    } 
}

void PeakMeterComponent::mouseDown(const juce::MouseEvent& event) {
    //if user clicks in the meter, remove clip notifier
    auto pos = event.getPosition();
    if (leftRect.contains(pos)) {
        leftClipNotifier = false;
    }
    else if (rightRect.contains(pos)) {
        rightClipNotifier = false;
    }
}

float PeakMeterComponent::getSmoothedValue(juce::SmoothedValue<float>& value) {
    //Faster attack vs release at the rate of UPWARD_ATTACK_MULTIPLIER
    if (value.getCurrentValue() > value.getTargetValue()) {
        return value.getNextValue();
    }
    else {
        return value.skip(UPWARD_ATTACK_MULTIPLIER);
    }
}

void PeakMeterComponent::resetValues() {
    leftPeakHold = 0.0f;
    rightPeakHold = 0.0f;
    currLeftPeak.setCurrentAndTargetValue(0.0f);
    currRightPeak.setCurrentAndTargetValue(0.0f);
    leftPeakHoldCounter = 0;
    rightPeakHoldCounter = 0;
}

//setup rects for meter, hold, and clip
void PeakMeterComponent::resizeMeter(juce::Rectangle<int>& m, juce::Rectangle<int>& c, juce::Rectangle<int>& h, juce::Rectangle<int> bounds) {
    m = bounds;
    c = { m.getX() - 1, m.getY() - PEAK_CLIP_HEIGHT, METER_WIDTH + 2, PEAK_CLIP_HEIGHT };
    h = { m.getX(), m.getY(), METER_WIDTH, 1 };
}

void PeakMeterComponent::setYs(juce::Array<float>& newYs) {
    //get editor ys, then fix ys[n] from changes made in editor background
    ys = newYs;
    int n = ys.size() - 1;
    ys.set(n, ys[n] - 4);
}





////==============================================================================
////mono versions!!!!!!
//PeakMeterComponent::PeakMeterComponent(SemiProQAudioProcessor& p) : audioProcessor(p) {
//    channels = 2;
//    fillColor = juce::Colours::lime;
//
//    //setup smooth values
//    currLeftPeak.reset(TIMER_FPS, PEAK_DECAY_TIME);
//    currLeftPeak.setCurrentAndTargetValue(0.0f);
//}
//
//void PeakMeterComponent::timerCallback() {
//    //destructive read
//    float leftPeakValue = audioProcessor.leftPeak.read();
//    //Replace peak
//    currLeftPeak.setTargetValue(leftPeakValue);
//    //repalce hold if larger
//    if (leftPeakValue > leftPeakHold) {
//        leftPeakHold = leftPeakValue;
//        leftPeakHoldCounter = PEAK_HOLD_FRAMES;
//    }
//    //estimated check for clipping
//    if (leftPeakValue >= EST_CLIP_BARRIER) {
//        leftClipNotifier = true;
//    }
//    //count down hold or linear drop after hold frames run out
//    (leftPeakHoldCounter > 0.0f) ? leftPeakHoldCounter-- : leftPeakHold = leftPeakHold * 0.975f;
//
//    repaint();
//}
//
//void PeakMeterComponent::paint(juce::Graphics& g) {
//    //left meter draw functions
//    drawFilledMeter(g, getSmoothedValue(currLeftPeak), leftRect);
//    drawOutlineRect(g, leftRect);
//    drawHoldLine(g, leftPeakHold, leftHoldRect, leftRect);
//    if (leftClipNotifier) {
//        drawClipNotifier(g, leftClipRect);
//    }
//    drawHoldText(g, leftPeakHold, leftRect);
//    drawLabelText(g);
//}
//
//void PeakMeterComponent::resized() {
//    auto bounds = getLocalBounds();
//    //the 8 accounts for the text outside of the bg area
//    bounds.removeFromLeft(BORDER_SPACING + METER_SPACING * 2);
//    //this is 0 dB in the bg
//    bounds.setTop(ys[8]);
//    //remove border but leave room for outline
//    bounds.removeFromBottom(BORDER_SPACING - 1);
//    //for each meter, set meter area, then set clip and hold based on that meter 
//    auto trimTo = (bounds.getWidth() - METER_WIDTH) / 2;
//    resizeMeter(leftRect, leftClipRect, leftHoldRect, bounds.withTrimmedLeft(trimTo).withTrimmedRight(trimTo));
//}
//
//void PeakMeterComponent::mouseDown(const juce::MouseEvent& event) {
//    //if user clicks in the meter, remove clip notifier
//    auto pos = event.getPosition();
//    if (leftRect.contains(pos)) {
//        leftClipNotifier = false;
//    }
//}
//
//void PeakMeterComponent::resetValues() {
//    leftPeakHold = 0.0f;
//    currLeftPeak.setCurrentAndTargetValue(0.0f);
//    leftPeakHoldCounter = 0;
//}
//
//void PeakMeterComponent::drawLabelText(juce::Graphics& g) {
//    //draw to the right of the right most channel
//    auto w = leftRect.getRight();
//    w += 4;
//    g.setColour(juce::Colours::lightgrey);
//    g.setFont(BG_FONT_SIZE);
//    juce::Rectangle<int> textR;
//    for (int i = 0; i < 9; ++i) {
//        textR.setBounds(w, ys[i] - 3, 14, 6);
//        g.drawFittedText(dBs[i], textR, juce::Justification::centred, 1);
//    }
//    textR.setBounds(w, ys[8] - 3, 14, 6);
//    g.setColour(juce::Colours::green);
//    g.drawFittedText(dBs[8], textR, juce::Justification::centred, 1);
//}