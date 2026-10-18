#include "FxHost.h"

FxHost::FxHost()
{
    formatManager.addDefaultFormats();   // VST3 (+ AU on macOS) when host defines are set
}

FxHost::~FxHost()
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    slots.clear();
}

void FxHost::prepare (double sr, int block)
{
    sampleRate = sr;
    blockSize  = block;

    const juce::SpinLock::ScopedLockType sl (audioLock);
    for (auto& s : slots)
        if (s->plugin != nullptr)
        {
            s->plugin->setPlayConfigDetails (2, 2, sampleRate, blockSize);
            s->plugin->prepareToPlay (sampleRate, blockSize);
        }
}

void FxHost::releaseResources()
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    for (auto& s : slots)
        if (s->plugin != nullptr)
            s->plugin->releaseResources();
}

void FxHost::process (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    const juce::SpinLock::ScopedTryLockType sl (audioLock);
    if (! sl.isLocked())
        return;

    for (auto& s : slots)
        if (s->plugin != nullptr && ! s->bypassed.load())
            s->plugin->processBlock (buffer, midi);
}

juce::String FxHost::addPlugin (const juce::File& file)
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

    auto slot = std::make_unique<Slot>();
    slot->plugin = std::move (instance);
    slot->name   = found[0]->name;

    const juce::SpinLock::ScopedLockType sl (audioLock);
    slots.push_back (std::move (slot));
    return {};
}

void FxHost::removeSlot (int index)
{
    std::unique_ptr<Slot> old;
    {
        const juce::SpinLock::ScopedLockType sl (audioLock);
        if (index < 0 || index >= (int) slots.size())
            return;
        old = std::move (slots[(size_t) index]);
        slots.erase (slots.begin() + index);
    }
    if (old && old->plugin)
        old->plugin->releaseResources();
    // 'old' destroyed here, outside the audio lock
}

void FxHost::moveSlot (int from, int to)
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    const int n = (int) slots.size();
    if (from < 0 || from >= n || to < 0 || to >= n || from == to)
        return;

    auto p = std::move (slots[(size_t) from]);
    slots.erase (slots.begin() + from);
    slots.insert (slots.begin() + to, std::move (p));
}

void FxHost::setSlotBypass (int index, bool bypassed)
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    if (index >= 0 && index < (int) slots.size())
        slots[(size_t) index]->bypassed.store (bypassed);
}

int FxHost::getNumSlots() const
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    return (int) slots.size();
}

juce::String FxHost::getSlotName (int index) const
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    if (index >= 0 && index < (int) slots.size())
        return slots[(size_t) index]->name;
    return {};
}

bool FxHost::isSlotBypassed (int index) const
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    if (index >= 0 && index < (int) slots.size())
        return slots[(size_t) index]->bypassed.load();
    return false;
}

juce::AudioPluginInstance* FxHost::getInstance (int index)
{
    const juce::SpinLock::ScopedLockType sl (audioLock);
    if (index >= 0 && index < (int) slots.size())
        return slots[(size_t) index]->plugin.get();
    return nullptr;
}
