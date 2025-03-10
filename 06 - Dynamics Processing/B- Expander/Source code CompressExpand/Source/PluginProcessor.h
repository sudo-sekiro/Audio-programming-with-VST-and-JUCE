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
class CompressExpandAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    CompressExpandAudioProcessor();
    ~CompressExpandAudioProcessor() override;

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

private:
    //==============================================================================
  juce::AudioParameterFloat* thresholdParam;
  juce::AudioParameterFloat* ratioParam;
  juce::AudioParameterFloat* attackTimeParam;
  juce::AudioParameterFloat* releaseTimeParam;
  juce::AudioParameterFloat* makeUpGainParam;
  juce::AudioParameterBool* bypassParam;
  juce::AudioParameterChoice* modeParam;

  juce::AudioSampleBuffer mixedDownInput;
  float x_g, y_g, x_l, y_l, y_l_prev, control, alphaAttack, alphaRelease;

  float inputLevel;
  float inverseSampleRate;
  float inverseE;
  float calculateAttackOrRelease(float value);
  float inputPhase = 0;             // Phase of the sinusoid, range 0 to 1
 };
