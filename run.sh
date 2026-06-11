#!/usr/bin/env bash
#
# run.sh - Convenience wrapper for a single simulation run.
#
# Usage:
#   ./run.sh <policy> <workload>
#
#   policy   : roundrobin | firstfit | bestfit | energyaware
#   workload : light | moderate | heavy
#
# Example:
#   ./run.sh roundrobin heavy
#     -> ./build/simulation platform/datacenter.xml roundrobin heavy

set -euo pipefail

# Always operate relative to the project root, regardless of caller's cwd.
cd "$(dirname "$0")"

PLATFORM="platform/datacenter.xml"
BUILD_DIR="build"
BINARY="$BUILD_DIR/simulation"

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <policy> <workload>"
    echo "  policy   : roundrobin | firstfit | bestfit | energyaware"
    echo "  workload : light | moderate | heavy"
    echo
    echo "Example: $0 roundrobin heavy"
    exit 1
fi

POLICY="$1"
WORKLOAD="$2"

# Configure the build directory the first time it's needed.
if [[ ! -d "$BUILD_DIR" ]]; then
    echo "==> Configuring CMake build in $BUILD_DIR/ ..."
    cmake -S . -B "$BUILD_DIR"
fi

# Rebuild if any source has changed; a no-op otherwise.
echo "==> Building..."
cmake --build "$BUILD_DIR"

echo "==> Running: $BINARY $PLATFORM $POLICY $WORKLOAD"
echo
"$BINARY" "$PLATFORM" "$POLICY" "$WORKLOAD"
