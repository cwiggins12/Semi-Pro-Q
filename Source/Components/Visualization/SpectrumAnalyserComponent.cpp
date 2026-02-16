
#include "SpectrumAnalyserComponent.h"
#include "PluginProcessor.h"

//==============================================================================
/** Analyser Component: all of my comments were lost because visual studio is garbage :(
*/
SpectrumAnalyserComponent::SpectrumAnalyserComponent(SemiProQAudioProcessor& p) : audioProcessor(p),
forwardFFT(FFT_ORDER), window(FFT_SIZE + 1, juce::dsp::WindowingFunction<float>::hann) {
    setInterceptsMouseClicks(false, false);
    setOpaque(false);
    lineColor = juce::Colours::lime;
    std::fill(std::begin(sampleData), std::end(sampleData), 0.0f);
    std::fill(std::begin(fftData), std::end(fftData), 0.0f);
    std::fill(std::begin(binScalars), std::end(binScalars), 0.0f);
    lastSampleRate = audioProcessor.getLastSampleRate();
    //if decent sr given, compute bin scalars(tilt and power scale)
    if (lastSampleRate > 1) {
        computeBinScalars();
    }
}

SpectrumAnalyserComponent::~SpectrumAnalyserComponent() {}

void SpectrumAnalyserComponent::timerCallback() {
    auto* fifo = audioProcessor.getAnalyserFifo();
    if (!fifo) {
        return;
    }
    //on reset, mode change, state change, or sample rate change
    if (needsClear.load()) {
        fifo->clear();
        bufferPos = 0;
        resetScopeData();
        needsClear.store(false);
    }
    //pop and accumulate
    const int numRead = fifo->pop(sampleData + bufferPos, FFT_HOP_SIZE);
    bufferPos += numRead;
    //when accumulated
    if (bufferPos >= FFT_SIZE) {
        accumulatedBuffer();
    }
    repaint();
}

void SpectrumAnalyserComponent::accumulatedBuffer() {
    //copy to fft scratch buffer
    std::copy(sampleData, sampleData + FFT_SIZE, fftData);
    //if new sample rate, update scalars and pixel bin buffers
    double sr = audioProcessor.getSampleRate();
    if (sr != lastSampleRate) {
        lastSampleRate = sr;
        computeBinScalars();
        updatePixelFrequencyMapping(lastWidth);
    }
    //window and fft
    window.multiplyWithWindowingTable(fftData, FFT_SIZE);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);
    //multiply by scalars
    for (int i = 0; i < FFT_BIN_AMT; ++i) {
        fftData[i] *= binScalars[i];
    }
    //draw new frame
    drawNextFrameOfSpectrum();
    //copy to get 75% overlap and use bufferPos to lose no samples
    bufferPos = bufferPos - FFT_HOP_SIZE;
    memmove(sampleData, sampleData + FFT_HOP_SIZE, bufferPos * sizeof(float));
}

void SpectrumAnalyserComponent::computeBinScalars() {
    //power scaling: window loss, fft scaling, and ~+12 dB to have a closer match to the peak scaling
    const float pScale = 4.0f / (float)FFT_SIZE;
    //slope / constant. Slope is to match a more musical analyser by boosting high end
    const float tiltExponent = FFT_SLOPE / 6.0206f;
    //used to get freq from bin
    float binMult = lastSampleRate / (float)FFT_SIZE;

    for (int i = 0; i < FFT_BIN_AMT; ++i) {
        float binFreq = (float)i * binMult;
        //simlified tilt function to remove log call
        float tilt = std::pow(binFreq / MID_FREQ, tiltExponent);
        //scalar for each bin
        binScalars[i] = tilt * pScale;
    }
}

void SpectrumAnalyserComponent::drawNextFrameOfSpectrum() {
    auto h = (float)getHeight();
    //for each pixel, either lerp based on surrounding bins for freq < 1000 or rms each bin in pixel area for freq > 1000
    for (int i = 0; i < lastWidth; ++i) {
        float dB;

        if (i < firstHighPixel) {
            dB = getLowFreqSmoothedValue(i, pixelBinIndices[i]);
        }
        else {
            dB = getHighFreqSmoothedValues(i);
        }
        //map, clamp, then set to target (maps to y pixel pos)
        auto level = juce::jmap(dB, MIN_ANALYSIS_DB, 0.0f, h, 0.0f);
        level = juce::jlimit(0.0f, h, level);
        pixelValues[i].setTargetValue(level);
    }
}

