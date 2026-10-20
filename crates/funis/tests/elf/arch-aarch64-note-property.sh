#!/usr/bin/env bash
. $(dirname $0)/common.inc

test_cflags -mbranch-protection=standard || skip

cat <<EOF | $CC -mbranch-protection=standard -c -o $t/a.o -xc -
void _start() {}
EOF

cat <<EOF | $CC -mbranch-protection=standard -c -o $t/b.o -xc -
void foo() {}
EOF

cat <<EOF | $CC -mbranch-protection=bti -c -o $t/c.o -xc -
void bar() {}
EOF

cat <<EOF | $CC -mbranch-protection=none -c -o $t/d.o -xc -
void baz() {}
EOF

./funis -o $t/exe1 $t/a.o $t/b.o
readelf -n $t/exe1 | grep 'AArch64 feature: BTI, PAC'
readelf -W --segments $t/exe1 | grep -w GNU_PROPERTY

./funis -o $t/exe2 $t/a.o $t/b.o $t/c.o
readelf -n $t/exe2 | grep 'AArch64 feature: BTI$'

./funis -o $t/exe3 $t/a.o $t/b.o $t/c.o $t/d.o
readelf -n $t/exe3 | not grep 'AArch64 feature'

./funis -r -o $t/e.o $t/a.o $t/b.o
readelf -n $t/e.o | grep 'AArch64 feature: BTI, PAC'
