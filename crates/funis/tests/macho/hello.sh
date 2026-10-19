#!/usr/bin/env bash
. $(dirname $0)/common.inc

cat <<EOF | $CC -o $t/a.o -c -xc -
#include <stdio.h>
int main() {
  printf("Hello world\n");
}
EOF

$CC --ld-path=$funis -o $t/exe $t/a.o
$RUN $t/exe | grep 'Hello world'
