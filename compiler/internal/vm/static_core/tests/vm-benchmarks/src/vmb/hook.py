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
import sys
import logging
from typing import List, Set
from abc import ABC, abstractmethod
from vmb.platform import PlatformBase
from vmb.cli import Args
from vmb.unit import BenchUnit
from vmb.helpers import Singleton, get_plugins

log = logging.getLogger('vmb')


class HookBase(ABC):
    """Hook plugins base class."""

    def __init__(self, args: Args) -> None:  # pylint: disable=unused-argument
        self.done = False

    @property
    @abstractmethod
    def name(self) -> str:
        return ''

    @classmethod
    def skipme(cls, args: Args) -> bool:  # pylint: disable=unused-argument
        """Do not register me.

        Hook could decide this depending on args provided
        """
        return False


class HookRegistry(metaclass=Singleton):
    def __init__(self) -> None:
        self.__b_s_hooks: List[HookBase] = []
        self.__a_s_hooks: List[HookBase] = []
        self.__b_u_hooks: List[HookBase] = []
        self.__a_u_hooks: List[HookBase] = []

    def register(self, hook: HookBase, args: Args) -> None:
        if hook.skipme(args):
            return
        b_u = getattr(hook, 'before_unit', None)
        b_s = getattr(hook, 'before_suite', None)
        a_u = getattr(hook, 'after_unit', None)
        a_s = getattr(hook, 'after_suite', None)
        if callable(b_u):
            self.__b_u_hooks.append(hook)
        if callable(b_s):
            self.__b_s_hooks.append(hook)
        if callable(a_u):
            self.__a_u_hooks.append(hook)
        if callable(a_s):
            self.__a_s_hooks.append(hook)

    def register_all_by_name(self,
                             names: Set[str],
                             args: Args) -> HookRegistry:
        for hook in [hook_module.Hook(args)
                     for (_, hook_module)
                     in get_plugins('hooks', list(names),
                                    extra=args.extra_plugins).items()]:
            log.debug('Register hook: %s', hook.name)
            self.register(hook, args)
        return self

    def run_before_unit(self, bu: BenchUnit) -> None:
        for hook in self.__b_u_hooks:
            log.debug('Run before-unit hook: %s', hook.name)
            hook.before_unit(bu)

    def run_after_unit(self, bu: BenchUnit) -> None:
        for hook in self.__a_u_hooks:
            log.debug('Run after-unit hook: %s', hook.name)
            hook.after_unit(bu)

    def run_before_suite(self, platform: PlatformBase) -> None:
        try:
            for hook in self.__b_s_hooks:
                log.debug('Run before-suite hook: %s', hook.name)
                hook.before_suite(platform)
        except Exception as e:  # pylint: disable=broad-exception-caught
            log.error(e)
            sys.exit(1)

    def run_after_suite(self, platform: PlatformBase) -> None:
        try:
            for hook in self.__a_s_hooks:
                log.debug('Run after-suite hook: %s', hook.name)
                hook.after_suite(platform)
        except Exception as e:  # pylint: disable=broad-exception-caught
            log.error(e)
