#include "PluginEditor.h"

// ═════════════════════════════════════════════════════════════════════
//   FreqResponseDisplay
// ═════════════════════════════════════════════════════════════════════

FreqResponseDisplay::FreqResponseDisplay(StereoCompressorProcessor& p)
    : processor(p)
{
    startTimerHz(30);
}

FreqResponseDisplay::~FreqResponseDisplay() { stopTimer(); }

void FreqResponseDisplay::timerCallback()
{
    const float hp = processor.getCurrentHpFreq();
    const float lp = processor.getCurrentLpFreq();
    const float gr = processor.getGainReductionDB();

    // GR smoothing per il display (decay più lento del peak)
    if (gr < displayedGR) displayedGR = gr;
    else                  displayedGR = displayedGR * 0.85f + gr * 0.15f;

    if (std::abs(hp - lastHp) > 0.5f
        || std::abs(lp - lastLp) > 5.0f
        || true /* GR cambia spesso */)
    {
        lastHp = hp;
        lastLp = lp;
        repaint();
    }
}

void FreqResponseDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // ── Background del display (dark recessed) ──
    {
        juce::ColourGradient bgGrad(NeomodernLookAndFeel::PANEL_DARK.darker(0.15f),
                                     bounds.getCentreX(), bounds.getY(),
                                     NeomodernLookAndFeel::PANEL_DARK,
                                     bounds.getCentreX(), bounds.getBottom(),
                                     false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
    }

    auto plotArea = bounds.reduced(8.0f);
    // Lascia 10 px in basso per la barra GR
    auto grBarArea = plotArea.removeFromBottom(8.0f);
    plotArea.removeFromBottom(4.0f);

    const double sr = processor.getCurrentSampleRate();
    const float  hpF = juce::jlimit(20.0f, 500.0f,  processor.getCurrentHpFreq());
    const float  lpF = juce::jlimit(2000.0f, 20000.0f, processor.getCurrentLpFreq());

    auto hpCoef = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, hpF);
    auto lpCoef = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, lpF);

    // ── Grid ──
    {
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        const int freqMarks[] = { 50, 100, 200, 500, 1000, 2000, 5000, 10000 };
        for (int f : freqMarks)
        {
            float t = std::log10((float) f / 20.0f) / std::log10(20000.0f / 20.0f);
            float x = plotArea.getX() + t * plotArea.getWidth();
            g.drawVerticalLine((int) x, plotArea.getY(), plotArea.getBottom());
        }
        for (int dB = -18; dB <= 6; dB += 6)
        {
            float t = (float) (dB + 18) / 24.0f;
            float y = plotArea.getBottom() - t * plotArea.getHeight();
            g.drawHorizontalLine((int) y, plotArea.getX(), plotArea.getRight());
        }

        // 0 dB più visibile
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        float yZero = plotArea.getBottom() - (18.0f / 24.0f) * plotArea.getHeight();
        g.drawHorizontalLine((int) yZero, plotArea.getX(), plotArea.getRight());

        // Etichette frequenze
        g.setColour(NeomodernLookAndFeel::TEXT_MUTED.withAlpha(0.7f));
        g.setFont(juce::Font(8.5f, juce::Font::plain));
        for (int f : { 100, 1000, 10000 })
        {
            float t = std::log10((float) f / 20.0f) / std::log10(20000.0f / 20.0f);
            float x = plotArea.getX() + t * plotArea.getWidth();
            juce::String lbl = (f >= 1000) ? juce::String(f / 1000) + "k" : juce::String(f);
            g.drawText(lbl,
                       (int) x - 12, (int) plotArea.getBottom() - 12, 24, 10,
                       juce::Justification::centred);
        }
    }

    // ── Curva di risposta (HP × LP) ──
    juce::Path curve;
    const int numPoints = 220;
    const float yZeroDb = plotArea.getBottom() - (18.0f / 24.0f) * plotArea.getHeight();
    const float dbPerPx = 24.0f / plotArea.getHeight();

    for (int i = 0; i <= numPoints; ++i)
    {
        float t = (float) i / (float) numPoints;
        float freq = 20.0f * std::pow(1000.0f, t); // 20 → 20k
        double mag = hpCoef->getMagnitudeForFrequency((double) freq, sr)
                    * lpCoef->getMagnitudeForFrequency((double) freq, sr);
        double dB = juce::Decibels::gainToDecibels(mag);
        dB = juce::jlimit(-30.0, 6.0, dB);

        float x = plotArea.getX() + t * plotArea.getWidth();
        float y = yZeroDb - (float) dB / dbPerPx;
        y = juce::jlimit(plotArea.getY(), plotArea.getBottom(), y);

        if (i == 0) curve.startNewSubPath(x, y);
        else        curve.lineTo(x, y);
    }

    // Fill sotto la curva
    {
        juce::Path filled = curve;
        filled.lineTo(plotArea.getRight(), plotArea.getBottom());
        filled.lineTo(plotArea.getX(),     plotArea.getBottom());
        filled.closeSubPath();

        juce::ColourGradient fillGrad(NeomodernLookAndFeel::ACCENT_CYAN.withAlpha(0.28f),
                                       plotArea.getCentreX(), plotArea.getY(),
                                       NeomodernLookAndFeel::ACCENT_CYAN.withAlpha(0.02f),
                                       plotArea.getCentreX(), plotArea.getBottom(),
                                       false);
        g.setGradientFill(fillGrad);
        g.fillPath(filled);
    }

    g.setColour(NeomodernLookAndFeel::ACCENT_CYAN);
    g.strokePath(curve, juce::PathStrokeType(2.0f,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // ── Barra GR in basso ──
    {
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.fillRoundedRectangle(grBarArea, 2.0f);

        // GR è negativo quando comprime; mappa abs(GR) su 0..20 dB → 0..1
        float grAbs  = juce::jlimit(0.0f, 20.0f, -displayedGR);
        float grNorm = grAbs / 20.0f;
        auto  fillR  = grBarArea.withWidth(grBarArea.getWidth() * grNorm);

        g.setColour(NeomodernLookAndFeel::METER_RED.withAlpha(0.85f));
        g.fillRoundedRectangle(fillR, 2.0f);

        g.setColour(NeomodernLookAndFeel::TEXT_MUTED);
        g.setFont(juce::Font(8.5f, juce::Font::bold));
        g.drawText("GR  " + juce::String(displayedGR, 1) + " dB",
                   grBarArea.toNearestInt(),
                   juce::Justification::centredRight);
    }
}

// ═════════════════════════════════════════════════════════════════════
//   VerticalMeter
// ═════════════════════════════════════════════════════════════════════

VerticalMeter::VerticalMeter(StereoCompressorProcessor& p, Side s)
    : processor(p), side(s)
{
    startTimerHz(30);
}

VerticalMeter::~VerticalMeter() { stopTimer(); }

void VerticalMeter::timerCallback()
{
    float tL = (side == Input) ? processor.getInputLevelDB(0) : processor.getOutputLevelDB(0);
    float tR = (side == Input) ? processor.getInputLevelDB(1) : processor.getOutputLevelDB(1);

    // Fast attack / slow decay
    auto smooth = [](float& cur, float target)
    {
        if (target > cur) cur = target;
        else              cur = cur * 0.88f + target * 0.12f;
    };
    smooth(displayedL, tL);
    smooth(displayedR, tR);
    repaint();
}

void VerticalMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background scuro
    {
        juce::ColourGradient bgGrad(NeomodernLookAndFeel::PANEL_DARK.darker(0.1f),
                                     bounds.getCentreX(), bounds.getY(),
                                     NeomodernLookAndFeel::PANEL_DARK,
                                     bounds.getCentreX(), bounds.getBottom(),
                                     false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
    }

    auto inner = bounds.reduced(4.0f);

    // Etichetta IN/OUT in basso
    auto labelArea = inner.removeFromBottom(14.0f);
    g.setColour(NeomodernLookAndFeel::TEXT_MUTED);
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.drawText(side == Input ? "IN" : "OUT",
               labelArea.toNearestInt(),
               juce::Justification::centred);

    inner.removeFromBottom(2.0f);

    // Due barre L/R
    const float gap = 2.0f;
    const float barW = (inner.getWidth() - gap) * 0.5f;
    auto barL = juce::Rectangle<float>(inner.getX(),            inner.getY(), barW, inner.getHeight());
    auto barR = juce::Rectangle<float>(inner.getX() + barW + gap, inner.getY(), barW, inner.getHeight());

    auto drawBar = [&](juce::Rectangle<float> r, float dB)
    {
        // -40 → 0  bottom→top
        const float minDb = -40.0f, maxDb = 6.0f;
        float norm = juce::jmap(juce::jlimit(minDb, maxDb, dB), minDb, maxDb, 0.0f, 1.0f);
        auto fillR = r.withTop(r.getBottom() - r.getHeight() * norm);

        juce::ColourGradient grad(NeomodernLookAndFeel::METER_RED,
                                   fillR.getCentreX(), r.getY(),
                                   NeomodernLookAndFeel::METER_GREEN,
                                   fillR.getCentreX(), r.getBottom(),
                                   false);
        grad.addColour(0.65, NeomodernLookAndFeel::METER_YELLOW);
        g.setGradientFill(grad);
        g.fillRect(fillR);
    };

    drawBar(barL, displayedL);
    drawBar(barR, displayedR);

    // Tacche dB (-20, -10, -6, -3, 0)
    g.setColour(juce::Colours::white.withAlpha(0.18f));
    for (int dB : { -20, -10, -6, -3, 0 })
    {
        float norm = juce::jmap((float) dB, -40.0f, 6.0f, 0.0f, 1.0f);
        float y = inner.getBottom() - inner.getHeight() * norm;
        g.drawHorizontalLine((int) y, inner.getX(), inner.getRight());
    }
}

// ═════════════════════════════════════════════════════════════════════
//   StereoCompressorEditor
// ═════════════════════════════════════════════════════════════════════

StereoCompressorEditor::StereoCompressorEditor(StereoCompressorProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      freqDisplay(p),
      inMeter(p, VerticalMeter::Input),
      outMeter(p, VerticalMeter::Output)
{
    setLookAndFeel(&lnf);
    setSize(760, 520);

    setupKnob(hpFreq,    "hpFreq",    "HI-PASS");
    setupKnob(lpFreq,    "lpFreq",    "LO-PASS");
    setupKnob(threshold, "threshold", "THRESHOLD");
    setupKnob(attack,    "attack",    "ATTACK");
    setupKnob(release,   "release",   "RELEASE");
    setupKnob(makeup,    "makeup",    "MAKEUP");
    setupKnob(habisso,   "habisso",   "HABISSO");
    setupKnob(width,     "width",     "WIDTH");

    addAndMakeVisible(freqDisplay);
    addAndMakeVisible(inMeter);
    addAndMakeVisible(outMeter);

    // Ratio buttons (1176-style radio group)
    const char* btnLabels[] = { "4", "8", "12", "20" };
    for (int i = 0; i < 4; ++i)
    {
        ratioButtons[i].setButtonText(btnLabels[i]);
        ratioButtons[i].setClickingTogglesState(true);
        ratioButtons[i].setRadioGroupId(1001);
        addAndMakeVisible(ratioButtons[i]);

        ratioButtons[i].onClick = [this, i]
        {
            if (!ratioButtons[i].getToggleState())
                return;
            auto* param = processor.apvts.getParameter("ratioSel");
            param->beginChangeGesture();
            param->setValueNotifyingHost(param->convertTo0to1((float) i));
            param->endChangeGesture();
        };
    }
    int initialIdx = (int) processor.apvts.getRawParameterValue("ratioSel")->load();
    initialIdx = juce::jlimit(0, 3, initialIdx);
    ratioButtons[initialIdx].setToggleState(true, juce::dontSendNotification);

    ratioLabel.setText("RATIO", juce::dontSendNotification);
    ratioLabel.setFont(juce::Font(10.5f, juce::Font::bold));
    ratioLabel.setColour(juce::Label::textColourId, NeomodernLookAndFeel::TEXT_MUTED);
    ratioLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ratioLabel);

    startTimerHz(30);
}

StereoCompressorEditor::~StereoCompressorEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void StereoCompressorEditor::setupKnob(KnobGroup& g,
                                        const juce::String& paramID,
                                        const juce::String& name)
{
    g.slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    g.slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                  juce::MathConstants<float>::pi * 2.8f,
                                  true);
    g.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 16);
    addAndMakeVisible(g.slider);

    g.label.setText(name, juce::dontSendNotification);
    g.label.setFont(juce::Font(10.0f, juce::Font::bold));
    g.label.setColour(juce::Label::textColourId, NeomodernLookAndFeel::TEXT_MUTED);
    g.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(g.label);

    g.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, paramID, g.slider);
}

