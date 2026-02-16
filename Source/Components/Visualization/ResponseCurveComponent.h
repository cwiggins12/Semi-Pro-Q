
#pragma once

#include <JuceHeader.h>
#include "Utils/AudioProcessing.h"
#include "Utils/Constants.h"

class SemiProQAudioProcessor;

//==============================================================================
/** Response curve logic and painting
*/
struct ResponseCurveComponent : juce::Component {
    ResponseCurveComponent(SemiProQAudioProcessor&);
    ~ResponseCurveComponent();

    //draws path for curve then triggers repaint
    void timerCallback();

private:
    void paint(juce::Graphics& g) override;
    //precomputed freq pixel positions based on logRange
    void precomputePixelFreqs();
    //sets magnitude array to 1.0
    void resetMags();
    //make coefficients for mags update from targetValues to be more responsive
    void makeGUICoefficients(FilterInfo& info, double sr);
    //update mags array for each filter
    void updateMagsFromFilters();
    //alternated interal juce coefficients function, requires coeffs pointer and expects allocated HeapBlocks
    void getMagnitudesFromFrequencyArray(double sampleRate, const float* coeffs, int stages) const noexcept;
    //cheats to fix flickering on low end, high q sweeps by using the ideal analog equation rather than the digital coeffs
    void getIdealPeakMags(double sampleRate, const double gain, const double f0, const double q) const noexcept;
    //cheats to fix flickering on low end, high q sweeps by using the ideal analog equation rather than the digital coeffs
    void getIdealNotchMags(double sampleRate, const double q, const double f0) const noexcept;
    //decibal conversion for each item in heapBlock
    void magsToDecibels();
    //maps dB mags to y pixel position
    void magsToYCoords();

    SemiProQAudioProcessor& audioProcessor;

    juce::NormalisableRange<double> freqRangeDbl{ logRange<double>(MIN_FREQ, MAX_FREQ) };
    juce::HeapBlock<double> mags, freqs;
    //size of all allocated HeapBlocks, down sample is the divisor of blockSize. Basically, work/space recs is divided by downsampleMult
    int blockSize;
    std::array<float, COEFF_SIZE + 1> tempCoeffs;
};
