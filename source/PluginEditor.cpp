#include "PluginEditor.h"
#include "BinaryData.h"

// ── Geometria base nello spazio dello sfondo (848 x 1234 px, barra preset rimossa) ──
namespace nemo
{
    constexpr float BG_W = 848.0f, BG_H = 1234.0f;
    constexpr float SCALE = 0.60f;                 // finestra = 509 x 740

    constexpr int   KNOB_CX = 258, KNOB_D = 74;    // knob -40% (era 124)
    const int       ROWS[8] = { 157, 275, 416, 517, 617, 718, 860, 981 };

    constexpr int   BOX_X = 333, BOX_W = 80, BOX_H = 30;

    constexpr int   MG_X0 = 589, MG_Y0 = 102, MG_X1 = 694, MG_Y1 = 918;

    constexpr int   INF_CX = 68, OUTF_CX = 778;
    constexpr int   FTRAVEL_TOP = 132, FTRAVEL_BOT = 1116, FCAP_W = 60;
    constexpr float FADER_ASPECT = 620.0f / 318.0f;

    constexpr int   PH_CX = 242, PH_CY = 1074, PH_SZ = 58; // switch PHASE (-40%)

    // Titolo NEMO (disegnato da codice con font artistico)
    const juce::String TITLE_FONT = "Snell Roundhand";

    const juce::Colour LED_COLOUR  { 0xff39e0a6 };  // verde-ciano accensione box
    const juce::Colour HABISS_RED  { 0xffe8342a };  // HABISS più saturato/rosso salendo
    const juce::Colour PARA_AZURE  { 0xff40b4ff };  // PARALARVA azzurra salendo

    inline juce::Colour lerpCol(juce::Colour a, juce::Colour b, float t)
    {
        return a.interpolatedWith(b, juce::jlimit(0.0f, 1.0f, t));
    }

    inline int S(float v) { return juce::roundToInt(v * SCALE); }
}

// ═════════════════════════════════════════════════════════════════════
//   MeterDisplay
// ═════════════════════════════════════════════════════════════════════
MeterDisplay::MeterDisplay(StereoCompressorProcessor& p) : processor(p) { startTimerHz(30); }
MeterDisplay::~MeterDisplay() { stopTimer(); }

void MeterDisplay::timerCallback()
{
    const float out = juce::jmax(processor.getOutputLevelDB(0), processor.getOutputLevelDB(1));
    const float gr  = processor.getGainReductionDB();
    if (out > displayedOut) displayedOut = out;
    else                    displayedOut = displayedOut * 0.88f + out * 0.12f;
    if (gr < displayedGR)   displayedGR = gr;
    else                    displayedGR = displayedGR * 0.85f + gr * 0.15f;
    repaint();
}

void MeterDisplay::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float maxDb = 3.0f, minDb = -60.0f;
    auto dbToY = [&](float db)
    {
        float t = juce::jmap(juce::jlimit(minDb, maxDb, db), minDb, maxDb, 0.0f, 1.0f);
        return b.getBottom() - t * b.getHeight();
    };

    const float gap  = b.getWidth() * 0.10f;
    const float colW = (b.getWidth() - gap) * 0.5f;
    auto outCol = juce::Rectangle<float>(b.getX(),              b.getY(), colW, b.getHeight());
    auto grCol  = juce::Rectangle<float>(b.getX() + colW + gap, b.getY(), colW, b.getHeight());

    // colori più stretti del 40% (centrati nella colonna)
    auto narrow = [](juce::Rectangle<float> c)
    {
        return c.withSizeKeepingCentre(c.getWidth() * 0.6f, c.getHeight());
    };

    // OUT (dal basso verso l'alto)
    {
        auto fillR = narrow(outCol).withTop(dbToY(displayedOut));
        juce::ColourGradient grad(NemoLookAndFeel::METER_RED,   fillR.getCentreX(), outCol.getY(),
                                  NemoLookAndFeel::METER_GREEN, fillR.getCentreX(), outCol.getBottom(), false);
        grad.addColour(0.62, NemoLookAndFeel::METER_YELLOW);
        g.setGradientFill(grad);
        g.fillRect(fillR);
    }
    // GR (dall'alto verso il basso)
    {
        float grAbs = juce::jlimit(0.0f, 24.0f, -displayedGR);
        float y1 = juce::jmap(grAbs, 0.0f, 24.0f, grCol.getY(), grCol.getBottom());
        g.setColour(NemoLookAndFeel::METER_RED.withAlpha(0.9f));
        g.fillRect(narrow(grCol).withBottom(y1));
    }

    // Segmentazione LED
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    const int segs = 46;
    for (int i = 1; i < segs; ++i)
        g.fillRect(b.getX(), b.getY() + b.getHeight() * (float) i / (float) segs, b.getWidth(), 1.0f);
}

