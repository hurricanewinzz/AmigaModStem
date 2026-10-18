#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <memory>
#include "../PluginProcessor.h"
#include "BeatGridComponent.h"
#include "VuMeter.h"

/**
    Main "PLAYER" tab: file load, transport, beatgrid, grid-alignment tools
    (auto-BPM / downbeat / rows-per-beat / time-signature), loop markers,
    BPM-lock, tempo, beatjump / quantize and the 4-channel mixer with VU meters.
*/
class PlayerPage : public juce::Component,
                   private juce::Timer
{
public:
    explicit PlayerPage (ModDeckProcessor& p);
    ~PlayerPage() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void chooseFile();
    void togglePlay();
    void applyBpmLock();
    void autoDetectBpm();
    void syncBpmUi();
    double currentBpm() const;        // live musical bpm
    double sixteenthSeconds() const;
    void beatJump (bool forward);
    void quantizeToGrid();
    void refreshTransportButtons();

    static juce::String formatTime (double seconds);
    static juce::Font mono (float h, bool bold = false);

    ModDeckProcessor& proc;

    // header
    juce::TextButton loadButton  { "LOAD .MOD" };
    juce::Label      titleLabel;
    juce::Label      timeLabel;

    // transport
    juce::TextButton playButton  { "PLAY" };
    juce::TextButton stopButton  { "STOP" };
    juce::ToggleButton loopToggle { "LOOP" };
    juce::TextButton quantizeButton { "QUANTIZE" };
    juce::TextButton jumpBackButton { "<<" };
    juce::TextButton jumpFwdButton  { ">>" };
    juce::ComboBox   jumpSizeBox;
    juce::Label      jumpLabel;

    BeatGridComponent beatGrid;

    // grid + loop tools
    juce::TextButton autoBpmButton  { "AUTO BPM" };
    juce::TextButton downbeatButton { "SET DOWNBEAT" };
    juce::Label      rpbCaption, sigCaption;
    juce::ComboBox   rpbBox;      // rows per beat
    juce::ComboBox   sigBox;      // beats per bar (time signature)
    juce::TextButton loopInButton   { "SET IN" };
    juce::TextButton loopOutButton  { "SET OUT" };
    juce::ToggleButton loopSectionToggle { "SECTION" };
    juce::TextButton loopClearButton { "CLR" };

    // bpm / tempo
    juce::Label        bpmCaption;
    juce::Slider       bpmSlider;
    juce::ToggleButton bpmLockToggle { "BPM LOCK" };
    juce::Label        tempoCaption;
    juce::Slider       tempoSlider;
    juce::Label        liveBpmLabel;

    // mixer
    std::array<juce::Slider, ModDeckProcessor::kNumMixChannels>       chSliders;
    std::array<juce::ToggleButton, ModDeckProcessor::kNumMixChannels> chMutes;
    std::array<juce::Label, ModDeckProcessor::kNumMixChannels>        chLabels;
    std::array<std::unique_ptr<VuMeter>, ModDeckProcessor::kNumMixChannels> chMeters;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::array<std::unique_ptr<SA>, ModDeckProcessor::kNumMixChannels> chAttach;
    std::unique_ptr<SA> tempoAttach;

    std::unique_ptr<juce::FileChooser> chooser;

    // layout rects reused by paint()
    juce::Rectangle<int> toolsPanel;
    juce::Rectangle<int> bpmPanel;
    juce::Rectangle<int> mixerPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerPage)
};
