#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

// ── Componente custom: display risposta in frequenza + GR overlay ──
class FreqResponseDisplay : public juce::Component,
                             private juce::Timer
{
public:
    explicit FreqResponseDisplay(StereoCompressorProcessor& p);
    ~FreqResponseDisplay() override;

    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;

    StereoCompressorProcessor& processor;
    float lastHp { -1.0f };
    float lastLp { -1.0f };
    float displayedGR { 0.0f };
};

// ── Meter verticale stereo (I o O) ──
class VerticalMeter : public juce::Component,
                       private juce::Timer
{
public:
    enum Side { Input, Output };

    VerticalMeter(StereoCompressorProcessor& p, Side s);
    ~VerticalMeter() override;

    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;

    StereoCompressorProcessor& processor;
    Side side;
    float displayedL { -60.0f };
    float displayedR { -60.0f };
};

// ── Editor principale ──
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
    void paintTentacleIcon(juce::Graphics& g, juce::Rectangle<int> bounds, float intensity);

    StereoCompressorProcessor& processor;
    NeomodernLookAndFeel lnf;

    struct KnobGroup
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KnobGroup hpFreq, lpFreq;
    KnobGroup threshold, attack, release, makeup;
    KnobGroup habisso, width;

    // Ratio 1176-style
    juce::TextButton ratioButtons[4];
    juce::Label      ratioLabel;

    // Display + meter
    FreqResponseDisplay freqDisplay;
    VerticalMeter       inMeter;
    VerticalMeter       outMeter;

    // Per evitare repaint inutili del tentacolo
    float lastHabissoVisual { -1.0f };

    // Rect dove disegnare il tentacolo (calcolato in resized())
    juce::Rectangle<int> tentacleArea;

    void setupKnob(KnobGroup& g, const juce::String& paramID, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoCompressorEditor)
};
