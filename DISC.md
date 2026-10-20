# Disc requirements

ActionManRecomp ships no game files of any kind. To build it, and to play it,
you need your own copy of the game, dumped from a disc you own.

## Supported release

| | |
|---|---|
| Title | Action Man: Mission Xtreme (PAL, Europe; sold in the UK as *Action Man: Mission Extreme*) |
| Serial | `SLES-01761` |
| Boot executable | `SLES_017.61` |
| Publisher / developer | Hasbro Interactive / Interactive Studios, 2000 |

Other releases (the North American *Action Man: Operation Extreme*,
`SLUS-00887`, re-releases with a different serial, demo discs) are **not**
supported. The recompiled code is generated from this exact executable.

## Expected dump

A single `MODE2/2352` data track in raw BIN/CUE form:

| Data track (`.bin`) | Value |
|---|---|
| Size | 552,705,888 bytes |
| CRC32 | `34eedfb3` |
| MD5 | `8e7ca51f688b0e4fa9e2e36bb6a4e8d1` |
| SHA-1 | `0b259a00da8ff2f7d3c3da7eb7954e2b30dd0730` |

These are the values `game.toml` (`[prepare_disc]`) checks. They come from the
dump this project was developed and tested with. A clean dump of
`SLES-01761` made with a Redump-style tool is expected to match, but the
values have not been compared against the Redump database entry yet.

A matching `.cue` looks like this (the file name may differ):

```
FILE "ActionMan.bin" BINARY
  TRACK 01 MODE2/2352
    INDEX 01 00:00:00
```

### Formats that do not work

- **`.iso` / 2048-byte images.** They drop the Mode 2 Form 2 sectors that
  carry the XA voice-over, music and FMV audio.
- **Compressed or patched images** (`.chd`, `.pbp`, `.ecm`, translations,
  "fixed" or trimmed dumps): the hashes above will not match. Convert back to
  the original BIN/CUE first.

## What the build does with the disc

`scripts/build.sh --disc <your.cue>` runs
`psxrecomp_cli.py generate`, which:

1. checks the track against the size and hashes above (a mismatch stops the
   build);
2. copies the image into `disc/` and extracts `SLES_017.61` there;
3. translates `SLES_017.61` into C under `generated/`.

`disc/` and `generated/` are git-ignored: they are copies of, or derived
from, the copyrighted game and must never be committed or published.

At run time the game reads all data (levels, audio, FMV, level code) from the
disc image you point the launcher at. Nothing is downloaded.

## BIOS

No BIOS dump is needed. Builds use the open-source
[OpenBIOS](https://github.com/grumpycoders/pcsx-redux) from PCSX-Redux (MIT),
recompiled by PSXRecomp. A retail `SCPH1001.BIN` can be selected in the
launcher but is optional and never included.
