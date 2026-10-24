# Release notes

## 0.4.5

**Downloads**:

- `Action-Man-Recomp-0.4.5-windows-x64.zip`

**Changes since 0.4.4**

- Frame Rate mod: render passes no longer switch off while driving on City
  streets (a sound call inside the pass faulted).
- Frame Rate mod: only 50 FPS (the new default) and 100 FPS remain, which
  divide the game's 25 fps evenly. Display refresh and the 60 – 240 FPS
  choices are removed; a rate chosen in 0.4.4 is reset to 50 FPS.
- Frame Rate mod: the real game frame is now shown between the in-betweens,
  and presents are locked to the emulated VBlanks, which removes most of the
  judder on City streets.
- The Windows zip bundles Python and TinyCC for the level-code toolchain;
  nothing has to be downloaded.

**Known issues.** As in 0.4.4, plus minor HUD glitches due to widescreen
rendering. Short judders remain in the heaviest scenes. Savestates are tied
to the build and may not load across versions.

## 0.4.4

First public release.

**Downloads** (extract the zip for your platform, run the game and select
your own disc image; see the README):

- `Action-Man-Recomp-0.4.4-windows-x64.zip`
- `Action-Man-Recomp-0.4.4-linux-x64.zip`

**Changes since the 0.3/0.4 test builds**

- Briefing voice-over plays in full (CD drive reports *Seek* while seeking).
- Frame Rate mod: no more flicker of the HQ mission-select picture and the
  in-game item menu.
- Widescreen: the apartment option menus no longer flicker; HUD and world are
  separated by vertex provenance (fixes the language-screen flag squares and
  HUD boxes drifting from their outlines).
- First Linux build.

**Contents.** No game assets, disc data or BIOS dump: graphics, audio, video
and levels are read from your disc image. The executable contains a machine
translation of the game's program code; level code is compiled on your
machine. OpenBIOS (MIT) is included.

**Known issues.** Vertex wobble as on the original hardware; a complete
playthrough has not been verified; in-game options are not saved between
sessions; uneven frame pacing in some levels; the first-person bow-aiming HUD
is missing some graphical elements; savestates from the test builds do not
load.
