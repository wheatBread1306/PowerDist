#pragma once

#include <JuceHeader.h>
#include <array>

class PowerDistProcessor
{
public:
    PowerDistProcessor() = default;
    ~PowerDistProcessor() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::dsp::AudioBlock<float> block);
    void setCurve(float newCurve) noexcept { curve.setTargetValue(newCurve); }
    void reset();

private:
    double currentSampleRate{44100.0};
    int currentBlockSize{512};

    void makeDistLUT(float curveValue);

    std::array<float, 1024> distLUT{};

    juce::SmoothedValue<float> curve{2.0f};
    float prevCurve{0.0f};
};