void StereoCompressorEditor::timerCallback()
{
    // Sync ratio buttons col parametro (per preset load / automazione)
    int currentIdx = (int) processor.apvts.getRawParameterValue("ratioSel")->load();
    currentIdx = juce::jlimit(0, 3, currentIdx);
    if (! ratioButtons[currentIdx].getToggleState())
        ratioButtons[currentIdx].setToggleState(true, juce::dontSendNotification);

    // Repaint solo se habisso è cambiato visibilmente (per il tentacolo)
    const float hab = processor.apvts.getRawParameterValue("habisso")->load() * 0.01f;
    if (std::abs(hab - lastHabissoVisual) > 0.01f)
    {
        lastHabissoVisual = hab;
        repaint(tentacleArea);
    }
}

void StereoCompressorEditor::paintTentacleIcon(juce::Graphics& g,
                                                 juce::Rectangle<int> bounds,
                                                 float intensity)
{
    if (bounds.isEmpty()) return;

    const float w  = (float) bounds.getWidth();
    const float h  = (float) bounds.getHeight();
    const float cx = (float) bounds.getCentreX();
    const float top = (float) bounds.getY() + 2.0f;

    const juce::Colour stroke = NeomodernLookAndFeel::ACCENT_CYAN
        .withAlpha(0.45f + 0.55f * intensity);
    const juce::Colour suctionFill = NeomodernLookAndFeel::ACCENT_CYAN
        .withAlpha(0.55f + 0.45f * intensity);

    // ── Curva sinuosa (corpo del tentacolo) ──
    juce::Path tentacle;
    tentacle.startNewSubPath(cx, top);
    tentacle.cubicTo(cx + w * 0.6f, top + h * 0.20f,
                     cx - w * 0.55f, top + h * 0.55f,
                     cx + w * 0.30f, top + h * 0.92f);

    g.setColour(stroke);
    g.strokePath(tentacle, juce::PathStrokeType(2.6f + intensity * 1.4f,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    // ── Ventose (cerchietti decrescenti lungo la curva) ──
    const float pathLen = tentacle.getLength();
    const float ts[] = { 0.22f, 0.45f, 0.68f, 0.88f };
    const float rs[] = { 3.8f, 3.2f, 2.6f, 2.0f };

    for (int i = 0; i < 4; ++i)
    {
        auto pt = tentacle.getPointAlongPath(ts[i] * pathLen);
        const float r = rs[i] * (0.85f + 0.15f * intensity);

        g.setColour(suctionFill);
        g.fillEllipse(pt.x - r, pt.y - r, r * 2.0f, r * 2.0f);

        g.setColour(NeomodernLookAndFeel::PANEL_LIGHT);
        const float ri = r * 0.5f;
        g.fillEllipse(pt.x - ri, pt.y - ri, ri * 2.0f, ri * 2.0f);
    }
}

void StereoCompressorEditor::paint(juce::Graphics& g)
{
    // ── Background: pannello chiaro brushed con gradiente verticale ──
    {
        juce::ColourGradient bgGrad(NeomodernLookAndFeel::PANEL_LIGHT_HI,
                                     0.0f, 0.0f,
                                     NeomodernLookAndFeel::PANEL_LIGHT.darker(0.04f),
                                     0.0f, (float) getHeight(),
                                     false);
        g.setGradientFill(bgGrad);
        g.fillRect(getLocalBounds());
    }

    // ── Header ──
    {
        g.setColour(NeomodernLookAndFeel::TEXT_DARK);
        g.setFont(juce::Font("Helvetica Neue", 19.0f, juce::Font::bold));
        g.drawText("STEREO  COMPRESSOR",
                   0, 10, getWidth(), 26,
                   juce::Justification::centred);

        g.setColour(NeomodernLookAndFeel::ACCENT_CYAN_DIM.withAlpha(0.6f));
        g.setFont(juce::Font("Helvetica Neue", 9.0f, juce::Font::plain));
        g.drawText("v1.1  ·  UTILITY PACK 01",
                   0, 36, getWidth(), 12,
                   juce::Justification::centred);

        g.setColour(NeomodernLookAndFeel::KNOB_RIM.withAlpha(0.4f));
        g.drawLine(40.0f, 54.0f, (float) getWidth() - 40.0f, 54.0f, 1.0f);
    }

    // ── Tentacolo (modulato da habisso) ──
    {
        const float hab = processor.apvts.getRawParameterValue("habisso")->load() * 0.01f;
        lastHabissoVisual = hab;
        paintTentacleIcon(g, tentacleArea, hab);
    }
}

void StereoCompressorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);
    area.removeFromTop(50); // header

    // ── Meter sinistro / destro ──
    auto leftMeterArea  = area.removeFromLeft(42);
    inMeter.setBounds(leftMeterArea);
    area.removeFromLeft(10);

    auto rightMeterArea = area.removeFromRight(42);
    outMeter.setBounds(rightMeterArea);
    area.removeFromRight(10);

    // ── Display freq response ──
    auto displayArea = area.removeFromTop(160);
    freqDisplay.setBounds(displayArea);
    area.removeFromTop(10);

    // ── Riga 1: HP / LP (sx)  |  THR / ATT / REL / MAK (dx) ──
    auto row1 = area.removeFromTop(135);

    auto filterArea = row1.removeFromLeft(180);
    {
        const int kw = 80;
        auto a1 = filterArea.removeFromLeft(kw);
        hpFreq.label .setBounds(a1.removeFromTop(16));
        hpFreq.slider.setBounds(a1);

        filterArea.removeFromLeft(10);

        auto a2 = filterArea.removeFromLeft(kw);
        lpFreq.label .setBounds(a2.removeFromTop(16));
        lpFreq.slider.setBounds(a2);
    }

    row1.removeFromLeft(20);

    KnobGroup* compGroups[] = { &threshold, &attack, &release, &makeup };
    const int compKnobW = row1.getWidth() / 4;
    for (auto* kg : compGroups)
    {
        auto a = row1.removeFromLeft(compKnobW);
        kg->label .setBounds(a.removeFromTop(16));
        kg->slider.setBounds(a);
    }

    area.removeFromTop(6);

    // ── Riga 2: RATIO (sx) | HABISSO + tentacle | WIDTH ──
    auto row2 = area.removeFromTop(110);

    auto ratioArea = row2.removeFromLeft(230);
    ratioLabel.setBounds(ratioArea.removeFromTop(18));
    ratioArea.removeFromTop(4);
    auto buttonsArea = ratioArea.removeFromTop(46);
    {
        const int bw = buttonsArea.getWidth() / 4;
        for (int i = 0; i < 4; ++i)
            ratioButtons[i].setBounds(buttonsArea.removeFromLeft(bw).reduced(3));
    }

    row2.removeFromLeft(30);

    // HABISSO: knob + tentacle a destra del knob
    auto habArea = row2.removeFromLeft(140);
    habisso.label.setBounds(habArea.removeFromTop(16));
    auto habKnobArea = habArea.removeFromLeft(95);
    habisso.slider.setBounds(habKnobArea);
    tentacleArea = habArea.reduced(2, 4); // resto = area tentacolo

    row2.removeFromLeft(30);

    auto widthArea = row2.removeFromLeft(95);
    width.label .setBounds(widthArea.removeFromTop(16));
    width.slider.setBounds(widthArea);
}

juce::AudioProcessorEditor* StereoCompressorProcessor::createEditor()
{
    return new StereoCompressorEditor(*this);
}
