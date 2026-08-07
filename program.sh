#!/usr/bin/env bash
# Build the Codira compiler (codira) from source on Linux/macOS.
#
# Requires: a Rust toolchain (cargo/rustc) and an LLVM 22.x dev package
# (headers + static libs for the target backends you need).
#
# Configure via environment variables before running:
#   LLVM_SYS_220_PREFIX         Path to the LLVM 22 install (headers/lib/bin).
#                               If unset, common system locations are probed.
#   CODIRA_LLD_ELF              Path to a standalone ld.lld (Linux linking).
#   CODIRA_LLD_MACHO            Path to a standalone ld64.lld (macOS linking).
#   BUILD_PROFILE               "release" (default) or "dev".

set -euo pipefail

if [ -z "${LLVM_SYS_220_PREFIX:-}" ]; then
    for candidate in /usr/lib/llvm-22 /usr/local/opt/llvm@22 /opt/homebrew/opt/llvm@22; do
        if [ -x "$candidate/bin/llvm-config" ]; then
            LLVM_SYS_220_PREFIX="$candidate"
            break
        fi
    done
fi

if [ -z "${LLVM_SYS_220_PREFIX:-}" ] || [ ! -x "$LLVM_SYS_220_PREFIX/bin/llvm-config" ]; then
    echo "ERROR: no LLVM 22 install found." >&2
    echo "Set LLVM_SYS_220_PREFIX to a directory containing an LLVM 22.x dev package." >&2
    exit 1
fi
export LLVM_SYS_220_PREFIX

case "$(uname -s)" in
    Darwin)
        if [ -z "${CODIRA_LLD_MACHO:-}" ] && ! command -v ld64.lld >/dev/null 2>&1; then
            echo "WARNING: ld64.lld not found on PATH and CODIRA_LLD_MACHO is unset." >&2
            echo "Linking codira-compiled modules will fail until one is available." >&2
        fi
        ;;
    *)
        if [ -z "${CODIRA_LLD_ELF:-}" ] && ! command -v ld.lld >/dev/null 2>&1; then
            echo "WARNING: ld.lld not found on PATH and CODIRA_LLD_ELF is unset." >&2
            echo "Linking codira-compiled modules will fail until one is available." >&2
        fi
        ;;
esac

BUILD_PROFILE="${BUILD_PROFILE:-release}"

echo "Building codira with LLVM_SYS_220_PREFIX=$LLVM_SYS_220_PREFIX"

if [ "$BUILD_PROFILE" = "release" ]; then
    cargo build --release -p codira
    OUT_DIR=release
else
    cargo build -p codira
    OUT_DIR=debug
fi

echo "=== DONE ==="
echo "Binary at: target/$OUT_DIR/codira"