#!/usr/bin/env python
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
from importlib.metadata import PackageNotFoundError

import pytest
from structlog.testing import capture_logs
from structlog.typing import EventDict

from src.arkts_playground.deps.runner import Runner
from src.arkts_playground.models.common import VerificationMode
from tests.fixtures.mock_subprocess import FakeCommand, MockAsyncSubprocess


@pytest.fixture(scope="session", name="RunnerSample")
def runner_fixture(ark_build):
    return Runner(ark_build[0])


def log_entry_assert(
    entry: EventDict,
    stage_expected: str | None = None,
    event_expected: str | None = None,
    level_expected: str | None = None,
):
    if stage_expected is not None:
        assert entry["stage"] == stage_expected
    if event_expected is not None:
        assert entry["event"] == event_expected
    if level_expected is not None:
        assert entry["log_level"] == level_expected


@pytest.mark.asyncio
@pytest.mark.parametrize("compile_opts, disasm, verifier", [
    (
            [],
            False,
            True
    ),
    (
            [],
            True,
            False
    ),
    (
            ["--opt-level=2"],
            True,
            False
    )
])
async def test_compile(monkeypatch, ark_build, compile_opts, disasm, verifier):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets", *compile_opts],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[2]),
                        opts=[],
                        stdout=b"disassembly",
                        return_code=0),
            FakeCommand(expected=str(ark_build[5]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets"],
                        stdout=b"verified",
                        return_code=0)

        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.compile_arkts("code", compile_opts, disasm=disasm, verifier=verifier)

    log_entry_assert(cap_logs[0], event_expected="Compilation pipeline started", stage_expected="compile")
    log_entry_assert(cap_logs[1], event_expected="Compilation succeeded", stage_expected="compile")
    if disasm and verifier:
        log_entry_assert(cap_logs[2], event_expected="Disassemblying completed")
        log_entry_assert(cap_logs[3], event_expected="Verification completed")
    elif disasm:
        log_entry_assert(cap_logs[2], event_expected="Disassembling completed")
    elif verifier:
        log_entry_assert(cap_logs[2], event_expected="Verification completed")

    expected = {
        "compile": {"error": "", "exit_code": 0, "output": "executed"},
        "disassembly": None, "verifier": None, "ir_dump": None
    }
    if disasm:
        expected["disassembly"] = {
            "code": "disassembly output",
            "error": "",
            "exit_code": 0,
            "output": "disassembly"
        }
    if verifier:
        expected["verifier"] = {
            "output": "verified",
            "error": "",
            "exit_code": 0
        }
    assert res == expected


def _get_mocker_compile_and_run(ark_build, compile_opts: list) -> MockAsyncSubprocess:
    return MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets", *compile_opts],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}",
                              "--load-runtimes=ets",
                              f"--icu-data-path={ark_build[6]}",
                              "ETSGLOBAL::main"],
                        stdout=b"run and compile",
                        return_code=0),
            FakeCommand(expected=str(ark_build[2]),
                        opts=[],
                        stdout=b"disassembly",
                        return_code=0),
            FakeCommand(expected=str(ark_build[5]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets"],
                        stdout=b"verified",
                        return_code=0)
        ]
    )