float SpectrumAnalyserComponent::getLowFreqSmoothedValue(int pixelIndex, float centerBinFloat) {
    //use fraction for mu in interp
    int bin1 = (int)centerBinFloat;
    float fraction = centerBinFloat - bin1;

    int bin0 = juce::jmax(0, bin1 - 1);
    int bin2 = juce::jmin(FFT_BIN_AMT - 1, bin1 + 1);
    int bin3 = juce::jmin(FFT_BIN_AMT - 1, bin1 + 2);

    //set bins to dB, hate doing this repeatedly, but it is what it is
    float y0 = juce::Decibels::gainToDecibels(fftData[bin0], MIN_ANALYSIS_DB);
    float y1 = juce::Decibels::gainToDecibels(fftData[bin1], MIN_ANALYSIS_DB);
    float y2 = juce::Decibels::gainToDecibels(fftData[bin2], MIN_ANALYSIS_DB);
    float y3 = juce::Decibels::gainToDecibels(fftData[bin3], MIN_ANALYSIS_DB);

    //get 4 bins, interpolate
    return cubicInterpolate(y0, y1, y2, y3, fraction);
}

float SpectrumAnalyserComponent::getHighFreqSmoothedValues(int pixelIndex) {    
    //never overlap rms
    int lowB = (int)pixelBinIndices[pixelIndex - 1] + 1;
    int highB = (int)pixelBinIndices[pixelIndex];

    //get root mean squared
    float sumSq = 0.0f;
    for (int i = lowB; i <= highB && i < FFT_BIN_AMT; ++i) {
        sumSq += fftData[i] * fftData[i];
    }
    float rms = std::sqrt(sumSq / (highB - lowB + 1));
    //return as dB
    return juce::Decibels::gainToDecibels(rms, MIN_ANALYSIS_DB);
}

float SpectrumAnalyserComponent::cubicInterpolate(float y0, float y1, float y2, float y3, float mu) {
    //Catmull-Rom spline interpolation
    float mu2 = mu * mu;
    float a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
    float a1 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float a2 = -0.5f * y0 + 0.5f * y2;
    float a3 = y1;

    return a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3;
}

void SpectrumAnalyserComponent::updatePixelFrequencyMapping(int width) {
    //gets bin at each pixel for smoothing ops, and get first bin over MID_FREQ for branch in drawNextFrame
    pixelBinIndices.resize(width);
    bool firstHighPixelFound = false;

    const float scale = FFT_SIZE / lastSampleRate;

    for (int i = 0; i < width; ++i) {
        float normalized = (float)i / (float)(width - 1);
        float freq = freqRange.convertFrom0to1(normalized);
        if (!firstHighPixelFound && freq > MID_FREQ) {
            firstHighPixel = i;
            firstHighPixelFound = true;
        }
        float binIndexFloat = freq * scale;

        pixelBinIndices[i] = juce::jlimit(0.0f, (float)FFT_BIN_AMT - 1, binIndexFloat);
    }
}

void SpectrumAnalyserComponent::paint(juce::Graphics& g) {
    //no vals, no draw
    if (pixelValues.empty()) return;

    //use helper to get next smoothed value and move x by downsample amount
    juce::Path path;
    path.startNewSubPath(0, getSmoothedValue(pixelValues[0]));
    for (int i = 1; i < pixelValues.size(); ++i) {
        path.lineTo((float)i * downsample, getSmoothedValue(pixelValues[i]));
    }
    //draw the path
    g.setColour(lineColor);
    g.strokePath(path, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
}

void SpectrumAnalyserComponent::resized() {
    //downsample pixels to divide the expense
    int width = getWidth() / downsample + 1;
    //if new, reset smoooth vals and pixel map
    if (width != lastWidth) {
        lastWidth = width;

        pixelValues.resize(width);

        resetScopeData();

        updatePixelFrequencyMapping(width);
    }
    //set up a clear in timer callback
    needsClear.store(true);
}

void SpectrumAnalyserComponent::resetScopeData() {
    //reset ramp, current, and target in all values
    auto h = getHeight();
    for (auto& value : pixelValues) {
        value.reset(TIMER_FPS);
        value.setCurrentAndTargetValue(h);
    }
}

float SpectrumAnalyserComponent::getSmoothedValue(juce::SmoothedValue<float>& value) {
    //separate attack and release values, backwards to account for y pixel positioning
    if (value.getCurrentValue() < value.getTargetValue()) {
        return value.skip(FFT_RELEASE);
    }
    else {
        return value.skip(FFT_ATTACK);
    }
}
