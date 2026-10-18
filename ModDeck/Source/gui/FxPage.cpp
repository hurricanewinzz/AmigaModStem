#include "FxPage.h"
#include "AmigaLookAndFeel.h"

namespace
{
    void setAccent (juce::Component& c, juce::Colour col)
    {
        c.getProperties().set ("accent", (juce::int64) (juce::uint32) col.getARGB());
    }

    juce::Font mono (float h, bool bold = false)
    {
        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h,
                                              bold ? juce::Font::bold : juce::Font::plain));
    }
}

//==============================================================================
FxSlotRow::FxSlotRow()
{
    using LF = AmigaLookAndFeel;

    addAndMakeVisible (idxLabel);
    idxLabel.setFont (mono (18.0f, true));
    idxLabel.setColour (juce::Label::textColourId, LF::neonMagenta);
    idxLabel.setJustificationType (juce::Justification::centred);

    addAndMakeVisible (nameLabel);
    nameLabel.setFont (mono (15.0f, true));
    nameLabel.setColour (juce::Label::textColourId, LF::neonGreen);
    nameLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (bypassBtn);
    bypassBtn.onClick = [this] { if (onBypass) onBypass (index, bypassBtn.getToggleState()); };

    addAndMakeVisible (guiBtn);
    setAccent (guiBtn, LF::neonCyan);
    guiBtn.onClick = [this] { if (onGui) onGui (index); };

    addAndMakeVisible (upBtn);
    setAccent (upBtn, LF::textDim);
    upBtn.onClick = [this] { if (onUp) onUp (index); };

    addAndMakeVisible (downBtn);
    setAccent (downBtn, LF::textDim);
    downBtn.onClick = [this] { if (onDown) onDown (index); };

    addAndMakeVisible (removeBtn);
    setAccent (removeBtn, LF::neonMagenta);
    removeBtn.onClick = [this] { if (onRemove) onRemove (index); };
}

void FxSlotRow::setData (int i, const juce::String& name, bool bypassed,
                         bool canMoveUp, bool canMoveDown)
{
    index = i;
    idxLabel.setText (juce::String (i + 1), juce::dontSendNotification);
    nameLabel.setText (name, juce::dontSendNotification);
    bypassBtn.setToggleState (bypassed, juce::dontSendNotification);
    upBtn.setEnabled (canMoveUp);
    downBtn.setEnabled (canMoveDown);
}

void FxSlotRow::paint (juce::Graphics& g)
{
    using LF = AmigaLookAndFeel;
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (LF::panel);
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (bypassBtn.getToggleState() ? LF::grid : LF::neonCyan.withAlpha (0.4f));
    g.drawRoundedRectangle (r, 5.0f, 1.2f);
}

void FxSlotRow::resized()
{
    auto r = getLocalBounds().reduced (8, 6);
    idxLabel.setBounds (r.removeFromLeft (26));
    removeBtn.setBounds (r.removeFromRight (40).reduced (2, 0));
    r.removeFromRight (6);
    downBtn.setBounds (r.removeFromRight (42).reduced (2, 0));
    upBtn.setBounds (r.removeFromRight (42).reduced (2, 0));
    r.removeFromRight (6);
    guiBtn.setBounds (r.removeFromRight (56).reduced (2, 0));
    bypassBtn.setBounds (r.removeFromRight (64).reduced (2, 2));
    r.removeFromRight (8);
    nameLabel.setBounds (r);
}

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

    addAndMakeVisible (headline);
    headline.setText ("FX  CHAIN  //  SERIAL EFFECT RACK", juce::dontSendNotification);
    headline.setJustificationType (juce::Justification::centredLeft);
    headline.setFont (mono (22.0f, true));
    headline.setColour (juce::Label::textColourId, LF::neonMagenta);

    addAndMakeVisible (hint);
    hint.setJustificationType (juce::Justification::topLeft);
    hint.setFont (mono (12.5f));
    hint.setColour (juce::Label::textColourId, LF::textDim);
    hint.setText ("> Add VST3/AU effects. They run top-to-bottom over the module. "
                  "Reorder with UP/DN, BYP to bypass one, GUI to edit.",
                  juce::dontSendNotification);

    addAndMakeVisible (addButton);
    setAccent (addButton, LF::neonAmber);
    addButton.onClick = [this] { addEffect(); };

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&chainContainer, false);
    viewport.setScrollBarsShown (true, false);

    rebuildChain();
}

