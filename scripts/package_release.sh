#!/usr/bin/env bash
# Package a build made by scripts/build.sh as a player-ready release zip, using
# the shared psxrecomp bundled-release packager.
#
# Contents: the game executable, runtime data, bundled OpenBIOS (MIT), the mod
# catalog, overlay_toolchain/ (generic recompiler + headers, plus pinned Python
# and TinyCC), licences and docs. The framework packager refuses generated C,
# BIOS dumps, disc images and memory cards. No game assets or disc data.
#
# The executable CONTAINS A MACHINE TRANSLATION OF THE GAME'S PROGRAM CODE,
# compiled from the C generated from the builder's disc (see README.md,
# "Distribution"). No level-code cache is packaged: each player's copy compiles
# its own from their disc (overlay_toolchain/).
#
# Usage:
#   scripts/package_release.sh <build-dir> <artifact-tag> [recompiler-build-dir]
#   Windows: scripts/package_release.sh build-win   windows-x64 build-recompiler-win
#   Linux:   scripts/package_release.sh build-linux linux-x64
#
# Environment:
#   AMX_OFFLINE_TOOLCHAIN=1  stage overlay_toolchain/ WITHOUT the downloaded
#       Python / TinyCC (for machines that cannot reach python.org, GitHub and
#       savannah.gnu.org). Windows players then run get_overlay_toolchain.ps1
#       once; Linux players fall back to a system gcc/cc/clang, else the
#       interpreter. This is the layout of the 0.3.x/0.4.x test packages.
#
# Writes: dist/Action-Man-Recomp-<VERSION>-<artifact-tag>.zip
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-}"
ARTIFACT_TAG="${2:-}"
RECOMPILER_BUILD="${3:-build-recompiler}"
ZIP_PREFIX="Action-Man-Recomp"
DISPLAY_NAME="Action Man Recomp"

if [[ -z "${BUILD_DIR}" || -z "${ARTIFACT_TAG}" ]]; then
  echo "usage: $0 <build-dir> <artifact-tag> [recompiler-build-dir]" >&2
  exit 2
fi

FRAMEWORK="${ROOT}/psxrecomp"
PACKAGER="${FRAMEWORK}/tools/package_game_release.sh"
if [[ ! -f "${PACKAGER}" ]]; then
  echo "error: missing ${PACKAGER} (run git submodule update --init --recursive)" >&2
  exit 1
fi

EXTRA=(--ship-without-overlay-cache-because \
  "no precompiled level code is distributed: overlay_toolchain/ compiles it from the player's own disc on the player's machine")
# Docs and helpers shipped at the zip root.
DOCS=(LICENSE DISC.md)
if [[ "${ARTIFACT_TAG}" == windows-* ]]; then
  # Fetches the pinned Python + TinyCC when overlay_toolchain/ lacks them.
  DOCS+=(scripts/get_overlay_toolchain.ps1)
fi
for _doc in "${DOCS[@]}"; do
  [[ -f "${ROOT}/${_doc}" ]] && EXTRA+=(--doc "${_doc}")
done

OFFLINE="${AMX_OFFLINE_TOOLCHAIN:-0}"
if [[ "${OFFLINE}" == 1 ]]; then
  EXTRA+=(--no-overlay-toolchain --stage-only)
fi

cd "${ROOT}"
bash "${PACKAGER}" \
  --root "${ROOT}" \
  --build-dir "${BUILD_DIR}" \
  --artifact "${ARTIFACT_TAG}" \
  --zip-prefix "${ZIP_PREFIX}" \
  --exe-name ActionManRecomp \
  --display-name "${DISPLAY_NAME}" \
  --recompiler-build "${RECOMPILER_BUILD}" \
  --version-env RELEASE_VERSION \
  --disc-hint "your own Action Man: Mission Xtreme (SLES-01761) disc image" \
  "${EXTRA[@]}"

if [[ "${OFFLINE}" == 1 ]]; then
  # Same files the packager stages before it downloads Python / TinyCC.
  STAGE="${ROOT}/dist/stage-game-${ARTIFACT_TAG}"
  TK="${STAGE}/overlay_toolchain"
  case "${ARTIFACT_TAG}" in
    windows-*) EXE_SUFFIX=.exe ;;
    *)         EXE_SUFFIX= ;;
  esac
  mkdir -p "${TK}/include" "${TK}/bios" "${TK}/recompiler/seeds"
  for bin in psxrecomp-game psxrecomp-bios; do
    src="${ROOT}/${RECOMPILER_BUILD}/${bin}${EXE_SUFFIX}"
    [[ -f "${src}" ]] || { echo "error: ${src} missing (build the recompiler for ${ARTIFACT_TAG})" >&2; exit 1; }
    cp "${src}" "${TK}/"
  done
  cp "${FRAMEWORK}/tools/compile_overlays.py" "${FRAMEWORK}/tools/bios_module_build.py" "${TK}/"
  cp "${FRAMEWORK}"/runtime/include/*.h "${TK}/include/"
  cp "${FRAMEWORK}"/runtime/include/*.c.inc "${TK}/include/" 2>/dev/null || true
  cp "${FRAMEWORK}"/bios/*.toml "${TK}/bios/"
  cp "${FRAMEWORK}"/recompiler/seeds/*.json "${TK}/recompiler/seeds/" 2>/dev/null || true
  VERSION="$(tr -d '[:space:]' < "${ROOT}/VERSION")"
  ZIP="${ROOT}/dist/${ZIP_PREFIX}-${VERSION}-${ARTIFACT_TAG}.zip"
  rm -f "${ZIP}"
  python3 "${FRAMEWORK}/tools/create_release_zip.py" --source "${STAGE}" --output "${ZIP}" >/dev/null
  echo "Wrote ${ZIP} (offline toolchain: no bundled Python/TinyCC)"
  du -h "${ZIP}"
fi
