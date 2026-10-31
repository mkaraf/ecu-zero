#!/usr/bin/env bash
# Fail if any commit in BASE..HEAD mixes public headers, tests and sources.
#
# Groups:
#   headers  src/include/**
#   tests    tests/**
#   src      src/** (outside src/include/)
# Paths outside these groups are not checked. Merge commits are skipped.
#
# Usage: scripts/check-commit-paths.sh BASE HEAD
set -euo pipefail

if [ $# -ne 2 ]; then
    echo "usage: $0 BASE HEAD" >&2
    exit 2
fi

fail=0

for commit in $(git rev-list --no-merges --reverse "$1..$2"); do
    headers=0 tests=0 src=0
    while IFS= read -r path; do
        case "$path" in
            src/include/*) headers=1 ;;
            tests/*)       tests=1 ;;
            src/*)         src=1 ;;
        esac
    done < <(git diff-tree --root --no-commit-id --name-only --no-renames -r "$commit")

    if [ $((headers + tests + src)) -gt 1 ]; then
        groups=""
        [ $headers -eq 1 ] && groups="$groups headers"
        [ $tests -eq 1 ]   && groups="$groups tests"
        [ $src -eq 1 ]     && groups="$groups src"
        echo "FAIL $(git log -1 --format='%h %s' "$commit"):$groups"
        fail=1
    fi
done

if [ $fail -ne 0 ]; then
    echo "A commit may touch only one of: src/include/, tests/, src/ (see CLAUDE.md)." >&2
fi
exit $fail
