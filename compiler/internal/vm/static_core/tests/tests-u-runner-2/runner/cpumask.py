#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import os
import subprocess
from pathlib import Path

from runner.common_exceptions import InvalidConfiguration, RunnerException
from runner.logger import Log
from runner.utils import remove_prefix

_LOGGER = Log.get_logger(__file__)


class CPUMask:
    props = ('online',)
    cpu_root = Path("/") / "sys" / "devices" / "system" / "cpu"
    sh = f"hdc {os.environ.get('HDC_OPTIONS', '')} shell"

    def __init__(self, mask: str) -> None:
        self.done = False
        self.saved_state: dict[int, dict[str, str]] = {}
        self.cpumask = list(map(lambda x: x == '1', mask))
        if not any(self.cpumask):
            raise InvalidConfiguration(f'Wrong cpumask: {mask}')

    def apply(self) -> None:
        _LOGGER.all("---before_suite:")

        def get_prop(cpu: int, prop: str) -> str:
            field = str(self.cpu_root / f"cpu{cpu}" / prop)
            result = subprocess.run(
                f'{self.sh} cat {field}',
                capture_output=True, text=True, check=True)
            output = result.stdout.strip()
            return output

        def set_prop(cpu: int, prop: str, val: str) -> None:
            field = str(self.cpu_root / f"cpu{cpu}" / prop)
            cmd = f"{self.sh} \"echo {val} > {field}\""
            subprocess.run(cmd, stderr=subprocess.STDOUT, check=True)

        result = subprocess.run(
            f'{self.sh} ls -1 -d {self.cpu_root / "cpu[0-9]*"!s}',
            capture_output=True, text=True, check=True)
        out = filter(lambda i: not i.startswith('ls:'), result.stdout.splitlines())
        if not out:
            raise RunnerException('Get cpus failed!')
        all_cores = sorted([
            int(remove_prefix(os.path.split(c)[1], 'cpu'))
            for c in out if str(self.cpu_root) in c])
        _LOGGER.all(f"all_cores={all_cores}")
        if len(all_cores) > len(self.cpumask):
            Log.short(
                _LOGGER,
                f'Wrong cpumask length for '
                f'{len(all_cores)} cores: {self.cpumask}')
            self.cpumask += [False] * (len(all_cores) - len(self.cpumask))
        _LOGGER.all(f"cpumask={self.cpumask}")
        # save initial cpu state
        self.saved_state = {cpu: {prop: get_prop(cpu, prop) for prop in CPUMask.props} for cpu in all_cores}
        _LOGGER.all(f"saved_state: {self.saved_state}")
        # update cpu settings
        on_cores = [index for index, on in enumerate(self.cpumask) if on]
        off_cores = [index for index, on in enumerate(self.cpumask) if not on]
        for core in on_cores:
            set_prop(core, 'online', '1')
        for core in off_cores:
            set_prop(core, 'online', '0')
        self.done = True

    def restore(self) -> None:
        _LOGGER.all(f"---after_suite {self.saved_state}")

        def set_prop(cpu: int, prop: str, val: str) -> None:
            field = str(self.cpu_root / f"cpu{cpu}" / prop)
            cmd = f"{self.sh} \"echo {val} > {field}\""
            subprocess.run(cmd, stderr=subprocess.STDOUT, check=True)

        if self.done and self.saved_state:
            # first back ALL cores online
            for core, _ in self.saved_state.items():
                set_prop(core, 'online', '1')
            # then restore all props
            for core, state in self.saved_state.items():
                for prop in CPUMask.props:
                    set_prop(core, prop, state[prop])
