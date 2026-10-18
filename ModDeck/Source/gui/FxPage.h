#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

/**
    "FX" tab.

    Per the current spec, effects / native-VST hosting is intentionally
    deferred - this page is a styled placeholder with the integration point
    already reserved. Because ModDeck is a native JUCE app, real VST3 effect
    hosting (JUCE AudioPluginFormatManager) can be wired in here later.
*/
class FxPage : public juce::Component
{
public:
    explicit FxPage (ModDeckProcessor& p);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ModDeckProcessor& proc;
    juce::Label headline;
    juce::Label body;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxPage)
};
