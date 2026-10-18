#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <memory>
#include <vector>

/**
    Serial VST3 (+ AU on macOS) effect chain host.

    Holds an ordered list of effect slots. The module audio is rendered first,
    then passed through each non-bypassed slot in order. Add / remove / reorder
    / bypass happen on the message thread; the audio thread uses a try-lock so
    it never blocks and simply skips the chain for a block while it is edited.
*/
class FxHost
{
public:
    FxHost();
    ~FxHost();

    void prepare (double sampleRate, int blockSize);
    void releaseResources();

    // audio thread
    void process (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);

    // message thread ------------------------------------------------------
    juce::String addPlugin (const juce::File& file);   // "" on success, else error
    void removeSlot (int index);
    void moveSlot (int from, int to);
    void setSlotBypass (int index, bool bypassed);

    int          getNumSlots() const;
    juce::String getSlotName (int index) const;
    bool         isSlotBypassed (int index) const;
    juce::AudioPluginInstance* getInstance (int index);

private:
    struct Slot
    {
        std::unique_ptr<juce::AudioPluginInstance> plugin;
        juce::String name;
        std::atomic<bool> bypassed { false };
    };

    juce::AudioPluginFormatManager formatManager;
    std::vector<std::unique_ptr<Slot>> slots;
    mutable juce::SpinLock audioLock;

    double sampleRate = 44100.0;
    int    blockSize  = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxHost)
};
