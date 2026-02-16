
#include "ResponseCurveComponent.h"
#include "PluginProcessor.h"

//==============================================================================
/** Response Curve: on parameter change only now, but can further be optimised by checking bandwidth of changed eq and clipping the paint,
*/
ResponseCurveComponent::ResponseCurveComponent(SemiProQAudioProcessor& p) : audioProcessor(p) {
    setInterceptsMouseClicks(false, false);
    setOpaque(false);
    blockSize = getLocalBounds().getWidth();
    freqs.allocate(blockSize, false);
    precomputePixelFreqs();
    mags.allocate(blockSize, false);
}

ResponseCurveComponent::~ResponseCurveComponent() {}

//only strokes cached path in area of passed clip
void ResponseCurveComponent::paint(juce::Graphics& g) {
    if (mags == nullptr) return;
    int xPos = getLocalBounds().getX();
    juce::Path path;
    path.startNewSubPath(xPos, mags[0]);
    for (int i = 1; i < blockSize; ++i) {
        path.lineTo(++xPos, mags[i]);
    }
    g.setColour(juce::Colours::white);
    g.strokePath(path, juce::PathStrokeType(3.0f));
}

void ResponseCurveComponent::timerCallback() {
    //get width and ensure arrays are the correct size, then compute freqs if size changed
    auto w = getLocalBounds().getWidth();
    if (w <= 0) {
        return;
    }
    if (blockSize != w) {
        blockSize = w;
        mags.realloc(blockSize);
        freqs.realloc(blockSize);
        precomputePixelFreqs();
    }
    //reset all mag array values to 1.0
    resetMags();

    //use helper to update mag array using filters
    updateMagsFromFilters();

    //sets each mag double to decibels
    magsToDecibels();

    //map mags to pixels for path in paint
    magsToYCoords();

    repaint();
}

//compute freq array based on size of component. Expects correct block size
void ResponseCurveComponent::precomputePixelFreqs() {
    for (int i = 0; i < blockSize; ++i) {
        double frac = double(i) / double(blockSize - 1);
        freqs[i] = freqRangeDbl.convertFrom0to1(frac);
    }
}

//reset all magnitudes to 1.0. Expects correct block size
void ResponseCurveComponent::resetMags() {
    for (int i = 0; i < blockSize; ++i) {
        mags[i] = 1.0;
    }
}

//updates every mag value based on filter values
void ResponseCurveComponent::updateMagsFromFilters() {
    //get sample rate, set to 44.1k if bad value (generally is around 0 if not the exact rate)
    auto sampleRate = audioProcessor.getLastSampleRate();
    //iterate through filters, if not bypassed or unititialized, copy safe coeff from req for use in modified getMagnitude to mult the entire arrays
    for (int i = 0; i < MAX_FILTERS; ++i) {
        auto& info = audioProcessor.getFilterInfo(i);
        if (info.bypass.load()) {
            continue;
        }
        if (info.type.load() == PEAK) {
            getIdealPeakMags(sampleRate, info.gain.load(), info.freq.load(), info.quality.load());
        }
        else if (info.type.load() == NOTCH) {
            getIdealNotchMags(sampleRate, info.freq.load(), info.quality.load());
        }
        else {
            float* tmpCfs = tempCoeffs.data();
            audioProcessor.getCoeffs(i, tmpCfs);
            getMagnitudesFromFrequencyArray(sampleRate, tmpCfs, info.b_worth.load());
        }
    }
}

//modified juce internal function from coefficients class. Expects a precomputed freq array, prereset mag arry(all at 1.0), and set blockSize
void ResponseCurveComponent::getMagnitudesFromFrequencyArray(double sampleRate, const float* coeffs, int stages) const noexcept {
    constexpr std::complex<double> j(0, 1);
    std::complex<double> numerator = 0.0, denominator = 1.0, factor = 1.0;

    for (int i = 0; i < blockSize; ++i) {
        numerator = 0.0, factor = 1.0, denominator = 1.0;
        std::complex<double> jw = std::exp(-juce::MathConstants<double>::twoPi * freqs[i] * j / sampleRate);

        for (int n = 0; n <= 2; ++n) {
            numerator += static_cast<double>(coeffs[n]) * factor;
            factor *= jw;
        }

        factor = jw;

        for (int n = 3; n <= 4; ++n) {
            denominator += static_cast<double> (coeffs[n]) * factor;
            factor *= jw;
        }

        double mag = std::abs(numerator / denominator);
        switch (stages) {
        case 0: mags[i] *= mag; break;
        case 1: mags[i] *= mag * mag; break;
        case 2: mags[i] *= mag * mag * mag; break;
        case 3: mags[i] *= mag * mag * mag * mag; break;
        }
    }
}

//more ideal way to build the peak mags than using the digital coeffs. Fixes visual bugs and shows user intent more clearly. Is more expensive though
void ResponseCurveComponent::getIdealPeakMags(double sampleRate, const double gain, const double f0, const double q) const noexcept {
    double w0 = 2.0 * juce::MathConstants<double>::pi * f0;
    double BW = w0 / q;
    double A = std::sqrt(juce::Decibels::decibelsToGain(gain, (double)NEG_INF_DB));

    for (int i = 0; i < blockSize; ++i) {
        double f = freqs[i];
        double w = 2.0 * juce::MathConstants<double>::pi * f;

        double num_real = w0 * w0 - w * w;
        double num_imag = w * BW * A;
        double num_mag_sq = num_real * num_real + num_imag * num_imag;

        double den_real = w0 * w0 - w * w;
        double den_imag = w * BW / A;
        double den_mag_sq = den_real * den_real + den_imag * den_imag;

        double mag = std::sqrt(num_mag_sq / den_mag_sq);

        mags[i] *= mag;
    }
}

//more ideal way to build the peak mags than using the digital coeffs. Fixes visual bugs, shows user intent more clearly, and is a bit cheaper
void ResponseCurveComponent::getIdealNotchMags(double sampleRate, const double f0, const double q) const noexcept {
    double w0 = 2.0 * juce::MathConstants<double>::pi * f0;

    for (size_t i = 0; i < blockSize; ++i) {
        double w = 2.0 * juce::MathConstants<double>::pi * freqs[i];

        double w_sq = w * w;
        double w0_sq = w0 * w0;
        double diff = w_sq - w0_sq;
        double diff_sq = diff * diff;

        double damping_term = (w * w0 / q);
        double damping_sq = damping_term * damping_term;

        double mag = std::abs(diff) / std::sqrt(diff_sq + damping_sq);

        mags[i] *= mag;
    }
}

//gain to decibels on all mags in array. Expects correct block size
void ResponseCurveComponent::magsToDecibels() {
    for (int i = 0; i < blockSize; ++i) {
        mags[i] = juce::Decibels::gainToDecibels(mags[i], (double)NEG_INF_DB);
    }
}

void ResponseCurveComponent::magsToYCoords() {
    //get area, make lambda map with values to map decibel mags,
    auto responseArea = getLocalBounds();
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input) { return juce::jmap(input, (double)MIN_DB, (double)MAX_DB, outputMin, outputMax); };
    for (int i = 0; i < blockSize; ++i) {
        mags[i] = map(mags[i]);
    }
}
