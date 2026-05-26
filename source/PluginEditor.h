#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class StereoCompressorEditor : public juce::AudioProcessorEditor,
                                private juce::Timer
{
public:
    explicit StereoCompressorEditor(StereoCompressorProcessor&);
    ~StereoCompressorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    StereoCompressorProcessor& processor;

    // Knob + label per ogni parametro
    struct KnobGroup {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KnobGroup threshold, ratio, attack, release, makeup, width;

    // GR meter
    juce::Label grLabel;
    float displayedGR { 0.0f };

    void setupKnob(KnobGroup& g, const juce::String& paramID, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoCompressorEditor)
};
