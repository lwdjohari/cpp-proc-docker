#!/usr/bin/env bash
set -euo pipefail

# Configure step
cmake -S . -B build

# Build step (use all available cores)
cmake --build build -j"$(nproc)"
