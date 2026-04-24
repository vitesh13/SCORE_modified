#!/usr/bin/env bash

# This script manages the nexus_definitions submodule.
# Usage:
# ./scripts/nexus_definitions.sh checkout <REV>
# <REV> can be:
# - commit hash
# - tag
# - branch name
#
# If <REV> is a branch, the submodule will track that branch in .gitmodules.
# If <REV> is a commit or tag, the submodule is left in detached HEAD state.

set -euo pipefail

NEXUS_FOLDER="src/cxx/thirdparty/mandatory/nexus_definitions"

update_mtex_version() {
  cd "$NEXUS_FOLDER" && echo "updating nexus-version.txt"
  printf "$(git describe --always --dirty --tags --long --abbrev=8 --match '*[0-9]*')" > ../nexus-version.txt
  cd ../../../
}

get_default_branch() {
  git -C "$NEXUS_FOLDER" remote show origin | awk '/HEAD branch/ {print $NF}'
}

checkout_definitions() {
  local ref="$1"

  echo "checking out definitions submodule at: $ref"

  git submodule update --init "$NEXUS_FOLDER"
  git -C "$NEXUS_FOLDER" fetch --tags origin

  git -C "$NEXUS_FOLDER" checkout "$ref"

  # Check whether we are on a branch or detached HEAD
  local current_branch
  current_branch="$(git -C "$NEXUS_FOLDER" branch --show-current)"

  if [[ -n "$current_branch" ]]; then
    echo "detected branch checkout: $current_branch"

    local default_branch
    default_branch="$(get_default_branch)"

    if [[ "$current_branch" == "$default_branch" ]]; then
      echo "removing submodule branch setting from .gitmodules (default branch)"
      git config -f .gitmodules --unset \
        submodule.$NEXUS_FOLDER.branch || true
    else
      echo "setting submodule to track branch: $current_branch"
      git config -f .gitmodules \
        submodule.$NEXUS_FOLDER.branch "$current_branch"
    fi
  else
    echo "detected detached checkout (commit or tag)"
    git config -f .gitmodules --unset \
      submodule.$NEXUS_FOLDER.branch || true
  fi

  git submodule sync "$NEXUS_FOLDER"
}

print_usage() {
echo "Usage:"
echo "  $0 checkout <REV>"
exit 1
}

main() {
if [[ $# -lt 1 ]]; then
print_usage
fi

project_dir=$(dirname "$(dirname "$(realpath "$0")")")
cd "$project_dir"

case "$1" in
checkout)
[[ $# -eq 2 ]] || print_usage
checkout_definitions "$2"
;;
*)
echo "Error: Unknown command '$1'"
print_usage
;;
esac

update_mtex_version
}

main "$@"
