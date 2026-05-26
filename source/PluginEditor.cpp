#include "PluginEditor.h"

static const juce::Colour BG     { 0xff1a1a2e };
static const juce::Colour PANEL  { 0xff16213e };
static const juce::Colour ACCENT { 0xff0f3460 };
static const juce::Colour KNOB   { 0xffe94560 };
static const juce::Colour TEXT   { 0xfff0f0f0 };
static const juce::Colour DIM    { 0xff888899 };

StereoCompressorEditor::StereoCompressorEditor(StereoCompressorProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(520, 260);

    setupKnob(threshold, "threshold", "THRESHOLD");
    setupKnob(ratio,     "ratio",     "RATIO");
    setupKnob(attack,    "attack",    "ATTACK");
    setupKnob(release,   "release",   "RELEASE");
    setupKnob(makeup,    "makeup",    "MAKEUP");
    setupKnob(width,     "width",     "WIDTH");

    grLabel.setFont(juce::Font("Courier New", 13.0f, juce::Font::plain));
    grLabel.setColour(juce::Label::textColourId, KNOB);
    grLabel.setJustificationType(juce::Justification::centred);
    grLabel.setText("GR: 0.0 dB", juce::dontSendNotification);
    addAndMakeVisible(grLabel);

    startTimerHz(30);
}

StereoCompressorEditor::~StereoCompressorEditor()
{
    stopTimer();
}

void StereoCompressorEditor::setupKnob(KnobGroup& g,
                                        const juce::String& paramID,
                                        const juce::String& name)
{
    g.slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    g.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    g.slider.setColour(juce::Slider::rotarySliderFillColourId,   KNOB);
    g.slider.setColour(juce::Slider::rotarySliderOutlineColourId, ACCENT);
    g.slider.setColour(juce::Slider::thumbColourId,               KNOB);
    g.slider.setColour(juce::Slider::textBoxTextColourId,         TEXT);
    g.slider.setColour(juce::Slider::textBoxBackgroundColourId,   PANEL);
    g.slider.setColour(juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    addAndMakeVisible(g.slider);

    g.label.setText(name, juce::dontSendNotification);
    g.label.setFont(juce::Font(10.5f, juce::Font::bold));
    g.label.setColour(juce::Label::textColourId, DIM);
    g.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(g.label);

    g.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, paramID, g.slider);
}

void StereoCompressorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BG);

    // Titolo
    g.setColour(TEXT);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("STEREO COMPRESSOR", 0, 8, getWidth(), 22, juce::Justification::centred);

    // Separatore sezioni
    g.setColour(ACCENT);
    g.drawLine(260.0f, 40.0f, 260.0f, 230.0f, 1.5f);

    g.setColour(DIM);
    g.setFont(juce::Font(9.5f, juce::Font::plain));
    g.drawText("COMPRESSOR", 10, 38, 240, 14, juce::Justification::centred);
    g.drawText("STEREO WIDENER", 270, 38, 240, 14, juce::Justification::centred);
}

void StereoCompressorEditor::resized()
{
    const int knobW = 80;
    const int knobH = 90;
    const int labelH = 16;
    const int startY = 55;

    // Sezione compressore (4 knob in 2x2)
    auto placeKnob = [&](KnobGroup& g, int x, int y) {
        g.label.setBounds(x, y, knobW, labelH);
        g.slider.setBounds(x, y + labelH, knobW, knobH);
    };

    // Row 1: threshold | ratio
    placeKnob(threshold,  20, startY);
    placeKnob(ratio,     110, startY);

    // Row 2: attack | release
    placeKnob(attack,   20, startY + knobH + labelH + 8);
    placeKnob(release, 110, startY + knobH + labelH + 8);

    // Makeup in centro-basso sezione compressore
    placeKnob(makeup, 65, startY + (knobH + labelH + 8) / 2);

    // Sezione width (centrata nella metà destra)
    placeKnob(width, 330, startY + (knobH / 2));

    // GR meter in basso
    grLabel.setBounds(0, getHeight() - 26, getWidth(), 20);
}

void StereoCompressorEditor::timerCallback()
{
    const float gr = processor.getGainReductionDB();
    // Smooth display
    displayedGR = 0.8f * displayedGR + 0.2f * gr;

    grLabel.setText("GR  " + juce::String(displayedGR, 1) + " dB",
                    juce::dontSendNotification);
}

juce::AudioProcessorEditor* StereoCompressorProcessor::createEditor()
{
    return new StereoCompressorEditor(*this);
}