@pytest.mark.asyncio
@pytest.mark.parametrize("compile_opts, disasm, verifier", [
    (
            [],
            False,
            False,
    ),
    (
            [],
            True,
            True
    ),
    (
            ["--opt-level=2"],
            True,
            False
    )
])
async def test_compile_and_run(monkeypatch, ark_build, compile_opts: list, disasm: bool, verifier: bool):
    mocker = _get_mocker_compile_and_run(ark_build, compile_opts)
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0], icu_data=ark_build[6])
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", compile_opts, disasm, verifier=verifier)

    log_entry_assert(cap_logs[0], event_expected="Compile and run pipeline started", stage_expected="compile")
    log_entry_assert(cap_logs[1], event_expected="Program run completed", stage_expected="compile-run")
    if disasm and verifier:
        log_entry_assert(cap_logs[2], event_expected="Disassembling completed")
        log_entry_assert(cap_logs[3], event_expected="Verification completed")
    elif disasm:
        log_entry_assert(cap_logs[2], event_expected="Disassembling completed")
    elif verifier:
        log_entry_assert(cap_logs[2], event_expected="Verification completed")

    expected = {
        "compile": {
            "error": "",
            "exit_code": 0,
            "output": "executed"
        },
        "disassembly": None,
        "verifier": None,
        "ir_dump": None,
        "run_aot": None,
        "run": {
            "error": "", "exit_code": 0, "output": "run and compile"
        }
    }
    if disasm:
        expected["disassembly"] = {
            "code": "disassembly output",
            "error": "",
            "exit_code": 0,
            "output": "disassembly"
        }
    if verifier:
        expected["verifier"] = {
            "output": "verified",
            "error": "",
            "exit_code": 0
        }
    assert res["run"]["exit_code"] == expected["run"]["exit_code"]
    assert res["run"]["error"] == expected["run"]["error"]
    assert res["run"]["output"] == expected["run"]["output"]
    res_without_run = {k: v for k, v in res.items() if k != "run"}
    expected_without_run = {k: v for k, v in expected.items() if k != "run"}
    assert res_without_run == expected_without_run


@pytest.mark.asyncio
async def test_run_when_compile_failed(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"compile error",
                        return_code=-1)
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())
    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", [], False)
    log_entry_assert(cap_logs[0], event_expected="Compile and run pipeline started")
    log_entry_assert(cap_logs[-1], event_expected="Compilation failed with nonzero exit code", level_expected="error")

    res = await r.compile_run_arkts("code", [], False)
    expected = {
        "compile": {
            "error": "",
            "exit_code": -1,
            "output": "compile error"
        },
        "disassembly": None,
        "verifier": None,
        "ir_dump": None,
        "run_aot": None,
        "run": None
    }
    assert res == expected


@pytest.mark.asyncio
async def test_run_when_compile_segfault(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"compile error",
                        return_code=-11)
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())
    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", [], False)
    log_entry_assert(cap_logs[0], event_expected="Compile and run pipeline started")
    log_entry_assert(cap_logs[1], event_expected="Compilation failed: segmentation fault", level_expected="error")
    expected = {
        "compile": {
            "error": "compilation: Segmentation fault",
            "exit_code": -11,
            "output": "compile error"
        },
        "disassembly": None,
        "verifier": None,
        "ir_dump": None,
        "run_aot": None,
        "run": None
    }
    assert res == expected


async def test_run_disasm_failed(monkeypatch, ark_build, ):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stderr=b"runtime error",
                        return_code=1),
            FakeCommand(expected=str(ark_build[2]),
                        opts=[],
                        stderr=b"disassembly failed",
                        return_code=-2)
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", [], True)
    log_entry_assert(cap_logs[0], event_expected="Compile and run pipeline started")
    log_entry_assert(cap_logs[2], event_expected="Disassembling failed with nonzero exit code")

    assert res["run"]["exit_code"] == 1
    assert res["run"]["error"] == "runtime error"
    assert res["run"]["output"] == ""
    expected = {
        "compile": {
            "error": "",
            "exit_code": 0,
            "output": "executed"
        },
        "disassembly": {
            "code": None,
            "exit_code": -2,
            "error": "disassembly failed",
            "output": ""
        },
        "verifier": None,
        "ir_dump": None,
        "run_aot": None
    }
    res_without_run = {k: v for k, v in res.items() if k != "run"}
    assert res_without_run == expected


async def test_compile_failed_with_disasm(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"compile error",
                        return_code=1),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())
    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.compile_arkts("code", [], disasm=True, verifier=False)
    log_entry_assert(cap_logs[0], event_expected="Compilation pipeline started")
    log_entry_assert(cap_logs[-1], event_expected="Compilation failed with nonzero exit code", level_expected="error")
    expected = {
        "compile": {
            "error": "",
            "exit_code": 1,
            "output": "compile error"
        },
        "disassembly": None,
        "verifier": None,
        "ir_dump": None
    }
    assert res == expected


