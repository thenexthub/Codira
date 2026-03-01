#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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
import os
from pathlib import Path
from typing import ClassVar, NamedTuple

from dotenv import load_dotenv

from runner.common_exceptions import InvalidInitialization
from runner.init_runner import IsPath, MandatoryProp, MandatoryProps, PropName, RequireExist
from runner.logger import Log
from runner.utils import FontColor

_LOGGER = Log.get_logger(__file__)


class MandatoryPropDescription(NamedTuple):
    name: PropName
    is_path: IsPath
    require_exist: RequireExist


class RunnerEnv:
    mandatory_props: ClassVar[list[MandatoryPropDescription]] = [
        MandatoryPropDescription('ARKCOMPILER_RUNTIME_CORE_PATH', is_path=True, require_exist=True),
        MandatoryPropDescription('ARKCOMPILER_ETS_FRONTEND_PATH', is_path=True, require_exist=True),
        MandatoryPropDescription('PANDA_BUILD', is_path=True, require_exist=True),
        MandatoryPropDescription('WORK_DIR', is_path=True, require_exist=False)
    ]
    urunner_path_name: ClassVar[str] = 'URUNNER_PATH'

    def __init__(self, *,
                 global_env: Path,
                 local_env: Path | None = None,
                 urunner_path: Path | None = None):
        self.global_env_path: Path = global_env
        self.local_env_path: Path | None = local_env
        self.urunner_path: Path | None = urunner_path

    @staticmethod
    def expand_mandatory_prop(prop_desc: MandatoryPropDescription, runner_help_mode: bool) -> None:
        if runner_help_mode:
            return

        var_value = os.getenv(prop_desc.name)
        if var_value is None:
            raise InvalidInitialization(
                f"Mandatory environment variable '{prop_desc.name}' is not set. \n\n"
                f"Run this command to initialize the runner: \n{FontColor.RED.value}./runner.sh "
                f"init{FontColor.RESET.value} \n"
                f"To see all available initialization options run:\n{FontColor.RED.value}./runner.sh "
                f"init --help{FontColor.RESET.value}\n")
        if not prop_desc.is_path:
            return
        expanded = Path(var_value).expanduser().resolve()
        if prop_desc.require_exist and not expanded.exists():
            raise InvalidInitialization(f"Mandatory environment variable '{prop_desc.name}' is set \n"
                                        f"to value {var_value} which does not exist.")
        os.environ[prop_desc.name] = expanded.as_posix()

    @classmethod
    def get_mandatory_props(cls) -> MandatoryProps:
        result: MandatoryProps = {}
        for (prop_name, is_path, require_exist) in cls.mandatory_props:
            result[prop_name] = MandatoryProp(value=os.getenv(prop_name), is_path=is_path, require_exist=require_exist)
        return result

    def load_environment(self, runner_help_mode: bool = False) -> None:
        self.load_home_env()
        self.load_local_env()

        for prop in self.mandatory_props:
            self.expand_mandatory_prop(prop, runner_help_mode)

        if self.urunner_path:
            os.environ[self.urunner_path_name] = self.urunner_path.as_posix()

    def load_local_env(self) -> None:
        """
        Loads the local environment file .env located near the main.py
        """
        if self.local_env_path and self.local_env_path.exists():
            load_dotenv(self.local_env_path)

    def load_home_env(self) -> None:
        """
        Loads the global environment file .urunner.env located at the user's home
        """
        if self.global_env_path.exists():
            load_dotenv(self.global_env_path)
