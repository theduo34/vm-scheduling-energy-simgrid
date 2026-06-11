#!/usr/bin/env bash
#
# run_all.sh - Runs every (policy x workload) combination and rebuilds
# results/simulation_results.csv from scratch (12 rows once all four
# policies exist: 4 policies x 3 workloads).
#
# This script is forward-compatible: ALL policy names are listed below from
# the start. Right now only "roundrobin" is implemented, so the other three
# combinations are detected (via the simulator's "Unknown policy" error) and
# skipped with a warning. As each new policy is added to the scheduler
# factory, its runs will start succeeding automatically - no edits needed
# here.

set -uo pipefail   # no -e: a skipped/failed run must not abort the whole loop

# Always operate relative to the project root, regardless of caller's cwd.
cd "$(dirname "$0")"

PLATFORM="platform/datacenter.xml"
BUILD_DIR="build"
BINARY="$BUILD_DIR/simulation"
RESULTS_CSV="results/simulation_results.csv"
LAST_LOG="/tmp/run_all_last.log"

POLICIES=(roundrobin firstfit bestfit energyaware)
WORKLOADS=(light moderate heavy)

# Configure + build (same as run.sh).
if [[ ! -d "$BUILD_DIR" ]]; then
    echo "==> Configuring CMake build in $BUILD_DIR/ ..."
    cmake -S . -B "$BUILD_DIR"
fi
echo "==> Building..."
cmake --build "$BUILD_DIR"

# Start from a clean results file - each run appends one row, so stale
# results from previous experiments must not linger.
mkdir -p results
rm -f "$RESULTS_CSV"

echo
echo "==> Running all (policy x workload) combinations..."
for policy in "${POLICIES[@]}"; do
    for workload in "${WORKLOADS[@]}"; do
        printf "  %-12s / %-8s ... " "$policy" "$workload"

        if "$BINARY" "$PLATFORM" "$policy" "$workload" > "$LAST_LOG" 2>&1; then
            echo "done"
        elif grep -q "Unknown policy" "$LAST_LOG"; then
            echo "skipped (policy not yet implemented)"
        else
            echo "FAILED (see $LAST_LOG)"
        fi
    done
done

echo
echo "==> $RESULTS_CSV:"
cat "$RESULTS_CSV"