// ═════════════════════════════════════════════════════════════════════
//   PhaseToggle
// ═════════════════════════════════════════════════════════════════════
PhaseToggle::PhaseToggle() : juce::Button("phase")
{
    onImg  = juce::ImageCache::getFromMemory(BinaryData::nemo_phase_on_png,  BinaryData::nemo_phase_on_pngSize);
    offImg = juce::ImageCache::getFromMemory(BinaryData::nemo_phase_off_png, BinaryData::nemo_phase_off_pngSize);
    setClickingTogglesState(true);
}

void PhaseToggle::paintButton(juce::Graphics& g, bool, bool)
{
    const auto& img = getToggleState() ? onImg : offImg;
    if (img.isValid())
    {
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.drawImageWithin(img, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::centred);
    }
}

// ═════════════════════════════════════════════════════════════════════
//   StereoCompressorEditor
// ═════════════════════════════════════════════════════════════════════
StereoCompressorEditor::StereoCompressorEditor(StereoCompressorProcessor& p)
    : AudioProcessorEditor(&p), processor(p), meterDisplay(p)
{
    setLookAndFeel(&lnf);
    background = juce::ImageCache::getFromMemory(BinaryData::nemo_background_jpg,
                                                 BinaryData::nemo_background_jpgSize);

    setupKnob(hpFreq,    "hpFreq");
    setupKnob(lpFreq,    "lpFreq");
    setupKnob(threshold, "threshold");
    setupKnob(attack,    "attack");
    setupKnob(release,   "release");
    setupKnob(makeup,    "makeup");
    setupKnob(habiss,    "habisso");
    setupKnob(paralarva, "width");

    auto setupFader = [this](juce::Slider& s,
                             std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& att,
                             const juce::String& id)
    {
        s.setSliderStyle(juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setPopupDisplayEnabled(true, true, this);
        addAndMakeVisible(s);
        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.apvts, id, s);
    };
    setupFader(inFader,  inAtt,  "inputGain");
    setupFader(outFader, outAtt, "outputGain");

    addAndMakeVisible(phaseButton);
    phaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "phase", phaseButton);

    addAndMakeVisible(meterDisplay);

    for (auto& c : ledColour) c = nemo::LED_COLOUR;

    setSize(nemo::S(nemo::BG_W), nemo::S(nemo::BG_H));
    startTimerHz(30);
}

StereoCompressorEditor::~StereoCompressorEditor() { setLookAndFeel(nullptr); stopTimer(); }

void StereoCompressorEditor::setupKnob(KnobGroup& g, const juce::String& paramID)
{
    g.slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    g.slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                  juce::MathConstants<float>::pi * 2.8f, true);
    g.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    g.slider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(g.slider);
    g.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, paramID, g.slider);
}

void StereoCompressorEditor::timerCallback()
{
    using namespace nemo;
    auto raw = [this](const char* id) { return processor.apvts.getRawParameterValue(id)->load(); };

    const float habiss = raw("habisso");   // 0..100
    const float width  = raw("width");     // 0..2

    float tgt[8];
    tgt[0] = raw("hpFreq") > 20.5f      ? 1.0f : 0.0f;                       // HI-PASS attivo
    tgt[1] = raw("lpFreq") < 19950.0f   ? 1.0f : 0.0f;                       // LO-PASS attivo
    tgt[2] = juce::jlimit(0.0f, 1.0f, -processor.getGainReductionDB() / 4.0f); // THRESHOLD (compressione)
    tgt[3] = processor.getAttackActivity()  > 0.02f ? 1.0f : 0.0f;          // ATTACK
    tgt[4] = processor.getReleaseActivity() > 0.02f ? 1.0f : 0.0f;          // RELEASE
    tgt[5] = raw("makeup") > 0.05f      ? 1.0f : 0.0f;                       // MAKEUP
    tgt[6] = juce::jlimit(0.0f, 1.0f, habiss / 30.0f);                       // HABISS
    tgt[7] = juce::jlimit(0.0f, 1.0f, width  / 2.0f);                        // PARALARVA

    // Colori reattivi: HABISS scurisce salendo, PARALARVA diventa più azzurra
    ledColour[6] = lerpCol(LED_COLOUR, HABISS_RED, habiss / 100.0f);
    ledColour[7] = lerpCol(LED_COLOUR, PARA_AZURE,  width  / 2.0f);

    for (int i = 0; i < 8; ++i)
        ledLevel[i] += (tgt[i] - ledLevel[i]) * 0.3f;

    repaint(S(BOX_X) - 4, S((float)ROWS[0]) - S(BOX_H),
            S(BOX_W) + 8, S((float)ROWS[7]) - S((float)ROWS[0]) + 2 * S(BOX_H));
}

