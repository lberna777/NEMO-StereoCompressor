#include "PluginProcessor.h"
#include "PluginEditor.h"

StereoCompressorProcessor::StereoCompressorProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout
StereoCompressorProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "threshold", "Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -12.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ratio", "Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f), 4.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack", juce::NormalisableRange<float>(0.1f, 200.0f, 0.1f, 0.5f), 10.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "release", "Release", juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.5f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "makeup", "Makeup Gain", juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "width", "Stereo Width", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.3f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    return { params.begin(), params.end() };
}

void StereoCompressorProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    envDB = 0.0f;

    // Pre-calcola i coefficienti (verranno ricalcolati nel processBlock se cambiano)
    auto attackMs  = apvts.getRawParameterValue("attack")->load();
    auto releaseMs = apvts.getRawParameterValue("release")->load();
    attackCoeff  = std::exp(-1.0f / (float(sampleRate) * attackMs  * 0.001f));
    releaseCoeff = std::exp(-1.0f / (float(sampleRate) * releaseMs * 0.001f));
}

void StereoCompressorProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() < 2) return;

    const int numSamples = buffer.getNumSamples();
    float* left  = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    // Leggi parametri
    const float threshold = apvts.getRawParameterValue("threshold")->load();
    const float ratio     = apvts.getRawParameterValue("ratio")->load();
    const float attackMs  = apvts.getRawParameterValue("attack")->load();
    const float releaseMs = apvts.getRawParameterValue("release")->load();
    const float makeup    = apvts.getRawParameterValue("makeup")->load();
    const float width     = apvts.getRawParameterValue("width")->load();

    // Ricalcola coefficienti
    attackCoeff  = std::exp(-1.0f / (float(currentSampleRate) * attackMs  * 0.001f));
    releaseCoeff = std::exp(-1.0f / (float(currentSampleRate) * releaseMs * 0.001f));

    float grAccum = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        // ── 1. COMPRESSORE ──────────────────────────────────────────────────

        // Detector: peak stereo (massimo tra L e R)
        float peak = std::max(std::abs(left[i]), std::abs(right[i]));
        float peakDB = (peak > 1e-6f) ? juce::Decibels::gainToDecibels(peak) : -120.0f;

        // Gain computer
        float gainDB = 0.0f;
        if (peakDB > threshold)
            gainDB = (threshold - peakDB) * (1.0f - 1.0f / ratio);

        // Envelope follower (attack/release separati)
        if (gainDB < envDB)
            envDB = attackCoeff  * envDB + (1.0f - attackCoeff)  * gainDB;
        else
            envDB = releaseCoeff * envDB + (1.0f - releaseCoeff) * gainDB;

        float gain = juce::Decibels::decibelsToGain(envDB + makeup);
        left[i]  *= gain;
        right[i] *= gain;

        grAccum += envDB;

        // ── 2. STEREO WIDENER (in serie dopo il compressore) ────────────────
        //    Codifica M/S → scala il Side → ridecoda
        float mid  = (left[i] + right[i]) * 0.5f;
        float side = (left[i] - right[i]) * 0.5f;

        side *= width;

        left[i]  = mid + side;
        right[i] = mid - side;
    }

    currentGR.store(grAccum / float(numSamples));
}

void StereoCompressorProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void StereoCompressorProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoCompressorProcessor();
}
