#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "gui/AmigaLookAndFeel.h"
#include "gui/PlayerPage.h"
#include "gui/FxPage.h"

class ModDeckEditor : public juce::AudioProcessorEditor
{
public:
    explicit ModDeckEditor (ModDeckProcessor&);
    ~ModDeckEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

private:
    ModDeckProcessor& proc;
    AmigaLookAndFeel  lnf;

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    PlayerPage playerPage;
    FxPage     fxPage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModDeckEditor)
};
