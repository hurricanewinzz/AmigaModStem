#pragma once

#include <libopenmpt/libopenmpt.hpp>
#include <libopenmpt/libopenmpt_ext.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

/**
    Thin, thread-safe wrapper around a single libopenmpt module.

    Owns an openmpt::module_ext so we can use the "interactive" extension to
    manipulate per-channel volume, mute state and the global tempo factor in
    real time while the module keeps playing. We never *edit* the song data
    itself (no pattern/note editing) exactly as requested - we only manipulate
    playback (volumes / tempo / position).
*/
class ModEngine
{
public:
    ModEngine() = default;
    ~ModEngine() = default;

    bool loadFile (const std::string& path, std::string& error);
    void unload();

    bool isLoaded() const noexcept { return loaded; }

    /** Render interleaved stereo float frames. Returns frames actually produced
        (0 means the song ended and looping was disabled). */
    std::size_t render (float* interleavedStereo, std::size_t frames, double sampleRate);

    // -------- transport ----------------------------------------------------
    double getDurationSeconds();
    double getPositionSeconds();
    void   setPositionSeconds (double seconds);

    // -------- channels (MOD = 4 on classic Amiga tunes) --------------------
    int    getNumChannels();
    void   setChannelVolume (int channel, double volume);   // 0..1 (linear)
    double getChannelVolume (int channel) const;
    void   setChannelMute   (int channel, bool muted);
    bool   getChannelMute   (int channel) const;
    double getChannelVu     (int channel);                  // live 0..1 meter

    // -------- tempo --------------------------------------------------------
    void   setTempoFactor (double factor);                  // 0.25 .. 4.0
    double getTempoFactor() const;
    double getBaseTempo()  const { return baseTempo; }      // module BPM at load
    double getCurrentTempo();                               // live module tempo param
    int    getCurrentSpeed();                               // ticks per row
    /** Musical BPM derived from the module's tempo & speed for a chosen
        rows-per-beat (classic Amiga = 4 rows per beat in 4/4). */
    double getMusicalBpm (int rowsPerBeat);

    // -------- looping ------------------------------------------------------
    void   setRepeat (bool loopForever);

    std::string getTitle() const { return title; }

private:
    void applyCachedState();

    mutable std::mutex lock;

    std::unique_ptr<openmpt::module_ext> mod;
    openmpt::ext::interactive*           interactive = nullptr;

    bool   loaded     = false;
    double baseTempo  = 125.0;
    std::string title;

    // cached desired state so UI changes survive a reload / are re-applied
    std::vector<double> channelVol;
    std::vector<char>   channelMuted;   // vector<bool> is special-cased; use char
    double tempoFactor = 1.0;
    bool   repeat      = true;
};
