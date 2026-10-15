#!/usr/bin/env bash
. $(dirname $0)/common.inc

nm funis | grep '__[at]san_init' && skip

# The wrapper is passed in a sealed memfd, which cannot be written to.
./funis -run bash -c 'echo $FUNIS_WRAPPER_FD' | grep -E '^[0-9]+$'
not ./funis -run bash -c 'eval "echo foo >&$FUNIS_WRAPPER_FD"'

# If a process closes the descriptor, the programs it runs get a new copy.
./funis -run bash -c 'eval "exec $FUNIS_WRAPPER_FD<&-"; bash -c "/usr/bin/ld --version"' |
  grep funis

# If another file takes the descriptor number, the programs that the
# process runs don't preload anything from it.
./funis -run bash -c 'eval "exec $FUNIS_WRAPPER_FD</dev/null"; env' >& $t/log
not grep -E '^FUNIS_WRAPPER_FD=|/proc/self/fd|^LD_PRELOAD_FDS=.|ld\.so' $t/log
