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
from threading import main_thread
from typing import ClassVar

from runner.common_exceptions import InvalidConfiguration
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


class LocalEnv:
    instance: ClassVar[dict[str, 'LocalEnv']] = {}

    def __init__(self) -> None:
        self.__env: dict[str, str] = {}

    @staticmethod
    def get_instance_id() -> str:
        return str(main_thread().ident)

    @classmethod
    def get(cls) -> 'LocalEnv':
        ident: str = cls.get_instance_id()
        if ident not in cls.instance:
            cls.instance[ident] = LocalEnv()
        return cls.instance[ident]

    def add(self, key: str, env_value: str) -> None:
        if key not in self.__env:
            self.__env[key] = env_value
            return
        if key in self.__env and self.__env[key] != env_value:
            raise InvalidConfiguration(
                "Attention: environment has been changed during test execution. "
                f"For key `{key}` the env_value `{self.__env[key]}` changed to `{env_value}`"
            )

    def is_present(self, key: str) -> bool:
        return key in self.__env

    def get_value(self, key: str) -> str:
        return str(self.__env[key])

    def list(self) -> str:
        return str(self.__env)
