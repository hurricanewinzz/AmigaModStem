#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <functional>
#include <map>
#include "../PluginProcessor.h"

/** One row in the FX chain: order #, name, per-slot bypass, GUI, up/down, remove. */
class FxSlotRow : public juce::Component
{
public:
    FxSlotRow();

    void setData (int index, const juce::String& name, bool bypassed,
                  bool canMoveUp, bool canMoveDown);

    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void (int)>       onGui, onRemove, onUp, onDown;
    std::function<void (int, bool)> onBypass;

private:
    int index = 0;
    juce::Label        idxLabel, nameLabel;
    juce::ToggleButton bypassBtn { "BYP" };
    juce::TextButton   guiBtn    { "GUI" };
    juce::TextButton   upBtn     { "UP" };
    juce::TextButton   downBtn   { "DN" };
    juce::TextButton   removeBtn { "X" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxSlotRow)
};

//==============================================================================
/**
    "FX" tab — a serial effect chain host. Add several VST3/AU effects, reorder
    them, bypass each individually and open each one's GUI. The module audio is
    routed through the chain in order.
*/
class FxPage : public juce::Component
{
public:
    explicit FxPage (ModDeckProcessor& p);
    ~FxPage() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class PluginWindow;

    void addEffect();
    void rebuildChain();
    void layoutChain();
    void openEditor (int index);
    void removeAt (int index);
    void moveSlot (int index, int delta);

    static constexpr int kRowHeight = 50;

    ModDeckProcessor& proc;

    juce::Label      headline;
    juce::Label      hint;
    juce::TextButton addButton { "+ ADD EFFECT" };

    juce::Viewport  viewport;
    juce::Component chainContainer;
    juce::OwnedArray<FxSlotRow> rows;

    std::unique_ptr<juce::FileChooser> chooser;
    std::map<juce::AudioPluginInstance*, std::unique_ptr<PluginWindow>> windows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxPage)
};
