
#pragma once

#include "Utils/Constants.h"

//==============================================================================
/** PARAMETERS, COEFFICIENTS, AND FILTERS
*/
//primary filter struct contains all filter values given by parameters value tree. Parameters in value tree are just used for save/load and listening. 
//All param changes are copied here, then utilized by Filter to tell the stage what its coeffs need to be
struct FilterInfo {
    //parameter copies
    std::atomic<float> freq;
    std::atomic<float> gain;
    std::atomic<float> quality;
    std::atomic<int> type{ 0 };
    std::atomic<int> b_worth{ 0 };
    std::atomic<bool> bypass{ true };
    std::atomic<bool> dirty{ false };
};
/*
This is the combined logic of JUCE Coefficient, Filter, and ProcessorDuplicator classes, but with no allocation after construction, smooth coefficient transitions, 
8 filter types, internal bypass logic, internal thread safe reads for the GUI & writes from the audio thread, while being smaller, faster, and contiguous.

If the filter is smoothing, it will linearly smooth the coefficients per sample until target is reached. Bypassed filters will forego processing and 
reset state once smoothing is complete. Topology is fixed and only handles mono or stereo.

The below only applies if you are not using this as a part of the Filter
WARNINGS: COEFF FACTORIES MUST BE CALLED FROM AUDIO THREAD! readCoeffs() MAY BUSY READ! prepare() RESETS STATE AND HARD-SETS SMOOTHED VALUES DIRECTLY TO TARGETS!
*/
struct EqStage {
    //needed for the compilers that don't always precompute
    static constexpr float inverseRootTwo = 0.70710678118654752440L;
    //same used in juce internal
    static constexpr float minimumDecibels = -300.0f;
    //==============================================================================
    //COEFFICIENTS FACTORY
    //NOT safe to call from any thread other than audio. Each call will build, factor out a0, then place as target in coeffs
    //Only called in the updateFilter function, which is only called in construction of processor and in process block
    //PEAK
    void makePeakFilter(double sampleRate, float frequency, float Q, float gainFactor) {
        const auto A = std::sqrt(juce::Decibels::gainWithLowerBound(gainFactor, (float)minimumDecibels));
        const auto omega = (2 * juce::MathConstants<float>::pi * juce::jmax(frequency, static_cast<float> (2.0))) / static_cast<float> (sampleRate);
        const auto alpha = std::sin(omega) / (Q * 2);
        const auto c2 = -2 * std::cos(omega);
        const auto alphaTimesA = alpha * A;
        const auto alphaOverA = alpha / A;

        factorAndWrite(1 + alphaTimesA, c2, 1 - alphaTimesA, 1 + alphaOverA, c2, 1 - alphaOverA);
    }
    //LOWPASS_OCT
    void makeLowPass(double sampleRate, float frequency) {
        makeLowPass(sampleRate, frequency, inverseRootTwo);
    }
    //HIGHPASS_OCT
    void makeHighPass(double sampleRate, float frequency) {
        makeHighPass(sampleRate, frequency, inverseRootTwo);
    }
    //LOWPASS_Q
    void makeLowPass(double sampleRate, float frequency, float Q) {
        const auto n = 1 / std::tan(juce::MathConstants<float>::pi * frequency / static_cast<float> (sampleRate));
        const auto nSquared = n * n;
        const auto invQ = 1 / Q;
        const auto c1 = 1 / (1 + invQ * n + nSquared);

        writeCoeffs(c1, c1 * 2, c1, c1 * 2 * (1 - nSquared), c1 * (1 - invQ * n + nSquared));
    }
    //HIGHPASS_Q
    void makeHighPass(double sampleRate, float frequency, float Q) {
        const auto n = std::tan(juce::MathConstants<float>::pi * frequency / static_cast<float> (sampleRate));
        const auto nSquared = n * n;
        const auto invQ = 1 / Q;
        const auto c1 = 1 / (1 + invQ * n + nSquared);

        writeCoeffs(c1, c1 * -2, c1, c1 * 2 * (nSquared - 1), c1 * (1 - invQ * n + nSquared));
    }
    //LOWSHELF
    void makeLowShelf(double sampleRate, float cutOffFrequency, float Q, float gainFactor) {
        const auto A = std::sqrt(juce::Decibels::gainWithLowerBound(gainFactor, (float)minimumDecibels));
        const auto aminus1 = A - 1;
        const auto aplus1 = A + 1;
        const auto omega = (2 * juce::MathConstants<float>::pi * juce::jmax(cutOffFrequency, static_cast<float> (2.0))) / static_cast<float> (sampleRate);
        const auto coso = std::cos(omega);
        const auto beta = std::sin(omega) * std::sqrt(A) / Q;
        const auto aminus1TimesCoso = aminus1 * coso;

        factorAndWrite(A * (aplus1 - aminus1TimesCoso + beta),
            A * 2 * (aminus1 - aplus1 * coso),
            A * (aplus1 - aminus1TimesCoso - beta),
            aplus1 + aminus1TimesCoso + beta,
            -2 * (aminus1 + aplus1 * coso),
            aplus1 + aminus1TimesCoso - beta);
    }
    //HIGHSHELF
    void makeHighShelf(double sampleRate, float cutOffFrequency, float Q, float gainFactor) {
        const auto A = std::sqrt(juce::Decibels::gainWithLowerBound(gainFactor, (float)minimumDecibels));
        const auto aminus1 = A - 1;
        const auto aplus1 = A + 1;
        const auto omega = (2 * juce::MathConstants<float>::pi * juce::jmax(cutOffFrequency, static_cast<float> (2.0))) / static_cast<float> (sampleRate);
        const auto coso = std::cos(omega);
        const auto beta = std::sin(omega) * std::sqrt(A) / Q;
        const auto aminus1TimesCoso = aminus1 * coso;

        factorAndWrite(A * (aplus1 + aminus1TimesCoso + beta),
            A * -2 * (aminus1 + aplus1 * coso),
            A * (aplus1 + aminus1TimesCoso - beta),
            aplus1 - aminus1TimesCoso + beta,
            2 * (aminus1 - aplus1 * coso),
            aplus1 - aminus1TimesCoso - beta);
    }
    //NOTCH
    void makeNotch(double sampleRate, float frequency, float Q) {
        const auto n = 1 / std::tan(juce::MathConstants<float>::pi * frequency / static_cast<float> (sampleRate));
        const auto nSquared = n * n;
        const auto invQ = 1 / Q;
        const auto c1 = 1 / (1 + n * invQ + nSquared);
        const auto b0 = c1 * (1 + nSquared);
        const auto b1 = 2 * c1 * (1 - nSquared);

        writeCoeffs(b0, b1, b0, b1, c1 * (1 - n * invQ + nSquared));
    }
    //BYPASS: Set to lerp to identity coeff, set isBypassed, and set armedForReset
    void makeBypassed() {
        writeCoeffs(1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        isBypassed.store(true);
        armedForReset.store(true);
    }
    //==============================================================================
    // GUI READ
    //seq-lock pattern to always prioritize writes in audio thread, but reads in gui try, check seq num, and try again if write happened during read
    //audio thread is only the writer, therefore never needs to call this
    void readCoeffs(float* dest) const {
        uint32_t seq;
        do {
            do {
                seq = sequence.load(std::memory_order_acquire);
            } while (seq & 1);
            for (int i = 0; i < COEFF_SIZE; ++i) {
                dest[i] = coefficients[i].getTargetValue();
            }
        } while (seq != sequence.load(std::memory_order_acquire));
    }
    //===============================================================================
    //prepare to play function: set coeffs based on sr, hard set current lerp val to target, get channels, reset state vars
    void prepare(const juce::dsp::ProcessSpec& spec) noexcept {
        resetCoeffs(spec.sampleRate);
        setCurrentsToTargets();
        //assert only mono or stereo, -1 of that channel amount will amount to true or false for isStereo
        jassert(spec.numChannels == 1 || spec.numChannels == 2);
        isStereo = spec.numChannels - 1;
        reset();
    }
    //top level process logic: calls based on channel amount, current smoothing state, or bypassed
    void process(const juce::dsp::ProcessContextReplacing<float>& context) noexcept {
        //if mono
        if (!isStereo) {
            //if current value != target on all of the coefficients
            if (isSmoothing()) {
                processInternalMono(context);
            }
            //if current vals == target vals, and filter is not bypassed
            else if (!isBypassed.load()) {
                processInternalNoSmoothMono(context);
            }
            else if (armedForReset.load()) {
                reset();
                armedForReset.store(false);
            }
        }
        //if stereo, follow same pattern
        else {
            if (isSmoothing()) {
                processInternalStereo(context);
            }
            else if (!isBypassed.load()) {
                processInternalNoSmoothStereo(context);
            }
            else if (armedForReset.load()) {
                reset();
                armedForReset.store(false);
            }
        }
    }

private:
    //==============================================================================
    //WRITE HELPERS
    //factor a0 from the few filter calculations that need it, then write
    void factorAndWrite(float b0in, float b1in, float b2in, float a0in, float a1in, float a2in) {
        if (std::abs(a0in) < 1e-10f) {
            writeCoeffs(b0in, b1in, b2in, a1in, a2in);
        }
        else {
            float a0Inv = 1.0f / a0in;
            writeCoeffs(b0in * a0Inv, b1in * a0Inv, b2in * a0Inv, a1in * a0Inv, a2in * a0Inv);
        }
    }
    //sequence locking write, only locks out GUI, never locks audio, only called by coeff internal functions
    void writeCoeffs(float b0in, float b1in, float b2in, float a1in, float a2in) {
        auto seq = sequence.load(std::memory_order_relaxed);
        sequence.store(seq + 1, std::memory_order_release);
        coefficients[0].setTargetValue(b0in);
        coefficients[1].setTargetValue(b1in);
        coefficients[2].setTargetValue(b2in);
        coefficients[3].setTargetValue(a1in);
        coefficients[4].setTargetValue(a2in);
        sequence.store(seq + 2, std::memory_order_release);
        isBypassed.store(false);
        armedForReset.store(false);
    }
    //==============================================================================
    //INTERNAL PROCESSORS
    //MONO PROCESSING WITH SMOOTHING
    void processInternalMono(const juce::dsp::ProcessContextReplacing<float>& context) noexcept {
        auto&& inputBlock = context.getInputBlock();
        auto&& outputBlock = context.getOutputBlock();

        auto numSamples = inputBlock.getNumSamples();
        auto* src = inputBlock.getChannelPointer(0);
        float* dst = outputBlock.getChannelPointer(0);

        auto& b0 = coefficients[0];
        auto& b1 = coefficients[1];
        auto& b2 = coefficients[2];
        auto& a1 = coefficients[3];
        auto& a2 = coefficients[4];

        auto lv1 = state[0];
        auto lv2 = state[1];

        for (size_t i = 0; i < numSamples; ++i) {
            auto input = src[i];
            auto output = (input * b0.getNextValue()) + lv1;
            dst[i] = output;

            lv1 = (input * b1.getNextValue()) - (output * a1.getNextValue()) + lv2;
            lv2 = (input * b2.getNextValue()) - (output * a2.getNextValue());
        }
        juce::dsp::util::snapToZero(lv1); state[0] = lv1;
        juce::dsp::util::snapToZero(lv2); state[1] = lv2;
    }
    //MONO PROCESSING WITHOUT SMOOTHING
    void processInternalNoSmoothMono(const juce::dsp::ProcessContextReplacing<float>& context) noexcept {
        auto&& inputBlock = context.getInputBlock();
        auto&& outputBlock = context.getOutputBlock();

        auto numSamples = inputBlock.getNumSamples();
        auto* src = inputBlock.getChannelPointer(0);
        float* dst = outputBlock.getChannelPointer(0);

        auto b0 = coefficients[0].getTargetValue();
        auto b1 = coefficients[1].getTargetValue();
        auto b2 = coefficients[2].getTargetValue();
        auto a1 = coefficients[3].getTargetValue();
        auto a2 = coefficients[4].getTargetValue();

        auto lv1 = state[0];
        auto lv2 = state[1];

        for (size_t i = 0; i < numSamples; ++i) {
            auto input = src[i];
            auto output = (input * b0) + lv1;
            dst[i] = output;

            lv1 = (input * b1) - (output * a1) + lv2;
            lv2 = (input * b2) - (output * a2);
        }
        juce::dsp::util::snapToZero(lv1); state[0] = lv1;
        juce::dsp::util::snapToZero(lv2); state[1] = lv2;
    }
    //STEREO PROCESSING WITH SMOOTHING
    void processInternalStereo(const juce::dsp::ProcessContextReplacing<float>& context) noexcept {
        auto&& inputBlock = context.getInputBlock();
        auto&& outputBlock = context.getOutputBlock();

        auto numSamples = inputBlock.getNumSamples();
        auto* srcL = inputBlock.getChannelPointer(0);
        float* dstL = outputBlock.getChannelPointer(0);
        auto* srcR = inputBlock.getChannelPointer(1);
        float* dstR = outputBlock.getChannelPointer(1);
        auto& b0r = coefficients[0];
        auto& b1r = coefficients[1];
        auto& b2r = coefficients[2];
        auto& a1r = coefficients[3];
        auto& a2r = coefficients[4];

        float b0, b1, b2, a1, a2;

        auto lv1 = state[0];
        auto lv2 = state[1];
        auto lv3 = state[2];
        auto lv4 = state[3];

        for (size_t i = 0; i < numSamples; ++i) {
            b0 = b0r.getNextValue();
            b1 = b1r.getNextValue();
            b2 = b2r.getNextValue();
            a1 = a1r.getNextValue();
            a2 = a2r.getNextValue();

            auto inputL = srcL[i];
            auto outputL = (inputL * b0) + lv1;
            dstL[i] = outputL;
            lv1 = (inputL * b1) - (outputL * a1) + lv2;
            lv2 = (inputL * b2) - (outputL * a2);

            auto inputR = srcR[i];
            auto outputR = (inputR * b0) + lv3;
            dstR[i] = outputR;
            lv3 = (inputR * b1) - (outputR * a1) + lv4;
            lv4 = (inputR * b2) - (outputR * a2);
        }
        juce::dsp::util::snapToZero(lv1); state[0] = lv1;
        juce::dsp::util::snapToZero(lv2); state[1] = lv2;
        juce::dsp::util::snapToZero(lv3); state[2] = lv3;
        juce::dsp::util::snapToZero(lv4); state[3] = lv4;
    }
    //STEREO PROCESSING WITHOUT SMOOTHING
    void processInternalNoSmoothStereo(const juce::dsp::ProcessContextReplacing<float>& context) noexcept {
        auto&& inputBlock = context.getInputBlock();
        auto&& outputBlock = context.getOutputBlock();

        auto numSamples = inputBlock.getNumSamples();
        auto* srcL = inputBlock.getChannelPointer(0);
        float* dstL = outputBlock.getChannelPointer(0);
        auto* srcR = inputBlock.getChannelPointer(1);
        float* dstR = outputBlock.getChannelPointer(1);

        auto b0 = coefficients[0].getTargetValue();
        auto b1 = coefficients[1].getTargetValue();
        auto b2 = coefficients[2].getTargetValue();
        auto a1 = coefficients[3].getTargetValue();
        auto a2 = coefficients[4].getTargetValue();

        auto lv1 = state[0];
        auto lv2 = state[1];
        auto lv3 = state[2];
        auto lv4 = state[3];

        for (size_t i = 0; i < numSamples; ++i) {
            auto inputL = srcL[i];
            auto outputL = (inputL * b0) + lv1;
            dstL[i] = outputL;
            lv1 = (inputL * b1) - (outputL * a1) + lv2;
            lv2 = (inputL * b2) - (outputL * a2);

            auto inputR = srcR[i];
            auto outputR = (inputR * b0) + lv3;
            dstR[i] = outputR;
            lv3 = (inputR * b1) - (outputR * a1) + lv4;
            lv4 = (inputR * b2) - (outputR * a2);
        }
        juce::dsp::util::snapToZero(lv1); state[0] = lv1;
        juce::dsp::util::snapToZero(lv2); state[1] = lv2;
        juce::dsp::util::snapToZero(lv3); state[2] = lv3;
        juce::dsp::util::snapToZero(lv4); state[3] = lv4;
    }
    //==============================================================================
    //SMOOTHED VALUE HELPERS
    //full is smoothing check for coeffs
    bool isSmoothing() {
        for (auto& val : coefficients) {
            if (val.isSmoothing()) {
                return true;
            }
        }
        return false;
    }
    //reset lerp timing on sample rate change
    void resetCoeffs(double sampleRate) {
        for (auto& val : coefficients) {
            val.reset(sampleRate, COEFF_RAMP_TIME);
        }
    }
    //called on init, prepare to play, and, possibly, on load
    void setCurrentsToTargets() {
        for (int i = 0; i < COEFF_SIZE; ++i) {
            auto target = coefficients[i].getTargetValue();
            coefficients[i].setCurrentAndTargetValue(target);
        }
    }
    //==============================================================================
    //STATE RESET
    //resets state floats to 0.0f
    void reset() {
        for (size_t i = 0; i < 4; ++i) {
            state[i] = 0.0f;
        }
    }
    //==============================================================================
    //MEMBER VARS
    //lerped coeffs
    std::array<juce::LinearSmoothedValue<float>, COEFF_SIZE> coefficients;
    //bypass, set in coeff factory, used in process
    std::atomic<bool> isBypassed{ true };
    //for resetting state after being set to bypassed and done lerping to identity finishes
    std::atomic<bool> armedForReset{ false };
    //channel count used in process and set in prepare
    std::atomic<bool> isStereo{ true };
    //sequence int for GUI lock
    std::atomic<uint32_t> sequence{ 0 };
    //pre allocated state vars. Filter order is locked at 2nd, Max Channels is 2, size is 2 * 2
    std::array<float, 4> state{ 0.0f, 0.0f, 0.0f, 0.0f };
};
/*
Array of EqStages at MAX_STAGES amount
prepare() should only be called in prepareToPlay() and prepares all stages' filters and coeffs
process() should only be called in processBlock() and processes for all filter stages. Bypass, channels, and smoothing are all handled internally
readCoeffs() should never be called from audio thread. It may busy read
update() should only be called from audio thread. It will write to each stage's coeffs
*/
struct SmoothFilter {
    void prepare(const juce::dsp::ProcessSpec& spec) {
        for (auto& s : stages) {
            s.prepare(spec);
        }
    }
    void process(const juce::dsp::ProcessContextReplacing<float>& context) {
        for (auto& s : stages) {
            s.process(context);
        }
    }
    void readCoeffs(float* dest) {
        stages[0].readCoeffs(dest);
    }
    void update(FilterInfo& info, double sr) {
        const auto bypass = info.bypass.load();
        const auto freq = info.freq.load();
        const auto q = info.quality.load();
        const auto gain = juce::Decibels::decibelsToGain(float(info.gain.load()), NEG_INF_DB);
        const auto type = info.type.load();
        const int stageAmt = juce::jlimit(1, MAX_STAGES, info.b_worth.load() + 1);
        //if bypassed, set all filters to bypass for smoothing to bypass state
        if (bypass) {
            for (auto& s : stages) {
                s.makeBypassed();
            }
        }
        //else make filter coeffs of type. If type is butterworth, make b_worth amount of filters, else make them bypassed. 
        //For all non b_worth filters, just make the first filter coeffs and make all the rest bypassed to smooth away the changes
        //all bypassing, smoothing, and processing is handled internally by the stage, this just sets the targets of the smoothed coefficients.
        else {
            switch (info.type) {
                case PEAK: {
                    stages[0].makePeakFilter(sr, freq, q, gain);
                    for (int j = 1; j < MAX_STAGES; ++j) {
                        stages[j].makeBypassed();
                    }
                    break;
                }
                case HIGHPASS_OCT: {
                    for (int j = 0; j < stageAmt; ++j) {
                        stages[j].makeHighPass(sr, freq);
                    }
                    for (int j = stageAmt; j < MAX_STAGES; ++j) {
                        stages[j].makeBypassed();
                    }
                    break;
                }
                case LOWPASS_OCT: {
                    for (int j = 0; j < stageAmt; ++j) {
                        stages[j].makeLowPass(sr, freq);
                    }
                    for (int j = stageAmt; j < MAX_STAGES; ++j) {
                        stages[j].makeBypassed();
                    }
                    break;
                }
                case HIGHPASS_Q: {
                    stages[0].makeHighPass(sr, freq, q);
                    for (int j = 1; j < MAX_STAGES; ++j) {
                        stages[j].makeBypassed();
                    }
                    break;
                }
                case LOWPASS_Q: {
                    stages[0].makeLowPass(sr, freq, q);
                    for (int j = 1; j < MAX_STAGES; ++j) {
                        stages[j].makeBypassed();
                    }
                    break;
                }
                case HIGHSHELF: {
                    stages[0].makeHighShelf(sr, freq, q, gain);
                    for (int j = 1; j < MAX_STAGES; ++j) {
                        stages[j].makeBypassed();
                    }
                    break;
                }
                case LOWSHELF: {
                    stages[0].makeLowShelf(sr, freq, q, gain);
                    for (int j = 1; j < MAX_STAGES; ++j) {
                        stages[j].makeBypassed();
                    }
                    break;
                }
                case NOTCH: {
                    stages[0].makeNotch(sr, freq, q);
                    for (int j = 1; j < MAX_STAGES; ++j) {
                        stages[j].makeBypassed();
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
        info.dirty.store(false);
    }

private:
    std::array<EqStage, MAX_STAGES> stages;
};
