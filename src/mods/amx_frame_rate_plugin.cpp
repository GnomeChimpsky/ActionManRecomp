#include "render_pass_replay.hpp"

#include <vector>

/*
 * Action Man: Mission Xtreme -- interpolated rendering with the game's own
 * draw code (docs/RENDER_PASSES.md, same method as Ape Escape's plugin).
 *
 * In-game main loop (0x0002BE48), once per game frame:
 *   2BFBC  jal 2332C        frame begin: VSync, ClearOTag of the next OT
 *   2BFC4  jal 2611C        \  logic + scene / HUD build into the OT
 *   2BFCC  jal 2DC40        /  (the replayed draw section)
 *   2BFDC  jal 234F4        flip: DrawSync(0), VSync(n), PutDispEnv(disp[i]),
 *                           PutDrawEnv(draw[i]), 23438 = finalize + DrawOTag
 *
 * DISPENV[i] (0x000A93C0, stride 0x14) shows the area DRAWENV[1-i]
 * (0x000A9304, stride 0x5C) draws; i is the halfword at 0x0009D64C. So at the
 * entry of 234F4 the previous frame is drawn and waits (PENDING flip) in the
 * rect disp[i] will show. Each pass restores RAM to the section entry, replays
 * the section with GTE projections placed between the last two frames, and
 * submits the result into that waiting rect with the game's own calls.
 */
extern "C" uint32_t psx_projection_replay_kept_total(void);

