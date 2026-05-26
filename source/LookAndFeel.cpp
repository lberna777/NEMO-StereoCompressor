#include "LookAndFeel.h"

const juce::Colour NeomodernLookAndFeel::PANEL_LIGHT    { 0xffd6dbe0 };
const juce::Colour NeomodernLookAndFeel::PANEL_LIGHT_HI { 0xffe4e8ec };
const juce::Colour NeomodernLookAndFeel::PANEL_DARK     { 0xff242830 };
const juce::Colour NeomodernLookAndFeel::ACCENT_CYAN    { 0xff7dc8e8 };
const juce::Colour NeomodernLookAndFeel::ACCENT_CYAN_DIM{ 0xff4e8aa6 };
const juce::Colour NeomodernLookAndFeel::KNOB_BODY      { 0xff1c2025 };
const juce::Colour NeomodernLookAndFeel::KNOB_BODY_HI   { 0xff3a4048 };
const juce::Colour NeomodernLookAndFeel::KNOB_RIM       { 0xff9098a2 };
const juce::Colour NeomodernLookAndFeel::TEXT_DARK      { 0xff222428 };
const juce::Colour NeomodernLookAndFeel::TEXT_MUTED     { 0xff6a7178 };
const juce::Colour NeomodernLookAndFeel::METER_GREEN    { 0xff7dc8e8 };
const juce::Colour NeomodernLookAndFeel::METER_YELLOW   { 0xfff0c060 };
const juce::Colour NeomodernLookAndFeel::METER_RED      { 0xfff04860 };

NeomodernLookAndFeel::NeomodernLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId,       TEXT_DARK);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId,               TEXT_MUTED);
}

juce::Font NeomodernLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(10.5f, juce::Font::bold);
}

void NeomodernLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                              int x, int y, int width, int height,
                                              float sliderPos,
                                              float rotaryStartAngle,
                                              float rotaryEndAngle,
                                              juce::Slider&)
{
    const float diameter = (float) juce::jmin(width, height) - 12.0f;
    const float radius   = diameter * 0.5f;
    const float cx       = (float) x + (float) width  * 0.5f;
    const float cy       = (float) y + (float) height * 0.5f;
    const float angle    = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // ── Anello esterno (rim metallico) ──
    {
        juce::ColourGradient rimGrad(KNOB_RIM.brighter(0.3f), cx, cy - radius - 4,
                                      KNOB_RIM.darker(0.4f),  cx, cy + radius + 4, false);
        g.setGradientFill(rimGrad);
        g.fillEllipse(cx - radius - 3, cy - radius - 3, (radius + 3) * 2, (radius + 3) * 2);
    }

    // ── Drop shadow soft sotto al corpo del knob ──
    {
        g.setColour(juce::Colours::black.withAlpha(0.18f));
        g.fillEllipse(cx - radius + 1, cy - radius + 3, radius * 2, radius * 2);
    }

    // ── Corpo del knob (gradiente verticale dark) ──
    {
        juce::ColourGradient bodyGrad(KNOB_BODY_HI, cx, cy - radius * 0.8f,
                                       KNOB_BODY,   cx, cy + radius * 0.9f, false);
        g.setGradientFill(bodyGrad);
        g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);
    }

    // ── Highlight sottile in alto ──
    {
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        const float hlW = radius * 1.3f;
        const float hlH = radius * 0.55f;
        g.fillEllipse(cx - hlW * 0.5f, cy - radius * 0.95f, hlW, hlH);
    }

    // ── Arco indicatore di posizione (ciano) ──
    {
        const float arcR = radius + 5.0f;
        juce::Path bgArc;
        bgArc.addCentredArc(cx, cy, arcR, arcR, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(ACCENT_CYAN_DIM.withAlpha(0.35f));
        g.strokePath(bgArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        juce::Path fgArc;
        fgArc.addCentredArc(cx, cy, arcR, arcR, 0.0f,
                            rotaryStartAngle, angle, true);
        g.setColour(ACCENT_CYAN);
        g.strokePath(fgArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // ── Indicatore (linea ciano dal centro verso il bordo) ──
    {
        const float ptrLength = radius * 0.62f;
        const float ptrWidth  = 3.0f;
        juce::Path ptr;
        ptr.addRoundedRectangle(-ptrWidth * 0.5f,
                                 -radius + 6.0f,
                                  ptrWidth,
                                  ptrLength,
                                  1.5f);
        ptr.applyTransform(juce::AffineTransform::rotation(angle).translated(cx, cy));
        g.setColour(ACCENT_CYAN);
        g.fillPath(ptr);
    }
}

void NeomodernLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                  const juce::Colour& /*backgroundColour*/,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.5f);
    const bool on = button.getToggleState() || shouldDrawButtonAsDown;

    juce::Colour base = on ? ACCENT_CYAN_DIM : KNOB_BODY;
    if (shouldDrawButtonAsHighlighted && !on) base = base.brighter(0.18f);

    juce::ColourGradient grad(base.brighter(0.32f), bounds.getCentreX(), bounds.getY(),
                               base.darker(0.25f),  bounds.getCentreX(), bounds.getBottom(),
                               false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Bordo
    g.setColour(on ? ACCENT_CYAN.brighter(0.2f) : KNOB_RIM.darker(0.2f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    if (on)
    {
        // Inner glow
        g.setColour(ACCENT_CYAN.withAlpha(0.55f));
        g.drawRoundedRectangle(bounds.reduced(1.8f), 3.0f, 1.0f);
    }
}

void NeomodernLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                            bool /*shouldDrawButtonAsHighlighted*/,
                                            bool shouldDrawButtonAsDown)
{
    const bool on = button.getToggleState() || shouldDrawButtonAsDown;
    g.setColour(on ? juce::Colours::white : TEXT_MUTED);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(button.getButtonText(),
               button.getLocalBounds(),
               juce::Justification::centred);
}
