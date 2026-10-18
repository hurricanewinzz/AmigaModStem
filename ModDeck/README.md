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
| Mixer       | 4 independent **channel volume sliders** + per-channel **MUTE**           |
| Tempo       | `TEMPO x` factor slider (0.25×–4×) driving libopenmpt's tempo factor      |
| BPM Lock    | Set a **target BPM** and lock it — tempo auto-matches for syncing          |
| Beatgrid    | Timeline of the module with bar/beat/16th grid lines + live playhead       |
| Quantize    | Snap the playhead to the nearest 1/16 grid line                            |
| Beatjump    | Jump back/forward by **1/16, 1/8, 1/4, 1/2, 1 bar**                        |
| Loop        | When the module ends it loops back to the start (toggleable)               |
| FX tab      | Reserved slot for future VST3 effect hosting (placeholder for now)         |

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

## How to use

1. Open the **Standalone** app (or load the VST3 in your DAW).
2. **LOAD .MOD** → pick a module. The title and total time appear; the beatgrid
   draws the song timeline.
3. **PLAY** to start. Drag the 4 channel sliders to balance voices; **MUTE**
   silences a voice.
4. Set a **TARGET BPM** and hit **BPM LOCK** to keep the tune locked to that
   tempo for syncing. Unlocked, use the **TEMPO x** slider freely.
5. Use **BEATJUMP `<< / >>`** with the size selector to skip around, and
   **QUANTIZE** to snap to the grid.
6. **LOOP** keeps the module repeating from the top when it reaches the end.

---

## Project layout

```
ModDeck/
├── CMakeLists.txt
└── Source/
    ├── ModEngine.{h,cpp}        # libopenmpt wrapper (playback manipulation)
    ├── PluginProcessor.{h,cpp}  # JUCE AudioProcessor + parameters
    ├── PluginEditor.{h,cpp}     # window + tabs + CRT overlay
    └── gui/
        ├── AmigaLookAndFeel.{h,cpp}
        ├── BeatGridComponent.{h,cpp}
        ├── PlayerPage.{h,cpp}
        └── FxPage.{h,cpp}
```

## Notes / roadmap
- Per-channel volume, mute and tempo use libopenmpt's `interactive` extension,
  so the original song data is never modified — only playback.
- The **FX** tab is a reserved placeholder; native VST3 effect hosting
  (JUCE `AudioPluginFormatManager`) can be added there next.
