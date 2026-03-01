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

import copy
import logging
import re
import sys
import warnings
from collections.abc import Iterator
from contextlib import contextmanager
from traceback import StackSummary, extract_tb
from types import TracebackType
from typing import Optional, Tuple, Type

import pluggy
from _pytest.nodes import Item
from pytest import FixtureRequest, Function, StashKey, fixture, hookimpl, mark
from rich.traceback import Traceback
from typing_extensions import TypeVar

from .logs import logger
from .rich_logging import remove_ansi_escape_sequences

LOG = logger(__name__)


class ExpectWarning(UserWarning):
    pass


class ExpectErrorWarning(UserWarning):
    pass


class ExpectError(Exception):
    pass


def tb_next(tb, step: int = 1) -> Optional[TracebackType]:
    for _ in range(step):
        if tb is None:
            return None
        tb = tb.tb_next
    return tb


def _extract_traceback(level: int = 4) -> Optional[Tuple[StackSummary, Traceback]]:
    match sys.exc_info():
        case (None, _, _) | (_, None, _) | (_, _, None):
            return None
        case (exc_type, exc_value, traceback):
            assert isinstance(exc_value, BaseException)
            new_exc_value = copy.copy(exc_value)
            new_exc_value.args = tuple(remove_ansi_escape_sequences(a) for a in new_exc_value.args)

            tb = tb_next(traceback, 2)
            return (
                extract_tb(tb),
                Traceback.from_exception(
                    exc_type=exc_type,  # type: ignore[arg-type]
                    exc_value=new_exc_value,
                    traceback=tb,
                    suppress=(__file__,),
                ),
            )
    return None


E = TypeVar("E", bound=BaseException)


class Expect:
    fail = False

    @contextmanager
    def _check(self, level: int, waring_type: Type[Warning]) -> Iterator[None]:
        try:
            yield
        except AssertionError as e:
            tb = _extract_traceback()
            if tb is None:
                raise e
            stacks, rich_tb = tb
            s = stacks[0]
            m = re.match(".*", str(e))
            msg = m.group() if m else str(e)
            LOG.log(level, "%s", msg, rich=rich_tb)
            warnings.warn_explicit(
                waring_type(e),
                category=waring_type,
                filename=s.filename,
                lineno=(s.lineno or 0),
                module=s.name,
            )

    @contextmanager
    def warning(self) -> Iterator[None]:
        """
        Return a context that registers AssertError as a warning and continues execution.
        """
        with self._check(logging.WARNING, ExpectWarning) as c:
            yield c

    @contextmanager
    def error(self) -> Iterator[None]:
        """
        Return a context that registers AssertError as a error and continues execution.
        """
        with self._check(logging.ERROR, ExpectErrorWarning) as c:
            try:
                yield c
            except BaseException:
                self.fail = True
                raise


def expect_xfail(**kwargs):
    """
    Pytest ``xfail`` mark for ``expect`` fixture.
    """

    raises: None | type[BaseException] | tuple[type[BaseException], ...] = kwargs.get("raises", tuple())
    if isinstance(raises, BaseException):
        raises = (raises,)
    elif isinstance(raises, tuple):
        pass
    elif raises is None:
        raises = tuple()
    else:
        raise ValueError(raises)
    kwargs = {
        "raises": (ExpectError, *raises),
        **kwargs,
    }
    return mark.xfail(**kwargs)


context_key = StashKey[Expect]()


@fixture(scope="function")
def expect(request: FixtureRequest) -> Expect:
    """
    Return a :class:`Expect` instance that implements the expect feature.
    """
    node: Function = request.node
    return node.stash.setdefault(context_key, Expect())


@hookimpl(hookwrapper=True)
def pytest_runtest_call(item: Item):
    __tracebackhide__ = True
    context: Expect = item.stash.setdefault(context_key, Expect())
    outcome: pluggy.Result = yield
    if context.fail:
        outcome.force_exception(ExpectError())
