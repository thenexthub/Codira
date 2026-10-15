#!/usr/bin/env bash
. $(dirname $0)/common.inc

cat <<EOF | $CC -o $t/a.o -c -xc -
int main() {
  return 0;
}
EOF

$CC --ld-path=$funis -o $t/exe $t/a.o
$RUN $t/exe
