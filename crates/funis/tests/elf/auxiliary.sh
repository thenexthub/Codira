#!/usr/bin/env bash
. $(dirname $0)/common.inc

cat <<EOF | $CC -o $t/a.o -c -x assembler -
  .text
  .globl _start
_start:
  nop
EOF

./funis -o $t/b.so $t/a.o -auxiliary foo -f bar -shared

readelf --dynamic $t/b.so > $t/log
grep -F 'Auxiliary library: [foo]' $t/log
grep -F 'Auxiliary library: [bar]' $t/log

not ./funis -o exe $t/a.o -f bar |& grep 'auxiliary may not be used without -shared'

# -fuse-ld is ignored
./funis -o exe $t/a.o -fuse-ld=funis
