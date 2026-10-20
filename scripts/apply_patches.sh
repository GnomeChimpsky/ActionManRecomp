#!/usr/bin/env bash
# Apply this project's framework patches (patches/<submodule>/*.patch) on top of
# the upstream submodule commits pinned by this repository.
#
# The patches are applied to the working tree only (git apply, no commits), so
# the submodule HEAD stays on the pinned upstream commit and
# framework_pins.txt / psxrecomp/tools/ci/verify_pins.sh keep matching.
#
# Idempotent: a patch that is already applied is detected and skipped; a patch
# that neither applies nor is already present stops the script with an error
# (usually: the submodule is not at the pinned commit, or has local edits).
#
# Usage (from anywhere):
#   scripts/apply_patches.sh            # apply all
#   scripts/apply_patches.sh --check    # report status, change nothing
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CHECK_ONLY=0
case "${1:-}" in
  "") ;;
  --check) CHECK_ONLY=1 ;;
  -h|--help) sed -n '2,15p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
  *) echo "error: unknown arg: $1" >&2; exit 2 ;;
esac

status=0
shopt -s nullglob
for dir in "${ROOT}"/patches/*/; do
  sub="$(basename "${dir}")"
  target="${ROOT}/${sub}"
  if [[ ! -e "${target}/.git" ]]; then
    echo "error: submodule ${sub} is not checked out; run: git submodule update --init --recursive" >&2
    exit 1
  fi
  for patch in "${dir}"*.patch; do
    name="${sub}/$(basename "${patch}")"
    if git -C "${target}" apply --check --reverse "${patch}" 2>/dev/null; then
      echo "already applied: ${name}"
    elif git -C "${target}" apply --check "${patch}" 2>/dev/null; then
      if [[ "${CHECK_ONLY}" == 1 ]]; then
        echo "not applied:     ${name}"
        status=1
      else
        git -C "${target}" apply "${patch}"
        echo "applied:         ${name}"
      fi
    else
      echo "error: ${name} does not apply to ${sub} at $(git -C "${target}" rev-parse --short HEAD)" >&2
      echo "       expected the commit in framework_pins.txt with a clean tree" >&2
      exit 1
    fi
  done
done
exit "${status}"