@pytest.mark.asyncio
@pytest.mark.parametrize("verification_mode,expected_opts", [
    (VerificationMode.DISABLED, []),
    (VerificationMode.AHEAD_OF_TIME, ["--verification-mode=ahead-of-time"]),
    (VerificationMode.ON_THE_FLY, ["--verification-mode=on-the-fly"]),
])
async def test_run_with_verification_modes(monkeypatch, ark_build, verification_mode, expected_opts):
    run_opts = [f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets"] + expected_opts + ["ETSGLOBAL::main"]
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=run_opts,
                        stdout=b"run output",
                        return_code=0),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], False, verification_mode=verification_mode)
    assert res["run"]["exit_code"] == 0
    assert res["run"]["error"] == ""
    assert res["run"]["output"] == "run output"
    assert res["compile"] == {"error": "", "exit_code": 0, "output": "executed"}
    assert res["disassembly"] is None
    assert res["verifier"] is None
    assert res["ir_dump"] is None
    assert res["run_aot"] is None


async def test_get_versions(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--version"],
                        expected=str(ark_build[3]),
                        stdout=b"arkts ver",
                        return_code=1),
            FakeCommand(opts=["--version"],
                        expected=str(ark_build[1]),
                        stderr=b"es2panda ver",
                        return_code=1),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())
    monkeypatch.setattr("src.arkts_playground.deps.runner.version", lambda _: "1.2.3")

    r = Runner(ark_build[0])
    res = await r.get_versions()
    expected = "1.2.3", "arkts ver", "es2panda ver"
    assert res == expected


@pytest.mark.asyncio
async def test_dump_ast_ok(monkeypatch, ark_build, fake_saved_stsfile):
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),  # es2panda
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc"],
            stdout=b"AST TREE",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())

    r = Runner(ark_build[0])
    ast_result = await r.dump_ast("code")
    assert ast_result == {
        "ast": "AST TREE",
        "output": "AST TREE",
        "error": "",
        "exit_code": 0,
    }


@pytest.mark.asyncio
async def test_dump_ast_with_opts(monkeypatch, ark_build, fake_saved_stsfile):
    compile_opts = ["--opt-level=2"]
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc", *compile_opts],
            stdout=b"AST",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())

    r = Runner(ark_build[0])
    ast_result = await r.dump_ast("code", compile_opts)

    assert ast_result == {
        "ast": "AST",
        "output": "AST",
        "error": "",
        "exit_code": 0,
    }


@pytest.mark.asyncio
async def test_dump_ast_stdout_empty_not_use_stderr(monkeypatch, ark_build, fake_saved_stsfile):
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc"],
            stdout=b"",
            stderr=b"some log in stderr",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())
    r = Runner(ark_build[0])
    ast_result = await r.dump_ast("code")
    assert ast_result == {
        "ast": None,
        "output": "",
        "error": "some log in stderr",
        "exit_code": 0,
    }


@pytest.mark.asyncio
async def test_dump_ast_segfault(monkeypatch, ark_build, fake_saved_stsfile):
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc"],
            stdout=b"",
            stderr=b"oops",
            return_code=-11,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())

    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        ast_result = await r.dump_ast("code")
    log_entry_assert(cap_logs[-1], event_expected="AST dump failed: segmentation fault", level_expected="error")
    assert ast_result == {
        "ast": None,
        "output": "",
        "error": "oopsast: Segmentation fault",
        "exit_code": -11,
    }


@pytest.mark.asyncio
async def test_compile_utf8_decode_with_replace(monkeypatch, ark_build):
    """
    Ensure _execute_cmd decodes stdout/stderr with errors='replace'
    and does not crash on non-UTF8 bytes.
    """
    mocker = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),         # es2panda
            opts=["--extension=ets"],
            stdout=b"\xf7\xfa non-utf8 \xa0",
            stderr=b"\xff\xfe error bytes",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_arkts("code", [], disasm=False, verifier=False)

    assert res["compile"]["exit_code"] == 0
    # Replacement characters should be present instead of raising UnicodeDecodeError
    assert "\ufffd" in res["compile"]["output"]
    assert "\ufffd" in res["compile"]["error"]


