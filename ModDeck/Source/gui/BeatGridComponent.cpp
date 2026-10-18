#include "BeatGridComponent.h"
#include "AmigaLookAndFeel.h"

BeatGridComponent::BeatGridComponent (ModDeckProcessor& p) : proc (p)
{
    startTimerHz (30);
}

BeatGridComponent::~BeatGridComponent()
{
    stopTimer();
}

void BeatGridComponent::paint (juce::Graphics& g)
{
    using LF = AmigaLookAndFeel;
    auto r = getLocalBounds().toFloat();

    g.setColour (LF::panel);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (LF::grid);
    g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);

    const double duration = proc.getEngine().getDurationSeconds();
    const double position = proc.getEngine().getPositionSeconds();
    const double bpm      = juce::jmax (1.0, proc.lockedBpm.load());

    auto inner = r.reduced (2.0f);
    const float W = inner.getWidth();
    const float H = inner.getHeight();
    const float x0 = inner.getX();
    const float y0 = inner.getY();

    if (duration <= 0.0)
    {
        g.setColour (LF::textDim);
        g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::plain)));
        g.drawText ("LOAD A .MOD TO SEE THE BEATGRID", inner, juce::Justification::centred);
        return;
    }

    const double beatDur = 60.0 / bpm;         // seconds per beat (quarter note)
    const double secPerPx = duration / (double) W;

    // sixteenth subdivisions (faint)
    const double sixteenth = beatDur / 4.0;
    g.setColour (LF::grid.withAlpha (0.5f));
    for (double t = 0.0; t <= duration; t += sixteenth)
    {
        const float x = x0 + (float) (t / secPerPx);
        g.drawVerticalLine ((int) x, y0 + H * 0.55f, y0 + H);
    }

    // beat lines
    int beatIndex = 0;
    for (double t = 0.0; t <= duration; t += beatDur, ++beatIndex)
    {
        const float x = x0 + (float) (t / secPerPx);
        const bool bar = (beatIndex % 4) == 0;
        g.setColour (bar ? LF::neonCyan.withAlpha (0.75f) : LF::grid.brighter (0.4f));
        g.drawVerticalLine ((int) x, y0, y0 + H);

        if (bar)
        {
            g.setColour (LF::neonCyan.withAlpha (0.85f));
            g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain)));
            g.drawText (juce::String (beatIndex / 4 + 1),
                        (int) x + 2, (int) y0 + 1, 24, 12, juce::Justification::left);
        }
    }

    // elapsed shade
    const float px = x0 + (float) (position / secPerPx);
    g.setColour (LF::neonGreen.withAlpha (0.10f));
    g.fillRect (juce::Rectangle<float> (x0, y0, px - x0, H));

    // playhead
    g.setColour (LF::neonGreen);
    g.fillRect (juce::Rectangle<float> (px - 1.0f, y0, 2.0f, H));
    g.setColour (LF::neonGreen.withAlpha (0.3f));
    g.fillRect (juce::Rectangle<float> (px - 3.0f, y0, 6.0f, H));

    // playhead cap
    juce::Path cap;
    cap.addTriangle (px - 6.0f, y0, px + 6.0f, y0, px, y0 + 8.0f);
    g.setColour (LF::neonGreen);
    g.fillPath (cap);
}

void BeatGridComponent::seekToX (int x)
{
    const double duration = proc.getEngine().getDurationSeconds();
    if (duration <= 0.0)
        return;

    auto inner = getLocalBounds().reduced (2);
    const double frac = juce::jlimit (0.0, 1.0,
        (double) (x - inner.getX()) / (double) juce::jmax (1, inner.getWidth()));
    proc.getEngine().setPositionSeconds (frac * duration);
    repaint();
}

void BeatGridComponent::mouseDown (const juce::MouseEvent& e) { seekToX (e.x); }
void BeatGridComponent::mouseDrag (const juce::MouseEvent& e) { seekToX (e.x); }
