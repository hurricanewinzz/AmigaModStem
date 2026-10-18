#include "PlayerPage.h"
#include "AmigaLookAndFeel.h"

namespace
{
    juce::Font mono (float h, bool bold = false)
    {
        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h,
                                              bold ? juce::Font::bold : juce::Font::plain));
    }

    // jump sizes expressed in beats (quarter note = 1 beat)
    const std::array<std::pair<const char*, double>, 5> kJumps {{
        { "1/16", 0.25 }, { "1/8", 0.5 }, { "1/4", 1.0 }, { "1/2", 2.0 }, { "1 BAR", 4.0 }
    }};
}

PlayerPage::PlayerPage (ModDeckProcessor& p)
    : proc (p), beatGrid (p)
{
    using LF = AmigaLookAndFeel;

    auto setAccent = [] (juce::Component& c, juce::Colour col)
    {
        c.getProperties().set ("accent", (juce::int64) (juce::uint32) col.getARGB());
    };

    // ---- header ----------------------------------------------------------
    addAndMakeVisible (loadButton);
    setAccent (loadButton, LF::neonAmber);
    loadButton.onClick = [this] { chooseFile(); };

    addAndMakeVisible (titleLabel);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (mono (18.0f, true));
    titleLabel.setColour (juce::Label::textColourId, LF::neonGreen);
    titleLabel.setText ("NO MODULE LOADED", juce::dontSendNotification);

    addAndMakeVisible (timeLabel);
    timeLabel.setJustificationType (juce::Justification::centredRight);
    timeLabel.setFont (mono (16.0f, true));
    timeLabel.setColour (juce::Label::textColourId, LF::neonCyan);
    timeLabel.setText ("00:00 / 00:00", juce::dontSendNotification);

    // ---- transport -------------------------------------------------------
    addAndMakeVisible (playButton);
    setAccent (playButton, LF::neonGreen);
    playButton.onClick = [this] { togglePlay(); };

    addAndMakeVisible (stopButton);
    setAccent (stopButton, LF::neonMagenta);
    stopButton.onClick = [this] { proc.stopAndRewind(); refreshTransportButtons(); };

    addAndMakeVisible (loopToggle);
    loopToggle.setToggleState (proc.loopEnabled.load(), juce::dontSendNotification);
    loopToggle.onClick = [this] { proc.loopEnabled.store (loopToggle.getToggleState()); };

    // ---- beat tools ------------------------------------------------------
    addAndMakeVisible (quantizeButton);
    setAccent (quantizeButton, LF::neonCyan);
    quantizeButton.onClick = [this] { quantizeToGrid(); };

    addAndMakeVisible (jumpLabel);
    jumpLabel.setText ("BEATJUMP", juce::dontSendNotification);
    jumpLabel.setFont (mono (12.0f, true));
    jumpLabel.setColour (juce::Label::textColourId, LF::textDim);
    jumpLabel.setJustificationType (juce::Justification::centred);

    addAndMakeVisible (jumpBackButton);
    setAccent (jumpBackButton, LF::neonCyan);
    jumpBackButton.onClick = [this] { beatJump (false); };

    addAndMakeVisible (jumpFwdButton);
    setAccent (jumpFwdButton, LF::neonCyan);
    jumpFwdButton.onClick = [this] { beatJump (true); };

    addAndMakeVisible (jumpSizeBox);
    for (int i = 0; i < (int) kJumps.size(); ++i)
        jumpSizeBox.addItem (kJumps[(size_t) i].first, i + 1);
    jumpSizeBox.setSelectedId (3, juce::dontSendNotification);   // default 1/4

    addAndMakeVisible (beatGrid);

    // ---- bpm / tempo -----------------------------------------------------
    addAndMakeVisible (bpmCaption);
    bpmCaption.setText ("TARGET BPM", juce::dontSendNotification);
    bpmCaption.setFont (mono (12.0f, true));
    bpmCaption.setColour (juce::Label::textColourId, LF::textDim);

    addAndMakeVisible (bpmSlider);
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setRange (40.0, 300.0, 0.1);
    bpmSlider.setValue (proc.lockedBpm.load(), juce::dontSendNotification);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 22);
    bpmSlider.setColour (juce::Slider::textBoxTextColourId, LF::neonGreen);
    bpmSlider.setColour (juce::Slider::textBoxOutlineColourId, LF::grid);
    bpmSlider.onValueChange = [this]
    {
        proc.lockedBpm.store (bpmSlider.getValue());
        if (proc.bpmLocked.load())
            applyBpmLock();
    };

    addAndMakeVisible (bpmLockToggle);
    bpmLockToggle.setToggleState (proc.bpmLocked.load(), juce::dontSendNotification);
    bpmLockToggle.onClick = [this]
    {
        const bool locked = bpmLockToggle.getToggleState();
        proc.bpmLocked.store (locked);
        tempoSlider.setEnabled (! locked);
        if (locked)
            applyBpmLock();
    };

    addAndMakeVisible (tempoCaption);
    tempoCaption.setText ("TEMPO x", juce::dontSendNotification);
    tempoCaption.setFont (mono (12.0f, true));
    tempoCaption.setColour (juce::Label::textColourId, LF::textDim);

    addAndMakeVisible (tempoSlider);
    tempoSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    tempoSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    tempoSlider.setColour (juce::Slider::textBoxTextColourId, LF::neonCyan);
    tempoSlider.setColour (juce::Slider::textBoxOutlineColourId, LF::grid);
    tempoSlider.setEnabled (! proc.bpmLocked.load());
    tempoAttach = std::make_unique<SA> (proc.apvts, "tempoFactor", tempoSlider);

    addAndMakeVisible (liveBpmLabel);
    liveBpmLabel.setJustificationType (juce::Justification::centred);
    liveBpmLabel.setFont (mono (26.0f, true));
    liveBpmLabel.setColour (juce::Label::textColourId, LF::neonMagenta);
    liveBpmLabel.setText ("--- BPM", juce::dontSendNotification);

    // ---- mixer -----------------------------------------------------------
    const std::array<juce::Colour, 4> stripCols { LF::neonGreen, LF::neonCyan, LF::neonMagenta, LF::neonAmber };
    for (int c = 0; c < ModDeckProcessor::kNumMixChannels; ++c)
    {
        auto& s = chSliders[(size_t) c];
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::LinearVertical);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        chAttach[(size_t) c] = std::make_unique<SA> (proc.apvts, "ch" + juce::String (c + 1), s);

        auto& m = chMutes[(size_t) c];
        addAndMakeVisible (m);
        m.setButtonText ("MUTE");
        m.onClick = [this, c] { proc.getEngine().setChannelMute (c, chMutes[(size_t) c].getToggleState()); };

        auto& l = chLabels[(size_t) c];
        addAndMakeVisible (l);
        l.setText ("CH " + juce::String (c + 1), juce::dontSendNotification);
        l.setFont (mono (14.0f, true));
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, stripCols[(size_t) c]);
    }

    refreshTransportButtons();
    startTimerHz (15);
}

