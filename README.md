# Action Man Recomp

A static recompilation of **Action Man: Mission Xtreme** (PlayStation, PAL,
`SLES-01761`) into a native PC program, built on
[PSXRecomp](https://github.com/mstan/psxrecomp) v4 and
[recomp-ui](https://github.com/RetroPortingToolKit/recomp-ui), following the
method of [ApeEscapeRecomp](https://github.com/mstan/ApeEscapeRecomp).

The game's MIPS executable is translated ahead of time into C, compiled
into a native program and run on PSXRecomp's PS1 hardware simulation (GPU, SPU,
GTE, CD-ROM, MDEC, memory cards) with the recompiled open-source OpenBIOS.

**This repository contains no game code, data or assets.** Game data is
always read from your own disc image (see [DISC.md](DISC.md)). The
[release downloads](#download-and-play) are ready-to-run builds; see
[Distribution](#distribution) for exactly what they contain.

> Status: **preview**. The game boots and plays through the parts tested so
> far, but a complete playthrough has not been verified. See
> [Known issues](#known-issues).

## Download and play

Releases are published on the
[Releases page](https://github.com/GnomeChimpsky/ActionManRecomp/releases):

| File | What it is |
|---|---|
| `Action-Man-Recomp-<version>-windows-x64.zip` | the game for Windows 10/11 x64 (required) |
| `Action-Man-Recomp-<version>-linux-x64.zip` | the game for Linux x86-64 (glibc 2.35+, OpenGL) |

1. Extract the zip for your platform into a folder of its own.
2. Run `ActionManRecomp.exe` (Windows) or `ActionManRecomp` (Linux).
3. In the launcher, select your own disc image – a raw BIN/CUE of
   *Action Man: Mission Xtreme* (PAL, `SLES-01761`); see [DISC.md](DISC.md).
   Do not convert it to `.iso`.
4. Play. Saves go to `saves/`, settings next to the executable. OpenBIOS is
   included; no BIOS dump is needed.

No developer tools are needed to play. To update, extract the new version
over the old folder. Level code compiled by an older version (`cache/`) is
ignored and rebuilt automatically.

### Launcher settings

The launcher opens before the game. Its **Settings** pages cover:

- **Video** – renderer (OpenGL is recommended), window size and
  fullscreen, internal resolution (up to 8K, default 1440p), antialiasing,
  texture and FMV filtering, screen model and scanlines, VSync, and the
  *Render thread* and *Present thread* options. For a higher frame rate use
  the Frame Rate mod.
- **Audio** – volume and high-quality SPU (sound chip) emulation.
- **Input** – DualShock analog or digital pad, button bindings and
  analog deadzone.
- **System** – BIOS choice (the included OpenBIOS works).
- **Hotkeys** – the keyboard shortcuts below, which can be rebound.

### Mods

The **Mods** page lists the bundled mods. All are off until you enable them:

- **Action Man Widescreen** – 16:9, 21:9 or *Adaptive* (follows the window
  shape), with the HUD kept at its original proportions.
- **Action Man Frame Rate** – draws extra in-between frames at 50 FPS
  (default, recommended) or 100 FPS, while game logic, timers and audio
  keep their original pace. Needs OpenGL; see [Frame rate](#frame-rate).
- **PGXP Precision** – sub-pixel vertex precision and perspective-correct
  textures to reduce polygon wobble. Needs supersampling of 2× or more.
- **CD Speed** – shortens loading by speeding up the emulated CD drive.
  Raise it gradually; very high values can stall a load.
- **HD Texture Packs** – loads DuckStation-compatible replacement textures
  (OpenGL) or dumps the originals. No artwork is included.
- **8 MB Main RAM** and **Fast Loading (host pacing)** – generic options that
  Action Man does not need.

### Hotkeys

Defaults (change them under **Settings › Hotkeys**):

- **Tab** – hold to speed up the game (fast-forward).
- **F7** – save states. Savestates are tied to the exact build and do not
  load across versions.

## Known issues

- Vertex wobble as on original hardware
- In-game options are not saved between sessions
- Minor HUD glitches due to widescreen rendering
- Short judders remain in the heaviest scenes
- Savestates are tied to the build and may not load across versions

## Status

| Area | State |
|---|---|
| Boot (OpenBIOS, BIOS logo skipped) | ✅ language select → intro FMV → title |
| Front end / HQ apartment | ✅ menus, briefings, mission select, options |
| Briefing voice-over (XA audio) | ✅ fixed in 0.4.1 (see below) |
| Gameplay: city streets (driving) | ✅ playable, digital and analog input |
| Attract-mode levels (on-foot base, docks boss, cutscenes) | ✅ run |
| FMV (MDEC) and XA music | ✅ |
| Controller | ✅ DualShock analog by default (the game locks analog mode itself) |
| Internal resolution | ✅ native up to 8K, default 1440p (OpenGL) |
| Widescreen 16:9 / 21:9 / adaptive | ✅ optional mod; HUD kept at 4:3 proportions |
| High frame rate | ✅ optional *Action Man Frame Rate* mod, 50 FPS (default) or 100 FPS |
| PGXP | ✅ built in; enable the bundled PGXP mod |
| Level code streamed from disc | ✅ interpreted on first visit, compiled to native code on your machine as you play |
| Complete playthrough | ⚠️ not yet verified |

### Recent changes

- **0.4.1 – voice-over.** The briefing voice clips in the apartment were cut
  off immediately. The emulated CD drive reported *Read* straight after a
  seeking read command, so the game's XA voice manager never armed its
  end-of-clip timer and paused every clip. The drive now reports *Seek* until
  the seek completes, as the hardware does.
  (`patches/psxrecomp/0001-…`)
- **0.4.2 – mission-select flicker.** With the Frame Rate mod, the HQ
  mission-select picture and the in-game item menu flickered while the camera
  moved. Image uploads the game makes into the frame being drawn are now held
  and applied after that frame's drawing, as on the console.
  (`src/mods/amx_frame_rate_plugin.cpp`)
- **0.4.3 – options-menu flicker in widescreen.** The see-through panels
  behind the apartment option menus flickered between HUD and world scaling.
  (`patches/psxrecomp/0002-…`)
- **0.4.4 – widescreen HUD.** HUD and world are now told apart by where each
  shape's corners come from: anything with a vertex projected by the 3D math
  unit (GTE) is world, every other 2D shape is HUD. This fixes the black
  flickering squares around the language-screen flag and HUD boxes coming
  apart from their outlines and contents; the 3D vehicle icons inside HUD boxes
  now move with their box. (`patches/psxrecomp/0003-…`,
  `[widescreen] auto_ui_provenance` in `game.toml`)
- **0.4.5 – smoother Frame Rate mod.** On City streets the render passes
  switched themselves off after a few seconds of driving (a sound call the
  game makes there faulted inside every pass); it is now skipped during
  passes. The rate choices are now 50 FPS (the new default) and 100 FPS,
  which divide the game's 25 fps evenly; see [Frame rate](#frame-rate).
  The real game frame is now shown between the in-betweens (at 50 FPS only
  the in-between was shown, twice), and presents are locked to the
  emulated VBlanks so they no longer slide against the game when it runs
  slightly behind real time. The release zip now bundles Python and TinyCC
  for the level-code toolchain, so nothing has to be downloaded.
- **0.4.6 – launcher clean-up.** The generic *Smooth motion* option is no
  longer offered (the Frame Rate mod replaces it), and the *FMV Skipping* and
  *Bezel Artwork* mods are removed. The Widescreen and Frame Rate mods are now
  version 1.1.0.
  (`src/mods/amx_frame_rate_plugin.cpp`, `patches/psxrecomp/0004-…`)

Savestates are tied to the exact build and do not load across versions.

## Building

The build has two inputs: this repository and **your own disc image**
(see [DISC.md](DISC.md) for the exact dump that is required).

### Prerequisites

- Linux x86-64 (or WSL2). Native Windows and macOS builds are not tested.
- `git`, `python3` (3.11+; the cmake-clang-v1 pack includes one), and a C/C++ toolchain with CMake ≥ 3.20 and
  Ninja. The recommended toolchain is the portable **cmake-clang-v1** pack
  from [RetroPorting-Toolchains](https://github.com/RetroPortingToolKit/RetroPorting-Toolchains/releases)
  (Clang/LLD, CMake, Ninja, SDL3 and a build sysroot); unpack it and point
  `PSXRECOMP_TOOLCHAIN_DIR` at it, or run its `install.sh`.
- Internet access during the first configure (CMake fetches SDL3, zlib,
  libwebp and libchdr at pinned versions).
- About 2 GB of free disk space (the build keeps a copy of your disc image in `disc/`).
- For Windows binaries: [llvm-mingw](https://github.com/mstorsjo/llvm-mingw/releases)
  (UCRT, x86_64 Linux host), with `LLVM_MINGW_ROOT` pointing at it.

### One command

```sh
git clone --recurse-submodules https://github.com/GnomeChimpsky/ActionManRecomp.git
cd ActionManRecomp
export PSXRECOMP_TOOLCHAIN_DIR=/path/to/cmake-clang-v1   # optional
scripts/build.sh --disc /path/to/ActionMan.cue                 # Linux build
LLVM_MINGW_ROOT=/opt/llvm-mingw scripts/build.sh --windows     # Windows build (reuses generated/)
```

`scripts/build.sh` performs these steps, which you can also run by hand:

1. `git submodule update --init --recursive` – checks out PSXRecomp,
   recomp-ui and their submodules at the upstream commits in
   [`framework_pins.txt`](framework_pins.txt).
2. `scripts/apply_patches.sh` – applies this project's framework changes in
   [`patches/`](patches/) to the submodule working trees (no forks; see
   [Framework patches](#framework-patches)). Safe to re-run.
3. `psxrecomp/tools/ci/build_emitters.sh` – builds the recompiler
   (`psxrecomp-game`, `psxrecomp-bios`) for your machine.
4. `python3 psxrecomp/psxrecomp_cli.py generate --config game.toml --project-root . --disc <cue>`
   – checks your dump against the hashes in `game.toml`, copies it into
   `disc/` and writes the game C into `generated/`. Both are git-ignored and
   must stay private.
5. CMake configure and build of the `psx-runtime` target with
   `-DPSXRECOMP_REQUIRE_GAME_C=ON` (Linux: `build-linux/`; Windows:
   `build-win/`, plus a Windows recompiler in `build-recompiler-win/` for the
   level-code toolchain).

Configuring without `generated/` stops with an explanation rather than
building a program without the game.

### Cross-compiling for Windows

Windows binaries are cross-compiled on Linux with llvm-mingw using
[`cmake/llvm-mingw.cmake`](cmake/llvm-mingw.cmake):

```sh
cmake -S . -B build-win -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/llvm-mingw.cmake \
  -DLLVM_MINGW_ROOT=/opt/llvm-mingw-<version>-ucrt-ubuntu-22.04-x86_64 \
  -DPSX_ENABLE_VULKAN=OFF -DPSXRECOMP_REQUIRE_GAME_C=ON
cmake --build build-win --target psx-runtime
```

The Vulkan renderer is disabled for Windows cross builds; OpenGL is the
default and recommended renderer.

### Packaging a release

```sh
scripts/package_release.sh build-win   windows-x64 build-recompiler-win
scripts/package_release.sh build-linux linux-x64
```

This writes `dist/Action-Man-Recomp-<version>-<platform>.zip` with the executable, OpenBIOS, the
  mods, `overlay_toolchain/` (the generic recompiler and headers plus pinned
  Python and TinyCC, downloaded and SHA-256-checked by the framework packager),
  licences, `LICENSE` and `DISC.md`. The framework packager refuses generated
  C, BIOS dumps, disc images and memory cards.
  No precompiled level-code cache is packaged; each player's copy builds its
  own (see [Level code](#level-code)).

On a machine that cannot download Python and TinyCC, set
`AMX_OFFLINE_TOOLCHAIN=1` to package `overlay_toolchain/` without them (the
layout of the 0.3/0.4 test packages). Windows players then run the included
`get_overlay_toolchain.ps1` once; Linux players use a system `gcc`/`cc`/`clang`
if present.

### Framework patches

The submodules point at unmodified upstream commits. This project's changes
to the framework are kept as patches and applied by `scripts/apply_patches.sh`:

| Patch | What it does |
|---|---|
| `psxrecomp/0001-cdrom-report-Seek-…` | CD drive reports *Seek* while a read seeks to its target (voice-over fix) |
| `psxrecomp/0002-widescreen-auto-UI-admit-see-through-…` | widescreen HUD: see-through backing panels are HUD at any OT rank |
| `psxrecomp/0003-widescreen-auto-UI-classify-HUD-by-vertex-provenance` | widescreen HUD: HUD vs world by vertex provenance (`auto_ui_provenance`) |
| `psxrecomp/0004-render-passes-show-the-real-frame-and-lock-presents-…` | render passes: show the real frame between in-betweens; lock presents to the guest's VBlanks |
| `recomp-ui/0001-PSX-launcher-show-Render-thread-Present-thread-…` | launcher: render thread / present thread settings (no generic Smooth motion) |

Applying them to the pinned commits reproduces the framework trees the
released builds were made from exactly. To update the framework, move the
submodule to a new upstream commit, re-run `scripts/apply_patches.sh`, fix
any patch that no longer applies, then refresh `framework_pins.txt` with
`psxrecomp/tools/ci/record_pins.sh`.

## Playing

1. Build (above) and run the executable in the build folder, or unzip a
   package you made and run `ActionManRecomp.exe`.
2. In the launcher, select your own disc image (`.cue`, see [DISC.md](DISC.md)).
   Keep the raw `.bin`; do not convert it to `.iso`.
3. OpenBIOS is included; a retail BIOS is optional.
4. Optional features on the **Mods** page: *Action Man Widescreen*
   (16:9 / 21:9 / adaptive), *Action Man Frame Rate*, *PGXP*, *CD Speed*. **Settings › Display**: internal
   resolution, antialiasing, texture filtering, render thread.

### The level-code toolchain

The game zip includes `overlay_toolchain/` (the PSXRecomp recompiler plus
pinned Python 3.13.1 and TinyCC 0.9.27), so the game can compile level code
on your machine. If a Windows package lacks the Python and
TinyCC parts (an offline package), run `get_overlay_toolchain.ps1` once from
the game folder: it downloads them and checks their SHA-256. Without them the
game still runs correctly; that code just stays on the interpreter.

### Level code

Action Man loads level code (overlays) from `ACTION.DAT` while it runs. The
static recompiler cannot see that code at build time, so the first time a
level is visited it runs on PSXRecomp's dirty-RAM interpreter. With
`[runtime] overlay_cache = true` (the default here) the runtime records the
code it sees (`overlay_captures.json`) and compiles it to native code in the
background into `cache/`, so later visits run natively. Releases do not
include a precompiled cache; it is built on each player's machine from their
own disc. Developers can pre-build one for their own machine with gcc using
`psxrecomp/tools/compile_overlays.py` (see
`psxrecomp/docs/COMPILING_OVERLAYS.md`). `cache/` and `overlay_captures.json`
contain code from the game: never commit or publish them.

### Frame rate

The game runs its logic once per rendered frame, at 25 fps (PAL 50 Hz / 2)
or lower in heavy scenes, as on a console. Running the emulated CPU faster
also speeds up the game, so it is not used. Instead, the **Action Man Frame
Rate** mod uses PSXRecomp's render passes: between two game frames it
restores the machine to the start of the frame's draw code, replays the
game's own drawing with every 3D projection placed part of the way between
the two frames, and submits the result with the game's own calls. Game logic,
timers and audio keep their original pace. Use it with the OpenGL renderer.
The framework's generic *Smooth motion* option is not offered, because it
causes stutter and texture flicker in this game.

*Presentation rate* offers two settings, both whole multiples of the game's
25 fps:

- **50 FPS** (default, recommended): one in-between per game frame. It is the
  smoothest and fits the time a pass needs at high internal resolutions.
- **100 FPS**: three in-betweens per game frame; needs a faster GPU.

Since 0.4.5 the other rates (display refresh, 60, 75, 90, 120, 144, 165 and
240 FPS) are gone: most do not divide 25 fps evenly and judder, and 75 FPS
looked worse than 50. A rate chosen in an older version is not carried over; the
mod starts at 50 FPS.

## Distribution

What the **repository** contains:

| Item | In the repository? |
|---|---|
| Project code, configuration, mods, scripts, docs | ✅ yes (PolyForm Noncommercial 1.0.0) |
| Framework patches (`patches/`) | ✅ yes (changes to PSXRecomp / recomp-ui, under their licences) |
| Addresses, hashes and reverse-engineering notes about the game (`game.toml`, `seeds/`, `docs/NOTES.md`, mod sources) | ✅ yes – descriptive data, no game code or assets |
| Game C translated from the disc (`generated/`) | ❌ never – generated locally |
| Disc images, extracted game files, BIOS dumps | ❌ never |
| Overlay captures (`overlay_captures.json`) | ❌ never |
| Compiled executables | ❌ not committed; attached to releases |
| Level-code caches (`cache/`) | ❌ never – built on each player's machine |

What the **release downloads** contain:

- **No game assets and no disc data.** Graphics, audio, video, levels and
  text are read from your disc image at run time.
- **The game executable contains a machine translation of the game's program
  code** (the boot executable `SLES_017.61`, translated to C and compiled),
  together with this project's code and the PSXRecomp runtime.
- **No precompiled level code.** Level code is read from your disc and, if
  compiled, compiled on your machine.
- Third-party components: OpenBIOS (MIT), recomp-ui (MIT), PSXRecomp
  (PolyForm Noncommercial 1.0.0), TinyCC (LGPL), Python (PSF) and the
  libraries listed in `licenses/`.

The translated code is derived from the original game, whose copyright
belongs to its owners; it is distributed only to let owners of the game play
their own copy, free of charge. If you are a rights holder and object,
please open an issue.

## Licence and legal

- **This project's own files** are licensed under the
  [PolyForm Noncommercial License 1.0.0](LICENSE): free to use, change and
  share for any noncommercial purpose.
- **Third-party code is under its own licences** and is not covered by
  `LICENSE`:
  - [PSXRecomp](https://github.com/mstan/psxrecomp) – PolyForm Noncommercial
    1.0.0 (Matthew Stanley). The patches in `patches/psxrecomp/` modify it and
    are offered under its terms.
  - [recomp-ui](https://github.com/RetroPortingToolKit/recomp-ui) – MIT.
  - [OpenBIOS](https://github.com/grumpycoders/pcsx-redux) (PCSX-Redux) – MIT.
  - Libraries used by the runtime (SDL3, zlib, libchdr, libwebp, libjuice,
    stb_image, toml11, xxHash, OpenXR SDK, TinyCC and others): see
    `psxrecomp/runtime/licenses/` and the submodules. Packages include all of
    these notices in `licenses/`.
  - `assets/psxrecomp.*` are PSXRecomp's icon, copied from the framework.
- **Action Man: Mission Xtreme**, its code, audio, video and artwork are
  copyright their respective owners. *Action Man* is a trademark of Hasbro.
  This project is not affiliated with or endorsed by Hasbro, Interactive
  Studios or Sony. *PlayStation* is a trademark of Sony Interactive
  Entertainment.
- You must own the original game and supply your own disc image. Do not ask
  for, or share, disc images, game files or generated code in this project's
  issues.

This section describes how the project is organised; it is not legal advice.

## Credits

- **[PSXRecomp](https://github.com/mstan/psxrecomp)** by Matthew Stanley
  (mstan) and contributors – the recompiler, runtime, launcher integration
  and mod system this port is built on.
- **[RetroPortingToolKit](https://github.com/RetroPortingToolKit)** –
  recomp-ui, recomp-net, rbengine and the portable toolchains.
- **[ApeEscapeRecomp](https://github.com/mstan/ApeEscapeRecomp)** – the
  reference for project layout, interpolated rendering and release practice.
- **[PCSX-Redux](https://github.com/grumpycoders/pcsx-redux)** – OpenBIOS.
- **[psx-spx](https://psx-spx.consoledev.net/)** – PlayStation hardware
  documentation.
- **Interactive Studios** and **Hasbro Interactive** – the original game.
- Developed with the help of Claude (Anthropic).

## Reverse-engineering notes

See [`docs/NOTES.md`](docs/NOTES.md).