namespace {
constexpr uint32_t Start = 0x0002BFC4, StartWord = 0x0C009847;  // jal 0x0002611C
constexpr uint32_t Stop = 0x0002BFD4;                            // after jal 0x0002DC40
constexpr uint32_t Submit = 0x000234F4, SubmitReturn = 0x0002BFE4;
constexpr uint32_t BufferIndex = 0x0009D64C;
constexpr uint32_t DispEnv = 0x000A93C0, DispStride = 0x14;
constexpr uint32_t DrawEnv = 0x000A9304, DrawStride = 0x5C;
constexpr uint32_t PutDrawEnv = 0x0007FA58, DrawSync = 0x0007F3F4;
constexpr uint32_t FlushOT = 0x00023438;  // finalize OT + DrawOTag
// Sound-effect start: scans voices with SpuGetKeyStatus (reads SPU ADSR
// volume registers, which a pass may not read). Inside a pass report "no free
// voice" (-1), the game's own result when every voice is busy; SPU writes are
// dropped by the sandbox anyway and RAM is restored after the pass.
constexpr uint32_t PlaySound = 0x0007ABDC;
// libspu SpuSetMute(on_off): read-modify-write of SPUCNT (0x1F801DAA), which a
// pass may not read. The game logic calls it through 0x0007B888/0x0007B8A8
// while driving on City streets; inside a pass return its result (on_off)
// untouched. Without this every pass there faulted and passes switched
// themselves off for the session after eight.
constexpr uint32_t SpuSetMute = 0x0008CD60;
// libcd primitives behind CdControl/CdControlB/CdControlF/CdSync/CdReady
// (XA music control from the game logic). Inside a pass the CD registers are
// off limits and its interrupts never arrive, so answer as an idle drive:
// command accepted (0), sync complete (CdlComplete = 2), nothing ready (0).
constexpr uint32_t CdCommand = 0x00093A24, CdSyncWait = 0x000934DC, CdReadyPoll = 0x0009375C;
// libgpu LoadImage(RECT*, u_long*). The HQ mission-select picture (APART
// overlay 0x000CB488, and its row-by-row wipe at 0x000CB45C) is uploaded
// straight into the frame buffer the previous flip finished -- the rect a pass
// draws into -- so on the console it lands on top of that finished frame. A
// replayed section uploads it BEFORE the pass's own OT is drawn, which then
// covers it: pass frames lacked the picture and the panel flickered while the
// camera moved. Inside a pass, uploads into the pass rect are held and applied
// after the OT, in their original order.
constexpr uint32_t LoadImage = 0x0007F6A0;

PSXDrawReplay replay;
uint32_t vblanks = 0;

struct HeldUpload { uint16_t rect[4]; uint32_t source; };
std::vector<HeldUpload> held;
bool releasing = false;
uint16_t pass_rect[4];  // x, y, w, h of the rect the current pass draws

void tick() { if (!g_psx_render_pass_active) ++vblanks; }

void begin(CPUState* cpu, uint32_t) {
    if (g_psx_render_pass_active) return;
    replay.capture(cpu, Start, Stop);
}

uint32_t index_now() { return psx_mod_read_half(BufferIndex) & 1u; }

int pass(CPUState* cpu, void*, uint32_t alpha) {
    if (!replay.restore(cpu, alpha)) return 0;
    // The waiting rect disp[i] (= the area DRAWENV[1-i] draws) is the pass rect.
    const uint32_t disp = DispEnv + DispStride * index_now();
    for (uint32_t k = 0; k < 4; ++k) pass_rect[k] = psx_mod_read_half(disp + 2 * k);
    held.clear();
    if (!replay.draw(cpu)) { psx_mod_counter_add("amx.fr.draw_failed", 1); return 0; }
    const uint32_t other = DrawEnv + DrawStride * (index_now() ^ 1u);
    PSXDrawReplay::call(cpu, PutDrawEnv, other);
    PSXDrawReplay::call(cpu, FlushOT);
    PSXDrawReplay::call(cpu, DrawSync, 0);
    if (!held.empty()) {
        // RECT scratch below the stack pointer; RAM is restored after the pass.
        const uint32_t rect = (cpu->gpr[29] - 0x100u) & ~7u;
        releasing = true;
        for (const HeldUpload& u : held) {
            for (uint32_t k = 0; k < 4; ++k) psx_mod_write_half(rect + 2 * k, u.rect[k]);
            PSXDrawReplay::call(cpu, LoadImage, rect, u.source);
        }
        releasing = false;
        PSXDrawReplay::call(cpu, DrawSync, 0);
        psx_mod_counter_add("amx.fr.held_uploads", held.size());
        held.clear();
    }
    psx_mod_counter_add("amx.fr.interpolated_projections", replay.stats.replayed);
    {
        static uint32_t last = 0;
        const uint32_t now = psx_projection_replay_kept_total();
        psx_mod_counter_add("amx.fr.replay_kept", now - last);
        last = now;
    }
    return 1;
}

void finish(CPUState* cpu, uint32_t) {
    // Only the in-game main loop's flip; loading screens flip from a VBlank
    // callback through the same function.
    if (g_psx_render_pass_active || cpu->gpr[31] != SubmitReturn) return;
    // The game's own first wait in 234F4: the previous frame finishes drawing
    // into the rect the next flip shows before that rect is captured.
    PSXDrawReplay::call(cpu, DrawSync, 0);
    const bool ready = replay.prepare(vblanks);
    psx_mod_counter_add("amx.fr.frames", 1);
    psx_mod_counter_add("amx.fr.projections", replay.stats.captured);
    psx_mod_counter_add("amx.fr.matched", replay.stats.matched);
    psx_mod_counter_add("amx.fr.moving", replay.stats.changed);
    psx_mod_counter_add("amx.fr.cuts", replay.stats.discontinuities);
    if (!ready) return;
    const uint32_t disp = DispEnv + DispStride * index_now();
    PSXModRenderPassFrame frame{};
    frame.struct_size = sizeof frame;
    frame.period_vblanks = replay.ticks;
    frame.x = psx_mod_read_half(disp);
    frame.y = psx_mod_read_half(disp + 2);
    frame.w = psx_mod_read_half(disp + 4);
    frame.h = psx_mod_read_half(disp + 6);
    psx_mod_counter_add("amx.fr.passes", psx_mod_render_pass_frame(cpu, &frame, pass, nullptr));
}

void play_sound(CPUState* cpu, uint32_t) {
    if (!g_psx_render_pass_active) return;
    cpu->gpr[2] = 0xFFFFFFFFu;
    psx_mod_finish_function(cpu);
}

void spu_set_mute(CPUState* cpu, uint32_t) {
    if (!g_psx_render_pass_active) return;
    cpu->gpr[2] = cpu->gpr[4];
    psx_mod_finish_function(cpu);
}

void load_image(CPUState* cpu, uint32_t) {
    if (!g_psx_render_pass_active || releasing) return;
    const uint32_t rect = cpu->gpr[4];
    HeldUpload u{};
    for (uint32_t k = 0; k < 4; ++k) u.rect[k] = psx_mod_read_half(rect + 2 * k);
    u.source = cpu->gpr[5];
    const bool inside = u.rect[0] >= pass_rect[0] && u.rect[1] >= pass_rect[1] &&
                        u.rect[0] + u.rect[2] <= pass_rect[0] + pass_rect[2] &&
                        u.rect[1] + u.rect[3] <= pass_rect[1] + pass_rect[3];
    if (!inside || held.size() >= 512) return;  // texture/CLUT uploads run as usual
    held.push_back(u);
    cpu->gpr[2] = 0;
    psx_mod_finish_function(cpu);
}

void cd_command(CPUState* cpu, uint32_t) {
    if (!g_psx_render_pass_active) return;
    cpu->gpr[2] = 0;
    psx_mod_finish_function(cpu);
}
void cd_sync(CPUState* cpu, uint32_t) {
    if (!g_psx_render_pass_active) return;
    cpu->gpr[2] = 2;
    psx_mod_finish_function(cpu);
}
void cd_ready(CPUState* cpu, uint32_t) {
    if (!g_psx_render_pass_active) return;
    cpu->gpr[2] = 0;
    psx_mod_finish_function(cpu);
}

void activate(unsigned fps) {
    replay.invalidate();
    // The top-down City camera turns up to ~20 degrees a frame high above the
    // street; at the default 512 units/tick those turns moved distant
    // geometry past the cut limit and every such frame snapped instead of
    // blending (an in-between frame that sat on the previous one).
    replay.max_move = 8192.0;
    vblanks = 0;
    PSXDrawReplay::rate(fps, PSX_MOD_RENDER_PASS_FLIP_PENDING);
}
void rate50() { activate(50); }
void rate100() { activate(100); }
}  // namespace

PSX_MOD_CONSTRUCTOR(amx_register_frame_rate_plugins) {
    const char* ids[] = {"amx.framerate.50", "amx.framerate.100"};
    void (*callbacks[])() = {rate50, rate100};
    for (unsigned i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i) {
        psx_mod_register_activation_plugin(ids[i], callbacks[i]);
        psx_mod_register_vblank_plugin(ids[i], tick);
        psx_mod_register_instruction_plugin(ids[i], Start, StartWord, begin);
        psx_mod_register_function_entry_plugin(ids[i], Submit, finish);
        psx_mod_register_function_entry_plugin(ids[i], PlaySound, play_sound);
        psx_mod_register_function_entry_plugin(ids[i], SpuSetMute, spu_set_mute);
        psx_mod_register_function_entry_plugin(ids[i], CdCommand, cd_command);
        psx_mod_register_function_entry_plugin(ids[i], CdSyncWait, cd_sync);
        psx_mod_register_function_entry_plugin(ids[i], CdReadyPoll, cd_ready);
        psx_mod_register_function_entry_plugin(ids[i], LoadImage, load_image);
    }
}