PlayerPage::~PlayerPage()
{
    stopTimer();
}

double PlayerPage::currentBpm() const
{
    return juce::jmax (1.0, proc.lockedBpm.load());
}

double PlayerPage::sixteenthSeconds() const
{
    return (60.0 / currentBpm()) / 4.0;
}

void PlayerPage::applyBpmLock()
{
    const double base = juce::jmax (1.0, proc.baseTempo.load());
    const double factor = juce::jlimit (0.25, 4.0, proc.lockedBpm.load() / base);
    if (auto* p = proc.apvts.getParameter ("tempoFactor"))
        p->setValueNotifyingHost (p->convertTo0to1 ((float) factor));
}

void PlayerPage::beatJump (bool forward)
{
    const double beats = kJumps[(size_t) juce::jlimit (0, 4, jumpSizeBox.getSelectedId() - 1)].second;
    const double delta = beats * (60.0 / currentBpm());
    const double dur = proc.getEngine().getDurationSeconds();
    double pos = proc.getEngine().getPositionSeconds() + (forward ? delta : -delta);
    if (dur > 0.0)
        pos = juce::jlimit (0.0, dur, pos);
    proc.getEngine().setPositionSeconds (juce::jmax (0.0, pos));
    beatGrid.repaint();
}

void PlayerPage::quantizeToGrid()
{
    const double step = sixteenthSeconds();
    if (step <= 0.0)
        return;
    const double pos = proc.getEngine().getPositionSeconds();
    const double snapped = std::round (pos / step) * step;
    proc.getEngine().setPositionSeconds (juce::jmax (0.0, snapped));
    beatGrid.repaint();
}

void PlayerPage::chooseFile()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Load an Amiga module", juce::File(),
        "*.mod;*.xm;*.s3m;*.it;*.mptm;*.med;*.stk");

    const auto flags = juce::FileBrowserComponent::openMode
                     | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();
        if (file == juce::File())
            return;

        juce::String error;
        if (proc.loadModFile (file, error))
        {
            titleLabel.setText (proc.getEngine().getTitle().empty()
                                    ? file.getFileName().toStdString()
                                    : proc.getEngine().getTitle(),
                                juce::dontSendNotification);
            bpmSlider.setValue (proc.lockedBpm.load(), juce::dontSendNotification);
            if (proc.bpmLocked.load())
                applyBpmLock();
            refreshTransportButtons();
            beatGrid.repaint();
        }
        else
        {
            juce::NativeMessageBox::showMessageBoxAsync (
                juce::AlertWindow::WarningIcon, "Load failed", error);
        }
    });
}

void PlayerPage::togglePlay()
{
    if (! proc.getEngine().isLoaded())
        return;
    proc.setPlaying (! proc.isPlaying());
    refreshTransportButtons();
}

void PlayerPage::refreshTransportButtons()
{
    playButton.setButtonText (proc.isPlaying() ? "PAUSE" : "PLAY");
    const bool loaded = proc.getEngine().isLoaded();
    playButton.setEnabled (loaded);
    stopButton.setEnabled (loaded);
    quantizeButton.setEnabled (loaded);
    jumpBackButton.setEnabled (loaded);
    jumpFwdButton.setEnabled (loaded);
}

