#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

/**
    Scrolling timeline / beatgrid for the loaded .mod.

      - draws the whole song duration as a horizontal timeline
      - overlays a beat grid derived from the *locked BPM*
      - emphasises bar lines (every 4 beats) and sixteenth subdivisions
      - shows a live playhead
      - click / drag to seek anywhere in the song
*/
class BeatGridComponent : public juce::Component,
                          private juce::Timer
{
public:
    explicit BeatGridComponent (ModDeckProcessor& p);
    ~BeatGridComponent() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

private:
    void timerCallback() override { repaint(); }
    void seekToX (int x);

    ModDeckProcessor& proc;
};
