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

import json
import subprocess  # noqa: S404
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Literal, Protocol

from pytest import fixture

from arkdb.runnable_module import ScriptFile, syntax

from .logs import logger

LOG = logger(__name__)

CompilerExtensions = Literal["js", "ts", "as", "ets"]

CompileLogLevel = Literal["debug", "info", "warning", "error", "fatal"]


class AstParser(Protocol):
    def __call__(
        self,
        compiler_output: str,
    ) -> dict[str, Any]:
        pass


@dataclass
class Options:
    """
    Options for ``es2panda`` execution.
    """

    app_path: Path = Path("bin", "es2panda")
    arktsconfig: Path = Path("bin", "arktsconfig.json")


@dataclass
class CompilerArguments:
    extension: CompilerExtensions = "ets"
    ets_module: bool = False
    opt_level: int = 0
    debug_info: bool = True
    dump_dynamic_ast: bool | None = None
    debugger_eval_panda_files: list[Path] | None = None
    debugger_eval_source: Path | None = None
    debugger_eval_line: int | None = None
    log_level: str | None = None

    @staticmethod
    def arg_to_str(arg: bool | list[Path] | Any) -> str:
        if isinstance(arg, bool):
            return str(arg).lower()
        if isinstance(arg, list):
            return ":".join(str(f) for f in arg)
        return str(arg)

    @staticmethod
    def to_cli_key(key: str) -> str:
        if key.startswith("debugger_eval_"):
            return f"--debugger-eval:{key[len('debugger_eval_'):].replace('_', '-')}"
        return f"--{key.replace('_', '-')}"

    def to_arguments_list(self) -> list[str]:
        result = []
        for key, value in vars(self).items():
            if value is not None:
                result.append(f"{CompilerArguments.to_cli_key(key)}={CompilerArguments.arg_to_str(value)}")
        return result


@dataclass
class EvaluateCompileExpressionArgs:
    ets_expression: str | Path
    eval_panda_files: list[Path] | None = None
    eval_source: Path | None = None
    eval_line: int | None = None
    eval_log_level: CompileLogLevel | None = None
    ast_parser: AstParser | None = None
    name: str = "evaluated_expression"
    extension: CompilerExtensions | None = None


class CompileError(Exception):
    def __init__(self, stdout: str) -> None:
        super().__init__([stdout])
        self.stdout = stdout


class Compiler:
    """
    Controls the ``es2panda``.
    """

    def __init__(self, options: Options) -> None:
        self.options = options

    @staticmethod
    def _compile(
        args: list[str],
        cwd: Path = Path(),
    ) -> tuple[str, str]:
        LOG.info("Compile %s", args)
        proc = subprocess.run(  # noqa: S603
            args=args,
            cwd=cwd,
            text=True,
            capture_output=True,
        )
        if proc.returncode == 1:
            raise CompileError(f"Stdout:\n{proc.stdout}\nStderr:\n{proc.stderr}")
        proc.check_returncode()
        return proc.stdout, proc.stderr

    def compile(
        self,
        source_file: Path,
        output_file: Path,
        cwd: Path = Path(),
        arguments: CompilerArguments | None = None,
    ) -> tuple[str, str]:
        args = [
            str(self.options.app_path),
            f"--arktsconfig={self.options.arktsconfig}",
            f"--output={output_file}",
        ]
        opt_args = arguments if arguments is not None else CompilerArguments()
        args += opt_args.to_arguments_list()
        args.append(str(source_file))

        return Compiler._compile(args=args, cwd=cwd)


@fixture
def ark_compiler(
    ark_compiler_options: Options,
) -> Compiler:
    """
    Return :class:`Compiler` instans that controls ``es2panda`` process.
    """
    return Compiler(ark_compiler_options)


@fixture
def ast_parser() -> AstParser:
    def parser(compiler_output: str) -> dict[str, Any]:
        return json.loads(compiler_output)

    return parser


