#include "mod_plugins.h"
#include "cpu_state.h"

/*
 * Language select at startup (US disc SLUS-00887).
 *
 * The European disc boots into a flag screen where the player picks one of
 * ten languages. The US disc still carries that screen (BIN\LANG.BIN, game
 * mode 8, overlay 9), its flag graphics and all eleven text/voice languages,
 * but its new logo screen (BIN\LOGO.BIN, overlay 13) skips it: when the logo
 * ends it requests mode 7 (the intro cutscene) where the European logo screen
 * requests mode 8 (the flag screen). The flag screen then requests the intro
 * itself, so changing that one value is enough.
 *
 *   logo  0x000C0B34  sw $v0, 0x10($v1)  ; $v0 = 7 -> 8
 *   flags 0x000C1EAC  sw $v0, 0x10($v1)  ; leaving for the intro
 *
 * The flag screen counts its languages 0..9 (0 = UK English); 10 is the US
 * disc's own US English. Picking the UK flag keeps the US English text the
 * disc was released with (voices are the same recording for both).
 */

#define AMX_OVERLAY_ID      0x000B00A6u
#define AMX_OVERLAY_LANG    9u
#define AMX_OVERLAY_LOGO    13u
#define AMX_LANGUAGE        0x0009BFF8u
#define AMX_LANG_UK         0u
#define AMX_LANG_US         10u
#define AMX_MODE_INTRO      7u
#define AMX_MODE_FLAGS      8u

#define AMX_LOGO_EXIT       0x000C0B34u
#define AMX_FLAGS_EXIT      0x000C1EACu
#define AMX_SW_MODE_OPCODE  0xAC620010u /* sw $v0, 0x10($v1) */

#define AMX_REG_V0 2

static void amx_logo_exit(struct CPUState* cpu, uint32_t address) {
    (void)address;
    if (psx_mod_read_half(AMX_OVERLAY_ID) != AMX_OVERLAY_LOGO) return;
    if (cpu->gpr[AMX_REG_V0] != AMX_MODE_INTRO) return;
    cpu->gpr[AMX_REG_V0] = AMX_MODE_FLAGS;
    /* The flag screen starts on the flag of the current language, and only
     * has flags for 0..9: the US default 10 would show an empty flag. */
    if (psx_mod_read_word(AMX_LANGUAGE) > 9u) psx_mod_write_word(AMX_LANGUAGE, AMX_LANG_UK);
}

static void amx_flags_exit(struct CPUState* cpu, uint32_t address) {
    (void)address;
    if (psx_mod_read_half(AMX_OVERLAY_ID) != AMX_OVERLAY_LANG) return;
    if (cpu->gpr[AMX_REG_V0] != AMX_MODE_INTRO) return;
    if (psx_mod_read_word(AMX_LANGUAGE) == AMX_LANG_UK)
        psx_mod_write_word(AMX_LANGUAGE, AMX_LANG_US);
}

PSX_MOD_CONSTRUCTOR(amx_register_language_plugin) {
    (void)psx_mod_register_instruction_plugin(
        "amx.language.select", AMX_LOGO_EXIT, AMX_SW_MODE_OPCODE, amx_logo_exit);
    (void)psx_mod_register_instruction_plugin(
        "amx.language.select", AMX_FLAGS_EXIT, AMX_SW_MODE_OPCODE, amx_flags_exit);
}
