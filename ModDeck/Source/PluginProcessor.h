#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <vector>

#include "ModEngine.h"
#include "FxHost.h"

class ModDeckProcessor : public juce::AudioProcessor
{
public:
    ModDeckProcessor();
    ~ModDeckProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ModDeck"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ----- app API used by the editor -------------------------------------
    ModEngine& getEngine() { return engine; }
    FxHost&    getFxHost() { return fxHost; }

    bool loadModFile (const juce::File& file, juce::String& error);

    void setPlaying (bool p) { playing.store (p); }
    bool isPlaying() const   { return playing.load(); }
    void stopAndRewind();

    static constexpr int kNumMixChannels = 4;

    juce::AudioProcessorValueTreeState apvts;

    // beat / sync settings owned by the processor so they persist & automate
    std::atomic<double> lockedBpm   { 125.0 };   // target (musical) BPM
    std::atomic<bool>   bpmLocked   { false };
    std::atomic<bool>   loopEnabled { true  };
    std::atomic<double> baseTempo   { 125.0 };   // musical BPM at tempo factor 1

    // beatgrid alignment (so you can match 4/4 or odd signatures)
    std::atomic<double> gridAnchorSec { 0.0 };
    std::atomic<int>    rowsPerBeat   { 4 };
    std::atomic<int>    beatsPerBar   { 4 };

    // section loop markers (seconds; <0 = unset)
    std::atomic<double> loopInSec     { -1.0 };
    std::atomic<double> loopOutSec    { -1.0 };
    std::atomic<bool>   loopSection   { false };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    ModEngine engine;
    FxHost    fxHost;
    std::atomic<bool> playing { false };
    double currentSampleRate = 44100.0;
    int    currentBlockSize  = 512;
    std::vector<float> interleaved;
    juce::AudioBuffer<float> dryBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModDeckProcessor)
};
