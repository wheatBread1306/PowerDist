/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <JucePluginDefines.h>
#include "Parameters/PluginParameters.h"
#include "DSP/PowerDistProcessor.h"

//==============================================================================
/**
 */
class PowerDistAudioProcessor : public juce::AudioProcessor
{
public:
  //==============================================================================
  PowerDistAudioProcessor();
  ~PowerDistAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
  void reset() override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
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
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", Parameters::createParameterLayout()};

private:
  //==============================================================================

  static constexpr size_t MaxChannels = 2;
  static constexpr size_t OversamplingStages = 2;
  static constexpr size_t OversamplingFactor = 1u << OversamplingStages;

  PowerDistProcessor powerDistProcessor;
  juce::dsp::Gain<float> inputGainProcessor, outputGainProcessor;
  juce::dsp::DryWetMixer<float> dryWetMixer;

  std::atomic<float> *inputGain = {nullptr};
  std::atomic<float> *outputGain = {nullptr};
  std::atomic<float> *wetMix = {nullptr};
  std::atomic<float> *curve = {nullptr};

  juce::dsp::Oversampling<float> oversamplingX4{MaxChannels,
                                               OversamplingStages,
                                               juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
                                               true,
                                               true};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PowerDistAudioProcessor)
};
