#include "mod_plugins.h"

/*
 * Cheats, built on the game's own developer options.
 *
 * The developer menu (0x0002F810) keeps its toggles as bits of the word at
 * 0x000B3270. Bit 27 ("AM infinite health") is read when Action Man's health
 * drops below zero: set, the game refills it to 1000 instead of killing him
 * (0x00048760). Bit 29 is the menu's "Easy Mode". The game clears some of
 * these bits when a mission starts (0x0002F764), so they are re-asserted on
 * every VBlank.
 *
 * All weapons: the inventory at 0x000B5B80 keeps 32 weapon slots at +0x3FC
 * (ids 1..16) and one ammo halfword per weapon id at +0x41C, with the ammo
 * limit per id in the table at 0x0009E79C. Each mission sets its own loadout
 * when it starts, so missing weapons are added (with full ammo) once the
 * loadout is in place; weapons the player already carries are left alone.
 *
 * Unlock all missions: the HQ front end (overlay 2) has its own unlock cheat,
 * a six-step button sequence counted in the byte at 0x000D5F7C. Once the
 * count reaches 6 the mission select (0x000D1544) opens every mission in the
 * save at 0x000B5B80 and plays a sound. The count is set to 6 while that
 * overlay is loaded and the first mission word (+0x210) is not yet all ones,
 * so the game's own routine does the unlocking once. Other overlays use the
 * same address, so the overlay's code at 0x000D1548 is checked first.
 */

#define AMX_DEBUG_FLAGS      0x000B3270u
#define AMX_FLAG_INFINITE_HP 0x08000000u
#define AMX_FLAG_EASY_MODE   0x20000000u

#define AMX_GAME_MODE        0x000B2AA4u /* 0..5 = levels, 6+ = front end */
#define AMX_INVENTORY        0x000B5B80u
#define AMX_WEAPON_SLOTS     (AMX_INVENTORY + 0x3FCu)
#define AMX_WEAPON_SLOT_COUNT 32u
#define AMX_AMMO             (AMX_INVENTORY + 0x41Cu)
#define AMX_AMMO_LIMITS      0x0009E79Cu

#define AMX_OVERLAY_ID       0x000B2E3Eu
#define AMX_OVERLAY_HQ       2u
#define AMX_HQ_CHEAT_CHECK   0x000D1548u /* lbu $v0, 0x5f7c($v1) */
#define AMX_HQ_CHEAT_OPCODE  0x90625F7Cu
#define AMX_HQ_CHEAT_COUNT   0x000D5F7Cu
#define AMX_HQ_CHEAT_STEPS   6u
#define AMX_MISSIONS         (AMX_INVENTORY + 0x210u)

/* The weapons that have ammo pickups or appear in a mission loadout:
 * handgun, machine gun, rocket launcher, grenade launcher, longbow, sniper
 * rifle, crossbow, grenades, sonic gun, tazer gun, and the default pistol. */
static const uint8_t amx_weapons[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0x10};

static void amx_set_debug_flag(uint32_t flag) {
    uint32_t flags = psx_mod_read_word(AMX_DEBUG_FLAGS);
    if (!(flags & flag)) psx_mod_write_word(AMX_DEBUG_FLAGS, flags | flag);
}

static void amx_invincibility_vblank(void) {
    amx_set_debug_flag(AMX_FLAG_INFINITE_HP);
}

static void amx_easy_mode_vblank(void) {
    amx_set_debug_flag(AMX_FLAG_EASY_MODE);
}

static int amx_has_weapon(uint8_t id, uint32_t* free_slot) {
    uint32_t i;
    *free_slot = AMX_WEAPON_SLOT_COUNT;
    for (i = 0; i < AMX_WEAPON_SLOT_COUNT; ++i) {
        uint8_t slot = psx_mod_read_byte(AMX_WEAPON_SLOTS + i);
        if (slot == id) return 1;
        if (!slot && *free_slot == AMX_WEAPON_SLOT_COUNT) *free_slot = i;
    }
    return 0;
}

static void amx_all_weapons_vblank(void) {
    uint32_t i;
    if (!psx_mod_game_started()) return;
    if (psx_mod_read_word(AMX_GAME_MODE) > 5u) return;
    /* An empty first slot means the mission loadout is not in place yet. */
    if (!psx_mod_read_byte(AMX_WEAPON_SLOTS)) return;
    for (i = 0; i < sizeof amx_weapons; ++i) {
        uint8_t id = amx_weapons[i];
        uint32_t slot;
        if (amx_has_weapon(id, &slot)) continue;
        if (slot == AMX_WEAPON_SLOT_COUNT) return;
        psx_mod_write_byte(AMX_WEAPON_SLOTS + slot, id);
        psx_mod_write_half(AMX_AMMO + 2u * id,
                           (uint16_t)psx_mod_read_word(AMX_AMMO_LIMITS + 4u * id));
    }
}

static void amx_unlock_missions_vblank(void) {
    if (!psx_mod_game_started()) return;
    if (psx_mod_read_half(AMX_OVERLAY_ID) != AMX_OVERLAY_HQ) return;
    if (psx_mod_read_word(AMX_HQ_CHEAT_CHECK) != AMX_HQ_CHEAT_OPCODE) return;
    if (psx_mod_read_word(AMX_MISSIONS) == 0xFFFFFFFFu) return;
    if (psx_mod_read_byte(AMX_HQ_CHEAT_COUNT) < AMX_HQ_CHEAT_STEPS)
        psx_mod_write_byte(AMX_HQ_CHEAT_COUNT, AMX_HQ_CHEAT_STEPS);
}

PSX_MOD_CONSTRUCTOR(amx_register_cheats_plugin) {
    (void)psx_mod_register_vblank_plugin(
        "amx.cheats.invincibility", amx_invincibility_vblank);
    (void)psx_mod_register_vblank_plugin(
        "amx.cheats.all-weapons", amx_all_weapons_vblank);
    (void)psx_mod_register_vblank_plugin(
        "amx.cheats.easy-mode", amx_easy_mode_vblank);
    (void)psx_mod_register_vblank_plugin(
        "amx.cheats.unlock-missions", amx_unlock_missions_vblank);
}
