#!/usr/bin/env bash
# ============================================================
# run_demo.sh - scripted end-to-end demonstration runner
# Course : Operating System and System Programming (25CS2104E)
#
# Runs the explorer inside a scratch directory (/tmp/fse_demo) so the
# real project tree is never modified.  The input comes from
# tests/demo_input.txt which drives every menu option once, including
# deliberate error cases (missing file, non-empty rmdir, bad offset...).
#
# Usage:   bash tests/run_demo.sh          (after: make)
# ============================================================
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXE="$SCRIPT_DIR/../build/explorer"
IN="$SCRIPT_DIR/demo_input.txt"
SCRATCH=/tmp/fse_demo

if [ ! -x "$EXE" ]; then
    echo "ERROR: $EXE not found. Run 'make' first."
    exit 1
fi

mkdir -p "$SCRATCH"
rm -rf "$SCRATCH"/* 2>/dev/null
cd "$SCRATCH" || exit 1

"$EXE" < "$IN"
STATUS=$?

echo "============================================================"
echo "SCRIPT EXIT CODE: $STATUS  (0 = clean exit via menu option 13)"
