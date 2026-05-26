#pragma once
#include <JuceHeader.h>

class StereoCompressorProcessor : public juce::AudioProcessor
{
public:
    StereoCompressorProcessor();
    ~StereoCompressorProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Stereo Compressor"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // GR meter (letto dall'editor)
    float getGainReductionDB() const { return currentGR.load(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Stato compressore
    float envDB { 0.0f };
    float attackCoeff  { 0.0f };
    float releaseCoeff { 0.0f };
    double currentSampleRate { 44100.0 };

    std::atomic<float> currentGR { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoCompressorProcessor)
};
