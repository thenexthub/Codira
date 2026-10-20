#!/usr/bin/env bash
. $(dirname $0)/common.inc

cat <<EOF | $CC -o $t/a.o -c -xc -
int main() {}
EOF

# An empty XDG_RUNTIME_DIR does not put the lock file in the current
# directory.
funis=$PWD/funis
mkdir -p $t/dir
rm -f $t/dir/funis-lock
(cd $t/dir; FUNIS_JOBS=1 XDG_RUNTIME_DIR= $funis -r -o b.o ../a.o)
not test -e $t/dir/funis-lock