def ark_compile(
    source_file: Path,
    tmp_path: Path,
    ark_compiler: Compiler,
    arguments: CompilerArguments | None = None,
    ast_parser: AstParser | None = None,
):
    if not source_file.exists():
        raise FileNotFoundError(source_file)
    if not tmp_path.exists():
        raise FileNotFoundError(tmp_path)

    panda_file = tmp_path / f"{source_file.stem}.abc"
    try:
        stdout, stderr = ark_compiler.compile(
            source_file=source_file,
            output_file=panda_file,
            cwd=tmp_path,
            arguments=arguments,
        )
        if stderr:
            LOG.debug("ES2PANDA STDERR:\n%s", stderr)

    except CompileError as e:
        LOG.error("Compile error", rich=syntax(source_file.read_text(), start_line=1))
        if e.stdout:
            LOG.error("STDOUT\n%s", e.stdout)
        raise e
    except subprocess.CalledProcessError as e:
        LOG.error("Compiler failed with %s code", e.returncode, rich=syntax(source_file.read_text(), start_line=1))
        if e.stdout:
            LOG.error("STDOUT\n%s", e.stdout)
        if e.stderr:
            LOG.error("STDERR\n%s", e.stderr)
        raise e

    ast: dict[str, Any] | None = None
    if ast_parser and arguments and arguments.dump_dynamic_ast:
        ast = ast_parser(stdout)
    return ScriptFile(source_file, panda_file, ast=ast)


class StringCodeCompiler:

    def __init__(self, tmp_path: Path, ark_compiler: Compiler) -> None:
        self._tmp_path = tmp_path
        self._ark_compiler = ark_compiler

    def compile(
        self,
        source_code: str,
        name: str = "test_string",
        arguments: CompilerArguments | None = None,
        ast_parser: AstParser | None = None,
    ) -> ScriptFile:
        """
        Compiles the ``ets``-file and returns the :class:`ScriptFile` instance.
        """
        args = arguments if arguments else CompilerArguments()
        source_file = self._write_into_file(source_code, name, args.extension)
        return ark_compile(
            source_file=source_file,
            tmp_path=self._tmp_path,
            ark_compiler=self._ark_compiler,
            arguments=args,
            ast_parser=ast_parser,
        )

    def compile_expression(
        self,
        eval_args: EvaluateCompileExpressionArgs,
    ) -> ScriptFile:
        args = CompilerArguments(
            ets_module=True,
            opt_level=0,
            dump_dynamic_ast=ast_parser is not None,
            debugger_eval_panda_files=eval_args.eval_panda_files,
            debugger_eval_source=eval_args.eval_source,
            debugger_eval_line=eval_args.eval_line,
            log_level=eval_args.eval_log_level if eval_args.eval_log_level is not None else "error",
        )
        if eval_args.extension is not None:
            args.extension = eval_args.extension

        ets_file: Path
        if isinstance(eval_args.ets_expression, str):
            ets_file = self._write_into_file(eval_args.ets_expression, eval_args.name, args.extension)
        else:
            ets_file = eval_args.ets_expression
        if not ets_file.exists():
            raise FileNotFoundError(ets_file)

        return ark_compile(
            ets_file,
            self._tmp_path,
            self._ark_compiler,
            arguments=args,
            ast_parser=eval_args.ast_parser,
        )

    def _write_into_file(self, source_code: str, filename: str, extension: str) -> Path:
        ets_file = self._tmp_path / f"{filename}.{extension}"
        ets_file.write_text(source_code)
        return ets_file


@fixture
def code_compiler(
    tmp_path: Path,
    ark_compiler: Compiler,
) -> StringCodeCompiler:
    """
    Return :class:`StringCodeCompiler` instance that can compile ``ets``-file.
    """
    return StringCodeCompiler(
        tmp_path=tmp_path,
        ark_compiler=ark_compiler,
    )
