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
from os import path

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.options.config import Config
from runner.runner_file_based import RunnerFileBased, PandaBinaries
from runner.utils import get_platform_binary_name

_LOGGER = logging.getLogger("runner.runner_js")


class PandaJSBinaries(PandaBinaries):
    @property
    def ark_quick(self) -> str:
        if self.conf_kind == ConfigurationKind.QUICK:
            return self.check_path(self.config.general.build, 'bin', get_platform_binary_name('arkquick'))
        return super().ark_quick


class RunnerJS(RunnerFileBased):
    def __init__(self, config: Config, name: str) -> None:
        RunnerFileBased.__init__(self, config, name, PandaJSBinaries)
        ecmastdlib_abc: str = f"{self.build_dir}/pandastdlib/arkstdlib.abc"
        if not path.exists(ecmastdlib_abc):
            ecmastdlib_abc = f"{self.build_dir}/gen/plugins/ecmascript/ecmastdlib.abc"  # stdlib path for GN build

        self.quick_args = []
        if self.conf_kind == ConfigurationKind.QUICK:
            ecmastdlib_abc = self.generate_quick_stdlib(ecmastdlib_abc)

        self.runtime_args.extend([
            f'--boot-panda-files={ecmastdlib_abc}',
            '--load-runtimes=ecmascript',
        ])

        if self.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            self.aot_args.extend([
                f'--boot-panda-files={ecmastdlib_abc}',
                '--load-runtimes=ecmascript',
            ])
