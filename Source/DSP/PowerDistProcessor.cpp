#include "PowerDistProcessor.h"

namespace
{
    float powDist(float x, float power)
    {
        jassert(power >= 0.0f);

        const float ab = std::abs(x);
        const float sign = std::copysign(1.0f, x);
        return sign * std::exp(power * std::log(ab));
    }

    float clampSample(float sample)
    {
        return std::clamp(sample, -1.0f, 1.0f);
    }
}

void PowerDistProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    curve.reset(sampleRate, 0.01);
}

void PowerDistProcessor::process(juce::AudioBuffer<float> &buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if(curve.isSmoothing()|| !juce::approximatelyEqual(curve.getCurrentValue(), prevCurve)){
        const float currentCurve = curve.getCurrentValue();
        prevCurve = currentCurve;
        makeDistLUT(currentCurve);
    }

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float inputSample = channelData[sample];

            const float clampedSample = clampSample(inputSample);

            const float absInput = std::abs(clampedSample);
            const int lutIndex = static_cast<int>(absInput * (distLUT.size() - 1));
            const float distSample = distLUT[lutIndex] * std::copysign(1.0f, clampedSample);
            channelData[sample] = distSample;
        }
    }
    curve.skip(numSamples);
}

void PowerDistProcessor::makeDistLUT(float curveValue)
{
    for (size_t i = 0; i < distLUT.size(); ++i)
    {
        const float x = static_cast<float>(i) / (distLUT.size() - 1);
        distLUT[i] = powDist(x, curveValue);
    }
}

void PowerDistProcessor::reset()
{
    curve.reset(currentSampleRate, 0.01);
    distLUT.fill(0.0f);
}