void StereoCompressorEditor::drawActivityLed(juce::Graphics& g, juce::Rectangle<int> r, float lvl, juce::Colour c)
{
    if (lvl <= 0.01f) return;
    auto rf = r.toFloat().reduced(2.0f);
    g.setColour(c.withAlpha(0.22f * lvl)); g.fillRoundedRectangle(rf.expanded(3.0f), 5.0f); // glow
    g.setColour(c.withAlpha(0.85f * lvl)); g.fillRoundedRectangle(rf, 3.0f);
    g.setColour(c.brighter(0.7f).withAlpha(0.5f * lvl));
    g.fillRoundedRectangle(rf.reduced(rf.getWidth() * 0.30f, rf.getHeight() * 0.30f), 2.0f);   // core
}

void StereoCompressorEditor::paint(juce::Graphics& g)
{
    using namespace nemo;
    if (background.isValid())
        g.drawImage(background, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
    else
        g.fillAll(juce::Colour(0xff9aa0a4));

    // ── Titolo NEMO (font artistico), sopra la linea divisoria ──
    {
        juce::Rectangle<int> titleArea(0, S(2), getWidth(), S(50));
        g.setFont(juce::Font(TITLE_FONT, (float) S(46), juce::Font::bold));
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawText("Nemo", titleArea.translated(0, 1), juce::Justification::centred);   // emboss
        g.setColour(juce::Colour(0xff2c2f33));
        g.drawText("Nemo", titleArea, juce::Justification::centred);
    }

    // ── Etichetta PHASE (la serigrafia è stata rimossa dallo sfondo) ──
    {
        g.setColour(juce::Colour(0xff3a3d40));
        g.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), (float) S(20), juce::Font::bold));
        g.drawText("PHASE", S(PH_CX) - S(60), S(PH_CY) + S(PH_SZ) / 2 - S(4),
                   S(120), S(24), juce::Justification::centred);
    }

    // ── Box che si accendono quando lo stage è attivo ──
    for (int i = 0; i < 8; ++i)
    {
        juce::Rectangle<int> box(S(BOX_X), S((float)ROWS[i]) - S(BOX_H) / 2, S(BOX_W), S(BOX_H));
        drawActivityLed(g, box, ledLevel[i], ledColour[i]);
    }
}

void StereoCompressorEditor::resized()
{
    using namespace nemo;

    KnobGroup* knobs[8] = { &hpFreq,&lpFreq,&threshold,&attack,&release,&makeup,&habiss,&paralarva };
    const int d = S(KNOB_D);
    for (int i = 0; i < 8; ++i)
        knobs[i]->slider.setBounds(S(KNOB_CX) - d / 2, S((float)ROWS[i]) - d / 2, d, d);

    const int fw = S(FCAP_W);
    const int thumbR = (int) ((float) fw * FADER_ASPECT * 0.5f);
    const int fTop = S(FTRAVEL_TOP) - thumbR;
    const int fH   = S(FTRAVEL_BOT) - S(FTRAVEL_TOP) + 2 * thumbR;
    inFader .setBounds(S(INF_CX)  - fw / 2, fTop, fw, fH);
    outFader.setBounds(S(OUTF_CX) - fw / 2, fTop, fw, fH);

    const int ps = S(PH_SZ);
    phaseButton.setBounds(S(PH_CX) - ps / 2, S(PH_CY) - ps / 2, ps, ps);

    meterDisplay.setBounds(S(MG_X0), S(MG_Y0), S(MG_X1) - S(MG_X0), S(MG_Y1) - S(MG_Y0));
}

juce::AudioProcessorEditor* StereoCompressorProcessor::createEditor()
{
    return new StereoCompressorEditor(*this);
}
