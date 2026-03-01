#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import math
import subprocess  # noqa: S404
from collections.abc import AsyncIterator
from contextlib import asynccontextmanager
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Literal, Union

import trio
from pytest import fixture

from .logs import ARK_ERR, ARK_OUT, logger
from .runnable_module import RunnableModule

LOG = logger(__name__)
DEFAULT_ENTRY_POINT = "ETSGLOBAL::main"
ComponentType = Literal[
    "all",
    "alloc",
    "mm-obj-events",
    "classlinker",
    "common",
    "core",
    "gc",
    "gc_trigger",
    "reference_processor",
    "interpreter",
    "compiler",
    "llvm",
    "pandafile",
    "memorypool",
    "runtime",
    "trace",
    "debugger",
    "interop",
    "verifier",
    "compilation_queue",
    "aot",
    "events",
    "ecmascript",
    "scheduler",
    "coroutines",
    "task_manager",
]


@dataclass
class Options:
    """
    Options for ArkTS runtime execution.
    """

    app_path: Path = Path("bin", "ark")
    load_runtimes: Literal["core", "ecmascript", "ets"] = "ets"
    interpreter_type: Literal["cpp", "irtoc", "llvm"] = "cpp"
    boot_panda_files: List[Path] = field(default_factory=list)
    debugger_library_path: Path = Path()
    log_debug: List[ComponentType] = field(default_factory=lambda: ["debugger"])
    print_stdout: bool = False


MessageMarker = Union[trio.Event]
Message = Union[str, MessageMarker]
SendChannel = trio.MemorySendChannel[Message]
ReceiveChannel = trio.MemoryReceiveChannel[Message]


def open_memory_channels():
    return trio.open_memory_channel[Message](math.inf)


@dataclass
class ProcessCapture:
    stdout: List[str] = field(default_factory=list)
    stderr: List[str] = field(default_factory=list)


async def _capture_channel(
    channel: ReceiveChannel,
    array: List[str],
) -> None:
    async with channel:
        async for data in channel:
            if isinstance(data, str):
                array.append(data)
            elif isinstance(data, trio.Event):
                data.set()


class RuntimeProcess:
    """
    The context of ArkTS runtime execution.
    """

    def __init__(
        self,
        process: trio.Process,
        nursery: trio.Nursery,
        stdout_channel: ReceiveChannel,
        stderr_channel: ReceiveChannel,
    ) -> None:
        self.stdout_parser: StreamParser | None = None
        self.stderr_parser: StreamParser | None = None
        self.process = process
        self.nursery = nursery
        self.capture = ProcessCapture()

        nursery.start_soon(
            _capture_channel,
            stdout_channel.clone(),
            self.capture.stdout,
            name="Runtime STDOUT capture to list",
        )
        nursery.start_soon(
            _capture_channel,
            stderr_channel.clone(),
            self.capture.stderr,
            name="Runtime STDERR capture to list",
        )

    @property
    def returncode(self) -> int | None:
        return self.process.returncode

    def start_parser(self, send_stdout: SendChannel, send_stderr: SendChannel):
        if self.process.stdout:
            self.stdout_parser = StreamParser(self.process.stdout, send_stdout.clone(), "STDOUT", ARK_OUT)
            self.nursery.start_soon(self.stdout_parser.parse, name="Runtime STDOUT parser")
        if self.process.stderr:
            self.stderr_parser = StreamParser(self.process.stderr, send_stderr.clone(), "STDERR", ARK_ERR)
            self.nursery.start_soon(self.stderr_parser.parse, name="Runtime STDERR parser")

    def terminate(self) -> None:
        self.process.terminate()

    def kill(self) -> None:
        self.process.kill()

    async def wait(self) -> int:
        return await self.process.wait()

    def cancel(self) -> None:
        self.nursery.cancel_scope.cancel()

    async def close(self) -> int:
        self.cancel()
        result = self.process.poll()
        if result is not None:
            await trio.lowlevel.checkpoint()
            return result
        self.terminate()
        return await self.wait()

    async def sync_capture(self) -> None:
        out = self.stdout_parser and self.stdout_parser.sync()
        err = self.stderr_parser and self.stderr_parser.sync()
        await trio.lowlevel.checkpoint()
        if out:
            await out.wait()
        if err:
            await err.wait()


