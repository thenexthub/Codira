#!/usr/bin/env bash
. $(dirname $0)/common.inc

# OneTBB isn't tsan-clean
nm funis | grep '__tsan_init' && skip

./funis -v | grep 'funis .*compatible with GNU ld'
./funis --version | grep 'funis .*compatible with GNU ld'

./funis -V | grep 'funis .*compatible with GNU ld'
./funis -V | grep elf_x86_64
./funis -V | grep elf_i386

cat <<EOF | $CC -c -xc -o $t/a.o -
#include <stdio.h>

int main() {
  printf("Hello world\n");
}
EOF

rm -f $t/exe
$CC -B. -Wl,--version -o $t/exe1 $t/a.o |& grep funis
not [ -f $t/exe1 ]

$CC -B. -Wl,-v -o $t/exe2 $t/a.o |& grep funis
$QEMU $t/exe2 | grep 'Hello world'

not ./funis --v |& grep 'unknown command line option:'
