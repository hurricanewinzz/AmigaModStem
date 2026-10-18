#include "BeatGridComponent.h"
#include "AmigaLookAndFeel.h"
#include <cmath>

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
    const double bpm = juce::jmax (1.0, proc.baseTempo.load() * proc.getEngine().getTempoFactor());
    const double anchor = proc.gridAnchorSec.load();
    const int    bpbar  = juce::jmax (1, proc.beatsPerBar.load());

    auto inner = r.reduced (2.0f);
    const float W  = inner.getWidth();
    const float H  = inner.getHeight();
    const float x0 = inner.getX();
    const float y0 = inner.getY();

    if (duration <= 0.0)
    {
        g.setColour (LF::textDim);
        g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::plain)));
        g.drawText ("LOAD A .MOD TO SEE THE BEATGRID", inner, juce::Justification::centred);
        return;
    }

    const double beatDur  = 60.0 / bpm;
    const double secPerPx = duration / (double) W;
    auto xForT = [&] (double t) { return x0 + (float) (t / secPerPx); };

    // loop-section shading
    if (proc.loopSection.load())
    {
        const double in  = proc.loopInSec.load();
        const double out = proc.loopOutSec.load();
        if (in >= 0.0 && out > in)
        {
            g.setColour (LF::neonAmber.withAlpha (0.14f));
            g.fillRect (juce::Rectangle<float> (xForT (in), y0, xForT (out) - xForT (in), H));
        }
    }

    // sixteenth subdivisions (faint), anchored to the downbeat
    const double sixteenth = beatDur / 4.0;
    g.setColour (LF::grid.withAlpha (0.5f));
    {
        double t = anchor - std::floor ((anchor) / sixteenth) * sixteenth;   // first grid <= 0 region
        for (; t <= duration; t += sixteenth)
            if (t >= 0.0)
                g.drawVerticalLine ((int) xForT (t), y0 + H * 0.55f, y0 + H);
    }

    // beat / bar lines, counted from the anchor downbeat
    const long firstBeat = (long) std::floor ((0.0 - anchor) / beatDur);
    const long lastBeat  = (long) std::ceil  ((duration - anchor) / beatDur);
    for (long k = firstBeat; k <= lastBeat; ++k)
    {
        const double t = anchor + (double) k * beatDur;
        if (t < 0.0 || t > duration)
            continue;

        const long mod = ((k % bpbar) + bpbar) % bpbar;   // true modulo
        const bool bar = (mod == 0);
        g.setColour (bar ? LF::neonCyan.withAlpha (0.8f) : LF::grid.brighter (0.4f));
        g.drawVerticalLine ((int) xForT (t), y0, y0 + H);

        if (bar)
        {
            g.setColour (LF::neonCyan.withAlpha (0.85f));
            g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain)));
            g.drawText (juce::String (k / bpbar + 1),
                        (int) xForT (t) + 2, (int) y0 + 1, 28, 12, juce::Justification::left);
        }
    }

    // elapsed shade
    const float px = xForT (position);
    g.setColour (LF::neonGreen.withAlpha (0.10f));
    g.fillRect (juce::Rectangle<float> (x0, y0, px - x0, H));

    // loop markers
    auto drawMarker = [&] (double t, juce::Colour col, const juce::String& tag)
    {
        if (t < 0.0) return;
        const float x = xForT (t);
        g.setColour (col);
        g.fillRect (juce::Rectangle<float> (x - 1.0f, y0, 2.0f, H));
        g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold)));
        g.drawText (tag, (int) x + 2, (int) (y0 + H - 14), 22, 12, juce::Justification::left);
    };
    drawMarker (proc.loopInSec.load(),  LF::neonAmber, "IN");
    drawMarker (proc.loopOutSec.load(), LF::neonAmber, "OUT");

    // playhead
    g.setColour (LF::neonGreen);
    g.fillRect (juce::Rectangle<float> (px - 1.0f, y0, 2.0f, H));
    g.setColour (LF::neonGreen.withAlpha (0.3f));
    g.fillRect (juce::Rectangle<float> (px - 3.0f, y0, 6.0f, H));
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
