#!/usr/bin/env bash
# Build ActionManRecomp from a fresh clone and YOUR OWN disc image.
#
# Steps (each is safe to re-run):
#   1. git submodule update --init --recursive
#   2. scripts/apply_patches.sh            (framework patches, see patches/)
#   3. build the recompiler (psxrecomp-game / psxrecomp-bios) for this host
#   4. psxrecomp_cli.py generate --disc    (verifies the disc against game.toml,
#                                           copies it to disc/, writes generated/)
#   5. configure + build the game (Linux native, or Windows via llvm-mingw)
#
# Usage:
#   scripts/build.sh --disc /path/to/ActionMan.cue [options]
#
# Options:
#   --disc PATH        your Action Man: Mission Xtreme (Europe) .cue (required
#                      unless generated/ already exists from an earlier run)
#   --windows          cross-compile Windows x64 binaries (needs LLVM_MINGW_ROOT)
#   --build-dir DIR    game build dir (default: build-linux or build-win)
#   --skip-generate    reuse the existing generated/ (no disc needed)
#   --jobs N           parallel jobs
#
# Environment:
#   LLVM_MINGW_ROOT    llvm-mingw install root (with --windows)
#   PSXRECOMP_TOOLCHAIN_DIR  optional cmake-clang-v1 pack (bin/ inside); when
#                      unset, cmake / ninja / clang from PATH are used
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

DISC=""
WINDOWS=0
BUILD_DIR=""
SKIP_GENERATE=0
JOBS="$(nproc 2>/dev/null || echo 4)"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --disc) DISC="${2:?}"; shift 2 ;;
    --windows) WINDOWS=1; shift ;;
    --build-dir) BUILD_DIR="${2:?}"; shift 2 ;;
    --skip-generate) SKIP_GENERATE=1; shift ;;
    --jobs) JOBS="${2:?}"; shift 2 ;;
    -h|--help) sed -n '2,27p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "error: unknown arg: $1" >&2; exit 2 ;;
  esac
done

if [[ -n "${PSXRECOMP_TOOLCHAIN_DIR:-}" ]]; then
  export PATH="${PSXRECOMP_TOOLCHAIN_DIR}/bin:${PATH}"
fi
for tool in git cmake ninja python3; do
  command -v "${tool}" >/dev/null || { echo "error: ${tool} not found on PATH" >&2; exit 1; }
done

echo "== 1/5 submodules"
git submodule sync --recursive
git submodule update --init --recursive

echo "== 2/5 framework patches"
bash scripts/apply_patches.sh

echo "== 3/5 recompiler (host)"
bash psxrecomp/tools/ci/build_emitters.sh --framework psxrecomp \
  --build-dir build-recompiler --jobs "${JOBS}" \
  ${PSXRECOMP_TOOLCHAIN_DIR:+--toolchain-dir "${PSXRECOMP_TOOLCHAIN_DIR}"}

echo "== 4/5 generate game C from your disc"
if [[ "${SKIP_GENERATE}" == 1 ]]; then
  [[ -f generated/SLES_017.61_dispatch.c ]] || {
    echo "error: --skip-generate but generated/ is empty; pass --disc" >&2; exit 1; }
  echo "skipped (reusing generated/)"
else
  if [[ -z "${DISC}" ]]; then
    if [[ -f generated/SLES_017.61_dispatch.c ]]; then
      echo "no --disc given; reusing existing generated/"
    else
      echo "error: --disc /path/to/ActionMan.cue is required (see DISC.md)" >&2
      exit 2
    fi
  else
    python3 psxrecomp/psxrecomp_cli.py generate --config game.toml \
      --project-root . --disc "${DISC}"
  fi
fi

echo "== 5/5 game build"
CMAKE_ARGS=(-G Ninja -DCMAKE_BUILD_TYPE=Release -DPSXRECOMP_REQUIRE_GAME_C=ON)
if [[ "${WINDOWS}" == 1 ]]; then
  : "${LLVM_MINGW_ROOT:?set LLVM_MINGW_ROOT to your llvm-mingw install (see README)}"
  BUILD_DIR="${BUILD_DIR:-build-win}"
  # The framework treats these as dependency hints; for a cross build they
  # would point at the Linux pack's SDL3 / zlib and break the Windows link.
  # cmake / ninja stay on PATH; Windows dependencies are fetched instead.
  unset PSXRECOMP_TOOLCHAIN_DIR RETCOMM_TOOLCHAIN_DIR TOOLCHAIN_DIR BPE_TOOLCHAIN_DIR
  CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="${ROOT}/cmake/llvm-mingw.cmake"
               -DLLVM_MINGW_ROOT="${LLVM_MINGW_ROOT}"
               -DPSX_ENABLE_VULKAN=OFF)
  # The release overlay_toolchain/ needs a Windows build of the recompiler too.
  cmake -S psxrecomp/recompiler -B build-recompiler-win "${CMAKE_ARGS[@]}"
  cmake --build build-recompiler-win --target psxrecomp-game psxrecomp-bios -j "${JOBS}"
else
  BUILD_DIR="${BUILD_DIR:-build-linux}"
  # Keep host OpenSSL / FreeType out of the portable build (optional features).
  CMAKE_ARGS+=(-DCMAKE_DISABLE_FIND_PACKAGE_OpenSSL=ON
               -DCMAKE_DISABLE_FIND_PACKAGE_Freetype=ON)
  if [[ -n "${PSXRECOMP_TOOLCHAIN_DIR:-}" && -f "${PSXRECOMP_TOOLCHAIN_DIR}/env.sh" ]]; then
    # cmake-clang-v1 pack: its compilers, build sysroot (glibc, OpenGL stubs)
    # and bundled SDL3 / zlib, so no host development packages are needed.
    # shellcheck disable=SC1091
    source "${PSXRECOMP_TOOLCHAIN_DIR}/env.sh"
    CMAKE_ARGS+=(-DCMAKE_C_COMPILER="${PSXRECOMP_TOOLCHAIN_DIR}/bin/clang"
                 -DCMAKE_CXX_COMPILER="${PSXRECOMP_TOOLCHAIN_DIR}/bin/clang++"
                 -DCMAKE_SYSROOT="${PSXRECOMP_TOOLCHAIN_DIR}/sysroot"
                 -DCMAKE_PREFIX_PATH="${PSXRECOMP_TOOLCHAIN_DIR}/deps"
                 -DOpenGL_GL_PREFERENCE=LEGACY)
  fi
fi
cmake -S . -B "${BUILD_DIR}" "${CMAKE_ARGS[@]}"
cmake --build "${BUILD_DIR}" --target psx-runtime -j "${JOBS}"

echo
echo "Built into ${BUILD_DIR}/ (run the executable there; it finds game.toml beside it)."
if [[ "${WINDOWS}" == 1 ]]; then
  echo "Package it: scripts/package_release.sh ${BUILD_DIR} windows-x64 build-recompiler-win"
else
  echo "Package it: scripts/package_release.sh ${BUILD_DIR} linux-x64"
fi
