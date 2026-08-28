#include "PowerDistProcessor.h"

namespace
{
    inline float powDist(float x, float power)
    {
        jassert(power > 0.0f);

        const float ab = std::abs(x);
        const float sign = std::copysign(1.0f, x);
        return sign * std::exp(power * std::log(ab));
    }

    inline float clampSample(float sample)
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


void PowerDistProcessor::process(juce::dsp::AudioBlock<float> block)
{
    const int numChannels = static_cast<int>(block.getNumChannels());
    const int numSamples = static_cast<int>(block.getNumSamples());

    if (curve.isSmoothing() || !juce::approximatelyEqual(curve.getCurrentValue(), prevCurve))
    {
        const float currentCurve = curve.getCurrentValue();
        prevCurve = currentCurve;
        makeDistLUT(currentCurve);
    }

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float *channelData = block.getChannelPointer(static_cast<size_t>(ch));

        for (int i = 0; i < numSamples; ++i)
        {
            float s = clampSample(channelData[i]);
            float absS = std::abs(s);

            float pos = absS * (distLUT.size() - 1);
            int idx0 = static_cast<int>(pos);
            idx0 = std::min(idx0, static_cast<int>(distLUT.size() - 1));
            int idx1 = std::min(idx0 + 1, static_cast<int>(distLUT.size() - 1));
            float frac = pos - static_cast<float>(idx0);
            float interp = distLUT[idx0] + (distLUT[idx1] - distLUT[idx0]) * frac;

            float output = interp * std::copysign(1.0f, s);
            channelData[i] = output;
        }
    }

    curve.skip(numSamples);
}

void PowerDistProcessor::makeDistLUT(float curveValue)
{
    constexpr float inv = 1.0f / static_cast<float>(distLUT.size() - 1)

    for (size_t i = 0; i < distLUT.size(); ++i)
    {
        const float x = static_cast<float>(i) * inv;
        distLUT[i] = powDist(x, curveValue);
    }
}

void PowerDistProcessor::reset()
{
    curve.reset(currentSampleRate, 0.01);
}