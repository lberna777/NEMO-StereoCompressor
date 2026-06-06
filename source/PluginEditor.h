#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

// ── Meter centrale OUT + GR, disegnato sopra il vetro dello skin ──
class MeterDisplay : public juce::Component,
                     private juce::Timer
{
public:
    explicit MeterDisplay(StereoCompressorProcessor& p);
    ~MeterDisplay() override;
    void paint(juce::Graphics&) override;
private:
    void timerCallback() override;
    StereoCompressorProcessor& processor;
    float displayedOut { -60.0f };
    float displayedGR  {   0.0f };
};

// ── Toggle PHASE: hotspot trasparente sopra lo switch stampato nello sfondo ──
// (estetica dello switch lasciata com'è nel background; lieve glow quando attivo)
class PhaseToggle : public juce::Button
{
public:
    PhaseToggle();
    void paintButton(juce::Graphics&, bool, bool) override;
};

// ── Editor principale (skin NEMO) ──
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
    NemoLookAndFeel lnf;
    juce::Image background;

    struct KnobGroup
    {
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KnobGroup hpFreq, lpFreq, threshold, attack, release, makeup, habiss, paralarva;

    juce::Slider inFader, outFader;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inAtt, outAtt;

    PhaseToggle phaseButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseAtt;

    MeterDisplay meterDisplay;

    float        ledLevel[8] { 0,0,0,0,0,0,0,0 };          // intensità accensione box
    juce::Colour ledColour[8];                              // colore per-box (HABISS/PARALARVA reattivi)

    void setupKnob(KnobGroup&, const juce::String& paramID);
    void drawActivityLed(juce::Graphics&, juce::Rectangle<int>, float level, juce::Colour);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoCompressorEditor)
};
