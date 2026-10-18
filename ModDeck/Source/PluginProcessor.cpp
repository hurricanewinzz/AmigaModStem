#include "PluginProcessor.h"
#include "PluginEditor.h"

ModDeckProcessor::ModDeckProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ModDeckProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    for (int c = 0; c < kNumMixChannels; ++c)
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { "ch" + String (c + 1), 1 },
            "Ch " + String (c + 1) + " Volume",
            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "tempoFactor", 1 }, "Tempo",
        NormalisableRange<float> (0.25f, 4.0f, 0.0001f, 0.5f), 1.0f));

    return layout;
}

void ModDeckProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    interleaved.assign ((size_t) samplesPerBlock * 2, 0.0f);
}

bool ModDeckProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

bool ModDeckProcessor::loadModFile (const juce::File& file, juce::String& error)
{
    std::string err;
    const bool ok = engine.loadFile (file.getFullPathName().toStdString(), err);
    if (! ok)
    {
        error = err;
        return false;
    }

    baseTempo.store (engine.getBaseTempo());
    lockedBpm.store (engine.getBaseTempo());
    engine.setRepeat (loopEnabled.load());
    setPlaying (false);
    engine.setPositionSeconds (0.0);
    return true;
}

void ModDeckProcessor::stopAndRewind()
{
    setPlaying (false);
    engine.setPositionSeconds (0.0);
}

void ModDeckProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (! engine.isLoaded() || ! playing.load())
        return;

    // push per-channel volumes from the (automatable) parameters
    for (int c = 0; c < kNumMixChannels; ++c)
        if (auto* p = apvts.getRawParameterValue ("ch" + juce::String (c + 1)))
            engine.setChannelVolume (c, (double) p->load());

    if (auto* t = apvts.getRawParameterValue ("tempoFactor"))
        engine.setTempoFactor ((double) t->load());

    engine.setRepeat (loopEnabled.load());

    const int frames = buffer.getNumSamples();
    if ((int) interleaved.size() < frames * 2)
        interleaved.assign ((size_t) frames * 2, 0.0f);

    const std::size_t rendered = engine.render (interleaved.data(), (std::size_t) frames, currentSampleRate);

    if (rendered == 0)
    {
        // reached the end with looping disabled -> stop & rewind
        playing.store (false);
        engine.setPositionSeconds (0.0);
        return;
    }

    const int numOut = buffer.getNumChannels();
    auto* L = buffer.getWritePointer (0);
    auto* R = numOut > 1 ? buffer.getWritePointer (1) : nullptr;

    for (std::size_t i = 0; i < rendered; ++i)
    {
        const float l = interleaved[i * 2];
        const float r = interleaved[i * 2 + 1];
        L[i] = l;
        if (R != nullptr)
            R[i] = r;
    }
}

void ModDeckProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("bpmLocked",   bpmLocked.load());
        xml->setAttribute ("lockedBpm",   lockedBpm.load());
        xml->setAttribute ("loopEnabled", loopEnabled.load());
        copyXmlToBinary (*xml, destData);
    }
}

void ModDeckProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));

        bpmLocked.store   (xml->getBoolAttribute   ("bpmLocked",   false));
        lockedBpm.store   (xml->getDoubleAttribute  ("lockedBpm",   125.0));
        loopEnabled.store (xml->getBoolAttribute    ("loopEnabled", true));
    }
}

juce::AudioProcessorEditor* ModDeckProcessor::createEditor()
{
    return new ModDeckEditor (*this);
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModDeckProcessor();
}
