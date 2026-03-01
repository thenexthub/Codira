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

from abc import ABC, abstractmethod
from vmb.shell import ShellBase, ShellHost, ShellDevice
from vmb.target import Target


class CrossShell(ABC):
    @property
    @abstractmethod
    def sh(self) -> ShellHost:
        pass

    @property
    @abstractmethod
    def andb(self) -> ShellDevice:
        pass

    @property
    @abstractmethod
    def hdc(self) -> ShellDevice:
        pass

    @property
    @abstractmethod
    def target(self) -> Target:
        pass

    @property
    def x_sh(self) -> ShellBase:
        """Dispatch command to a propre shell."""
        if Target.HOST == self.target:
            return self.sh
        if Target.DEVICE == self.target:
            return self.andb
        if Target.OHOS == self.target:
            return self.hdc
        raise NotImplementedError(f'No shell for {self.target}!')
