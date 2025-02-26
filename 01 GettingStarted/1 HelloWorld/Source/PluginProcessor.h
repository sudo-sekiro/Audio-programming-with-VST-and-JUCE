/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class HelloWorld1AudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    HelloWorld1AudioProcessor();
    ~HelloWorld1AudioProcessor() override;

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

    void updateFilterParams(float cutOffFrequency, float sampleRate);
private:
    //==============================================================================
    juce::AudioParameterFloat* gain;
    juce::AudioParameterFloat* delayTimeParam;
    juce::AudioParameterFloat* widthParam;
    juce::AudioParameterFloat* cutOffParam;

    juce::AudioParameterBool* pluckParam;
    float NoiseGain = 0.0f;

    // Circular buffer variables for implementing delay
    juce::AudioSampleBuffer delayBuffer;
    int delayBufferLength;
    int delayReadPosition = 0, delayWritePosition = 0;
    float delayTime = 0.01f;
    float feedbackGain = 0.99f;
    float noiseWidth = 0.01f;
    float cutOffFrequency = 1000.0f;

    juce::dsp::IIR::Filter<float> lowPassFilter;
    juce::dsp::IIR::Coefficients<float> lowPassCoefficients;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloWorld1AudioProcessor)
};
