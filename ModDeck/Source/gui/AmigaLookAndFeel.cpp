#include "AmigaLookAndFeel.h"

const juce::Colour AmigaLookAndFeel::bg          { 0xff07060d };
const juce::Colour AmigaLookAndFeel::panel       { 0xff12101f };
const juce::Colour AmigaLookAndFeel::grid        { 0xff2a2740 };
const juce::Colour AmigaLookAndFeel::neonGreen   { 0xff39ff14 };
const juce::Colour AmigaLookAndFeel::neonCyan    { 0xff00f0ff };
const juce::Colour AmigaLookAndFeel::neonMagenta { 0xffff2fb9 };
const juce::Colour AmigaLookAndFeel::neonAmber   { 0xffffb000 };
const juce::Colour AmigaLookAndFeel::textDim     { 0xff7a86a8 };

AmigaLookAndFeel::AmigaLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, bg);
    setColour (juce::Slider::backgroundColourId, grid);
    setColour (juce::Slider::trackColourId, neonCyan);
    setColour (juce::Slider::thumbColourId, neonGreen);
    setColour (juce::Label::textColourId, neonGreen);
    setColour (juce::TextButton::buttonColourId, panel);
    setColour (juce::TextButton::textColourOnId, neonGreen);
    setColour (juce::TextButton::textColourOffId, neonGreen);
    setColour (juce::ComboBox::backgroundColourId, panel);
    setColour (juce::ComboBox::textColourId, neonGreen);
    setColour (juce::ComboBox::outlineColourId, grid);
    setColour (juce::PopupMenu::backgroundColourId, panel);
    setColour (juce::PopupMenu::textColourId, neonGreen);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, neonMagenta.withAlpha (0.35f));
}

juce::Font AmigaLookAndFeel::getMonoFont (float height, bool bold) const
{
    auto opts = juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height,
                                   bold ? juce::Font::bold : juce::Font::plain);
    return juce::Font (opts);
}

void AmigaLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float, float,
                                         juce::Slider::SliderStyle style, juce::Slider& s)
{
    const bool vertical = (style == juce::Slider::LinearVertical);

    if (vertical)
    {
        const float cx = x + width * 0.5f;
        const float trackW = 6.0f;
        juce::Rectangle<float> track (cx - trackW * 0.5f, (float) y, trackW, (float) height);

        g.setColour (grid);
        g.fillRoundedRectangle (track, 3.0f);

        // tick marks
        g.setColour (grid.brighter (0.2f));
        for (int i = 0; i <= 10; ++i)
        {
            const float ty = juce::jmap ((float) i, 0.0f, 10.0f, (float) (y + height), (float) y);
            g.drawHorizontalLine ((int) ty, cx - 10.0f, cx + 10.0f);
        }

        // filled portion
        juce::Rectangle<float> fill (cx - trackW * 0.5f, sliderPos, trackW, (float) (y + height) - sliderPos);
        g.setColour (neonCyan.withAlpha (0.85f));
        g.fillRoundedRectangle (fill, 3.0f);

        // glowing thumb
        juce::Rectangle<float> thumb (cx - 13.0f, sliderPos - 7.0f, 26.0f, 14.0f);
        g.setColour (neonGreen.withAlpha (0.25f));
        g.fillRoundedRectangle (thumb.expanded (4.0f), 5.0f);
        g.setColour (neonGreen);
        g.fillRoundedRectangle (thumb, 4.0f);
        g.setColour (bg);
        g.drawHorizontalLine ((int) sliderPos, thumb.getX() + 4.0f, thumb.getRight() - 4.0f);
    }
    else
    {
        juce::Rectangle<float> track ((float) x, y + height * 0.5f - 3.0f, (float) width, 6.0f);
        g.setColour (grid);
        g.fillRoundedRectangle (track, 3.0f);

        juce::Rectangle<float> fill ((float) x, track.getY(), sliderPos - x, 6.0f);
        g.setColour (neonCyan.withAlpha (0.85f));
        g.fillRoundedRectangle (fill, 3.0f);

        g.setColour (neonGreen);
        g.fillRoundedRectangle (sliderPos - 7.0f, y + height * 0.5f - 9.0f, 14.0f, 18.0f, 4.0f);
    }

    juce::ignoreUnused (s);
}

void AmigaLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                             bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    auto accent = neonGreen;
    if (auto* prop = b.getProperties().getVarPointer ("accent"))
        accent = juce::Colour ((juce::uint32) (int64_t) *prop);

    g.setColour (down ? accent.withAlpha (0.35f)
                      : (highlighted ? panel.brighter (0.25f) : panel));
    g.fillRoundedRectangle (r, 4.0f);

    g.setColour (accent.withAlpha (highlighted ? 1.0f : 0.7f));
    g.drawRoundedRectangle (r, 4.0f, 1.4f);
}

juce::Font AmigaLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return getMonoFont (juce::jmin (15.0f, buttonHeight * 0.5f), true);
}

void AmigaLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b,
                                         bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = b.getToggleState();
    auto accent = on ? neonMagenta : grid;

    g.setColour (on ? neonMagenta.withAlpha (0.28f) : panel);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (accent.withAlpha (highlighted ? 1.0f : 0.8f));
    g.drawRoundedRectangle (r, 4.0f, 1.4f);

    g.setColour (on ? neonMagenta : textDim);
    g.setFont (getMonoFont (13.0f, true));
    g.drawText (b.getButtonText(), r, juce::Justification::centred);
}

void AmigaLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox& box)
{
    juce::Rectangle<float> r (0, 0, (float) width, (float) height);
    g.setColour (panel);
    g.fillRoundedRectangle (r.reduced (1.0f), 4.0f);
    g.setColour (neonCyan.withAlpha (0.8f));
    g.drawRoundedRectangle (r.reduced (1.0f), 4.0f, 1.2f);

    juce::Path tri;
    const float cx = width - 14.0f, cy = height * 0.5f;
    tri.addTriangle (cx - 5, cy - 3, cx + 5, cy - 3, cx, cy + 4);
    g.setColour (neonGreen);
    g.fillPath (tri);
    juce::ignoreUnused (box);
}

void AmigaLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 0, box.getWidth() - 28, box.getHeight());
    label.setFont (getMonoFont (14.0f, true));
    label.setColour (juce::Label::textColourId, neonGreen);
}
