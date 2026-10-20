#!/usr/bin/env bash
. $(dirname $0)/common.inc

echo 'void _start() {}' | $CC -c -o $t/a.o -xc -
./funis -o $t/exe $t/a.o -gdb-index
readelf -WS $t/exe | not grep -F .gdb_index
