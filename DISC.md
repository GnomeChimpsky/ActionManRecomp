# Disc requirements

ActionManRecomp ships no game files of any kind. To build it, and to play it,
you need your own copy of the game, dumped from a disc you own.

## Supported release

| | |
|---|---|
| Title | Action Man: Operation Extreme (NTSC-U, North America) |
| Serial | `SLUS-00887` |
| Boot executable | `SLUS_008.87` |
| Publisher / developer | Hasbro Interactive / Interactive Studios, 2000 |

Other releases are **not** supported: the European *Action Man: Mission
Xtreme* (`SLES-01761`, supported up to 0.5.0), re-releases with a different
serial and demo discs. The recompiled code is generated from this exact
executable. The US disc carries all ten European languages and their
voice-over; the *Language Select* mod makes them selectable.

## Expected dump

A single `MODE2/2352` data track in raw BIN/CUE form:

| Data track (`.bin`) | Value |
|---|---|
| Size | 553,089,264 bytes |
| CRC32 | `9d4dbbcb` |
| MD5 | `8b0b65e5ea7c3431ab0bb7391c81118d` |
| SHA-1 | `d0e4834cb0ee6aca998223c8068872972da1c869` |

These are the values `game.toml` (`[prepare_disc]`) checks. They come from the
dump this project was developed and tested with; the SHA-1 matches the
TASVideos-verified dump of `SLUS-00887`.

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
2. copies the image into `disc/` and extracts `SLUS_008.87` there;
3. translates `SLUS_008.87` into C under `generated/`.

`disc/` and `generated/` are git-ignored: they are copies of, or derived
from, the copyrighted game and must never be committed or published.

At run time the game reads all data (levels, audio, FMV, level code) from the
disc image you point the launcher at. Nothing is downloaded.

## BIOS

No BIOS dump is needed. Builds use the open-source
[OpenBIOS](https://github.com/grumpycoders/pcsx-redux) from PCSX-Redux (MIT),
recompiled by PSXRecomp. A retail `SCPH1001.BIN` can be selected in the
launcher but is optional and never included.
