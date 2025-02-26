/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HelloWorld1AudioProcessor::HelloWorld1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
  addParameter(gain = new juce::AudioParameterFloat("gain", "Decay", 0.8f, 0.99f, 0.99f));
  addParameter(delayTimeParam = new juce::AudioParameterFloat("delayTime", "Delay Time",NormalisableRange(0.0f, 0.02f, 0.001f), 0.005f));
  addParameter(widthParam = new juce::AudioParameterFloat("width", "Noise Width", NormalisableRange(0.0f, 0.02f, 0.001f), 0.01f));
  addParameter(cutOffParam = new juce::AudioParameterFloat("cutOff", "Cut Off Frequency", NormalisableRange(0.0f, 5000.0f, 10.0f), 1000.0f));

  addParameter(pluckParam = new juce::AudioParameterBool("pluck", "Pluck string", 0));

}

HelloWorld1AudioProcessor::~HelloWorld1AudioProcessor()
{
}

//==============================================================================
const juce::String HelloWorld1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HelloWorld1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HelloWorld1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HelloWorld1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HelloWorld1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HelloWorld1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HelloWorld1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void HelloWorld1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HelloWorld1AudioProcessor::getProgramName (int index)
{
    return {};
}

void HelloWorld1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HelloWorld1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  delayBufferLength = (int)(2.0f * sampleRate);
  if (delayBufferLength < 1) {
    delayBufferLength = 1;
  }
  DBG("delayBufferLength: " + String(delayBufferLength));
  delayBuffer.setSize(2, delayBufferLength);
  delayBuffer.clear();

  juce::dsp::ProcessSpec processSpec;
  processSpec.sampleRate = sampleRate;
  processSpec.maximumBlockSize = samplesPerBlock;
  processSpec.numChannels = 2;
  lowPassFilter.prepare(processSpec);

  lowPassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(sampleRate, 1000.0f);
  lowPassFilter.reset();
}

void HelloWorld1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HelloWorld1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void HelloWorld1AudioProcessor::updateFilterParams(float cutOffFrequency, float sampleRate) {
  lowPassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(sampleRate, cutOffFrequency);
  // lowPassFilter.reset();
}

void HelloWorld1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  auto sampleRate = getSampleRate();
  delayReadPosition = (int)(delayWritePosition - (delayTime * sampleRate) + delayBufferLength) % delayBufferLength;
  auto numSamples = buffer.getNumSamples();

  delayTime = delayTimeParam->get();
  noiseWidth = widthParam->get();
  feedbackGain = gain->get();
  cutOffFrequency = cutOffParam->get();
  updateFilterParams(cutOffFrequency, sampleRate);
  auto pluck = pluckParam->get();
  if (pluck) {
    pluckParam->setValueNotifyingHost(0);
    NoiseGain = 1.0f;
  }
  for (int i = 0; i < numSamples; ++i){
    for (int j = 0; j < 2; ++j) {
      float in = 0.f;
      if (NoiseGain > 0.f) {
        in = 2.f * NoiseGain * ((double) rand() / RAND_MAX) - static_cast<double>(1.0);
      }
      // delayData is a circular buffer
      float* delayData = delayBuffer.getWritePointer(j);

      float out = in + feedbackGain * lowPassFilter.processSample(delayData[delayReadPosition]);
      delayData[delayWritePosition] = out;

      buffer.getWritePointer(j)[i] = out;
    }
    if (++delayReadPosition >= delayBufferLength) {
      delayReadPosition = 0;
    }
    if (++delayWritePosition >= delayBufferLength) {
      delayWritePosition = 0;
    }
    if (NoiseGain >= 0.f) {
      NoiseGain = NoiseGain - 1.f / (noiseWidth * (float) sampleRate);
    }
  }
}

//==============================================================================
bool HelloWorld1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HelloWorld1AudioProcessor::createEditor()
{
    return new GenericAudioProcessorEditor (*this);
}

//==============================================================================
void HelloWorld1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HelloWorld1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HelloWorld1AudioProcessor();
}
