/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PowerDistAudioProcessor::PowerDistAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
    inputGain = apvts.getRawParameterValue(Parameters::INPUT_GAIN_ID);
    outputGain = apvts.getRawParameterValue(Parameters::OUTPUT_GAIN_ID);
    wetMix = apvts.getRawParameterValue(Parameters::WET_MIX_ID);
    curve = apvts.getRawParameterValue(Parameters::CURVE_ID);
}

PowerDistAudioProcessor::~PowerDistAudioProcessor()
{
}

//==============================================================================
const juce::String PowerDistAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PowerDistAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PowerDistAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PowerDistAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PowerDistAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PowerDistAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int PowerDistAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PowerDistAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String PowerDistAudioProcessor::getProgramName(int index)
{
    return {};
}

void PowerDistAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void PowerDistAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    powerDistProcessor.prepare(sampleRate, samplesPerBlock);
}

void PowerDistAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PowerDistAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void PowerDistAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    powerDistProcessor.setCurve(curve->load());

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    powerDistProcessor.process(buffer);
}

void PowerDistAudioProcessor::reset()
{
    powerDistProcessor.reset();
}

//==============================================================================
bool PowerDistAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PowerDistAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PowerDistAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PowerDistAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PowerDistAudioProcessor();
}
