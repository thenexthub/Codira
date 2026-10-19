#!/usr/bin/env bash
. $(dirname $0)/common.inc

echo ' -help' > $t/rsp
$funis @$t/rsp | grep Usage
