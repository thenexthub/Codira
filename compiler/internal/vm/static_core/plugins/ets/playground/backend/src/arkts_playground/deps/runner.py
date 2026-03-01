#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import asyncio
import os
from collections.abc import Sequence
from functools import lru_cache
from importlib.metadata import PackageNotFoundError, version
from pathlib import Path
from typing import NamedTuple, TypedDict, final

import aiofiles
import structlog
from typing_extensions import Unpack

from ..config import get_settings
from ..metrics import (AST_FAILURES, COMPILER_FAILURES, DISASM_FAILURES,
                       RUNTIME_FAILURES)
from ..models.common import VerificationMode

logger = structlog.stdlib.get_logger(__name__)


class Binary(NamedTuple):
    ark_bin: str
    es2panda: str
    disasm_bin: str
    verifier: str
    ark_aot: str


class SubprocessExecResult(TypedDict):
    output: str
    error: str
    exit_code: int | None


class AstViewExecResult(TypedDict):
    output: str
    error: str
    exit_code: int | None
    ast: str | None


class DisasmResult(TypedDict):
    output: str
    error: str
    code: str | None
    exit_code: int | None


class IrDumpResult(TypedDict):
    output: str
    error: str
    compiler_dump: str | None
    disasm_dump: str | None
    exit_code: int | None


class IrDumpOptions(TypedDict, total=False):
    enabled: bool
    compiler_dump: bool
    disasm_dump: bool


class IrDumpPaths(NamedTuple):
    output_an: Path
    dump_folder: Path
    disasm_file: Path


class AotOutputData(NamedTuple):
    compile_out: str
    run_out: str
    compile_err: str
    run_err: str


class CompileOptions(TypedDict, total=False):
    disasm: bool
    verifier: bool
    ir_dump: IrDumpOptions | None


class CompileResult(TypedDict):
    compile: SubprocessExecResult
    disassembly: DisasmResult | None
    verifier: SubprocessExecResult | None
    ir_dump: IrDumpResult | None


class CompileRunResult(TypedDict):
    compile: SubprocessExecResult
    disassembly: DisasmResult | None
    verifier: SubprocessExecResult | None
    run: SubprocessExecResult | None
    run_aot: SubprocessExecResult | None
    ir_dump: IrDumpResult | None


class CompileRunParams(TypedDict, total=False):
    verification_mode: VerificationMode
    verifier: bool
    ir_dump: IrDumpOptions | None
    aot_mode: bool


