#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <memory>

/**
    Minimal single-slot VST3 (and AU on macOS) effect host.

    The loaded module audio is rendered first, then passed through the hosted
    effect in processBlock. Loading / unloading happens on the message thread;
    the audio thread uses a try-lock so it never blocks and simply skips FX for
    a block if a swap is in progress.
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

    // message thread
    juce::String loadPlugin (const juce::File& file);   // "" on success, else error
    void unload();

    bool        hasPlugin() const { return loaded.load(); }
    juce::String getName()  const { return pluginName; }
    void        setBypassed (bool b) { bypassed.store (b); }
    bool        isBypassed() const   { return bypassed.load(); }

    juce::AudioPluginInstance* getInstance() { return plugin.get(); }

private:
    juce::AudioPluginFormatManager formatManager;
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    juce::SpinLock audioLock;

    double sampleRate = 44100.0;
    int    blockSize  = 512;

    std::atomic<bool> loaded   { false };
    std::atomic<bool> bypassed { false };
    juce::String pluginName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxHost)
};
