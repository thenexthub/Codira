#!/usr/bin/env bash
. $(dirname $0)/common.inc

lto_library=$(dirname "$(xcrun -f clang)")/../lib/libLTO.dylib

cat <<EOF | $CC -flto -c -xc - -o $t/a.o
#include <stdio.h>
int main() { printf("Hello\n"); }
EOF

# Without -lto_library, the linker takes the libLTO in lib beside the
# bin directory it runs from, as ld-prime links its toolchain's. (The
# system's ld is a shim that can't run from a copy; it finds Xcode's.)
# The copy keeps the name ld64.funis, under which funis acts as the Mach-O
# linker.
ld=$funis
if is_funis; then
  rm -rf $t/tc
  mkdir -p $t/tc/bin $t/tc/lib
  ln $funis $t/tc/bin/ld64.funis 2> /dev/null || cp $funis $t/tc/bin/ld64.funis
  ln -s $lto_library $t/tc/lib/libLTO.dylib
  ld=$t/tc/bin/ld64.funis
fi
$ld -arch $ARCH -platform_version ${PLATFORM_VERSION:-macos 13.0 13.0} -syslibroot $SDK -lSystem $t/a.o -o $t/exe
$RUN $t/exe | grep -q Hello
