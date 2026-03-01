#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

from __future__ import annotations
import logging
from typing import List, Optional, Type  # noqa
from abc import ABC, abstractmethod
from pathlib import Path
from vmb.tool import ToolBase, OptFlags, VmbToolExecError
from vmb.target import Target
from vmb.unit import BenchUnit, UNIT_PREFIX
from vmb.helpers import get_plugins, get_plugin, die
from vmb.cli import Args
from vmb.shell import ShellHost, ShellAdb, ShellHdc, ShellDevice
from vmb.result import ExtInfo
from vmb.x_shell import CrossShell
from vmb.gensettings import GenSettings


log = logging.getLogger('vmb')


# pylint: disable-next=too-many-public-methods
class PlatformBase(CrossShell, ABC):
    """Platform Base."""

    DEFAULT_TIMEOUT = 900

    def __init__(self, args: Args) -> None:
        self.flags = args.get_opts_flags()
        self.no_run = OptFlags.NO_RUN in self.flags
        self.__sh = ShellHost(args.timeout, no_run=self.no_run)
        self.__andb = None
        self.__hdc = None
        self.__dev_dir = Path(args.get('device_dir', 'unknown'))
        self.args_langs = args.get('langs', set())
        self.ext_info: ExtInfo = {}
        if self.target == Target.DEVICE:
            self.__andb = ShellAdb(dev_serial=args.device,
                                   timeout=args.timeout,
                                   tmp_dir=args.device_dir,
                                   no_run=self.no_run)
        if self.target == Target.OHOS:
            self.__hdc = ShellHdc(dev_serial=args.device,
                                  dev_host=args.device_host,
                                  timeout=args.timeout,
                                  tmp_dir=args.device_dir,
                                  no_run=self.no_run)
        ToolBase.sh_ = self.sh
        ToolBase.andb_ = self.andb
        ToolBase.hdc_ = self.hdc
        ToolBase.dev_dir = self.dev_dir
        self.tools = {}
        # dir with shared libs (init before the suite)
        ToolBase.libs = Path(args.get_shared_path()).joinpath('libs').resolve()
        ToolBase.libs.mkdir(parents=True, exist_ok=True)
        # init all the tools
        tool_plugins = get_plugins(
            'tools',
            self.required_tools,
            extra=args.extra_plugins)
        for n, t in tool_plugins.items():
            tool: ToolBase = t.Tool(self.target,
                                    self.flags,
                                    args.custom_opts.get(n, []),
                                    args.custom_paths.get(n, ''))
            self.tools[n] = tool
            log.info('%s %s', tool.name, tool.version)

    @property
    @abstractmethod
    def name(self) -> str:
        """Name of platform."""
        return ''

    @property
    @abstractmethod
    def required_tools(self) -> List[str]:
        """Expose tools required by platform."""
        return []

    @property
    @abstractmethod
    def langs(self) -> List[str]:
        """Each platform should declare which templates it could use."""
        return []

    @property
    def template(self) -> Optional[GenSettings]:
        """Override generation settings. Default is use of `lang`."""
        return None

    @property
    def required_hooks(self) -> List[str]:
        """List of hooks requred by platform."""
        return []

    @property
    def dev_dir(self) -> Path:
        """Remote working dirrectory."""
        return self.__dev_dir

    @property
    def sh(self) -> ShellHost:
        """Host shell."""
        return self.__sh

    @property
    def andb(self) -> ShellDevice:
        """HOS remote shell."""
        if self.__andb is not None:
            return self.__andb
        # fake object for host platforms
        return ShellAdb.__new__(ShellAdb)

    @property
    def hdc(self) -> ShellDevice:
        """OHOS remote shell."""
        if self.__hdc is not None:
            return self.__hdc
        # fake object for host platforms
        return ShellHdc.__new__(ShellHdc)

    @property
    @abstractmethod
    def target(self) -> Target:
        """Default target is host."""
        return Target.HOST

    @property
    def gc_parcer(self) -> Optional[Type]:
        """GC parcer class."""
        return None

    @staticmethod
    def search_units(paths: List[Path]) -> List[BenchUnit]:
        """Find bench units."""
        bus = []
        for bu_path in paths:
            if not bu_path.exists():
                log.warning('Requested unexisting path: %s', bu_path)
                continue
            # if file name provided add it unconditionally
            if bu_path.is_file():
                bus.append(BenchUnit(Path(bu_path).parent))
                continue
            # in case of dir search by file extention
            for p in bu_path.glob(f'**/{UNIT_PREFIX}*'):
                if p.is_dir():
                    bus.append(BenchUnit(p.resolve()))
        return bus

    @classmethod
    def create(cls, args: Args) -> PlatformBase:
        try:
            platform_module = get_plugin(
                'platforms',
                args.platform,
                extra=args.extra_plugins)
            platform = platform_module.Platform(args)
        except VmbToolExecError as e:
            die(True, e.out)
        except Exception as e:  # pylint: disable=broad-exception-caught
            die(True, 'Plugin (%s) load error: %s', args.platform, e)
        return platform

    @abstractmethod
    def run_unit(self, bu: BenchUnit) -> None:
        pass

    def lazy_setup(self) -> None:
        """Override and put some actions after hooks but before suite."""

    def cleanup(self, bu: BenchUnit) -> None:
        """Do default cleanup."""
        for binary in bu.binaries:
            log.trace('Deleting: %s', binary)
            binary.unlink(missing_ok=True)
        if Target.HOST != self.target:
            self.device_cleanup(bu)

    def push_unit(self, bu: BenchUnit, *ext) -> None:
        """Push bench unit to device."""
        if Target.HOST == self.target:
            return
        bu.device_path = self.dev_dir.joinpath(bu.path.name)
        self.x_sh.run(f'mkdir -p {bu.device_path.as_posix()}')
        if self.no_run:
            self.x_sh.push(bu.path.joinpath('*'), bu.device_path)
        for f in bu.path.glob('*'):
            # skip if suffix filter provided
            if ext and f.suffix not in ext:
                continue
            p = f.resolve()
            self.x_sh.push(p, bu.device_path.joinpath(f.name))
        resources = bu.path.joinpath('resources')
        if resources.exists():
            device_resources = bu.device_path.joinpath('resources').as_posix()
            self.x_sh.run(f'mkdir -p {device_resources}')
            for f in resources.glob('*'):
                p = f.resolve()
                self.x_sh.push(p, bu.device_path.joinpath('resources').joinpath(f.name).as_posix())

    def push_libs(self) -> None:
        if Target.HOST == self.target:
            return
        self.x_sh.push(ToolBase.libs, self.dev_dir)

    def device_cleanup(self, bu: BenchUnit) -> None:
        if bu.device_path is None:
            return  # Bench Unit wasn't pused to device
        log.trace('Cleaning: %s', bu.device_path)
        self.x_sh.run(f'rm -rf {bu.device_path.as_posix()}')

    def set_affinity(self, arg: str) -> None:
        self.x_sh.set_affinity(arg)

    def dry_run_stop(self, bu: BenchUnit) -> bool:
        if OptFlags.DRY_RUN in self.flags:
            for binary in bu.binaries:
                log.info('Path to binary: %s', binary)
            return True
        return False

    def tools_get(self, name: str) -> ToolBase:
        tool = self.tools.get(name)
        if not tool:
            raise RuntimeError(f'Tool {name} is not available')
        return tool
