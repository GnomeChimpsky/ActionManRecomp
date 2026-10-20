# Action Man: Mission Xtreme — reverse-engineering notes

Addresses are KUSEG (the boot EXE is linked and runs at `0x000xxxxx`, not
`0x800xxxxx`).

## Boot executable

| | |
|---|---|
| File | `SLES_017.61` (591,872 bytes) |
| Load / entry | `0x00018000` / `0x00085550` |
| Text size | `0x90000` (ends `0x000A8000`) |
| SYSTEM.CNF | `TCB = 4`, `EVENT = 10`, `STACK = 801FFF00` |

The game installs its own exception handler at `0x00018480`, patching a
`lui k0 / addiu k0 / jr k0` trampoline into its own text at `0x000184D8`
at run time; those few instructions run on the dirty-RAM interpreter by design.

## Disc layout

Single MODE2/2352 data track. Root files: `ACTION.DAT` (56.7 MB archive),
`SLES_017.61`, `SYSTEM.CNF`, XA streams (`MUSIC1/2.XA`, `VOICE.XA`,
`CUTSCENE.XA`, `CUTSCEN2.XA`, `DKCAPTUR.XA`, `DSEND.XA`, `ICEND.XA`,
`ICPENT.XA`) and `DUMMY.` padding.

### ACTION.DAT

```
u32 magic = 0x12345678
u32 count = 0x246 (582)
{ u32 name_hash; u32 sector; u32 size } [count]   // sector = 2048-byte units
```

Members 19–28 are the code overlays (MIPS, uncompressed). File names are
hashed; the EXE keeps the overlay paths as strings at `0x00098550`:
`BIN\DOCKS.BIN`, `DOCKS2`, `STREET`, `ICEFLOW`, `COPTER`, `CUTSCENE`,
`DESERT`, `APART`, `ICEBRG`, `LANG`, `CREDITS`, `MODVIEW`, `LOGO`.

### Overlay loader

* `load_overlay(id)` at `0x0002D1E4`; `id` is 1-based, current id at
  halfword `0x000B2E3E`. Jump table `0x00098624`:
  1 DESERT, 2 APART, 3 STREET, 4 CUTSCENE, 5 DOCKS, 6 ICEBRG, 7 ICEFLOW,
  8 COPTER, 9 LANG, 10 CREDITS, 11 MODVIEW, 12 DOCKS2, 13 LOGO.
* Each case calls `read_file(name, dest = *(u32*)0x000B2A90)` (`0x0007AA2C`)
  until it succeeds. `dest` is `0x000C3710`; the overlay image's first
  `0x4000` bytes are not code, so file offset `X` executes at
  `0x000BF710 + X` (e.g. the STREET overlay's level code at `0x000C3C14`).
* Game mode word at `0x000B2AA4` (struct at `0x000B2A94`), switched through
  the table at `0x00098A1C` in the state function around `0x0002E4xx`:
  0 STREET (city driving), 1 DESERT, 2 ICEFLOW, 3 ICEBRG, 4 DOCKS/DOCKS2,
  5 COPTER, 6 APART (HQ front end), 7 CUTSCENE, 8 LANG, 9 CREDITS,
  10 MODVIEW, 11 LOGO. Writing it directly from outside crashes the game
  (inconsistent mission state); the attract-mode demos cycle 1, 4, 6, 7.

## Rendering / widescreen

* GPU display 640×256 (PAL interlace-free), draw offset (320,128); the game
  flips every second VBlank (25 fps) when it has the time.
* Polygon funnels cull a primitive when **all** vertices lie past one edge,
  in signed screen space: X `< -320` (`slti x,0xFEC0`), X `>= 320`
  (`slti x,0x140` / `0x141`), Y `< -128` / `>= 128`. 29 left-edge sites in
  the EXE (listed in `game.toml`), plus two in the STREET overlay at
  `0x000C77D8` / `0x000C78BC`. The right edges are found automatically by
  `auto_screen_x` using the W/H immediate signature.

## Timing

Logic advances once per rendered frame. With `guest_cycle_scale = 4` the game
renders at 50 fps but objects move ~2.3× further per VBlank — the game is not
delta-timed, so CPU overclocking is not a valid frame-rate enhancement.

## Input

DualShock compatible; the game switches the pad to analog and locks it. Left
stick steers / moves.

## Interpolated rendering (amx.enhancement.frame-rate)

In-game main loop `0x0002BE48`, one iteration per game frame:

```
2BFBC  jal 2332C   frame begin: VSync, ClearOTag (OT ctx ptr at 0x0009D650)
2BFC4  jal 2611C   \  logic + scene + HUD into the OT       (replayed span
2BFCC  jal 2DC40   /                                         2BFC4..2BFD4)
2BFDC  jal 234F4   flip: VSync(1), DrawSync(0), VSync(*0x9D66C = 2),
                   PutDispEnv(disp[i]), PutDrawEnv(draw[i]),
                   23438 = finalize OT (23D94) + DrawOTag(7F9E8)
```

* libgpu: PutDispEnv `0x7FC24`, PutDrawEnv `0x7FA58`, DrawSync `0x7F3F4`,
  DrawOTag `0x7F9E8`, ClearOTagR `0x7F8E0`, VSync `0x83AF8`.
* DISPENV `0x000A93C0` (stride 0x14), DRAWENV `0x000A9304` (stride 0x5C),
  buffer index halfword `0x0009D64C`. DISPENV[i] shows the area DRAWENV[1-i]
  draws (y 0 / 256), so at the entry of `234F4` the previous frame waits in
  the rect disp[i] shows next (PENDING flip).
* The flip function is also driven from a VBlank callback during loading
  (`0x236C0`); the plugin only acts when `$ra == 0x0002BFE4`.
* The replayed logic calls the sound-effect start `0x7ABDC` (scans voices with
  SpuGetKeyStatus `0x8BAC0`, reading SPU ADSR registers) and libcd
  (`_cd_cw 0x93A24`, `_cd_sync 0x934DC`, `_cd_ready 0x9375C` behind
  CdControl/B/F, CdSync, CdReady) for XA music. Inside a pass those are
  answered without device access (no voice / accepted / complete / not ready).
  Verified with `PSX_RENDER_PASS_VERIFY=1` across HQ, city, desert, docks and
  cutscene demo modes: 0 span failures, 0 aborts, 0 verify mismatches.
