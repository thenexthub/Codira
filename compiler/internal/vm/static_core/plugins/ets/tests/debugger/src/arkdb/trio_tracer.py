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

from collections.abc import Sequence
from contextlib import contextmanager
from typing import Callable

import trio
from pytest import Config, Parser
from typing_extensions import override

from .ark_config import get_config, str2bool
from .logs import TRIO, logger

LOG = logger(__name__)


@contextmanager
def task_instrumentation(**kwargs):
    try:
        t = TrioTracer(**kwargs)
        trio.lowlevel.add_instrument(t)
        yield
    finally:
        trio.lowlevel.remove_instrument(t)


class TrioTracer(trio.abc.Instrument):

    def __init__(self, show_io: bool = True, show_run: bool = True) -> None:
        super().__init__()
        self.show_io = show_io
        self.show_run = show_run
        self._sleep_time = trio.current_time()

    @override
    def before_run(self):
        self._log("!!! STARTED")

    @override
    def after_run(self):
        self._log("!!! FINISHED")

    @override
    def task_spawned(self, task: trio.lowlevel.Task):
        self._task_log("+++ SPAWNED     ", task)

    @override
    def task_scheduled(self, task: trio.lowlevel.Task):
        self._task_log("    SCHEDULED   ", task)

    @override
    def before_task_step(self, task: trio.lowlevel.Task):
        self._task_log("* { BEFORE STEP ", task)

    @override
    def after_task_step(self, task: trio.lowlevel.Task):
        self._task_log("  } AFTER STEP  ", task)

    @override
    def task_exited(self, task: trio.lowlevel.Task):
        self._task_log("--- EXITED      ", task)

    @override
    def before_io_wait(self, timeout):
        if not self.show_io:
            return
        if timeout:
            self._log(f". waiting for I/O for up to {timeout} seconds")
        else:
            self._log(". doing a quick check for I/O")
        self._sleep_time = trio.current_time()

    @override
    def after_io_wait(self, timeout):
        if not self.show_io:
            return
        duration = trio.current_time() - self._sleep_time
        self._log(f". finished I/O check (took {duration} seconds)")

    def _log(self, msg: str, *args, **kwargs):
        LOG.log(TRIO, msg, *args, **kwargs)

    def _task_log(self, msg: str, task: trio.lowlevel.Task):
        self._log(f"{msg}: %r", task)


def pytest_addoption(parser: Parser) -> None:
    """Add options to control log capturing."""
    group = parser.getgroup("arkdb")

    parser.addini(
        name="task_trace",
        default=False,
        type="bool",
        help="Default value for --task-trace",
    )
    group.addoption(
        "--task-trace",
        "-T",
        dest="task_trace",
        help="Enable trio task traces.",
        type=str2bool,
    )


def choose_run(config: Config) -> Callable[..., None]:

    def run(
        *args,
        instruments: Sequence[trio.abc.Instrument] = (),
        strict_exception_groups: bool = False,
        **kwargs,
    ) -> None:
        if get_config(config, "task_trace"):
            if not any([isinstance(i, TrioTracer) for i in instruments]):
                instruments = [*instruments, TrioTracer()]
        trio.run(
            *args,
            instruments=instruments,
            strict_exception_groups=strict_exception_groups,
            **kwargs,
        )

    return run
