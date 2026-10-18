#include "PluginEditor.h"

ModDeckEditor::ModDeckEditor (ModDeckProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      playerPage (p), fxPage (p)
{
    setLookAndFeel (&lnf);

    using LF = AmigaLookAndFeel;
    tabs.setColour (juce::TabbedComponent::backgroundColourId, LF::bg);
    tabs.setColour (juce::TabbedComponent::outlineColourId, LF::grid);
    tabs.setOutline (0);
    tabs.addTab ("PLAYER", LF::bg, &playerPage, false);
    tabs.addTab ("FX",     LF::bg, &fxPage,     false);
    tabs.getTabbedButtonBar().setColour (juce::TabbedButtonBar::frontTextColourId, LF::neonGreen);
    tabs.getTabbedButtonBar().setColour (juce::TabbedButtonBar::tabTextColourId, LF::textDim);
    addAndMakeVisible (tabs);

    setResizable (true, true);
    setResizeLimits (760, 560, 1600, 1100);
    setSize (940, 660);
}

ModDeckEditor::~ModDeckEditor()
{
    setLookAndFeel (nullptr);
}

void ModDeckEditor::paint (juce::Graphics& g)
{
    using LF = AmigaLookAndFeel;
    g.fillAll (LF::bg);

    // subtle radial vignette glow
    auto b = getLocalBounds().toFloat();
    juce::ColourGradient grad (LF::panel.brighter (0.05f), b.getCentreX(), b.getCentreY() * 0.4f,
                               LF::bg, b.getX(), b.getBottom(), true);
    g.setGradientFill (grad);
    g.fillRect (b);
}

void ModDeckEditor::paintOverChildren (juce::Graphics& g)
{
    using LF = AmigaLookAndFeel;
    auto b = getLocalBounds();

    // CRT scanlines
    g.setColour (juce::Colours::black.withAlpha (0.10f));
    for (int y = 0; y < b.getHeight(); y += 3)
        g.drawHorizontalLine (y, 0.0f, (float) b.getWidth());

    // screen edge glow
    g.setColour (LF::neonCyan.withAlpha (0.10f));
    g.drawRoundedRectangle (b.toFloat().reduced (1.5f), 4.0f, 2.0f);
}

void ModDeckEditor::resized()
{
    tabs.setBounds (getLocalBounds());
}
