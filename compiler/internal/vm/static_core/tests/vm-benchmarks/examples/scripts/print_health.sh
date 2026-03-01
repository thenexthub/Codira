#!/bin/sh

#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

# These var are available from caller vmb script:
# VMB_BU_PATH - full path to work dir with current test
# VMB_BU_NAME - test name as it stored in report

set -e

echo "
Cpu online  : $(echo $(cat /sys/devices/system/cpu/cpu[0-9]*/online) )
Current freq: $(echo $(cat /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_cur_freq) )
Temp        : $(echo $(for i in /sys/class/thermal/thermal_zone* ; do echo $(cat "$i"/temp) ; done) )
"
which dumpsys && echo "
Batt Status : $(dumpsys battery | grep 'status:' | sed -e 's/^.*: *//')
Batt Status : $(dumpsys battery | grep 'Charge counter:' | sed -e 's/^.*: *//')
" || true