FxPage::~FxPage()
{
    windows.clear();   // delete all plugin editors before teardown
}

void FxPage::addEffect()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Add a VST3 / AU effect", juce::File(), "*.vst3;*.component");

    const auto flags = juce::FileBrowserComponent::openMode
                     | juce::FileBrowserComponent::canSelectFiles
                     | juce::FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file == juce::File())
            return;

        const auto err = proc.getFxHost().addPlugin (file);
        if (err.isNotEmpty())
            juce::NativeMessageBox::showMessageBoxAsync (
                juce::AlertWindow::WarningIcon, "Plugin load failed", err);

        rebuildChain();
    });
}

void FxPage::rebuildChain()
{
    rows.clear();
    const int n = proc.getFxHost().getNumSlots();

    for (int i = 0; i < n; ++i)
    {
        auto* row = new FxSlotRow();
        row->setData (i, proc.getFxHost().getSlotName (i),
                      proc.getFxHost().isSlotBypassed (i),
                      i > 0, i < n - 1);
        row->onGui    = [this] (int idx) { openEditor (idx); };
        row->onRemove = [this] (int idx) { removeAt (idx); };
        row->onUp     = [this] (int idx) { moveSlot (idx, -1); };
        row->onDown   = [this] (int idx) { moveSlot (idx, +1); };
        row->onBypass = [this] (int idx, bool b) { proc.getFxHost().setSlotBypass (idx, b); repaint(); };
        chainContainer.addAndMakeVisible (row);
        rows.add (row);
    }

    layoutChain();
    repaint();
}

void FxPage::layoutChain()
{
    const int w = juce::jmax (100, viewport.getMaximumVisibleWidth());
    const int n = rows.size();
    chainContainer.setSize (w, juce::jmax (viewport.getHeight(), n * kRowHeight));

    for (int i = 0; i < n; ++i)
        rows[i]->setBounds (0, i * kRowHeight, w, kRowHeight - 6);
}

void FxPage::openEditor (int index)
{
    auto* inst = proc.getFxHost().getInstance (index);
    if (inst == nullptr)
        return;

    auto it = windows.find (inst);
    if (it != windows.end() && it->second != nullptr)
    {
        it->second->toFront (true);
        return;
    }

    if (auto* e = inst->createEditorIfNeeded())
        windows[inst] = std::make_unique<PluginWindow> (*inst, e, [this, inst]
        {
            juce::MessageManager::callAsync ([this, inst] { windows.erase (inst); });
        });
    else
        juce::NativeMessageBox::showMessageBoxAsync (
            juce::AlertWindow::InfoIcon, "No editor",
            "This plugin does not provide a custom editor.");
}

void FxPage::removeAt (int index)
{
    if (auto* inst = proc.getFxHost().getInstance (index))
        windows.erase (inst);                 // close its GUI first
    proc.getFxHost().removeSlot (index);
    rebuildChain();
}

void FxPage::moveSlot (int index, int delta)
{
    const int to = index + delta;
    if (to < 0 || to >= proc.getFxHost().getNumSlots())
        return;
    proc.getFxHost().moveSlot (index, to);
    rebuildChain();
}

void FxPage::paint (juce::Graphics& g)
{
    using LF = AmigaLookAndFeel;
    auto r = getLocalBounds().reduced (18).toFloat();
    g.setColour (LF::panel.withAlpha (0.5f));
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (LF::neonMagenta.withAlpha (0.5f));
    g.drawRoundedRectangle (r.reduced (3.0f), 8.0f, 1.4f);

    if (proc.getFxHost().getNumSlots() == 0)
    {
        g.setColour (LF::textDim);
        g.setFont (mono (15.0f));
        g.drawText ("CHAIN EMPTY  —  ADD AN EFFECT", viewport.getBounds(),
                    juce::Justification::centred);
    }
}

void FxPage::resized()
{
    auto area = getLocalBounds().reduced (34);

    auto top = area.removeFromTop (34);
    headline.setBounds (top.removeFromLeft (top.getWidth() - 160));
    addButton.setBounds (top.removeFromRight (150).reduced (0, 1));

    area.removeFromTop (6);
    hint.setBounds (area.removeFromTop (38));
    area.removeFromTop (10);

    viewport.setBounds (area);
    layoutChain();
}
