#!/bin/bash
source "$(dirname "$0")"/common.inc

cat <<EOF | $CC -o $t/a.o -c -xc -
int main() {}
EOF

$CC --ld-path=$funis -o $t/exe $t/a.o
otool -l $t/exe | grep 'tool 54321'
