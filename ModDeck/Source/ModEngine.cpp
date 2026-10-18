#include "ModEngine.h"

#include <fstream>
#include <algorithm>
#include <cmath>

bool ModEngine::loadFile (const std::string& path, std::string& error)
{
    std::lock_guard<std::mutex> g (lock);
    try
    {
        std::ifstream file (path, std::ios::binary);
        if (! file)
        {
            error = "Could not open file: " + path;
            return false;
        }

        auto newMod = std::make_unique<openmpt::module_ext> (file);

        interactive = static_cast<openmpt::ext::interactive*> (
            newMod->get_interface (openmpt::ext::interactive_id));

        mod = std::move (newMod);

        // reset playback params that libopenmpt supports
        mod->set_repeat_count (repeat ? -1 : 0);

        baseTempo = mod->get_current_tempo();
        if (baseTempo <= 0.0)
            baseTempo = 125.0;

        try { title = mod->get_metadata ("title"); } catch (...) { title.clear(); }
        if (title.empty())
        {
            auto slash = path.find_last_of ("/\\");
            title = (slash == std::string::npos) ? path : path.substr (slash + 1);
        }

        const int ch = mod->get_num_channels();
        channelVol.assign   ((size_t) ch, 1.0);
        channelMuted.assign ((size_t) ch, 0);

        loaded      = true;
        tempoFactor = 1.0;

        applyCachedState();
        return true;
    }
    catch (const std::exception& e)
    {
        error = std::string ("libopenmpt error: ") + e.what();
        mod.reset();
        interactive = nullptr;
        loaded = false;
        return false;
    }
}

void ModEngine::unload()
{
    std::lock_guard<std::mutex> g (lock);
    mod.reset();
    interactive = nullptr;
    loaded = false;
}

void ModEngine::applyCachedState()
{
    if (! mod || interactive == nullptr)
        return;

    for (size_t c = 0; c < channelVol.size(); ++c)
    {
        interactive->set_channel_volume ((int32_t) c, channelVol[c]);
        interactive->set_channel_mute_status ((int32_t) c, channelMuted[c] != 0);
    }
    interactive->set_tempo_factor (tempoFactor);
}

std::size_t ModEngine::render (float* interleavedStereo, std::size_t frames, double sampleRate)
{
    std::lock_guard<std::mutex> g (lock);
    if (! mod)
        return 0;

    return mod->read_interleaved_stereo ((int32_t) std::lround (sampleRate),
                                         frames, interleavedStereo);
}

double ModEngine::getDurationSeconds()
{
    std::lock_guard<std::mutex> g (lock);
    return mod ? mod->get_duration_seconds() : 0.0;
}

double ModEngine::getPositionSeconds()
{
    std::lock_guard<std::mutex> g (lock);
    return mod ? mod->get_position_seconds() : 0.0;
}

void ModEngine::setPositionSeconds (double seconds)
{
    std::lock_guard<std::mutex> g (lock);
    if (mod)
        mod->set_position_seconds (std::max (0.0, seconds));
}

int ModEngine::getNumChannels()
{
    std::lock_guard<std::mutex> g (lock);
    return mod ? mod->get_num_channels() : 0;
}

void ModEngine::setChannelVolume (int channel, double volume)
{
    std::lock_guard<std::mutex> g (lock);
    volume = std::clamp (volume, 0.0, 1.0);
    if (channel >= 0 && channel < (int) channelVol.size())
        channelVol[(size_t) channel] = volume;
    if (interactive && mod && channel >= 0 && channel < mod->get_num_channels())
        interactive->set_channel_volume (channel, volume);
}

double ModEngine::getChannelVolume (int channel) const
{
    std::lock_guard<std::mutex> g (lock);
    if (channel >= 0 && channel < (int) channelVol.size())
        return channelVol[(size_t) channel];
    return 1.0;
}

void ModEngine::setChannelMute (int channel, bool muted)
{
    std::lock_guard<std::mutex> g (lock);
    if (channel >= 0 && channel < (int) channelMuted.size())
        channelMuted[(size_t) channel] = muted ? 1 : 0;
    if (interactive && mod && channel >= 0 && channel < mod->get_num_channels())
        interactive->set_channel_mute_status (channel, muted);
}

bool ModEngine::getChannelMute (int channel) const
{
    std::lock_guard<std::mutex> g (lock);
    if (channel >= 0 && channel < (int) channelMuted.size())
        return channelMuted[(size_t) channel] != 0;
    return false;
}

void ModEngine::setTempoFactor (double factor)
{
    std::lock_guard<std::mutex> g (lock);
    factor = std::clamp (factor, 0.25, 4.0);
    tempoFactor = factor;
    if (interactive)
        interactive->set_tempo_factor (factor);
}

double ModEngine::getTempoFactor() const
{
    std::lock_guard<std::mutex> g (lock);
    return tempoFactor;
}

double ModEngine::getCurrentTempo()
{
    std::lock_guard<std::mutex> g (lock);
    return mod ? mod->get_current_tempo() : baseTempo;
}

void ModEngine::setRepeat (bool loopForever)
{
    std::lock_guard<std::mutex> g (lock);
    repeat = loopForever;
    if (mod)
        mod->set_repeat_count (loopForever ? -1 : 0);
}
