
#pragma once

#include <JuceHeader.h>
#include "Utils/Constants.h"
class SemiProQAudioProcessor;

//==============================================================================
/** Spectrum Analyser with rms for high end and interpolation for low end
*/
struct SpectrumAnalyserComponent : juce::Component {
    SpectrumAnalyserComponent(SemiProQAudioProcessor&);
    ~SpectrumAnalyserComponent();

    //pull from fifo and see if able to draw
    void timerCallback();

    juce::Colour lineColor;
    std::atomic<bool> needsClear{ false };

private:
    void paint(juce::Graphics& g) override;
    void resized() override;

    //op when buffer full
    void accumulatedBuffer();
    //get all the y pixels for each pixel position
    void drawNextFrameOfSpectrum();
    //precompute frequency-to-bin mapping for each pixel
    void updatePixelFrequencyMapping(int w);
    //scale based on window loss, fft norm, and tilt factor for music
    void computeBinScalars();
    //reset scope data on sample rate change or spec config change
    void resetScopeData();
    //helper to decide how fast rate should be based on whether the value is lowering or rising
    float getSmoothedValue(juce::SmoothedValue<float>& value);
    //gets scope y pixel value for pixels < 1000 hz using cubic interpolation
    float getLowFreqSmoothedValue(int pixelIndex, float centerBinFloat);
    //gets scope y pixel value for pixels >= 1000 hz using rms
    float getHighFreqSmoothedValues(int pixelIndex);
    //cubic helper for getLowFreq function
    float cubicInterpolate(float y0, float y1, float y2, float y3, float mu);

    SemiProQAudioProcessor& audioProcessor;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    //including buffer in fifo these all amount to ~160 kB now. (FFT_SIZE * sizeof(float) * (4 + 2 * (HOP_SIZE / FFT_SIZE) + .5(FFT_BIN_AMT))
    //Ring buffer that handles fifo and sampleData logic would add a ton of complexity and thread safety issues, but it could chop 40 kB off of this

    //sample buffer with hopping logic
    float sampleData[FFT_SIZE + FFT_HOP_SIZE];
    //fft processing buffer
    float fftData[FFT_SIZE * 2];
    //saved value of each pixel's scaling and tilt. I don't love this tradeoff. Cache 4097 floats or compute this every new frame
    float binScalars[FFT_BIN_AMT];

    //pixel-based arrays (dynamically sized based on component width / downsampling + hasModulo) at max width (1093) and downsampling = 2,
    //sizes are 547 and costs (547 * 24 bytes)(24 is from sizes: float + SmoothValue) + (24 * 2) bytes from ptrs per vec,  ~12kb

    //corresponding FFT bin index (float for interpolation)
    std::vector<float> pixelBinIndices;
    //smoothed output values
    std::vector<juce::SmoothedValue<float>> pixelValues;

    //buffer pos signals when new frame is available
    int bufferPos = 0;
    //cached sample rate
    double lastSampleRate = 0.0;
    //cached width / downsample + 1
    int lastWidth = 0;
    //downsample how may pixels are processed
    int downsample = 2;
    //first pixel at freq above 1000hz
    int firstHighPixel = 0;
};