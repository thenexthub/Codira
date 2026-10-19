#!/usr/bin/env bash
. $(dirname $0)/common.inc

not ./funis -zfoo |& grep 'unknown command line option: -zfoo'
not ./funis -z foo |& grep 'unknown command line option: -z foo'
not ./funis -abcdefg |& grep 'unknown command line option: -abcdefg'
not ./funis --abcdefg |& grep 'unknown command line option: --abcdefg'
