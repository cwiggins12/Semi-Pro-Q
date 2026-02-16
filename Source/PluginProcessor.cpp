
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SemiProQAudioProcessor::SemiProQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true))
#endif 
{
    //cache sample rate
    lastSampleRate = getSampleRate();
    if (lastSampleRate < 1) {
        lastSampleRate = 44100.0;
    }
    //init param listeners
    for (auto& id : params) {
        tree.addParameterListener(id, this);
    }
    for (int i = 0; i < MAX_FILTERS; ++i) {
        //initialize filter structs
        auto& info = filterData[i];
        info.freq.store(*tree.getRawParameterValue(params[FREQ + i * PARAMS_PER_FILTER]));
        info.gain.store(*tree.getRawParameterValue(params[GAIN + i * PARAMS_PER_FILTER]));
        info.quality.store(*tree.getRawParameterValue(params[QUALITY + i * PARAMS_PER_FILTER]));
        info.type.store(static_cast<int>(*tree.getRawParameterValue(params[TYPE + i * PARAMS_PER_FILTER])));
        info.b_worth.store(static_cast<int>(*tree.getRawParameterValue(params[B_WORTH + i * PARAMS_PER_FILTER])));
        info.bypass.store(*tree.getRawParameterValue(params[BYPASS + i * PARAMS_PER_FILTER]) >= 0.5f);
        initProperty(i, false);
        info.dirty.store(true);
        //initialize filter vectors with coefficients from info
        filters[i].update(info, lastSampleRate);
    }

    //analyserOn, analyserMode, minimizeGain, minimizeSelectedEq, minimizeConfigs, peakOn, peakMode, and selectedEq properties
    initProperty(ANALYSER_ON, true);
    initProperty(ANALYSER_MODE, true); //TRUE IS POST
    initProperty(PEAK_ON, true);
    initProperty(PEAK_MODE, true);     //TRUE IS POST 
    initProperty(MINIMIZE_GAIN, false);
    initProperty(MINIMIZE_SELECTED, false);
    initProperty(MINIMIZE_SETTINGS, false);
    initProperty(SELECTED_FILTER, -1);
}

SemiProQAudioProcessor::~SemiProQAudioProcessor() {
    for (auto& id : params) {
        tree.removeParameterListener(id, this);
    }
}

//==============================================================================
//internal functions, unchanged
const juce::String SemiProQAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool SemiProQAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SemiProQAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SemiProQAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SemiProQAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int SemiProQAudioProcessor::getNumPrograms() {
    return 1;
}

int SemiProQAudioProcessor::getCurrentProgram() {
    return 0;
}

void SemiProQAudioProcessor::setCurrentProgram(int index) {}

const juce::String SemiProQAudioProcessor::getProgramName(int index) {
    return {};
}

void SemiProQAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

//==============================================================================
// Mission critical functions
//called on sample rate change, buffer size change, and on start up. Is DAW/host dependent
void SemiProQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    //set spec, prepare filters, prepare gains, make or clear analyser fifo
    lastSampleRate = sampleRate;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    for (auto& filter : filters) {
        filter.prepare(spec);
    }

    preGain.setRampDurationSeconds(GAIN_RAMP_TIME);
    preGain.prepare(spec);
    preGain.setGainDecibels(*tree.getRawParameterValue(params[PREGAIN]));

    postGain.setRampDurationSeconds(GAIN_RAMP_TIME);
    postGain.prepare(spec);
    postGain.setGainDecibels(*tree.getRawParameterValue(params[POSTGAIN]));
    
    analyserFifo = std::make_unique<Fifo<float>>(FFT_SIZE + FFT_HOP_SIZE);
    if (analyserFifo) {
        analyserFifo->clear();
    }
}

void SemiProQAudioProcessor::releaseResources() {
    analyserFifo.reset();
}

//if there's a need for a standalone, remove preset channel cofigs and uncomment this
//bool SemiProQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
//    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
//        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
//        return false;
//    }
//    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
//        return false;
//    }
//    return true;
//}

void SemiProQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    //get channels and numSamples
    juce::ScopedNoDenormals noDenormals;
    auto* left = buffer.getReadPointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : nullptr;
    const int numSamples = buffer.getNumSamples();
    //analyser & peak bools from properties
    const bool analyserOn = analyserFifo && tree.state[props[ANALYSER_ON]];
    const bool analyserMode = tree.state[props[ANALYSER_MODE]];
    const bool peakOn = tree.state[props[PEAK_ON]];
    const bool peakMode = tree.state[props[PEAK_MODE]];
    //use property bools to get analysis state bools
    const bool analyserPre = analyserOn && !analyserMode;
    const bool analyserPost = analyserOn && analyserMode;
    const bool peakPre = peakOn && !peakMode;
    const bool peakPost = peakOn && peakMode;

    //pre eq spectrum analysis and peak readings
    if (analyserPre) {
        analyserFifo->getBufferSamples(left, right, numSamples);
    }
    if (peakPre) {
        leftPeak.getPeakFromBlock(left, numSamples);
        if (right) {
            rightPeak.getPeakFromBlock(right, numSamples);
        }
    }

    //update dirty filters
    updateFilters();

    //prepare block
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    //process pre gain
    preGain.process(context);
    //process filters
    for (auto& filter : filters) {
        filter.process(context);
    }
    //process post gain
    postGain.process(context);

    //post eq spectrum analysis and peak readings
    if (analyserPost) {
        analyserFifo->getBufferSamples(left, right, numSamples);
    }
    if (peakPost) {
        leftPeak.getPeakFromBlock(left, numSamples);
        if (right) {
            rightPeak.getPeakFromBlock(right, numSamples);
        }
    }
}

//==============================================================================
//editor construction
bool SemiProQAudioProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* SemiProQAudioProcessor::createEditor() {
    return new SemiProQAudioProcessorEditor(*this);
    //return new juce::GenericAudioProcessorEditor(*this); //use to test without editor
}

//==============================================================================
//Save load for parameters and user prefs
//Save and Load for when the instance window is closed, but still running in DAW
void SemiProQAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::MemoryOutputStream mos(destData, true);
    tree.state.writeToStream(mos);
}

//loads the tree that contains all parameters and properties, 
//based on tests: all paramChange msgs run before the filter updates in prepare to play, needs confirmation in DAWS other than ableton
void SemiProQAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto readData = juce::ValueTree::readFromData(data, sizeInBytes);
    if (readData.isValid()) {
        tree.replaceState(readData);
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SemiProQAudioProcessor();
}

//==============================================================================
//paramter layout creation on processor construction
juce::AudioProcessorValueTreeState::ParameterLayout SemiProQAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 0; i < MAX_FILTERS; ++i) {
        //init freq: first at 500, then +500 for each subsequent
        layout.add(std::make_unique<juce::AudioParameterFloat>(params[FREQ + i * PARAMS_PER_FILTER], params[FREQ + i * PARAMS_PER_FILTER], 
            logRange<float>(MIN_FREQ, MAX_FREQ), 500.0f + 500.0f * i, juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction([](float value, int) {
                return formatFrequency(value);
                })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
                })
        ));
        //init gain to 0 db
        layout.add(std::make_unique<juce::AudioParameterFloat>(params[GAIN + i * PARAMS_PER_FILTER], params[GAIN + i * PARAMS_PER_FILTER], 
            juce::NormalisableRange<float>(MIN_DB, MAX_DB), 0.0f, juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction([](float value, int) {
                return formatGain(value);
                })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
                })
        ));
        //init quality to 1.0
        layout.add(std::make_unique<juce::AudioParameterFloat>(params[QUALITY + i * PARAMS_PER_FILTER], params[QUALITY + i * PARAMS_PER_FILTER], 
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f), 1.0f, juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction([](float value, int) {
                return formatQuality(value);
                })
            .withValueFromStringFunction([](const juce::String& text) {
                return text.getFloatValue();
                })
        ));
        //init type(0 is peak, 1 is low cut(b_worth), 2 is high cut(b_worth), 3 is low cut(resonant), 4 is high cut(resonant), 5 is high shelf, 6 is low shelf, 7 is notch)
        layout.add(std::make_unique<juce::AudioParameterChoice>(params[TYPE + i * PARAMS_PER_FILTER], params[TYPE + i * PARAMS_PER_FILTER], filterTypes, 0));
        //init butterworth int for stages if filter is in a butterworth mode
        layout.add(std::make_unique<juce::AudioParameterChoice>(params[B_WORTH + i * PARAMS_PER_FILTER], params[B_WORTH + i * PARAMS_PER_FILTER], b_worths, 0));
        //init bypass to true
        layout.add(std::make_unique<juce::AudioParameterBool>(params[BYPASS + i * PARAMS_PER_FILTER], params[BYPASS + i * PARAMS_PER_FILTER], true));
    }
    //init pre and post gain
    layout.add(std::make_unique<juce::AudioParameterFloat>(params[PREGAIN], params[PREGAIN], MIN_ANALYSIS_DB, MAX_DB, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(params[POSTGAIN], params[POSTGAIN], MIN_ANALYSIS_DB, MAX_DB, 0.0f));
    return layout;
}

