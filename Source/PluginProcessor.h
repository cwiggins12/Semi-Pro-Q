#pragma once

#include <JuceHeader.h>
#include "Utils/Constants.h"
#include "Utils/AudioProcessing.h"
#include "Utils/VisualizerProcesing.h"

//==============================================================================
/**
*/
class SemiProQAudioProcessor : public juce::AudioProcessor, juce::AudioProcessorValueTreeState::Listener {
public:
    //==============================================================================
    SemiProQAudioProcessor();
    ~SemiProQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState tree{ *this, nullptr, "Parameters", createParameterLayout() };

    //this is the only way for the front end to change a parameter without attachments, use configs as ints and normalized 0 to 1 newValue
    void updateParameter(int id, int paramInd, float newValue);
    //this is just a collection of updateParameter calls for quick resets
    void resetEq(int ind);

    //get the eq at index atomic parameters + coeffs without dealing with the tree
    FilterInfo& getFilterInfo(int index) { return filterData[index]; }

    //front end getters for ptrs to fifos
    Fifo<float>* getAnalyserFifo() { return analyserFifo.get(); }

    //spec getter for front end
    const juce::dsp::ProcessSpec& getSpec() const { return spec; }

    //sample rate getter that always hands front end something reasonable
    double getLastSampleRate() {
        if (lastSampleRate <= 1.0) {
            lastSampleRate = 44100.0;
        }
        return lastSampleRate; 
    }

    //logic to coalesce response curve paint calls
    const bool getCurveStatus() const { return dirtyCurve.load(); }
    void setCurveStatus(bool b) { dirtyCurve.store(b); }
    
    //thread safe read to get target val
    void getCoeffs(int i, float* dest) { filters[i].readCoeffs(dest); }

    //peak metering objects to measure each channel
    PeakMeasurement leftPeak;
    PeakMeasurement rightPeak;

private:
    //property helper
    void initProperty(int idx, int val) {
        if (!tree.state.hasProperty(props[idx])) {
            tree.state.setProperty(props[idx], val, nullptr);
        }
    }
    //called in processor constructor to build layout for all automatable parameters
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //internal param change from msg thread
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    //update FilterInfo helper from parameterChanged
    void updateFilterStruct(FilterInfo& inf, juce::String paramName, float newValue, int bandIndex);
    //pre is 0, post is 1, only gets signal from attachments
    void updateGain(bool id, float newValue);
    //checks all filters and updates the ones that need it using filterInfo structs
    void updateFilters();

    //all 12 allocated in the constructor based MAX_EQs
    std::array<SmoothFilter, MAX_FILTERS> filters;
    //faster and safer than grabbing from ValueTree
    std::array<FilterInfo, MAX_FILTERS> filterData;
    //spec to prepare dsp objects
    juce::dsp::ProcessSpec spec;
    //ptr for analyser fifo. needs reset on release resources
    std::unique_ptr<Fifo<float>> analyserFifo;
    //gain dsp object with internal smoothedValues
    juce::dsp::Gain<float> preGain, postGain;
    //cached sample rate
    double lastSampleRate;
    //coalesces response curve repaint msgs
    std::atomic<bool> dirtyCurve = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SemiProQAudioProcessor)
};