juce::String PlayerPage::formatTime (double seconds)
{
    if (seconds < 0.0) seconds = 0.0;
    const int total = (int) seconds;
    return juce::String::formatted ("%02d:%02d", total / 60, total % 60);
}

void PlayerPage::timerCallback()
{
    const double pos = proc.getEngine().getPositionSeconds();
    const double dur = proc.getEngine().getDurationSeconds();
    timeLabel.setText (formatTime (pos) + " / " + formatTime (dur), juce::dontSendNotification);

    const double base = juce::jmax (1.0, proc.baseTempo.load());
    const double factor = proc.getEngine().getTempoFactor();
    const double liveBpm = base * factor;
    liveBpmLabel.setText (juce::String (liveBpm, 1) + " BPM", juce::dontSendNotification);

    if (! proc.bpmLocked.load())
        bpmSlider.setValue (liveBpm, juce::dontSendNotification);

    if (playButton.getButtonText() == "PLAY" && proc.isPlaying())
        refreshTransportButtons();
    if (playButton.getButtonText() == "PAUSE" && ! proc.isPlaying())
        refreshTransportButtons();
}

void PlayerPage::paint (juce::Graphics& g)
{
    using LF = AmigaLookAndFeel;

    auto drawPanel = [&g] (juce::Rectangle<int> r, const juce::String& caption)
    {
        g.setColour (LF::panel.withAlpha (0.6f));
        g.fillRoundedRectangle (r.toFloat(), 6.0f);
        g.setColour (LF::grid);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 6.0f, 1.0f);
        if (caption.isNotEmpty())
        {
            g.setColour (LF::textDim);
            g.setFont (mono (11.0f, true));
            g.drawText (caption, r.getX() + 10, r.getY() + 4, 220, 14, juce::Justification::left);
        }
    };

    drawPanel (bpmPanel, {});
    drawPanel (mixerPanel, "4-CHANNEL MIXER  //  AMIGA PAULA");
}

void PlayerPage::resized()
{
    auto area = getLocalBounds().reduced (16);

    // header
    auto header = area.removeFromTop (40);
    loadButton.setBounds (header.removeFromLeft (150).reduced (0, 2));
    timeLabel.setBounds (header.removeFromRight (180));
    titleLabel.setBounds (header.reduced (14, 0));

    area.removeFromTop (10);

    // transport
    auto transport = area.removeFromTop (46);
    playButton.setBounds (transport.removeFromLeft (110).reduced (0, 3));
    transport.removeFromLeft (8);
    stopButton.setBounds (transport.removeFromLeft (90).reduced (0, 3));
    transport.removeFromLeft (8);
    loopToggle.setBounds (transport.removeFromLeft (90).reduced (0, 6));

    // right-aligned beatjump cluster
    jumpFwdButton.setBounds (transport.removeFromRight (56).reduced (0, 3));
    transport.removeFromRight (6);
    jumpSizeBox.setBounds (transport.removeFromRight (90).reduced (0, 6));
    transport.removeFromRight (6);
    jumpBackButton.setBounds (transport.removeFromRight (56).reduced (0, 3));
    transport.removeFromRight (6);
    jumpLabel.setBounds (transport.removeFromRight (90).reduced (0, 6));
    transport.removeFromRight (14);
    quantizeButton.setBounds (transport.removeFromRight (110).reduced (0, 3));

    area.removeFromTop (10);

    // beatgrid
    beatGrid.setBounds (area.removeFromTop (120));

    area.removeFromTop (12);

    // bpm / tempo panel
    bpmPanel = area.removeFromTop (96);
    {
        auto p = bpmPanel.reduced (12);

        // big live-BPM readout on the right
        liveBpmLabel.setBounds (p.removeFromRight (180));

        auto row1 = p.removeFromTop (p.getHeight() / 2);
        bpmCaption.setBounds (row1.removeFromLeft (100));
        bpmLockToggle.setBounds (row1.removeFromRight (120).reduced (4, 4));
        bpmSlider.setBounds (row1.reduced (6, 2));

        p.removeFromTop (6);
        tempoCaption.setBounds (p.removeFromLeft (100));
        tempoSlider.setBounds (p.reduced (6, 2));
    }

    area.removeFromTop (12);

    // mixer panel
    mixerPanel = area;
    auto m = mixerPanel.reduced (14);
    m.removeFromTop (18); // caption space
    const int n = ModDeckProcessor::kNumMixChannels;
    const int stripW = m.getWidth() / n;
    for (int c = 0; c < n; ++c)
    {
        auto strip = m.removeFromLeft (stripW).reduced (10, 0);
        chLabels[(size_t) c].setBounds (strip.removeFromTop (20));
        chMutes[(size_t) c].setBounds (strip.removeFromBottom (26).reduced (8, 2));
        chSliders[(size_t) c].setBounds (strip.reduced (0, 6));
    }
}
