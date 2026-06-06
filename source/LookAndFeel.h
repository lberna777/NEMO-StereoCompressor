#pragma once
#include <JuceHeader.h>

// Look-and-feel dello skin NEMO: knob e fader disegnati con gli asset PNG
// (alluminio fotorealistico). Il knob ruota; il cap del fader scorre.
class NemoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NemoLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider&) override;

    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;

    int getSliderThumbRadius(juce::Slider&) override;

    // ── Palette per meter / testo dei box valore ──
    static const juce::Colour DISPLAY_TEXT;   // testo chiaro nei box
    static const juce::Colour METER_GREEN;
    static const juce::Colour METER_YELLOW;
    static const juce::Colour METER_RED;

private:
    juce::Image knobImg;
    juce::Image faderImg;
};
