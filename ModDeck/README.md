# ModDeck — Amiga MOD Player (Standalone + VST3)

A single-window **Amiga `.mod` player deck** built with **JUCE 8** and
**libopenmpt**. Dark CRT / neon demoscene theme. Loads classic 4-channel Amiga
modules (and XM/S3M/IT/MED), plays them with authentic Paula sound, and lets you
**manipulate playback** — per-channel volume, tempo, BPM-lock, a beatgrid,
quantize, beatjump and auto-loop. It does **not** edit the song data itself.

Builds as a **Standalone application** and a **VST3 plugin** on **macOS**
(Intel + Apple Silicon) and **Windows**.

> ⚠️ This project is C++/JUCE native code. It must be **compiled on your own
> machine** with the toolchain below — it is not a web app.

---

## Features

| Area        | What it does                                                              |
|-------------|---------------------------------------------------------------------------|
| Player      | Load `.mod` (also `.xm .s3m .it .med .mptm`), play / pause / stop          |
| Mixer       | 4 independent **channel volume sliders** + **MUTE** + glowing **VU meters**|
| Tempo       | `TEMPO x` factor slider (0.25×–4×) driving libopenmpt's tempo factor      |
| BPM Lock    | Set a **target BPM** and lock it — tempo auto-matches for syncing          |
| Auto-BPM    | Derives the **musical BPM** from the module's own tempo+speed on load      |
| Grid align  | **Rows/beat** + **beats/bar** selectors and a **SET DOWNBEAT** anchor so the grid matches 4/4 or odd signatures |
| Beatgrid    | Timeline of the module with bar/beat/16th grid lines + live playhead       |
| Quantize    | Snap the playhead to the nearest 1/16 grid line (relative to the downbeat) |
| Beatjump    | Jump back/forward by **1/16, 1/8, 1/4, 1/2, 1 bar**                        |
| Loop        | Loop the whole module back to the start when it ends (toggleable)          |
| Loop markers| Set **IN / OUT** points and loop just that **section**                     |
| FX tab      | **Serial VST3 / AU effect chain** — add several, reorder, bypass each, open each GUI, + master **DRY/WET** mix |

---

## Prerequisites (both platforms)

- **CMake ≥ 3.22**
- A **C++17** compiler
- **Git** (JUCE is fetched automatically by CMake)
- **libopenmpt** development package (instructions below)

JUCE itself is downloaded for you on the first configure (via `FetchContent`),
so you do **not** need to install it separately.

---

## macOS build

1. Install tools + libopenmpt (Homebrew):
   ```bash
   brew install cmake pkg-config libopenmpt
   # Xcode command-line tools if you don't have them:
   xcode-select --install
   ```
2. Configure & build:
   ```bash
   cd ModDeck
   cmake -B build -G Xcode
   cmake --build build --config Release
   ```
   (Omit `-G Xcode` to use Makefiles/Ninja if you prefer.)

3. Results:
   - Standalone app: `build/ModDeck_artefacts/Release/Standalone/ModDeck.app`
   - VST3: `build/ModDeck_artefacts/Release/VST3/ModDeck.vst3`
     → copy to `~/Library/Audio/Plug-Ins/VST3/`

For a universal (Intel + Apple Silicon) binary add:
`-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"` at configure time.

---

## Windows build

