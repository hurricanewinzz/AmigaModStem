#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/** Glowing vertical VU meter with smoothed decay and a peak cap. */
class VuMeter : public juce::Component
{
public:
    explicit VuMeter (juce::Colour c) : colour (c) {}

    void setLevel (float newLevel)
    {
        newLevel = juce::jlimit (0.0f, 1.0f, newLevel);
        level = juce::jmax (newLevel, level * 0.80f);          // fast attack, slow decay
        peak  = (newLevel >= peak) ? newLevel : juce::jmax (newLevel, peak - 0.015f);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff12101f));
        g.fillRoundedRectangle (r, 2.0f);
        g.setColour (juce::Colour (0xff2a2740));
        g.drawRoundedRectangle (r.reduced (0.5f), 2.0f, 1.0f);

        auto fillH = r.getHeight() * level;
        juce::Rectangle<float> bar (r.getX(), r.getBottom() - fillH, r.getWidth(), fillH);
        g.setColour (colour.withAlpha (0.25f));
        g.fillRect (bar.expanded (1.5f, 0.0f));
        g.setColour (colour);
        g.fillRoundedRectangle (bar.reduced (1.0f, 0.0f), 1.5f);

        const float py = r.getBottom() - r.getHeight() * peak;
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.fillRect (r.getX(), py - 1.0f, r.getWidth(), 2.0f);
    }

private:
    juce::Colour colour;
    float level = 0.0f, peak = 0.0f;
};
