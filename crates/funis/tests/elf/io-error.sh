#!/usr/bin/env bash
. $(dirname $0)/common.inc

# An I/O error is reported with the text strerror gives for it.

echo 'int main() {}' | $CC -c -o $t/a.o -xc -

not ./funis -o $t/exe $t/a.o $t/nonexistent.o |&
  grep -Fx "funis: fatal: cannot open $t/nonexistent.o: No such file or directory"

not ./funis -o $t/exe $t/a.o -Map $t/nonexistent/map |&
  grep -Fx "funis: fatal: --print-map: cannot open $t/nonexistent/map: No such file or directory"

not ./funis -o $t/nonexistent/exe $t/a.o |&
  grep -E "^funis: fatal: cannot open $t/nonexistent/\.exe\.[0-9]+: No such file or directory$"
