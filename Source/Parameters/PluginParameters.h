/*
  ==============================================================================

    PluginParameters.h
    パラメータID定義ファイル

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Parameters
{
    // パラメータID (AudioProcessorValueTreeState で使用)
    constexpr auto INPUT_GAIN_ID = "Param_962e981d";
    constexpr auto INPUT_GAIN_NAME = "Input Gain";

    constexpr auto OUTPUT_GAIN_ID = "Param_5fab7387";
    constexpr auto OUTPUT_GAIN_NAME = "Output Gain";

    constexpr auto WET_MIX_ID = "Param_69854352";
    constexpr auto WET_MIX_NAME = "Wet Mix";

    constexpr auto CURVE_ID = "Param_68ed7f16";
    constexpr auto CURVE_NAME = "Curve";

    // パラメータのセットアップ関数
    inline juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            INPUT_GAIN_ID, INPUT_GAIN_NAME,
            juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f,
            "dB", juce::AudioProcessorParameter::genericParameter,
            [](float value, int)
            { return juce::String(value, 2); },
            [](const juce::String &text)
            { return text.getFloatValue(); }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            OUTPUT_GAIN_ID, OUTPUT_GAIN_NAME,
            juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f,
            "dB", juce::AudioProcessorParameter::genericParameter,
            [](float value, int)
            { return juce::String(value, 2); },
            [](const juce::String &text)
            { return text.getFloatValue(); }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            WET_MIX_ID, WET_MIX_NAME,
            juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f,
            "%", juce::AudioProcessorParameter::genericParameter,
            [](float value, int)
            { return juce::String(value * 100, 1); },
            [](const juce::String &text)
            { return text.getFloatValue() / 100.0f; }));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            CURVE_ID, CURVE_NAME,
            juce::NormalisableRange<float>(0.1f, 10.0f), 2.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float value, int)
            { return juce::String(value, 2); },
            [](const juce::String &text)
            { return text.getFloatValue(); }));

        return layout;
    }
}
