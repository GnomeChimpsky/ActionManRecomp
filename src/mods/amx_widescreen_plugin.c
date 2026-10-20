#include "mod_plugins.h"

/*
 * Action Man: Mission Xtreme widescreen.
 *
 * The projection squash, cull widening and HUD proportion correction are
 * framework configuration ([widescreen] in game.toml). These trusted
 * activation callbacks only choose the presented aspect when the player
 * enables the game-owned mod package.
 */
static void amx_widescreen_16_9_activate(void) {
    (void)psx_mod_set_fixed_display_aspect(16u, 9u);
}

static void amx_widescreen_21_9_activate(void) {
    (void)psx_mod_set_fixed_display_aspect(21u, 9u);
}

static void amx_widescreen_adaptive_activate(void) {
    (void)psx_mod_set_fixed_display_aspect(16u, 9u);
    (void)psx_mod_set_adaptive_display_aspect(21u, 9u);
}

PSX_MOD_CONSTRUCTOR(amx_register_widescreen_plugins) {
    (void)psx_mod_register_activation_plugin(
        "amx.widescreen.16-9", amx_widescreen_16_9_activate);
    (void)psx_mod_register_activation_plugin(
        "amx.widescreen.21-9", amx_widescreen_21_9_activate);
    (void)psx_mod_register_activation_plugin(
        "amx.widescreen.adaptive", amx_widescreen_adaptive_activate);
}
