/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/ParametricEQ.h"

//==============================================================================
/**
*/
class SimpleEqualizerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEqualizerAudioProcessor();
    ~SimpleEqualizerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //for doc
    //Processor
    enum Parameters {
        MasterBypass = 0,
        Frequency,
        BandWidth,
        Gain,
        SampleRate,
        totalNumParam
    };

    int getNumParameters() override;
    float getParameter(int index) override;
    void setParameter(int index, float value) override;
    const String getParameterName(int index) override;
    const String getParameterText(int index) override;

private:
    //Processor
    float UserParams[totalNumParam];
    ParametricEQ parametricEQ[2]; //Create PeakingEQ for 2ch
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEqualizerAudioProcessor)
};
