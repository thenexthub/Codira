#!/usr/bin/env bash
. $(dirname $0)/common.inc

# Verify that funis can link object files compiled with Clang's HWASAN
# instrumentation. On aarch64 such objects contain relocations like
# R_AARCH64_MOVW_PREL_G3 that funis previously rejected
# (https://github.com/rui314/mold/issues/1592).
[ "$CC" = cc ] || skip

# Disable HWASAN's global-variable tagging. It is orthogonal to the
# relocations exercised here, and old Clang implements it by baking a tag
# into the high bits of a 32-bit relocation's addend, producing objects
# that no linker (funis, GNU ld or lld) can resolve.
cat <<EOF | clang -fsanitize=hwaddress -mllvm -hwasan-globals=0 -c -o $t/a.o -xc - 2> /dev/null || skip
#include <stdio.h>
int main() { printf("Hello world\n"); }
EOF

# Skip unless this toolchain can produce a HWASAN executable at all (e.g.
# the sanitizer runtime may not be installed). We probe with Clang's
# default linker so that a funis-specific failure below is still reported
# as a test failure rather than silently skipped.
clang -fsanitize=hwaddress -o $t/exe $t/a.o 2> /dev/null || skip

clang -fsanitize=hwaddress -B. -o $t/exe $t/a.o