//==============================================================================
//parameter updating
//give eq ind, param ind, and 0 to 1 value to change
void SemiProQAudioProcessor::updateParameter(int eqInd, int paramInd, float newValue) {
    if (auto* pParam = tree.getParameter(params[paramInd + eqInd * PARAMS_PER_FILTER])) {
        pParam->beginChangeGesture();
        pParam->setValueNotifyingHost(newValue);
        pParam->endChangeGesture();
    }
}

//update req struct and gui coeffs, b_worth doesn't require coeffs changes, so it returns right away
void SemiProQAudioProcessor::parameterChanged(const juce::String& paramID, float newValue) {
    int filterIndex = -1;
    int digitEnd = 0;
    //iterate and grab digits to get the right eq
    while (digitEnd < paramID.length() && juce::CharacterFunctions::isDigit(paramID[digitEnd])) {
        digitEnd++;
    }
    //if digits grabbed, get param name and update filter struct with newValue accordingly with helper
    if (digitEnd > 0) {
        filterIndex = paramID.substring(0, digitEnd).getIntValue() - 1;
        if (filterIndex >= 0 && filterIndex < MAX_FILTERS) {
            auto& info = filterData[filterIndex];
            updateFilterStruct(info, paramID.substring(digitEnd), newValue, filterIndex);
            return;
        }
    }
    //if no digits grabbed, check pre/post gains, do nothing on analyser param changes
    if (paramID == params[PREGAIN]) { 
        updateGain(0, newValue); 
    }
    else if (paramID == params[POSTGAIN]) {
        updateGain(1, newValue);
    }
}

//param change helper to update filterData structs
void SemiProQAudioProcessor::updateFilterStruct(FilterInfo& info, juce::String paramName, float newValue, int bandIndex) {
    //always needs coeff update
    if (paramName == "Freq") {
        info.freq.store(newValue);
        info.dirty.store(true);
    }
    //gain only affects peak and shelf filters
    else if (paramName == "Gain") {
        info.gain.store(newValue);
        if (info.type == PEAK || info.type == HIGHSHELF || info.type == LOWSHELF) {
            info.dirty.store(true);
        }
    }
    //q doesn't affect dB/oct filters
    else if (paramName == "Quality") {
        info.quality.store(newValue);
        if (!(info.type == HIGHPASS_OCT || info.type == LOWPASS_OCT)) {
            info.dirty.store(true);
        }
    }
    //always needs coeff update
    else if (paramName == "Type") {
        info.type.store(static_cast<int>(newValue));
        info.dirty.store(true);
    }
    //never needs coeff update, but still needs a responseCurveRepaint msg
    else if (paramName == "dB/Oct") {
        info.b_worth.store(static_cast<int>(newValue));
        if (info.type == HIGHPASS_OCT || info.type == LOWPASS_OCT) {
            info.dirty.store(true);
        }
    }
    //always needs coeff update
    else if (paramName == "Bypass") {
        const bool b = newValue >= 0.5f;
        info.bypass.store(b);
        if (!b) {
            //if automation has unbypassed the filter, show it in the editor if it is open
            tree.state.setProperty(props[bandIndex], true, nullptr);
        }
        info.dirty.store(true);
    }
}

//false for pre, true for post
void SemiProQAudioProcessor::updateGain(bool id, float newValue) {
    if (id == 0) {
        preGain.setGainDecibels(newValue);
    }
    else if (id == 1) {
        postGain.setGainDecibels(newValue);
    }
}

//reset all params when filter is uninitialized, except freq and gain
void SemiProQAudioProcessor::resetEq(int ind) {
    if (ind < 0 || ind >= MAX_FILTERS) {
        return;
    }
    updateParameter(ind, QUALITY, 0.1f);
    updateParameter(ind, TYPE, PEAK);
    updateParameter(ind, B_WORTH, 0);
    updateParameter(ind, BYPASS, true);
    tree.state.setProperty(props[ind], false, nullptr);
}

//==============================================================================
//Filter updating
//helper for process block to use filterInfo to decide on which filters are updated
void SemiProQAudioProcessor::updateFilters() {
    const double sr = lastSampleRate;
    for (int i = 0; i < MAX_FILTERS; ++i) {
        auto& info = filterData[i];
        if (info.dirty.load()) {
            filters[i].update(info, sr);
            dirtyCurve.store(true);
        }
    }
}
