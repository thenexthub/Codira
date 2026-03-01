#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

from dataclasses import replace
from pathlib import Path

from pytest import Config, FixtureRequest, fixture

from arkdb import compiler, disassembler, runtime


def get_config(config: Config, name: str):
    return config.getoption(name, config.getini(name), True)


# CC-OFFNXT(G.CTL.01) This is exactly the signature of the function that required.
# Any changes will complicate the code and its processing.
def str2bool(val: str | None) -> None | bool:
    if val is None:
        return None
    val = val.lower()
    if val in ("", "none", "null"):
        return None
    if val in ("y", "yes", "t", "true", "on", "1"):
        return True
    if val in ("n", "no", "f", "false", "off", "0"):
        return False
    raise ValueError("invalid truth value %r" % (val,))


@fixture(scope="session")
def ark_build_path(
    request: FixtureRequest,
) -> Path:
    """
    Path to cmake ``build`` directory of panda.
    """
    return Path(get_config(request.config, "ark_build_path")).absolute()


@fixture(scope="session")
def ark_compiler_default_options(
    ark_build_path: Path,
) -> compiler.Options:
    """
    Return a :class:`compiler.Options` instance for `ark_compiler_options` fixture.
    """
    return compiler.Options(
        app_path=ark_build_path / "bin" / "es2panda",
        arktsconfig=ark_build_path / "bin" / "arktsconfig.json",
    )


@fixture
def ark_compiler_options(
    ark_compiler_default_options: compiler.Options,
) -> compiler.Options:
    """
    Return a :class:`compiler.Options` instance for `ark_compiler` fixture.

    You can override fixture in your tests or conftest
    See https://docs.pytest.org/en/stable/how-to/fixtures.html#overriding-fixtures-on-various-levels
    """
    return replace(ark_compiler_default_options)


@fixture
def ark_disassembler_default_options(
    ark_build_path: Path,
) -> disassembler.Options:
    """
    Return a :class:`disassembler.Options` instance for `ark_disassembler_options` fixture.
    """
    return disassembler.Options(
        app_path=ark_build_path / "bin" / "ark_disasm",
    )


@fixture
def ark_disassembler_options(
    ark_disassembler_default_options: disassembler.Options,
) -> disassembler.Options:
    """
    Return a :class:`disassembler.Options` instance for `ark_disassembler` fixture.

    You can override fixture in your tests or conftest
    See https://docs.pytest.org/en/stable/how-to/fixtures.html#overriding-fixtures-on-various-levels
    """
    return replace(ark_disassembler_default_options)


@fixture
def ark_runtime_default_options(
    ark_build_path: Path,
) -> runtime.Options:
    """
    Return a :class:`runtime.Options` instance for `ark_runtime_options` fixture.
    """
    return runtime.Options(
        app_path=ark_build_path / "bin" / "ark",
        log_debug=["debugger"],  # Always log debugger for CI launches
        interpreter_type="cpp",
        boot_panda_files=[ark_build_path / "plugins" / "ets" / "etsstdlib.abc"],
        debugger_library_path=ark_build_path / "lib" / "libarkinspector.so",
    )


@fixture
def ark_runtime_options(
    ark_runtime_default_options: runtime.Options,
) -> runtime.Options:
    """
    Return a :class:`runtime.Options` instance for `ark_runtime` fixture.

    You can override fixture in your tests or conftest
    See https://docs.pytest.org/en/stable/how-to/fixtures.html#overriding-fixtures-on-various-levels
    """
    return replace(ark_runtime_default_options)
