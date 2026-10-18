#include "FxPage.h"
#include "AmigaLookAndFeel.h"

//==============================================================================
class FxPage::PluginWindow : public juce::DocumentWindow
{
public:
    PluginWindow (juce::AudioPluginInstance& p, juce::AudioProcessorEditor* e,
                  std::function<void()> onClose)
        : DocumentWindow (p.getName(), juce::Colour (0xff07060d), DocumentWindow::closeButton),
          plugin (p), editor (e), closeCb (std::move (onClose))
    {
        setUsingNativeTitleBar (true);
        setContentNonOwned (editor, true);
        setResizable (editor->isResizable(), false);
        centreWithSize (juce::jmax (240, editor->getWidth()),
                        juce::jmax (120, editor->getHeight()));
        setVisible (true);
        toFront (true);
    }

    ~PluginWindow() override
    {
        clearContentComponent();
        plugin.editorBeingDeleted (editor);
        delete editor;
    }

    void closeButtonPressed() override { if (closeCb) closeCb(); }

private:
    juce::AudioPluginInstance&  plugin;
    juce::AudioProcessorEditor* editor;
    std::function<void()>       closeCb;
};

//==============================================================================
FxPage::FxPage (ModDeckProcessor& p) : proc (p)
{
    using LF = AmigaLookAndFeel;

    auto setAccent = [] (juce::Component& c, juce::Colour col)
    {
        c.getProperties().set ("accent", (juce::int64) (juce::uint32) col.getARGB());
    };

    addAndMakeVisible (headline);
    headline.setText ("FX  RACK  //  EXTERNAL EFFECT SLOT", juce::dontSendNotification);
    headline.setJustificationType (juce::Justification::centredLeft);
    headline.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 22.0f, juce::Font::bold)));
    headline.setColour (juce::Label::textColourId, LF::neonMagenta);

    addAndMakeVisible (nameLabel);
    nameLabel.setJustificationType (juce::Justification::centredLeft);
    nameLabel.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 18.0f, juce::Font::plain)));
    nameLabel.setColour (juce::Label::textColourId, LF::neonGreen);

    addAndMakeVisible (hint);
    hint.setJustificationType (juce::Justification::topLeft);
    hint.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain)));
    hint.setColour (juce::Label::textColourId, LF::textDim);
    hint.setText ("> Load a VST3 (or AU on macOS) effect. The module audio is\n"
                  "  routed through it live. Open its GUI to tweak parameters.",
                  juce::dontSendNotification);

    addAndMakeVisible (loadButton);
    setAccent (loadButton, LF::neonAmber);
    loadButton.onClick = [this] { loadPlugin(); };

    addAndMakeVisible (showButton);
    setAccent (showButton, LF::neonCyan);
    showButton.onClick = [this] { showEditor(); };

    addAndMakeVisible (removeButton);
    setAccent (removeButton, LF::neonMagenta);
    removeButton.onClick = [this] { removePlugin(); };

    addAndMakeVisible (bypassToggle);
    bypassToggle.onClick = [this] { proc.getFxHost().setBypassed (bypassToggle.getToggleState()); };

    refresh();
}

FxPage::~FxPage()
{
    pluginWindow.reset();   // delete plugin editor before anything else tears down
}

void FxPage::loadPlugin()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Load a VST3 / AU effect", juce::File(), "*.vst3;*.component");

    const auto flags = juce::FileBrowserComponent::openMode
                     | juce::FileBrowserComponent::canSelectFiles
                     | juce::FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file == juce::File())
            return;

        pluginWindow.reset();   // close any open GUI before swapping
        const auto err = proc.getFxHost().loadPlugin (file);
        if (err.isNotEmpty())
            juce::NativeMessageBox::showMessageBoxAsync (
                juce::AlertWindow::WarningIcon, "Plugin load failed", err);

        refresh();
    });
}

void FxPage::showEditor()
{
    if (pluginWindow != nullptr)
    {
        pluginWindow->toFront (true);
        return;
    }

    auto* inst = proc.getFxHost().getInstance();
    if (inst == nullptr)
        return;

    if (auto* e = inst->createEditorIfNeeded())
        pluginWindow = std::make_unique<PluginWindow> (*inst, e, [this]
        {
            juce::MessageManager::callAsync ([this] { pluginWindow.reset(); });
        });
    else
        juce::NativeMessageBox::showMessageBoxAsync (
            juce::AlertWindow::InfoIcon, "No editor",
            "This plugin does not provide a custom editor.");
}

void FxPage::removePlugin()
{
    pluginWindow.reset();
    proc.getFxHost().unload();
    bypassToggle.setToggleState (false, juce::dontSendNotification);
    refresh();
}

void FxPage::refresh()
{
    const bool has = proc.getFxHost().hasPlugin();
    nameLabel.setText (has ? ("LOADED:  " + proc.getFxHost().getName())
                           : "NO EFFECT LOADED",
                       juce::dontSendNotification);
    showButton.setEnabled (has);
    removeButton.setEnabled (has);
    bypassToggle.setEnabled (has);
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
    auto area = getLocalBounds().reduced (44);

    headline.setBounds (area.removeFromTop (34));
    area.removeFromTop (8);
    nameLabel.setBounds (area.removeFromTop (30));
    area.removeFromTop (16);

    auto row = area.removeFromTop (40);
    loadButton.setBounds (row.removeFromLeft (170).reduced (0, 2));
    row.removeFromLeft (10);
    showButton.setBounds (row.removeFromLeft (120).reduced (0, 2));
    row.removeFromLeft (10);
    removeButton.setBounds (row.removeFromLeft (110).reduced (0, 2));
    row.removeFromLeft (10);
    bypassToggle.setBounds (row.removeFromLeft (110).reduced (0, 4));

    area.removeFromTop (20);
    hint.setBounds (area.removeFromTop (60));
}