1. Install **Visual Studio 2022** (Desktop C++ workload) and **CMake**.
2. Install **libopenmpt** via [vcpkg](https://github.com/microsoft/vcpkg):
   ```bat
   git clone https://github.com/microsoft/vcpkg
   .\vcpkg\bootstrap-vcpkg.bat
   .\vcpkg\vcpkg install libopenmpt:x64-windows
   ```
3. Configure & build (point CMake at the vcpkg toolchain):
   ```bat
   cd ModDeck
   cmake -B build -G "Visual Studio 17 2022" -A x64 ^
         -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build build --config Release
   ```
4. Results:
   - Standalone: `build\ModDeck_artefacts\Release\Standalone\ModDeck.exe`
   - VST3: `build\ModDeck_artefacts\Release\VST3\ModDeck.vst3`
     → copy to `C:\Program Files\Common Files\VST3\`
   - Make sure `openmpt.dll` (from vcpkg `installed\x64-windows\bin`) is on the
     PATH or next to `ModDeck.exe` for the standalone to find it.

---

## Linux build (verified)

This project has been **compiled and linked successfully on Linux** (Debian 12,
arm64) producing both the Standalone app and the VST3. Steps:
```bash
sudo apt-get install -y cmake pkg-config ninja-build libopenmpt-dev \
     libasound2-dev libx11-dev libxext-dev libxinerama-dev libxrandr-dev \
     libxcursor-dev libxcomposite-dev libfreetype6-dev libfontconfig1-dev \
     libgl1-mesa-dev libglu1-mesa-dev
cd ModDeck
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```
Artefacts:
- Standalone: `build/ModDeck_artefacts/Release/Standalone/ModDeck`
- VST3: `build/ModDeck_artefacts/Release/VST3/ModDeck.vst3`

---

## How to use

1. Open the **Standalone** app (or load the VST3 in your DAW).
2. **LOAD .MOD** → pick a module. The title + total time appear, the beatgrid
   draws the timeline, and **Auto-BPM** locks the grid to the module's musical
   tempo automatically.
3. **PLAY** to start. Drag the 4 channel sliders to balance voices; **MUTE**
   silences a voice; the **VU meters** show each Amiga voice moving.
4. **Matching the grid to the music (MOD timing is row/speed based):**
   - **ROWS/BEAT** — classic Amiga 4/4 tunes use **4**; change it if a tune
     feels half/double time, then it re-derives the BPM.
   - **BEATS/BAR** — set your time signature numerator (4 = 4/4, 3 = 3/4, …).
   - **SET DOWNBEAT** — play to beat 1 of a bar and press it to anchor the grid
     there, so bar lines and quantize line up with the music.
   - **AUTO BPM** — re-derive and re-lock the musical BPM at any time.
5. Use **BEATJUMP `<< / >>`** with the size selector to skip around, and
   **QUANTIZE** to snap to the grid.
6. **Loop section:** **SET IN** / **SET OUT** at the playhead, enable **SECTION**
   to loop just that region, **CLR** to clear. **LOOP** loops the whole song.
7. **FX tab:** **+ ADD EFFECT** to append a VST3/AU to the chain. Effects run
   top-to-bottom over the module. Per row: **BYP** bypasses just that effect,
   **GUI** opens its editor, **UP/DN** reorder it, **X** removes it. The
   **DRY/WET** knob blends the untouched module against the whole chain.

> Hosting is enabled via `JUCE_PLUGINHOST_VST3=1` (and `JUCE_PLUGINHOST_AU=1` on
> macOS) in `CMakeLists.txt`.

---

## Project layout

```
ModDeck/
├── CMakeLists.txt
└── Source/
    ├── ModEngine.{h,cpp}        # libopenmpt wrapper (playback manipulation + VU + musical BPM)
    ├── FxHost.{h,cpp}           # single-slot VST3/AU effect host
    ├── PluginProcessor.{h,cpp}  # JUCE AudioProcessor + parameters + section loop
    ├── PluginEditor.{h,cpp}     # window + tabs + CRT overlay
    └── gui/
        ├── AmigaLookAndFeel.{h,cpp}
        ├── BeatGridComponent.{h,cpp}   # anchored grid + loop region
        ├── VuMeter.{h,cpp}
        ├── PlayerPage.{h,cpp}
        └── FxPage.{h,cpp}              # effect host UI + plugin window
```

## Notes
- Per-channel volume, mute, tempo and VU use libopenmpt's `interactive`
  extension + VU API, so the original song data is never modified — only playback.
- Musical BPM is derived as `tempo × 24 / (speed × rowsPerBeat)` — e.g. the
  classic 125 tempo / 6 speed / 4 rows-per-beat gives 125 BPM in 4/4.
- The **FX** tab hosts one external VST3/AU effect in series after the module.