@pytest.fixture(name="out_pa_file")
def out_file_with_cleanup():
    out_file = "out.pa"
    yield out_file
    if os.path.exists(out_file):
        os.remove(out_file)


@pytest.mark.asyncio
async def test_disassembly_opens_file_with_errors_replace(monkeypatch, ark_build, out_pa_file):
    """
    Ensure disassembly opens the .pa file with encoding='utf-8' and errors='replace'.
    """
    mocker = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[2]),  # ark_disasm
            opts=[],
            stdout=b"",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    captured: dict[str, object] = {}

    class _DummyFile:
        async def read(self) -> str:
            return "X"

    class _DummyCtx:
        async def __aenter__(self):
            return _DummyFile()

        async def __aexit__(self, exc_type, exc, tb):
            return False

    def open_spy(path, mode="r", encoding=None, errors=None):
        captured["path"] = path
        captured["mode"] = mode
        captured["encoding"] = encoding
        captured["errors"] = errors
        return _DummyCtx()

    monkeypatch.setattr("src.arkts_playground.deps.runner.aiofiles.open", open_spy)

    r = Runner(ark_build[0])
    res = await r.disassembly("in.abc", out_pa_file)

    assert res["exit_code"] == 0
    assert res["code"] == "X"

    assert captured.get("encoding") == "utf-8"
    assert captured.get("errors") == "replace"


def test_parse_compyile_options():
    """Test parse_compile_options extracts opt-level correctly."""
    assert not Runner.parse_compile_options({})
    assert Runner.parse_compile_options({"--opt-level": "2"}) == ["--opt-level=2"]
    assert not Runner.parse_compile_options({"--other": "val"})


@pytest.mark.asyncio
async def test_run_segfault(monkeypatch, ark_build):
    """Test run handles segfault correctly."""
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stderr=b"",
                        return_code=-11),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", [], False)
    assert res["run"]["exit_code"] == -11
    assert "run: Segmentation fault" in res["run"]["error"]
    segfault_logs = [l for l in cap_logs if "segmentation fault" in l.get("event", "")]
    assert len(segfault_logs) > 0


@pytest.mark.asyncio
async def test_ast_dump_failure_nonzero(monkeypatch, ark_build, fake_saved_stsfile):
    """Test AST dump with non-zero non-segfault exit code."""
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc"],
            stdout=b"",
            stderr=b"parse error",
            return_code=1,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())

    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.dump_ast("code")
    assert res["exit_code"] == 1
    assert res["ast"] is None
    log_entry_assert(cap_logs[-1], event_expected="Ast dump completed")


@pytest.mark.asyncio
async def test_compile_run_with_aot_mode(monkeypatch, ark_build):
    """Test compile_run_arkts with AOT mode enabled."""
    boot_panda = f"--boot-panda-files={ark_build[4]}"
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[boot_panda, "--load-runtimes=ets", "--aot-file=", "ETSGLOBAL::main"],
                        stdout=b"aot run output",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[boot_panda, "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stdout=b"jit output",
                        return_code=0),
            FakeCommand(expected=str(ark_build[7]),
                        opts=["--paoc-mode=aot", "--load-runtimes=ets", "--compiler-ignore-failures=false"],
                        stdout=b"aot compile output",
                        return_code=0),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], False, aot_mode=True)
    assert res["run"]["exit_code"] == 0
    assert res["run"]["output"] == "jit output"
    assert res["run_aot"]["exit_code"] == 0
    assert "aot run output" in res["run_aot"]["output"]


