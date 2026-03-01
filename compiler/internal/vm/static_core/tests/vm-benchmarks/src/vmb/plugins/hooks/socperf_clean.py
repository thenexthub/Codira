#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#

import logging
import time

from vmb.cli import Args
from vmb.hook import HookBase
from vmb.shell import ShellDevice
from vmb.target import Target
from vmb.platform import PlatformBase

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Cleanup socperf settings in OHOS device."""

    @property
    def name(self) -> str:
        return 'Cleanup socperf'

    @classmethod
    def skipme(cls, args: Args) -> bool:
        """Skip for not OHOS devices."""
        return 'ohos' not in args.platform

    def before_suite(self, platform: PlatformBase) -> None:
        if not platform.target == Target.OHOS:
            raise RuntimeError(f'{self.name} on OHOS only!')
        hdc: ShellDevice = platform.hdc
        res = hdc.run('cat /sys_prod/etc/soc_perf/socperf_*_config*.xml')
        if not res or res.ret != 0:
            raise RuntimeError('Device seems not ready!')
        if res.out.strip() == '__RET_VAL__=0':
            log.info('Socperf is empty')
            return
        log.info('Socperf needs cleaning')
        hdc.run('mount -o rw,remount /sys_prod')
        hdc.run('echo > /sys_prod/etc/soc_perf/socperf_boost_config.xml')
        hdc.run('echo > /sys_prod/etc/soc_perf/socperf_boost_config_ext.xml')
        hdc.run('echo > /sys_prod/etc/soc_perf/socperf_resource_config.xml')
        hdc.run('reboot')
        remaining_seconds = 300
        while remaining_seconds > 0:
            res = hdc.run('param get ohos.boot.sn')
            if res.ret != 0:
                time.sleep(30)
                remaining_seconds -= 30
                continue
            log.info('Device reboot OK')
            return
        raise RuntimeError('Device reboot failed!')
