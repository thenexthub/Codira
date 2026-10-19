#!/usr/bin/env bash
. $(dirname $0)/common.inc

cat <<EOF | $CC -c -o $t/a.o -xc -
int main() { return 0; }
EOF

not ./funis -o $t/exe $t/a.o --emit-relocs --strip-all |&
  grep -F -- '--strip-all may not be used with --emit-relocs'

./funis -r -o $t/b.o $t/a.o --strip-all
