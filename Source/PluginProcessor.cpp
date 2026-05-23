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
    oversamplingX4.initProcessing(static_cast<size_t>(samplesPerBlock));
    oversamplingX4.reset();

    const auto oversampledSampleRate = sampleRate * static_cast<double>(OversamplingFactor);
    const auto oversampledBlockSize = samplesPerBlock * static_cast<int>(OversamplingFactor);

    powerDistProcessor.prepare(oversampledSampleRate, oversampledBlockSize);

    const auto numChannels = static_cast<juce::uint32>(getTotalNumInputChannels());
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), numChannels};
    juce::dsp::ProcessSpec oversampledSpec{oversampledSampleRate, static_cast<juce::uint32>(oversampledBlockSize), numChannels};

    inputGainProcessor.prepare(oversampledSpec);
    outputGainProcessor.prepare(spec);

    inputGainProcessor.setRampDurationSeconds(0.01);
    outputGainProcessor.setRampDurationSeconds(0.01);

    dryWetMixer.prepare(oversampledSpec);
    dryWetMixer.setMixingRule(juce::dsp::DryWetMixer<float>::MixingRule::linear);
    dryWetMixer.setWetLatency(0.0f);

    const auto oversamplingLatency = oversamplingX4.getLatencyInSamples();
    setLatencySamples(juce::roundToInt(oversamplingLatency));
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
    inputGainProcessor.setGainDecibels(inputGain->load());
    outputGainProcessor.setGainDecibels(outputGain->load());
    dryWetMixer.setWetMixProportion(wetMix->load());

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampledBlock = oversamplingX4.processSamplesUp(block);

    dryWetMixer.pushDrySamples(oversampledBlock);

    juce::dsp::ProcessContextReplacing<float> oversampledContext(oversampledBlock);
    inputGainProcessor.process(oversampledContext);
    powerDistProcessor.process(oversampledBlock);
    dryWetMixer.mixWetSamples(oversampledBlock);

    oversamplingX4.processSamplesDown(block);

    juce::dsp::ProcessContextReplacing<float> context(block);
    outputGainProcessor.process(context);
}

void PowerDistAudioProcessor::reset()
{
    powerDistProcessor.reset();
    oversamplingX4.reset();
    inputGainProcessor.reset();
    outputGainProcessor.reset();
    dryWetMixer.reset();
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
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PowerDistAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PowerDistAudioProcessor();
}