class StreamParser:
    def __init__(
        self,
        stream: trio.abc.ReceiveStream,
        channel: SendChannel,
        name: str,
        loglevel: int,
    ) -> None:
        self._sync_event = trio.Event()
        self._sync_event.set()
        self._stream = stream
        self._channel = channel
        self._name = name
        self._loglevel = loglevel

    def sync(self) -> trio.Event:
        e = trio.Event()
        self._sync_event = e
        return e

    async def parse(self) -> None:
        try:
            buffer = bytearray()
            async with self._stream, self._channel:
                await self._process_buffer(buffer)
        except trio.Cancelled:
            with trio.CancelScope(deadline=1, shield=True):
                await self._process_buffer(buffer)
            raise
        finally:
            if len(buffer) > 0:
                msg = bytes(buffer)
                self._send(msg)
            self._sync_event.set()

    def _send(self, data: bytes):
        text = data.decode(errors="replace")
        LOG.log(self._loglevel, text)
        self._channel.send_nowait(text)

    async def _process_buffer(self, buffer: bytearray):
        async for data in self._stream:
            LOG.debug("Read from '%s': %s", self._name, data)
            if len(data) == 0:
                continue
            start = 0
            while True:
                end = data.find(b"\n", start)
                if end == -1:
                    buffer += data[start:]
                    break
                else:
                    msg = bytes(buffer + data[start:end])
                    start = end + 1
                    buffer.clear()
                    self._send(msg)
            self._sync_event.set()


class Runtime:
    """
    Controls the ArkTS runtime.
    """

    def __init__(self, options: Options) -> None:
        self.options = options
        pass

    @asynccontextmanager
    async def run(
        self,
        nursery: trio.Nursery,
        /,
        module: RunnableModule,
        entry_point: str = DEFAULT_ENTRY_POINT,
        cwd: Path | None = None,
        debug: bool = True,
        profile: bool = False,
        additional_options: Options | None = None,
    ) -> AsyncIterator[RuntimeProcess]:
        module.check_exists()
        o = self.options
        ao = additional_options
        boot_panda_files = [str(f) for f in o.boot_panda_files + module.boot_abc]
        if ao is not None:
            boot_panda_files += [str(f) for f in ao.boot_panda_files]

        command = [
            str(o.app_path),
            f"--load-runtimes={o.load_runtimes}",
            f"--interpreter-type={o.interpreter_type}",
            f"--boot-panda-files={':'.join(boot_panda_files)}",
            "--load-in-boot",
        ]
        debugger_library_option: str = f"--debugger-library-path={str(o.debugger_library_path)}"
        if profile:
            command.append("--sampling-profiler-create")
            command.append("--workers-type=threadpool")
            command.append(debugger_library_option)
        elif debug:
            command.append("--debugger-break-on-start")
            command.append(debugger_library_option)
            command.extend([f"--log-debug={c}" for c in o.log_debug])
        command.append(str(module.entry_abc))
        command.append(f"{module.entry_abc.stem}.{entry_point}")

        send_stdout, receive_stdout = open_memory_channels()
        send_stderr, receive_stderr = open_memory_channels()
        LOG.debug("Memory channels have been created.")
        async with send_stdout, receive_stdout, send_stderr, receive_stderr:
            process: trio.Process = await trio.lowlevel.open_process(
                command=command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=cwd,
            )
            LOG.info("Exec runtime %s", command)
            run_proc = RuntimeProcess(
                process=process,
                nursery=nursery,
                stdout_channel=receive_stdout,
                stderr_channel=receive_stderr,
            )
            run_proc.start_parser(send_stdout=send_stdout, send_stderr=send_stderr)

        async with trio.open_nursery() as runtime_nursery:
            runtime_nursery.start_soon(_runtime_wait_task, process)
            yield run_proc


async def _runtime_wait_task(process: trio.Process):
    LOG.debug("Waiting for Runtime.")
    try:
        returncode = await process.wait()
        LOG.debug("Runtime exit status %s.", returncode)
    except BaseException:
        process.terminate()
        with trio.CancelScope(deadline=1, shield=True):
            try:
                await process.wait()
            except BaseException:
                process.kill()
                raise
        raise


@fixture
def ark_runtime(
    ark_runtime_options: Options,
) -> Runtime:
    """
    Return a :class:`runtime.Runtime` instance that controls the ArkTS runtime.
    """
    return Runtime(options=ark_runtime_options)


@fixture
@asynccontextmanager
async def ark_runtime_run_data_files(
    nursery: trio.Nursery,
    ark_runtime: Runtime,
    module: RunnableModule,
) -> AsyncIterator[RuntimeProcess]:
    async with ark_runtime.run(nursery, module=module) as process:
        yield process
