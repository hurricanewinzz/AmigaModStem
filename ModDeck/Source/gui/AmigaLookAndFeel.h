#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
    CRT / neon demoscene look & feel for ModDeck.
    Near-black background, phosphor-green / cyan / magenta accents,
    monospace type and glowing controls.
*/
class AmigaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AmigaLookAndFeel();

    // palette (public so components can reuse exact colours)
    static const juce::Colour bg;
    static const juce::Colour panel;
    static const juce::Colour grid;
    static const juce::Colour neonGreen;
    static const juce::Colour neonCyan;
    static const juce::Colour neonMagenta;
    static const juce::Colour neonAmber;
    static const juce::Colour textDim;

    juce::Font getMonoFont (float height, bool bold = false) const;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
};
