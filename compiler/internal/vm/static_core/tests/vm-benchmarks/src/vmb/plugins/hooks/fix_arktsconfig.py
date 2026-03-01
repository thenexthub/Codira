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

import logging
from vmb.platform import PlatformBase
from vmb.hook import HookBase
from vmb.cli import Args
from vmb.plugins.tools.es2panda import fix_arktsconfig

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Update paths in arktsconfig.

    This is for the case when there is no ark source tree
    and stdlib is included inside build directory.
    """

    @classmethod
    def skipme(cls, args: Args) -> bool:
        """Deprecate, artifacts should use generated declarations."""
        return True

    @property
    def name(self) -> str:
        return 'Update arktsconfig.json'

    # pylint: disable-next=unused-argument
    def before_suite(self, platform: PlatformBase) -> None:
        fix_arktsconfig()
