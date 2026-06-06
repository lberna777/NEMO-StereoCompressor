#include "LookAndFeel.h"
#include "BinaryData.h"

const juce::Colour NemoLookAndFeel::DISPLAY_TEXT { 0xffcfe6e0 };
const juce::Colour NemoLookAndFeel::METER_GREEN  { 0xff5ad17a };
const juce::Colour NemoLookAndFeel::METER_YELLOW { 0xfff0c84a };
const juce::Colour NemoLookAndFeel::METER_RED    { 0xffe8503c };

NemoLookAndFeel::NemoLookAndFeel()
{
    knobImg  = juce::ImageCache::getFromMemory(BinaryData::nemo_knob_png,
                                               BinaryData::nemo_knob_pngSize);
    faderImg = juce::ImageCache::getFromMemory(BinaryData::nemo_fader_png,
                                               BinaryData::nemo_fader_pngSize);
}

void NemoLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPos,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider&)
{
    if (knobImg.isNull()) return;

    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Riquadro del knob (quadrato, centrato nel componente)
    const float side = (float) juce::jmin(width, height);
    const float cx   = (float) x + (float) width  * 0.5f;
    const float cy   = (float) y + (float) height * 0.5f;

    const float scale = side / (float) knobImg.getWidth();

    auto t = juce::AffineTransform::translation(-knobImg.getWidth()  * 0.5f,
                                                -knobImg.getHeight() * 0.5f)
                 .scaled(scale)
                 .rotated(angle)
                 .translated(cx, cy);

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImageTransformed(knobImg, t);
}

void NemoLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPos, float, float,
                                        juce::Slider::SliderStyle, juce::Slider&)
{
    if (faderImg.isNull()) return;

    // La traccia/groove è già nello sfondo: disegno solo il cap alla posizione.
    const float capW = (float) width;
    const float capH = capW * (float) faderImg.getHeight() / (float) faderImg.getWidth();

    const float cx = (float) x + (float) width * 0.5f;
    const float cy = sliderPos; // centro del thumb in coordinate Y assolute

    juce::ignoreUnused(y, height);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(faderImg,
                (int) (cx - capW * 0.5f), (int) (cy - capH * 0.5f),
                (int) capW, (int) capH,
                0, 0, faderImg.getWidth(), faderImg.getHeight());
}

int NemoLookAndFeel::getSliderThumbRadius(juce::Slider& s)
{
    // Metà altezza del cap: JUCE mantiene il thumb dentro i bounds del componente.
    if (faderImg.isNull()) return juce::LookAndFeel_V4::getSliderThumbRadius(s);
    const float aspect = (float) faderImg.getHeight() / (float) faderImg.getWidth();
    return (int) ((float) s.getWidth() * aspect * 0.5f);
}