@pytest.mark.asyncio
async def test_aot_compile_failure(monkeypatch, ark_build):
    """Test AOT compilation failure handling."""
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stdout=b"jit output",
                        return_code=0),
            FakeCommand(expected=str(ark_build[7]),
                        opts=["--paoc-mode=aot", "--load-runtimes=ets", "--compiler-ignore-failures=false"],
                        stdout=b"aot stdout",
                        stderr=b"aot error",
                        return_code=1),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", [], False, aot_mode=True)
    assert res["run_aot"]["exit_code"] == 1
    assert "AOT compilation failed:" in res["run_aot"]["error"]
    aot_fail_logs = [l for l in cap_logs if "AOT compilation failed" in l.get("event", "")]
    assert len(aot_fail_logs) > 0


@pytest.mark.asyncio
async def test_aot_compile_segfault(monkeypatch, ark_build):
    """Test AOT compilation segfault handling."""
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stdout=b"jit output",
                        return_code=0),
            FakeCommand(expected=str(ark_build[7]),
                        opts=["--paoc-mode=aot", "--load-runtimes=ets", "--compiler-ignore-failures=false"],
                        stderr=b"crash",
                        return_code=-11),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], False, aot_mode=True)
    assert res["run_aot"]["exit_code"] == -11
    assert "AOT compilation: Segmentation fault" in res["run_aot"]["error"]


@pytest.mark.asyncio
async def test_aot_run_nonzero(monkeypatch, ark_build):
    """Test AOT run with non-zero exit code."""
    boot_panda = f"--boot-panda-files={ark_build[4]}"
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[boot_panda, "--load-runtimes=ets", "--aot-file=", "ETSGLOBAL::main"],
                        stderr=b"runtime error",
                        return_code=1),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[boot_panda, "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stdout=b"jit output",
                        return_code=0),
            FakeCommand(expected=str(ark_build[7]),
                        opts=["--paoc-mode=aot", "--load-runtimes=ets", "--compiler-ignore-failures=false"],
                        stdout=b"aot compile",
                        return_code=0),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], False, aot_mode=True)
    assert res["run_aot"]["exit_code"] == 1


@pytest.mark.asyncio
async def test_aot_missing_binary(monkeypatch, tmp_path):
    """Test AOT mode when ark_aot binary doesn't exist."""
    build_path = tmp_path / "build_no_aot"
    bin_path = build_path / "bin"
    plugin_path = build_path / "plugins" / "ets"
    plugin_path.mkdir(parents=True)
    bin_path.mkdir()
    (bin_path / "es2panda").write_text("")
    (bin_path / "ark_disasm").write_text("")
    (bin_path / "ark").write_text("")
    (plugin_path / "etsstdlib.abc").write_text("")
    (bin_path / "verifier").write_text("")

    stdlib_abc = plugin_path / "etsstdlib.abc"
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(bin_path / "es2panda"),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(bin_path / "ark"),
                        opts=[f"--boot-panda-files={stdlib_abc}", "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stdout=b"jit output",
                        return_code=0),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(build_path)
    res = await r.compile_run_arkts("code", [], False, aot_mode=True)
    assert res["run_aot"]["exit_code"] == 1
    assert "AOT mode requires ark_aot binary" in res["run_aot"]["error"]


@pytest.mark.asyncio
async def test_compile_run_with_ir_dump(monkeypatch, ark_build):
    """Test compile_run_arkts with IR dump options."""
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stdout=b"run output",
                        return_code=0),
            FakeCommand(expected=str(ark_build[7]),
                        opts=["--paoc-mode=aot", "--load-runtimes=ets", "--compiler-dump"],
                        stdout=b"ir dump output",
                        return_code=0),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], False, ir_dump={"compiler_dump": True, "disasm_dump": False})
    assert res["run"]["exit_code"] == 0
    assert res["ir_dump"] is not None


@pytest.mark.asyncio
async def test_compile_with_ir_dump(monkeypatch, ark_build):
    """Test compile_arkts with IR dump options."""
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[7]),
                        opts=["--paoc-mode=aot", "--load-runtimes=ets", "--compiler-dump"],
                        stdout=b"ir dump",
                        return_code=0),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    ir_dump_opts = {"compiler_dump": True, "disasm_dump": False}
    res = await r.compile_arkts("code", [], disasm=False, verifier=False, ir_dump=ir_dump_opts)
    assert res["ir_dump"] is not None


