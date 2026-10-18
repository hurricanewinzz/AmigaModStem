# ModDeck — Amiga MOD Player (native JUCE Standalone + VST3)

## Original problem statement
Standalone VST, Amiga MOD player like OctaMED but playback-only (no song/sound
editing). Per-track volume sliders (4 Amiga channels), single-window Amiga
gaming theme, tempo + BPM lock for sync, beatgrid to track/quantize the .mod
timeline, beatjump in 1/16…1 increments, loop back to start when the mod ends,
and a custom tab to load other VSTs (effects). Must run on macOS and Windows.

## User choices (clarified)
- Delivery: **native C++/JUCE VST3 + Standalone** (NOT web).
- Engine: **libopenmpt** for authentic Amiga 4-channel .mod playback.
- Effects/VST hosting: **deferred** (FX tab is a reserved placeholder for now).
- Theme: **dark CRT / neon demoscene**.
- BPM: manual target + lock (defaults).
- Platforms: **macOS (Intel+ARM) and Windows**.

## Architecture
- JUCE 8 (FetchContent) + libopenmpt (Homebrew/pkg-config on mac, vcpkg on Win).
- `ModEngine` wraps `openmpt::module_ext` + interactive ext for live per-channel
  volume/mute + tempo factor + seek; song data never edited.
- `ModDeckProcessor` (AudioProcessor + APVTS params: ch1-4 vol, tempoFactor).
- UI: TabbedComponent [PLAYER | FX], AmigaLookAndFeel, BeatGridComponent.

## Implemented (2026-06)
- Load .mod/.xm/.s3m/.it/.med; play/pause/stop; auto-loop to start on end.
- 4 channel volume sliders + per-channel mute + glowing VU meters (libopenmpt VU).
- Tempo factor slider; target-BPM slider + BPM LOCK.
- Auto-BPM: musical BPM derived from module tempo×24/(speed×rowsPerBeat); auto-locks on load.
- Grid alignment: ROWS/BEAT + BEATS/BAR selectors + SET DOWNBEAT anchor (match 4/4 or odd).
- Beatgrid timeline (anchored bar/beat/16th + playhead), click/drag to seek.
- Quantize-to-grid (relative to downbeat); beatjump 1/16..1 bar.
- Loop markers: SET IN/OUT + SECTION loop + CLR; drawn on beatgrid.
- FX tab: serial VST3/AU effect **chain** — add multiple, reorder (UP/DN), bypass each (BYP), open each GUI; master **DRY/WET** mix (fxMix param) blends dry module vs chain; audio routed through the chain in order.
- CRT/neon theme; cross-platform CMake (JUCE_PLUGINHOST_VST3/AU=1) + README for macOS & Windows.

## Status
Source complete and **compiled + linked successfully on Linux** (Debian 12
arm64) → Standalone + VST3 both build with zero errors against JUCE 8.0.4 +
libopenmpt 0.6.9. macOS/Windows use the same cross-platform CMake (not built
here — compile per README). GUI not run in the headless container (no display).

## Backlog / next
- Tap-tempo for BPM; nudge buttons for fine grid alignment.
- Drag-to-reorder FX rows (currently UP/DN buttons); per-slot wet/dry.
- Per-channel VU peak-hold numeric dB; waveform render on beatgrid.
- Preset save/recall of mixer + grid + FX chain state.
