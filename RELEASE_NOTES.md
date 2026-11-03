# Release notes

## 0.6.0

**Downloads**:

- `Action-Man-Recomp-0.6.0-windows-x64.zip`
- `Action-Man-Recomp-0.6.0-linux-x64.zip`

**Changes since 0.5.0**

- Now built from the US disc, *Action Man: Operation Extreme* (`SLUS-00887`),
  running at its native 60 Hz. The PAL code was sped up for 50 Hz, so the US
  code gives the right speed and timing at 60 Hz. The PAL disc is no longer
  supported; PAL owners can stay on 0.5.0.
- Mods: new *Action Man Language Select* (on by default) brings back the
  European flag screen: ten languages with their voice-over, English uses the
  US text.
- Mods: *Action Man Frame Rate* now offers 60 FPS (default) and 120 FPS,
  locked to the NTSC VBlanks, and shows each in-between at its proper time.
  In 20 fps scenes at 60 FPS one image used to stay up ~30 ms and the next
  ~4 ms; frame times are now mostly within 2 ms of target.
- Mods: *Action Man Widescreen* and *Action Man Cheats* are ported to the US
  disc. The bow scope no longer briefly flashes apart when a floor polygon
  touches the camera's near plane.
- Launcher: *Antialiasing* is off by default and shown as *Off* / *Bilinear*,
  which is what it is (the old 2x/4x/8x choices all did the same). *Render
  thread* and *Present thread* are greyed out while the Frame Rate mod is on.
  The *8 MB Main RAM* and *Fast Loading* mods are removed.
- Installing in a deep folder (paths over 260 characters) no longer stops the
  level-code cache from loading, which made everything run interpreted with a
  ~120 ms freeze every 6 seconds. The background compiler also backs off when
  a compile does not help.

**Known issues**

- Vertex wobble as on original hardware
- Minor HUD glitches due to widescreen rendering
- Frame Rate mod: mild uneven cadence in 30 fps scenes at 60 FPS; 120 FPS
  pacing is uneven
- A fresh install judders on the first visit to each area until that level's
  code has been compiled
- Savestates are tied to the build and do not load across versions (PAL
  savestates do not load)

## 0.5.0

**Downloads**:

- `Action-Man-Recomp-0.5.0-windows-x64.zip`
- `Action-Man-Recomp-0.5.0-linux-x64.zip`

**Changes since 0.4.6**

- Widescreen: the bow's scope view stays in one piece. The scope mask no
  longer has holes, the scope is round, the zoom meter sits in its panel and
  the first-person bow's wings stay attached.

**Known issues**

- Vertex wobble as on original hardware
- Minor HUD glitches due to widescreen rendering
- Short judders remain in the heaviest scenes
- Savestates are tied to the build and may not load across versions

## 0.4.6

**Downloads**:

- `Action-Man-Recomp-0.4.6-windows-x64.zip`
- `Action-Man-Recomp-0.4.6-linux-x64.zip`

**Changes since 0.4.5**

- Launcher: the generic *Smooth motion* option is removed from
  Settings › Display (use the Frame Rate mod). If it was on, it is turned off.
- Mods: *Action Man FMV Skipping* and *Bezel Artwork* are removed.
- Mods: *Action Man Widescreen* and *Action Man Frame Rate* are now
  version 1.1.0.
- Mods: new *Action Man Cheats* (Cheats group): Invincibility, All weapons,
  Easy Mode and Unlock all missions, built on the game's own developer options
  and unlock cheat.

**Known issues**

- Vertex wobble as on original hardware
- Minor HUD glitches due to widescreen rendering
- Short judders remain in the heaviest scenes
- Savestates are tied to the build and may not load across versions

## 0.4.5

**Downloads**:

- `Action-Man-Recomp-0.4.5-windows-x64.zip`
- `Action-Man-Recomp-0.4.5-linux-x64.zip`

**Changes since 0.4.4**

- Frame Rate mod: render passes no longer switch off while driving on City
  streets (a sound call inside the pass faulted).
- Frame Rate mod: only 50 FPS (the new default) and 100 FPS remain, which
  divide the game's 25 fps evenly. Display refresh and the 60 – 240 FPS
  choices are removed; a rate chosen in 0.4.4 is reset to 50 FPS.
- Frame Rate mod: the real game frame is now shown between the in-betweens,
  and presents are locked to the emulated VBlanks, which removes most of the
  judder on City streets.
- Both zips bundle Python and TinyCC for the level-code toolchain;
  nothing has to be downloaded.

**Known issues**

- Vertex wobble as on original hardware
- Minor HUD glitches due to widescreen rendering
- Short judders remain in the heaviest scenes
- Savestates are tied to the build and may not load across versions

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
