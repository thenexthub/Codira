#!/usr/bin/env bash
. $(dirname $0)/common.inc

nm -D funis | grep __msan_init && skip

not ./funis -m elf_x86_64 '' |& grep 'cannot open :'
