cmake_minimum_required(VERSION 3.20)
set(PSXRECOMP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/psxrecomp" CACHE PATH
    "Path to psxrecomp")
if(NOT EXISTS "${PSXRECOMP_ROOT}/runtime/runtime.cmake")
    message(FATAL_ERROR
        "PSXRecomp runtime is missing: ${PSXRECOMP_ROOT}/runtime/runtime.cmake\n"
        "This project expects the psxrecomp framework checkout at ${PSXRECOMP_ROOT}.\n"
        "Run: git submodule update --init --recursive\n"
        "If psxrecomp/ is not a submodule, set -DPSXRECOMP_ROOT=<path-to-psxrecomp>.")
endif()

project(ActionManRecomp LANGUAGES C CXX)

# The game C in generated/ is translated from the boot EXE on the builder's own
# disc and is never committed. Fail early with a pointer to the documented
# build instead of silently producing a game-less setup host. (A setup host can
# still be requested explicitly with -DPSXRECOMP_FORCE_SETUP_HOST=ON.)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/SLES_017.61_dispatch.c"
   AND NOT PSXRECOMP_FORCE_SETUP_HOST)
    message(FATAL_ERROR
        "generated/ is missing. This repository does not contain any game code;\n"
        "generate it from your own disc first:\n"
        "  scripts/build.sh --disc /path/to/ActionMan.cue\n"
        "or: python3 psxrecomp/psxrecomp_cli.py generate --config game.toml "
        "--project-root . --disc /path/to/ActionMan.cue\n"
        "See README.md (Building) and DISC.md.")
endif()

# --- Opt-in product surfaces (OFF by default in the framework) -------------
# Scaffold prompts / flags fill these. Without recomp-ui, PSX_RECOMP_UI is OFF.
# The wizard (WIZARD_CMAKE_BLOCK / ENABLE_SETUP_WIZARD) is the first-run disc
# picker. It is off: builds compile the game C generated locally from the
# builder's disc (scripts/build.sh), so nothing is generated on a player's
# machine at run time.
# recomp-ui submodule present (PSX_RECOMP_UI defaults ON).
# Full netplay UI — enable after adding recomp-ui + testing:
# if(EXISTS "${PSXRECOMP_ROOT}/lib/recomp-net/CMakeLists.txt")
#     set(PSX_NETPLAY ON CACHE BOOL "…" FORCE)
# endif()
# First-run setup wizard — enable after testing:
# set(PSX_SETUP_WIZARD ON CACHE BOOL "…" FORCE)
include("${PSXRECOMP_ROOT}/runtime/runtime.cmake")

# This project's own mod catalog. The FRAMEWORK stages it -- together with its
# builtin psx.* packages -- into <exe-dir>/mods/bundled, and refuses to
# configure if a catalog exists here but is not declared. Never copy it by
# hand: five titles did, and framework 4cc04be3's rename of the staged
# directory broke all five silently (bead beads-eio.3.101).
set(PSX_PRELOADED_MODS "${CMAKE_CURRENT_SOURCE_DIR}/mods/preloaded")
if(NOT IS_DIRECTORY "${PSX_PRELOADED_MODS}")
    set(PSX_PRELOADED_MODS "NONE")
endif()

psxrecomp_add_game_runtime(psx-runtime
    # PGXP hook flavor: lets the bundled psx.enhancement.pgxp mod arm subpixel
    # geometry / perspective-correct textures. Off at runtime until selected.
    PGXP
    PRELOADED_MODS_DIR "${PSX_PRELOADED_MODS}"
    # ENABLE_NETPLAY_IF_PRESENT
    # NETPLAY_LOBBY_URL "ws://netplay.retcomm.net:8765"
    # ENABLE_SETUP_WIZARD
    WINDOW_TITLE "ActionManRecomp"
    DEFAULT_GAME_CONFIG_PATH "game.toml"
    MAX_PLAYERS 1
    # LAUNCHER_BOXART "${CMAKE_CURRENT_SOURCE_DIR}/launcher_assets/img/boxart.tga"
    APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/psxrecomp.ico"
    # Game C generated locally from your disc (psxrecomp_cli.py generate, run
    # by scripts/build.sh; never committed). scripts/build.sh configures with
    # -DPSXRECOMP_REQUIRE_GAME_C=ON as well.
    GEN_MARKER "generated/SLES_017.61_dispatch.c"
    GEN_FULL_GLOB "generated/SLES_017.61_full_*.c"
    CODEGEN_SETUP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/codegen_setup.c"
    # Framework builtins Action Man does not ship. Keep this keyword last:
    # it takes every value after it.
    EXCLUDE_BUILTIN_MODS psx.presentation.bezel
)

# Action Man's own mod plugins (widescreen, frame rate).
file(GLOB AMX_MOD_PLUGIN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mods/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mods/*.cpp")
target_sources(psx-runtime PRIVATE ${AMX_MOD_PLUGIN_SOURCES})
target_include_directories(psx-runtime PRIVATE "${PSXRECOMP_ROOT}/runtime/include")

# Stage runtime configs next to the exe for direct build/ launches. Only copy
# files that exist — never fail POST_BUILD when game_options.toml is absent.
foreach(_PSX_RUNTIME_CONFIG game.toml game_options.toml)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_PSX_RUNTIME_CONFIG}")
        add_custom_command(TARGET psx-runtime POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${_PSX_RUNTIME_CONFIG}"
                "$<TARGET_FILE_DIR:psx-runtime>/${_PSX_RUNTIME_CONFIG}"
            COMMENT "Staging ${_PSX_RUNTIME_CONFIG}"
            VERBATIM)
    endif()
endforeach()
