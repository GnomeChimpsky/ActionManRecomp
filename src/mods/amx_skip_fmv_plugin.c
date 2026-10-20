#include "mod_plugins.h"

/*
 * Skip FMVs: enable the runtime's accelerated, muted movie playback. The game
 * still runs its own movie teardown, so completion behaves exactly as when a
 * movie finishes normally.
 */
static void amx_skip_fmvs_activate(void) {
    (void)psx_mod_set_auto_skip_fmv(1);
}

PSX_MOD_CONSTRUCTOR(amx_register_skip_fmv_plugin) {
    (void)psx_mod_register_activation_plugin(
        "amx.fmv.skip", amx_skip_fmvs_activate);
}
