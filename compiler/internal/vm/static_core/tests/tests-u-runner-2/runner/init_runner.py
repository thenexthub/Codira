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
import argparse
import sys
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar, NamedTuple

from runner.common_exceptions import FileNotFoundException
from runner.utils import write_2_file

Str2Path = Callable[[str], Path]

RequireExist = bool
IsPath = bool
PropName = str
PropValue = str


class MandatoryProp(NamedTuple):
    value: PropValue | None
    is_path: IsPath
    require_exist: RequireExist


MandatoryProps = dict[PropName, MandatoryProp]

HOME_PROP = "home"
LOCAL_PROP = "local"
CONFIG_PATH = "config_path"


class InitRunner:
    # Number of repeats to type a value for one variable
    attempts: ClassVar[int] = 3
    init_option: ClassVar[str] = "init"
    help_options: ClassVar[set[str]] = {"--help", "-h"}
    urunner_env_name_default: ClassVar[str] = ".urunner.env"

    def __init__(self, urunner_env: str = urunner_env_name_default):
        self.urunner_env_name: str = urunner_env
        self.urunner_env_path: Path = self.search_global_env(urunner_env)

    @property
    def urunner_env_default(self) -> Path:
        return Path.home() / self.urunner_env_name

    @staticmethod
    def input_value(prop: str, require_exist: bool, attempts: int) -> PropValue:
        for i in range(1, attempts + 1):
            main_text = f"Attempt #{i} of {attempts}: type the value of the mandatory variable {prop}: "
            next_text = f"Empty or not-existing path are NOT suitable!\n{main_text}"
            value = input(main_text if i == 1 else next_text)
            if value and (Path(value).expanduser().exists() or not require_exist):
                return value
        raise FileNotFoundError(f"All {attempts} attempts to enter value for '{prop}' are out.")

    @staticmethod
    def convert_2_path(arg: str) -> Path:
        return Path(arg).expanduser().resolve()

    @staticmethod
    def require_exists(arg: str) -> Path:
        current = InitRunner.convert_2_path(arg)
        if current.exists():
            return current
        raise FileNotFoundException(f"Path {current} does not exist, but must.")

    @classmethod
    def require_path(cls, require_exist: bool) -> Str2Path:
        return cls.require_exists if require_exist else cls.convert_2_path

    @classmethod
    def add_cli_args(cls, parser: argparse.ArgumentParser, mandatory_props: MandatoryProps) -> None:
        subparser = parser.add_subparsers()
        init_parser = subparser.add_parser("init", help='Run URunner initialization')
        init_parser.prog = parser.prog
        group = init_parser.add_mutually_exclusive_group()
        group.add_argument(
            f'--{HOME_PROP}', action='store_true', default=False,
            help='Save global config into home directory')
        group.add_argument(
            f'--{LOCAL_PROP}', action='store', default=".",
            dest=CONFIG_PATH,
            help='Set path where to save the URunner config to')

        for prop_name, (prop_value, is_path, require_exist) in mandatory_props.items():
            process_type = str if not is_path else cls.require_path(require_exist)
            expected_value = 'an existing path' if is_path and require_exist \
                else 'a path that will be created during runner work' if is_path and not require_exist \
                else 'any value'
            init_parser.add_argument(
                f'--{prop_name.lower()}', action='store', default=prop_value,
                dest=prop_name, type=process_type,
                help=f'Set mandatory property {prop_name}. Expected {expected_value}.')

    @classmethod
    def should_runner_initialize(cls, argv: list[str]) -> bool:
        return cls.init_option in argv

    @classmethod
    def request_runner_help(cls, argv: set[str]) -> bool:
        return bool(cls.help_options & argv)

    def search_global_env(self, urunner_env: str) -> Path:
        current = Path.cwd()
        while current != current.parent:
            config_path = current / urunner_env
            if config_path.exists():
                return config_path
            current = current.parent
        return self.urunner_env_default

    def is_runner_initialized(self) -> bool:
        return self.urunner_env_path.exists()

    def initialize(self, mandatory_props: MandatoryProps) -> None:
        """
        Create a global environment file and save there the mandatory environment variables.
        If any mandatory variable is not specified (None or empty) or does not exist but must,
        it will ask the user to type path from the console
        """
        parser = argparse.ArgumentParser(description="URunner initializer options")
        self.add_cli_args(parser, mandatory_props)
        init_args = parser.parse_args(sys.argv[1:]).__dict__

        if not init_args[HOME_PROP]:
            self.urunner_env_path = self.convert_2_path(init_args[CONFIG_PATH]) / self.urunner_env_name

        props: dict[PropName, PropValue] = {}
        for prop, (_, is_path, require_exist) in mandatory_props.items():
            cli_value = init_args[prop]
            if cli_value is None:
                props[prop] = self.init_property(prop, cli_value, is_path, require_exist)
            else:
                props[prop] = cli_value

        result = [f"{prop}={value}" for prop, value in props.items()]
        write_2_file(self.urunner_env_path, "\n".join(result))

    def init_property(self, prop: str, value: str | None, is_path: IsPath, require_exist: RequireExist) -> str:
        """
        Initialize the property under name `prop` with provided `value`
        if it is None - ask for user to type the value from the console
        """
        if value is None or len(value) == 0 or (not Path(value).expanduser().exists() and require_exist):
            value = self.input_value(prop, is_path and require_exist, self.attempts)
            if is_path:
                path_value = Path(value).expanduser().resolve()
                value = path_value.as_posix()
        return value
