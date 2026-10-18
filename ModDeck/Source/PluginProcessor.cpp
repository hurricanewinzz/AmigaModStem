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

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "fxMix", 1 }, "FX Mix",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));

    return layout;
}

void ModDeckProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;
    interleaved.assign ((size_t) samplesPerBlock * 2, 0.0f);
    dryBuffer.setSize (2, samplesPerBlock);
    fxHost.prepare (sampleRate, samplesPerBlock);
}

void ModDeckProcessor::releaseResources()
{
    fxHost.releaseResources();
}

bool ModDeckProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

bool ModDeckProcessor::loadModFile (const juce::File& file, juce::String& error)
{
    std::string err;
    if (! engine.loadFile (file.getFullPathName().toStdString(), err))
    {
        error = err;
        return false;
    }

    // Auto BPM detect: derive musical BPM from the module and lock the grid to it.
    const int rpb = rowsPerBeat.load();
    const double musical = engine.getMusicalBpm (rpb);
    baseTempo.store (musical);
    lockedBpm.store (musical);
    bpmLocked.store (true);

    if (auto* p = apvts.getParameter ("tempoFactor"))
        p->setValueNotifyingHost (p->convertTo0to1 (1.0f));

    gridAnchorSec.store (0.0);
    loopInSec.store (-1.0);
    loopOutSec.store (-1.0);
    loopSection.store (false);

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

void ModDeckProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (! engine.isLoaded() || ! playing.load())
        return;

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
        playing.store (false);
        engine.setPositionSeconds (0.0);
        return;
    }

    const int numOut = buffer.getNumChannels();
    auto* L = buffer.getWritePointer (0);
    auto* R = numOut > 1 ? buffer.getWritePointer (1) : nullptr;

    for (std::size_t i = 0; i < rendered; ++i)
    {
        L[i] = interleaved[i * 2];
        if (R != nullptr)
            R[i] = interleaved[i * 2 + 1];
    }

    // run the hosted VST3/AU effect chain over the module audio, with a
    // master wet/dry blend against the untouched module signal.
    const double mix = (double) (apvts.getRawParameterValue ("fxMix") != nullptr
                                     ? apvts.getRawParameterValue ("fxMix")->load() : 1.0f);

    if (dryBuffer.getNumSamples() < frames)
        dryBuffer.setSize (2, frames, false, false, true);

    for (int ch = 0; ch < juce::jmin (numOut, dryBuffer.getNumChannels()); ++ch)
        dryBuffer.copyFrom (ch, 0, buffer, ch, 0, frames);

    fxHost.process (buffer, midi);

    if (mix < 0.999)
    {
        for (int ch = 0; ch < juce::jmin (numOut, dryBuffer.getNumChannels()); ++ch)
        {
            auto* wet = buffer.getWritePointer (ch);
            const auto* dry = dryBuffer.getReadPointer (ch);
            for (int i = 0; i < frames; ++i)
                wet[i] = (float) (dry[i] * (1.0 - mix) + wet[i] * mix);
        }
    }

    // section loop: jump back to the IN point once we pass OUT
    if (loopSection.load())
    {
        const double in  = loopInSec.load();
        const double out = loopOutSec.load();
        if (in >= 0.0 && out > in && engine.getPositionSeconds() >= out)
            engine.setPositionSeconds (in);
    }
}

void ModDeckProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("bpmLocked",   bpmLocked.load());
        xml->setAttribute ("lockedBpm",   lockedBpm.load());
        xml->setAttribute ("loopEnabled", loopEnabled.load());
        xml->setAttribute ("rowsPerBeat", rowsPerBeat.load());
        xml->setAttribute ("beatsPerBar", beatsPerBar.load());
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
        rowsPerBeat.store (xml->getIntAttribute     ("rowsPerBeat", 4));
        beatsPerBar.store (xml->getIntAttribute     ("beatsPerBar", 4));
    }
}

juce::AudioProcessorEditor* ModDeckProcessor::createEditor()
{
    return new ModDeckEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModDeckProcessor();
}