@final
class Runner:
    def __init__(self, build: str, timeout: int = 120, icu_data: str = ""):
        self._exec_timeout = timeout
        _build = Path(build).expanduser()
        bin_path = _build / "bin"
        self._version = {
            "ark": "",
            "playground": "",
            "es2panda": ""
        }
        self.binary = Binary(
            ark_bin=str(bin_path / "ark"),
            disasm_bin=str(bin_path / "ark_disasm"),
            es2panda=str(bin_path / "es2panda"),
            verifier=str(bin_path / "verifier"),
            ark_aot=str(bin_path / "ark_aot")
        )
        self.stdlib_abc = str(_build / "plugins" / "ets" / "etsstdlib.abc")
        self._icu_data = icu_data if Path(icu_data).exists() else ""
        self._validate()

    @staticmethod
    def parse_compile_options(options: dict[str, str]) -> list[str]:
        res: list[str] = []
        if op := options.get("--opt-level"):
            res.append(f"--opt-level={op}")
        return res

    @staticmethod
    async def _save_code(tempdir: str, code: str) -> str:
        """Save code to temporary file in the temporary directory
        :param tempdir:
        :param code:
        :rtype: str
        """
        async with aiofiles.tempfile.NamedTemporaryFile(dir=tempdir,
                                                        mode="w",
                                                        suffix=".ets",
                                                        delete=False) as stsfile:
            await stsfile.write(code)
            return str(stsfile.name)

    @staticmethod
    def _empty_ir_dump(error: str = "", exit_code: int | None = None) -> IrDumpResult:
        """Create empty IR dump result"""
        return {"output": "", "error": error, "compiler_dump": None, "disasm_dump": None, "exit_code": exit_code}

    @staticmethod
    def _format_aot_output(data: AotOutputData) -> tuple[str, str]:
        """Format combined AOT output and error messages"""
        output = ""
        if data.compile_out:
            output += f"=== AOT Compilation ===\n{data.compile_out}\n"
        if data.run_out:
            if output:
                output += "\n"
            output += data.run_out
        error = f"{data.compile_err}\n{data.run_err}" if data.compile_err else data.run_err
        return output, error

    async def get_versions(self) -> tuple[str, str, str]:
        """Get arkts_playground package and ark versions
        :rtype: Tuple[str, str]
        """
        if not self._version["ark"]:
            await self._get_arkts_version()
        if not self._version["playground"]:
            self._get_playground_version()
        if not self._version["es2panda"]:
            await self._get_es2panda_version()
        return self._version["playground"], self._version["ark"], self._version['es2panda']

    async def compile_arkts(self,
                            code: str,
                            options: list[str],
                            **kwargs: Unpack[CompileOptions]) -> CompileResult:
        """Compile code and make disassemble if disasm argument is True
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="compile")
        log.info("Compilation pipeline started", disasm=kwargs.get("disasm"), verifier=kwargs.get("verifier"))
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile = await self._save_code(tempdir, code)
            res: CompileResult = {
                "compile": await self._compile(stsfile, options),
                "disassembly": None, "verifier": None, "ir_dump": None
            }
            log = log.bind(compile_ret=res["compile"]['exit_code'])

            if res["compile"]["exit_code"] != 0:
                log.error("Compilation failed with nonzero exit code")
                return res
            log.info("Compilation succeeded")

            abcfile = f"{stsfile}.abc"
            if kwargs.get("disasm", False):
                disasm_res = await self.disassembly(abcfile, f"{stsfile}.pa")
                res["disassembly"] = disasm_res
                log.debug("Disassembling completed", dis_exit_code=disasm_res["exit_code"])
            if kwargs.get("verifier", False):
                verify_res = await self._verify(abcfile)
                res["verifier"] = verify_res
                log.debug("Verification completed", verify_exit_code=verify_res["exit_code"])
            ir_dump = kwargs.get("ir_dump")
            if ir_dump and (ir_dump.get("compiler_dump", False) or ir_dump.get("disasm_dump", False)):
                dump_res = await self.generate_ir_dump(abcfile, tempdir, ir_dump)
                res["ir_dump"] = dump_res
                log.debug("IR dump completed", ir_dump_exit_code=dump_res["exit_code"])

            return res

    async def compile_run_arkts(self,
                                code: str,
                                options: list[str],
                                disasm: bool = False,
                                **kwargs: Unpack[CompileRunParams]) -> CompileRunResult:
        """Compile, run code and make disassemble if disasm argument is True
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="compile-run")
        log.info("Compile and run pipeline started", stage="compile", disasm=disasm)
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile = await self._save_code(tempdir, code)
            abcfile = f"{stsfile}.abc"
            res: CompileRunResult = {
                "compile": await self._compile(stsfile, options),
                "run": None, "run_aot": None, "disassembly": None, "verifier": None, "ir_dump": None
            }
            log = log.bind(compile_ret=res["compile"]['exit_code'])

            if res["compile"]["exit_code"] != 0:
                log.error("Compilation failed with nonzero exit code")
                return res

            ver_mode = kwargs.get("verification_mode", VerificationMode.AHEAD_OF_TIME)
            log = log.bind(verification_mode=ver_mode.value, aot_mode=kwargs.get("aot_mode", False))

            res["run"] = await self._run(abcfile, verification_mode=ver_mode)
            if kwargs.get("aot_mode", False):
                res["run_aot"] = await self._run_aot(
                    abcfile, tempdir, runtime_verify=ver_mode != VerificationMode.DISABLED
                )
            log.info("Program run completed")

            if disasm:
                disasm_res = await self.disassembly(abcfile, f"{stsfile}.pa")
                res["disassembly"] = disasm_res
                log.debug("Disassembling completed", dis_exit_code=disasm_res["exit_code"])
            if kwargs.get("verifier", False):
                verifier_res = await self._verify(abcfile)
                res["verifier"] = verifier_res
                log.debug("Verification completed", verify_exit_code=verifier_res["exit_code"])
            ir_dump_opts = kwargs.get("ir_dump")
            if ir_dump_opts and (ir_dump_opts.get("compiler_dump") or ir_dump_opts.get("disasm_dump")):
                ir_dump_res = await self.generate_ir_dump(abcfile, tempdir, ir_dump_opts)
                res["ir_dump"] = ir_dump_res
                log.debug("IR dump completed", ir_dump_exit_code=ir_dump_res["exit_code"])
            return res

    async def disassembly(self, input_file: str, output_file: str) -> DisasmResult:
        """Call ark_disasm and read disassembly from file
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="disasm")
        stdout, stderr, retcode = await self._execute_cmd(self.binary.disasm_bin, input_file, output_file)
        log = log.bind(disasm_ret=retcode)
        if retcode == -11:
            stderr += "disassembly: Segmentation fault"
            log.error("Disassembling failed: segmentation fault")
            DISASM_FAILURES.labels("segfault").inc()
        elif retcode != 0:
            DISASM_FAILURES.labels("normal").inc()
        result: DisasmResult = {"output": stdout, "error": stderr, "code": None, "exit_code": 0}
        if retcode != 0:
            result["exit_code"] = retcode
            log.error("Disassembling failed with nonzero exit code")
            return result
        async with aiofiles.open(output_file, mode="r", encoding="utf-8", errors="replace") as f:
            result["code"] = await f.read()
        return result

    async def generate_ir_dump(self, abc_file: str, tempdir: str,
                               ir_dump_options: IrDumpOptions | None = None) -> IrDumpResult:
        """Generate IR dump using ark_aot compiler"""
        log = logger.bind(stage="ir_dump")

        if not ir_dump_options or not (
            ir_dump_options.get("compiler_dump") or ir_dump_options.get("disasm_dump")
        ):
            return self._empty_ir_dump()

        if not Path(self.binary.ark_aot).exists():
            log.error("ark_aot binary not found", ark_aot_path=self.binary.ark_aot)
            return self._empty_ir_dump(f"ark_aot binary not found at {self.binary.ark_aot}", 1)

        if not Path(abc_file).exists():
            log.error("ABC file not found", abc_file=abc_file)
            return self._empty_ir_dump(f"ABC file not found: {abc_file}", 1)

        dump_folder = Path(tempdir) / "ir_dump"
        dump_folder.mkdir(exist_ok=True)
        paths = IrDumpPaths(Path(tempdir) / "output.an", dump_folder, dump_folder / "ir_disasm.txt")

        cmd_args = self._build_ir_dump_cmd(abc_file, paths, ir_dump_options)
        log.info("Starting IR dump generation", options=ir_dump_options)
        stdout, stderr, retcode = await self._execute_cmd(self.binary.ark_aot, *cmd_args)

        result: IrDumpResult = {
            "output": stdout, "error": stderr,
            "compiler_dump": None, "disasm_dump": None, "exit_code": retcode
        }

        if retcode == -11:
            result["error"] += "\nir_dump: Segmentation fault"
            log.error("IR dump failed: segmentation fault")
            return result
        if retcode != 0:
            log.error("IR dump failed with nonzero exit code")
            return result

        if ir_dump_options.get("disasm_dump") and paths.disasm_file.exists():
            async with aiofiles.open(paths.disasm_file, mode="r", encoding="utf-8", errors="replace") as f:
                result["disasm_dump"] = await f.read()
        if ir_dump_options.get("compiler_dump"):
            result["compiler_dump"] = await self._read_compiler_dump_files(paths.dump_folder)

        log.info("IR dump completed successfully",
                 has_compiler_dump=result["compiler_dump"] is not None,
                 has_disasm_dump=result["disasm_dump"] is not None)
        return result

    async def dump_ast(self, code: str, options: Sequence[str] | None = None) -> AstViewExecResult:
        """Parse ETS code and return AST dump strictly from stdout."""
        log = logger.bind(stage="astdump")
        opts = list(options) if options else []
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile_path = await self._save_code(tempdir, code)
            base = ["--dump-ast", "--extension=ets", f"--output={stsfile_path}.abc"]
            args = base + opts + [stsfile_path]

            stdout, stderr, retcode = await self._execute_cmd(self.binary.es2panda, *args)
            log = log.bind(astdump_ret=retcode)

            if retcode == -11:
                stderr += "ast: Segmentation fault"
                log.error("AST dump failed: segmentation fault")
                AST_FAILURES.labels("segfault").inc()
            elif retcode != 0:
                AST_FAILURES.labels("normal").inc()
                log.info("Ast dump completed")

            ast_out = stdout if (retcode == 0 and stdout) else None

            return {
                "ast": ast_out,
                "output": stdout,
                "error": stderr,
                "exit_code": retcode
            }

    async def _read_compiler_dump_files(self, dump_folder: Path) -> str | None:
        """Read and concatenate compiler dump files from folder"""
        if not dump_folder.exists():
            return None
        dump_files = list(dump_folder.glob("*.ir"))
        if not dump_files:
            return None
        contents = []
        for dump_file in sorted(dump_files):
            async with aiofiles.open(dump_file, mode="r", encoding="utf-8", errors="replace") as f:
                contents.append(f"=== {dump_file.name} ===\n{await f.read()}")
        return "\n\n".join(contents) if contents else None

    def _build_ir_dump_cmd(self, abc_file: str, paths: IrDumpPaths,
                           ir_dump_options: IrDumpOptions) -> list[str]:
        """Build command arguments for IR dump generation"""
        cmd_args = [
            f"--boot-panda-files={self.stdlib_abc}",
            "--paoc-mode=aot", "--load-runtimes=ets",
            f"--paoc-panda-files={abc_file}", f"--paoc-output={paths.output_an}",
            "--compiler-ignore-failures=false",
        ]
        if ir_dump_options.get("compiler_dump", False):
            cmd_args.extend(["--compiler-dump", f"--compiler-dump:folder={paths.dump_folder}"])
        if ir_dump_options.get("disasm_dump", False):
            cmd_args.append(
                f"--compiler-disasm-dump:single-file=true,code-info=true,code=true,file-name={paths.disasm_file}"
            )
        return cmd_args

    async def _compile(self, filename: str, options: list[str]) -> SubprocessExecResult:
        """Compiles ets code stored in file
        :rtype: Dict[str, Any]
        """
        options.extend(["--extension=ets", f"--output={filename}.abc", filename])
        stdout, stderr, retcode = await self._execute_cmd(self.binary.es2panda, *options)
        if retcode == -11:
            stderr += "compilation: Segmentation fault"
            logger.error("Compilation failed: segmentation fault")
            COMPILER_FAILURES.labels("segfault").inc()
        elif retcode != 0:
            COMPILER_FAILURES.labels("normal").inc()
        return {"output": stdout, "error": stderr, "exit_code": retcode}

    async def _verify(self, input_file: str) -> SubprocessExecResult:
        """Runs verifier for input abc file
        :rtype: Dict[str, Any]
        """
        stdout, stderr, retcode = await self._execute_cmd(
            self.binary.verifier,
            f"--boot-panda-files={self.stdlib_abc}",
            "--load-runtimes=ets",
            input_file
        )
        if retcode == -11:
            stderr += "compilation: Segmentation fault"
            logger.error("Verification failed: segmentation fault")
        return {"output": stdout, "error": stderr, "exit_code": retcode}

    async def _execute_cmd(self, cmd: str, *args: str) -> tuple[str, str, int | None]:
        """Create and executes command
        :rtype: Tuple[str, str, int | None]
        """
        env = os.environ.copy()
        build_lib = str(Path(self.binary.ark_bin).parent.parent / "lib")
        system_lib_paths = (
            "/usr/lib/aarch64-linux-gnu:/usr/lib/x86_64-linux-gnu:"
            "/lib/aarch64-linux-gnu:/lib/x86_64-linux-gnu"
        )

        if "LD_LIBRARY_PATH" in env:
            env["LD_LIBRARY_PATH"] = f"{build_lib}:{system_lib_paths}:{env['LD_LIBRARY_PATH']}"
        else:
            env["LD_LIBRARY_PATH"] = f"{build_lib}:{system_lib_paths}"

        if "ark_aot" in cmd:
            logger.debug("Executing ark_aot", source="aot", ld_library_path=env.get("LD_LIBRARY_PATH"))

        proc = await asyncio.create_subprocess_exec(
            cmd,
            *args,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            env=env
        )
        try:
            stdout_b, stderr_b = await asyncio.wait_for(proc.communicate(), timeout=self._exec_timeout)
        except asyncio.exceptions.TimeoutError:
            proc.kill()
            stdout_b, stderr_b = b"", b"Timeout expired"
            logger.error("Subprocess execution timed out", timeout=self._exec_timeout)

        stdout = stdout_b.decode("utf-8", errors="replace")
        stderr = stderr_b.decode("utf-8", errors="replace")
        return stdout, stderr, proc.returncode

    async def _get_arkts_version(self) -> None:
        """Get ark version
        :rtype: None
        """
        output, _, _ = await self._execute_cmd(self.binary.ark_bin, "--version")
        self._version["ark"] = output

    async def _get_es2panda_version(self):
        """Get ark version
        :rtype: None
        """
        _, stderr, _ = await self._execute_cmd(self.binary.es2panda, "--version")
        self._version["es2panda"] = stderr

    def _get_playground_version(self) -> None:
        """Get arkts_playground package version
        :rtype: None
        """
        try:
            self._version["playground"] = version("arkts_playground")
        except PackageNotFoundError:
            self._version["playground"] = "not installed"

    async def _run(
        self,
        abcfile_name: str,
        verification_mode: VerificationMode = VerificationMode.AHEAD_OF_TIME
    ) -> SubprocessExecResult:
        """Runs abc file
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="run")
        entry_point = f"{Path(abcfile_name).with_suffix('').stem}.ETSGLOBAL::main"
        run_cmd = [f"--boot-panda-files={self.stdlib_abc}", "--load-runtimes=ets"]
        if verification_mode != VerificationMode.DISABLED:
            run_cmd += [f"--verification-mode={verification_mode.value}"]
        if self._icu_data:
            run_cmd.append(f"--icu-data-path={self._icu_data}")
        run_cmd.append(abcfile_name)
        run_cmd.append(entry_point)

        stdout, stderr, retcode = await self._execute_cmd(self.binary.ark_bin, *run_cmd)

        log = log.bind(run_ret=retcode)
        if retcode == -11:
            stderr += "run: Segmentation fault"
            log.error("Program run failed: segmentation fault",
                      verification_mode=verification_mode,
                      abcfile_name=abcfile_name)
            RUNTIME_FAILURES.labels("segfault").inc()
        elif retcode != 0:
            RUNTIME_FAILURES.labels("normal").inc()

        return {"output": stdout, "error": stderr, "exit_code": retcode}

    async def _aot_compile(self, abcfile: str, output_an: Path) -> tuple[str, str, int | None]:
        """Compile ABC to native code using ark_aot"""
        aot_cmd = [
            f"--boot-panda-files={self.stdlib_abc}", "--paoc-mode=aot", "--load-runtimes=ets",
            f"--paoc-panda-files={abcfile}", f"--paoc-output={output_an}", "--compiler-ignore-failures=false",
        ]
        stdout, stderr, retcode = await self._execute_cmd(self.binary.ark_aot, *aot_cmd)
        return stdout, stderr, retcode

    def _build_aot_run_cmd(self, abcfile: str, output_an: Path, runtime_verify: bool) -> list[str]:
        """Build command for running AOT compiled code"""
        entry_point = f"{Path(abcfile).with_suffix('').stem}.ETSGLOBAL::main"
        cmd = [f"--boot-panda-files={self.stdlib_abc}", "--load-runtimes=ets", f"--aot-file={output_an}"]
        if runtime_verify:
            cmd.append("--verification-mode=ahead-of-time")
        if self._icu_data:
            cmd.append(f"--icu-data-path={self._icu_data}")
        cmd.extend([abcfile, entry_point])
        return cmd

    def _handle_aot_compile_failure(self, aot_out: str, aot_err: str, aot_ret: int | None) -> SubprocessExecResult:
        """Handle AOT compilation failure and return error result"""
        parts = ["AOT compilation failed:"]
        if aot_out.strip():
            parts.append(f"stdout:\n{aot_out}")
        if aot_err.strip():
            parts.append(f"stderr:\n{aot_err}")
        error = "\n".join(parts)
        if aot_ret == -11:
            error += "\nAOT compilation: Segmentation fault"
            RUNTIME_FAILURES.labels("aot_segfault").inc()
        else:
            RUNTIME_FAILURES.labels("aot_compile_failure").inc()
        return {"output": "", "error": error, "exit_code": aot_ret}

    async def _run_aot_execution(self, abcfile: str, output_an: Path,
                                  runtime_verify: bool, aot_data: tuple[str, str]) -> SubprocessExecResult:
        """Execute AOT compiled code and return result"""
        aot_out, aot_err = aot_data
        stdout, stderr, retcode = await self._execute_cmd(
            self.binary.ark_bin, *self._build_aot_run_cmd(abcfile, output_an, runtime_verify)
        )
        output, error = self._format_aot_output(
            AotOutputData(aot_out, stdout, aot_err, stderr)
        )
        if retcode == -11:
            error += "\nrun (AOT): Segmentation fault"
            logger.error("AOT program run failed: segmentation fault", source="aot")
            RUNTIME_FAILURES.labels("aot_run_segfault").inc()
        elif retcode != 0:
            RUNTIME_FAILURES.labels("aot_run_failure").inc()
        return {"output": output, "error": error, "exit_code": retcode}

    async def _run_aot(self, abcfile: str, tempdir: str, runtime_verify: bool = False) -> SubprocessExecResult:
        """Compiles ABC to native code using AOT and runs it"""
        log = logger.bind(source="aot")

        if not Path(self.binary.ark_aot).exists():
            log.error("ark_aot binary not found", ark_aot_path=self.binary.ark_aot)
            return {"output": "", "error": f"AOT mode requires ark_aot binary at {self.binary.ark_aot}", "exit_code": 1}

        output_an = Path(tempdir) / "output.an"
        log.info("Starting AOT compilation", abc_file=abcfile)
        aot_out, aot_err, aot_ret = await self._aot_compile(abcfile, output_an)

        if aot_ret != 0:
            log.error("AOT compilation failed", exit_code=aot_ret)
            return self._handle_aot_compile_failure(aot_out, aot_err, aot_ret)

        log.info("AOT compilation succeeded")
        return await self._run_aot_execution(abcfile, output_an, runtime_verify, (aot_out, aot_err))

    def _validate(self):
        for f in [self.binary.disasm_bin, self.binary.ark_bin, self.binary.es2panda, self.stdlib_abc]:
            if not Path(f).exists():
                raise RuntimeError(f"\"{f}\" doesn't exist")
        if not Path(self.binary.ark_aot).exists():
            logger.warning("ark_aot binary not found - IR dump feature will not work",
                           source="aot", ark_aot_path=self.binary.ark_aot)


@lru_cache
def get_runner() -> Runner:
    """Return Runner instance
    :rtype: Runner
    """
    sett = get_settings()
    return Runner(sett.binary.build, sett.binary.timeout, sett.binary.icu_data)
