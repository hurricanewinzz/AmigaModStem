#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include "../PluginProcessor.h"
#include "BeatGridComponent.h"

/**
    The main "PLAYER" tab: file load, transport, beatgrid, BPM-lock, tempo,
    beatjump / quantize and the 4-channel volume mixer.
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
    void applyBpmLock();                 // push target BPM -> tempoFactor param
    double currentBpm() const;
    double sixteenthSeconds() const;
    void beatJump (bool forward);
    void quantizeToGrid();
    void refreshTransportButtons();

    static juce::String formatTime (double seconds);

    ModDeckProcessor& proc;

    // header
    juce::TextButton loadButton  { "LOAD .MOD" };
    juce::Label      titleLabel;
    juce::Label      timeLabel;

    // transport
    juce::TextButton playButton  { "PLAY" };
    juce::TextButton stopButton  { "STOP" };
    juce::ToggleButton loopToggle { "LOOP" };

    // beat tools
    juce::TextButton quantizeButton { "QUANTIZE" };
    juce::TextButton jumpBackButton { "<<" };
    juce::TextButton jumpFwdButton  { ">>" };
    juce::ComboBox   jumpSizeBox;
    juce::Label      jumpLabel;

    BeatGridComponent beatGrid;

    // bpm / tempo
    juce::Label        bpmCaption;
    juce::Slider       bpmSlider;        // target BPM (drives tempo when locked)
    juce::ToggleButton bpmLockToggle { "BPM LOCK" };
    juce::Label        tempoCaption;
    juce::Slider       tempoSlider;      // tempoFactor (0.25..4)
    juce::Label        liveBpmLabel;

    // mixer
    std::array<juce::Slider, ModDeckProcessor::kNumMixChannels>     chSliders;
    std::array<juce::ToggleButton, ModDeckProcessor::kNumMixChannels> chMutes;
    std::array<juce::Label, ModDeckProcessor::kNumMixChannels>      chLabels;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::array<std::unique_ptr<SA>, ModDeckProcessor::kNumMixChannels> chAttach;
    std::unique_ptr<SA> tempoAttach;

    std::unique_ptr<juce::FileChooser> chooser;

    // layout rects filled in resized(), reused by paint()
    juce::Rectangle<int> bpmPanel;
    juce::Rectangle<int> mixerPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerPage)
};
