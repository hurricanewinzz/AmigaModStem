#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <functional>
#include "../PluginProcessor.h"

/**
    "FX" tab — a single-slot external effect host. Load a VST3 (or AU on macOS),
    open its GUI, bypass it or remove it. The module audio is routed through it
    in the processor's processBlock.
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

    void loadPlugin();
    void showEditor();
    void removePlugin();
    void refresh();

    ModDeckProcessor& proc;

    juce::Label        headline;
    juce::Label        nameLabel;
    juce::Label        hint;
    juce::TextButton   loadButton   { "LOAD VST3 / AU" };
    juce::TextButton   showButton   { "SHOW GUI" };
    juce::TextButton   removeButton  { "REMOVE" };
    juce::ToggleButton bypassToggle { "BYPASS" };

    std::unique_ptr<juce::FileChooser> chooser;
    std::unique_ptr<PluginWindow>      pluginWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxPage)
};
