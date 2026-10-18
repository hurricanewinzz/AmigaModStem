#include "FxPage.h"
#include "AmigaLookAndFeel.h"

FxPage::FxPage (ModDeckProcessor& p) : proc (p)
{
    using LF = AmigaLookAndFeel;

    addAndMakeVisible (headline);
    headline.setText ("FX  RACK", juce::dontSendNotification);
    headline.setJustificationType (juce::Justification::centred);
    headline.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 34.0f, juce::Font::bold)));
    headline.setColour (juce::Label::textColourId, LF::neonMagenta);

    addAndMakeVisible (body);
    body.setJustificationType (juce::Justification::centred);
    body.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::plain)));
    body.setColour (juce::Label::textColourId, LF::textDim);
    body.setText ("> VST3 EFFECT HOSTING SLOT\n"
                  "> RESERVED // COMING ONLINE SOON\n\n"
                  "This native build can load external VST3 effect plugins here.",
                  juce::dontSendNotification);

    juce::ignoreUnused (proc);
}

void FxPage::paint (juce::Graphics& g)
{
    using LF = AmigaLookAndFeel;
    auto r = getLocalBounds().reduced (24).toFloat();

    g.setColour (LF::panel.withAlpha (0.5f));
    g.fillRoundedRectangle (r, 8.0f);

    g.setColour (LF::neonMagenta.withAlpha (0.55f));
    g.drawRoundedRectangle (r.reduced (3.0f), 8.0f, 1.6f);
    g.setColour (LF::neonMagenta.withAlpha (0.15f));
    g.drawRoundedRectangle (r.reduced (6.0f), 8.0f, 1.0f);
}

void FxPage::resized()
{
    auto area = getLocalBounds().reduced (24);
    headline.setBounds (area.removeFromTop (area.getHeight() / 2).withTrimmedBottom (10));
    body.setBounds (area);
}
