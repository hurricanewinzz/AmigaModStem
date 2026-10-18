#include "FxHost.h"

FxHost::FxHost()
{
    formatManager.addDefaultFormats();   // VST3 (+ AU on macOS) when host defines are set
}

FxHost::~FxHost()
{
    unload();
}

void FxHost::prepare (double sr, int block)
{
    sampleRate = sr;
    blockSize  = block;

    const juce::SpinLock::ScopedLockType sl (audioLock);
    if (plugin != nullptr)
    {
        plugin->setPlayConfigDetails (2, 2, sampleRate, blockSize);
        plugin->prepareToPlay (sampleRate, blockSize);
    }
}

void FxHost::releaseResources()
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    if (plugin != nullptr)
        plugin->releaseResources();
}

void FxHost::process (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    if (bypassed.load() || ! loaded.load())
        return;

    const juce::SpinLock::ScopedTryLockType sl (audioLock);
    if (! sl.isLocked() || plugin == nullptr)
        return;

    plugin->processBlock (buffer, midi);
}

juce::String FxHost::loadPlugin (const juce::File& file)
{
    juce::OwnedArray<juce::PluginDescription> found;

    for (auto* fmt : formatManager.getFormats())
        fmt->findAllTypesForFile (found, file.getFullPathName());

    if (found.isEmpty())
        return "No VST3/AU plugin type found in:\n" + file.getFullPathName();

    juce::String error;
    auto instance = formatManager.createPluginInstance (*found[0], sampleRate, blockSize, error);

    if (instance == nullptr)
        return error.isNotEmpty() ? error : juce::String ("Could not create plugin instance.");

    instance->setPlayConfigDetails (2, 2, sampleRate, blockSize);
    instance->prepareToPlay (sampleRate, blockSize);

    const auto newName = found[0]->name;

    {
        const juce::SpinLock::ScopedLockType sl (audioLock);
        plugin = std::move (instance);
    }

    pluginName = newName;
    loaded.store (true);
    return {};
}

void FxHost::unload()
{
    std::unique_ptr<juce::AudioPluginInstance> old;
    {
        const juce::SpinLock::ScopedLockType sl (audioLock);
        loaded.store (false);
        old = std::move (plugin);
    }
    if (old != nullptr)
        old->releaseResources();
    pluginName.clear();
    // 'old' destroyed here, outside the audio lock
}