@pytest.mark.asyncio
async def test_ir_dump_abc_not_found(ark_build):
    """Test generate_ir_dump when ABC file doesn't exist."""
    r = Runner(ark_build[0])
    ir_dump_opts = {"compiler_dump": True, "disasm_dump": False}
    res = await r.generate_ir_dump("/nonexistent/file.abc", "/tmp", ir_dump_opts)
    assert res["exit_code"] == 1
    assert "ABC file not found" in res["error"]


@pytest.mark.asyncio
async def test_ir_dump_nonzero_exit(monkeypatch, ark_build, tmp_path):
    """Test generate_ir_dump handles non-zero exit code."""
    abc_file = tmp_path / "test.abc"
    abc_file.write_text("")

    mocker = MockAsyncSubprocess([
        FakeCommand(expected=str(ark_build[7]),
                    opts=["--paoc-mode=aot", "--load-runtimes=ets", "--compiler-dump"],
                    stderr=b"error",
                    return_code=1),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        res = await r.generate_ir_dump(str(abc_file), str(tmp_path), {"compiler_dump": True, "disasm_dump": False})
    assert res["exit_code"] == 1
    log_entry_assert(cap_logs[-1], event_expected="IR dump failed with nonzero exit code", level_expected="error")


@pytest.mark.asyncio
async def test_ir_dump_success_with_files(monkeypatch, ark_build, tmp_path):
    """Test generate_ir_dump successfully reads dump files."""
    abc_file = tmp_path / "test.abc"
    abc_file.write_text("")

    mocker = MockAsyncSubprocess([
        FakeCommand(expected=str(ark_build[7]),
                    opts=["--paoc-mode=aot", "--load-runtimes=ets", "--compiler-dump"],
                    stdout=b"success",
                    return_code=0),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    dump_folder = tmp_path / "ir_dump"
    dump_folder.mkdir()
    (dump_folder / "dump1.ir").write_text("IR content 1")
    (dump_folder / "dump2.ir").write_text("IR content 2")
    disasm_file = dump_folder / "ir_disasm.txt"
    disasm_file.write_text("disasm content")

    r = Runner(ark_build[0])
    res = await r.generate_ir_dump(str(abc_file), str(tmp_path), {"compiler_dump": True, "disasm_dump": True})
    assert res["exit_code"] == 0
    assert res["compiler_dump"] is not None
    assert "IR content 1" in res["compiler_dump"]
    assert "IR content 2" in res["compiler_dump"]
    assert res["disasm_dump"] == "disasm content"


@pytest.mark.asyncio
async def test_execute_cmd_timeout(monkeypatch, ark_build):
    """Test _execute_cmd handles timeout."""

    class TimeoutProcess:
        returncode = None

        async def communicate(self):
            await asyncio.sleep(10)
            return b"", b""

        def kill(self):
            pass

    async def mock_create_subprocess(*_args, **_kwargs):
        return TimeoutProcess()

    monkeypatch.setattr("asyncio.create_subprocess_exec", mock_create_subprocess)

    r = Runner(ark_build[0], timeout=0.01)
    with capture_logs() as cap_logs:
        # pylint: disable=protected-access
        _, stderr, _ = await r._execute_cmd("test_cmd")
    assert stderr == "Timeout expired"
    log_entry_assert(cap_logs[-1], event_expected="Subprocess execution timed out", level_expected="error")


@pytest.mark.asyncio
async def test_playground_version_not_installed(monkeypatch, ark_build):
    """Test _get_playground_version handles PackageNotFoundError."""

    def raise_not_found(_):
        raise PackageNotFoundError()

    monkeypatch.setattr("src.arkts_playground.deps.runner.version", raise_not_found)

    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--version"],
                        expected=str(ark_build[3]),
                        stdout=b"arkts ver",
                        return_code=1),
            FakeCommand(opts=["--version"],
                        expected=str(ark_build[1]),
                        stderr=b"es2panda ver",
                        return_code=1),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    playground_ver, _, _ = await r.get_versions()
    assert playground_ver == "not installed"
