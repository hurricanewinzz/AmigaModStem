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
- 4 channel volume sliders + per-channel mute.
- Tempo factor slider; target-BPM slider + BPM LOCK (auto-matches tempo).
- Beatgrid timeline (bar/beat/16th lines + playhead), click/drag to seek.
- Quantize-to-grid; beatjump back/fwd in 1/16,1/8,1/4,1/2,1 bar.
- CRT/neon theme with scanline overlay.
- Cross-platform CMake + README build steps for macOS & Windows.

## Status
Source is complete and build-ready but **NOT compiled/verified** — this web
container cannot build native JUCE/VST apps. User must compile per README.

## Backlog / next
- P1: Real VST3 effect hosting in FX tab (JUCE AudioPluginFormatManager).
- P2: Auto-detect BPM from module tempo/speed.
- P2: Per-channel VU meters; waveform/pattern view on the beatgrid.
- P2: Preset save/recall of mixer + BPM lock state.
