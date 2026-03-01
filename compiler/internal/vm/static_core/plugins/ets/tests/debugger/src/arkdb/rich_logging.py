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

import io
import logging
import os
import re
import sys
from contextlib import nullcontext
from io import StringIO
from pathlib import Path
from types import TracebackType
from typing import Any, Final, Generator, Generic, Type, TypeVar

# CC-OFFNXT(warning_suppression) Pylint warning suppression.
from _pytest import nodes  # pylint:disable=protected-access

# CC-OFFNXT(warning_suppression) Pylint warning suppression.
from _pytest.capture import CaptureManager  # pylint:disable=protected-access

# CC-OFFNXT(warning_suppression) Pylint warning suppression.
from _pytest.config import Config, UsageError, hookimpl  # pylint:disable=protected-access

# CC-OFFNXT(warning_suppression) Pylint warning suppression.
from _pytest.config.argparsing import Parser  # pylint:disable=protected-access

# CC-OFFNXT(warning_suppression) Pylint warning suppression.
from _pytest.main import Session  # pylint:disable=protected-access

# CC-OFFNXT(warning_suppression) Pylint warning suppression.
from _pytest.stash import StashKey  # pylint:disable=protected-access
from pytest import OptionGroup
from rich import get_console, reconfigure
from rich.box import SIMPLE
from rich.console import Console, ConsoleRenderable
from rich.logging import RichHandler
from rich.panel import Panel
from rich.style import Style
from rich.text import Text
from rich.theme import Theme
from typing_extensions import override

from . import logs

_CapLogRecordType = dict[str, list[logging.LogRecord]]

_ANSI_ESCAPE_SEQ = re.compile(r"\x1b\[[\d;]+m")
_CAPLOG_HANDLER_KEY = StashKey["LogCaptureHandler"]()
_CAPLOG_RECORDS_KEY = StashKey[_CapLogRecordType]()

_THEME = Theme(
    styles={
        f"logging.level.{logs.ARK_OUT_NAME.lower()}": Style(color="cyan"),
        f"logging.level.{logs.ARK_ERR_NAME.lower()}": Style(color="hot_pink"),
        f"logging.level.{logs.TRIO_NAME.lower()}": Style(color="magenta"),
    },
)

_LOG_LEVEL_NAME: Final[str] = "log_level"
_LOG_FILE_LEVEL_NAME: Final[str] = "log_file_level"
_LOG_FILE_NAME: Final[str] = "log_file"
_LOG_FILE_MODE_NAME: Final[str] = "log_file_mode"
_LOG_CLI_LEVEL_NAME: Final[str] = "log_cli_level"

_PHASE_SETUP: Final[str] = "setup"
_PHASE_START: Final[str] = "start"
_PHASE_CALL: Final[str] = "call"
_PHASE_SESSION_FINISH: Final[str] = "sessionfinish"
_PHASE_TEARDOWN: Final[str] = "teardown"
_PHASE_FINISH: Final[str] = "finish"


def remove_ansi_escape_sequences(text: str) -> str:
    return _ANSI_ESCAPE_SEQ.sub("", text)


def get_option_ini(config: Config, *names: str):
    for name in names:
        ret = config.getoption(name)  # 'default' arg won't work as expected
        if ret is None:
            ret = config.getini(name)
        if ret:
            return ret
    return None


def _clean_option_init(group: OptionGroup):
    parser = group.parser
    if not parser:
        return
    remove = [o.dest for o in group.options]

    # CC-OFFNXT(warning_suppression) Pylint warning suppression.
    ininames = parser._ininames  # pylint:disable=protected-access
    # CC-OFFNXT(warning_suppression) Pylint warning suppression.
    inidict = parser._inidict  # pylint:disable=protected-access

    for dest in remove:
        if dest in ininames:
            ininames.remove(dest)
        if dest in inidict:
            del inidict[dest]
        for o in group.options:
            if dest == o.dest:
                group.options.remove(o)
                break


def pytest_addoption(parser: Parser) -> None:
    """Add options to control log capturing."""
    group = parser.getgroup("logging")
    _clean_option_init(group)

    def _add_option_ini(option, dest, default=None, opt_type=None, **kwargs):
        parser.addini(
            dest,
            default=default,
            type=opt_type,
            help=f"Default value for {option}",
        )
        group.addoption(option, dest=dest, **kwargs)

    _add_option_ini(
        "--log-level",
        dest=_LOG_LEVEL_NAME,
        default=None,
        metavar="LEVEL",
        help=(
            "Level of messages to catch/display."
            " Not set by default, so it depends on the root/parent log handler's"
            ' effective level, where it is "WARNING" by default.'
        ),
    )
    parser.addini(
        "log_cli", default=False, type="bool", help='Enable log display during test run (also known as "live logging")'
    )
    _add_option_ini("--log-cli-level", dest=_LOG_CLI_LEVEL_NAME, default=None, help="CLI logging level")
    _add_option_ini(
        "--log-file", dest=_LOG_FILE_NAME, default=None, help="Path to a file when logging will be written to"
    )
    _add_option_ini(
        "--log-file-mode", dest=_LOG_FILE_MODE_NAME, default="w", choices=["w", "a"], help="Log file open mode"
    )
    _add_option_ini("--log-file-level", dest=_LOG_FILE_LEVEL_NAME, default=None, help="Log file logging level")
    _add_option_ini("--width", dest="width", default="120", help="Default width of rich.Console()")
    group.addoption(
        "--log-disable",
        action="append",
        default=[],
        dest="logger_disable",
        help="Disable a logger by name. Can be passed multiple times.",
    )


_HandlerType = TypeVar("_HandlerType", bound=logging.Handler)


# Not using @contextmanager for performance reasons.
class CatchingLogs(Generic[_HandlerType]):

    __slots__ = ("handler", "level", "orig_level")

    def __init__(self, handler: _HandlerType, level: int | None = None) -> None:
        self.handler = handler
        self.level = level
        self.orig_level = logging.getLogger().level

    def __enter__(self) -> _HandlerType:
        if self.level is not None:
            self.handler.setLevel(self.level)

        root = logging.getLogger()
        root.addHandler(self.handler)
        if self.level is not None:
            self.orig_level = root.level
            min_level = min(self.orig_level, self.level)
            root.setLevel(min_level)
        return self.handler

    def __exit__(
        self,
        exc_type: Type[BaseException] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None,
    ) -> None:
        root_logger = logging.getLogger()
        if self.level is not None:
            root_logger.setLevel(self.orig_level)
        root_logger.removeHandler(self.handler)


class ExtRichHandler(RichHandler):
    @override
    def render_message(self, record: logging.LogRecord, message: str) -> ConsoleRenderable:
        msg = super().render_message(record, message)
        rich = getattr(record, "rich", None)
        if rich and isinstance(msg, Text):
            return Panel(title=msg, title_align="left", renderable=rich, box=SIMPLE, expand=False)
        return msg


class LogCaptureHandler(ExtRichHandler):

    def __init__(self, console: Console) -> None:
        super().__init__(console=console)
        self.console.file = StringIO()
        self.records: list[logging.LogRecord] = []

    @property
    def stream(self) -> StringIO:
        file = self.console.file
        if not isinstance(file, StringIO):
            raise RuntimeError()
        return file

    def emit(self, record: logging.LogRecord) -> None:
        self.records.append(record)
        super().emit(record)

    def clear(self) -> None:
        self.records.clear()
        self.console.file = StringIO()

    def reset(self) -> None:
        self.records = []
        self.console.file = StringIO()

    # CC-OFFNXT(G.NAM.01) This is override method of base class logging.Handler.handleError.
    def handleError(self, _: logging.LogRecord) -> None:
        if logging.raiseExceptions:
            e: BaseException | None = sys.exc_info()[1]
            if e:
                raise e


def _find_level(config: Config, *setting_names: str):
    for name in setting_names:
        level = config.getoption(name)
        if level is None:
            level = config.getini(name)
        if level:
            return name, level
    return None, None


def find_log_level(config: Config, *setting_names: str) -> int | None:
    name, level = _find_level(config, *setting_names)
    if name is None:
        return None
    level = level.upper() if isinstance(level, str) else level

    try:
        return int(getattr(logging, level, level))
    except ValueError as e:
        raise UsageError(
            f"'{level}' is not recognized as a logging level name for "
            f"'{name}'. Please consider passing the "
            "logging level num instead."
        ) from e


@hookimpl(trylast=True)
def pytest_configure(config: Config) -> None:
    config.pluginmanager.set_blocked("logging")
    config.pluginmanager.register(RichLoggingPlugin(config), "rich-logging")


@hookimpl(specname="pytest_configure")
def pytest_rich_configure(config: Config) -> None:
    reconfigure(**console_config(config))


def console_config(config: Config) -> dict[str, Any]:
    console_options: dict[str, Any] = {"theme": _THEME}

    console = get_console()
    if config.getoption("color") == "yes":
        console_options.update(force_terminal=True)
    elif config.getoption("color") == "no":
        console_options.update(force_terminal=False)
    else:
        console_options.update(force_terminal=console.is_terminal)
    console_options.update(width=console.width)
    width: int = console.width if console.is_terminal else 120
    if (width_opt := config.getoption("width")) and isinstance(width_opt, str) and width_opt.isdigit():
        width = int(width_opt)
    elif (width_opt := os.environ.get("COLUMNS")) and width_opt.isdigit():
        width = int(width_opt)
    console_options.update(width=width)
    os.environ["COLUMNS"] = str(width)

    return console_options


def create_console(config: Config, **kwargs) -> Console:
    return Console(**(console_config(config) | kwargs))


def create_directory_of_file(filepath: Path) -> None:
    # Will check directory existence as well
    if not filepath.parent.is_dir():
        filepath.parent.mkdir(parents=True, exist_ok=True)


def get_log_file_path(config: Config) -> str:
    log_file_path = get_option_ini(config, _LOG_FILE_NAME) or os.devnull
    if log_file_path != os.devnull:
        create_directory_of_file(Path(log_file_path).absolute())
    return log_file_path


class RichLoggingPlugin:
    def __init__(self, config: Config) -> None:
        self._config = config

        # File logging settings
        self.log_file_level = find_log_level(self._config, _LOG_FILE_LEVEL_NAME, _LOG_LEVEL_NAME)
        self.log_file_mode = get_option_ini(self._config, _LOG_FILE_MODE_NAME) or "w"
        self.log_file_handler = _FileHandler(
            get_log_file_path(self._config),
            mode=self.log_file_mode,
            encoding="UTF-8",
        )

        # Report logging settings
        self.log_level = find_log_level(self._config, _LOG_LEVEL_NAME)
        self.report_handler = LogCaptureHandler(
            create_console(self._config),
        )
        self.caplog_handler = LogCaptureHandler(
            create_console(self._config),
        )
        formater = logging.Formatter(fmt="%(message)s", datefmt="[%X]")
        self.report_handler.setFormatter(formater)
        self.caplog_handler.setFormatter(formater)

        # CLI logging settings
        self.log_cli_level = find_log_level(self._config, _LOG_CLI_LEVEL_NAME, _LOG_LEVEL_NAME)
        if self._log_cli_enabled():
            self.log_cli_handler: _RichLoggingStreamHandler | _RichLoggingNullHandler = _RichLoggingStreamHandler(
                create_console(self._config),
                self._config.pluginmanager.get_plugin("capturemanager"),
            )
            self.log_cli_handler.setFormatter(formater)
        else:
            self.log_cli_handler = _RichLoggingNullHandler()
        RichLoggingPlugin._disable_loggers(loggers_to_disable=self._config.option.logger_disable)

    @staticmethod
    def _disable_loggers(loggers_to_disable: list[str]) -> None:
        if not loggers_to_disable:
            return

        for name in loggers_to_disable:
            logger = logging.getLogger(name)
            logger.disabled = True

    def set_log_path(self, filename: str) -> None:
        path = Path(filename)
        path = path if path.is_absolute() else self._config.rootpath / path
        create_directory_of_file(path)

        stream: io.TextIOWrapper = path.open(mode=self.log_file_mode, encoding="UTF-8")  # type: ignore[assignment]
        if prev_stream := self.log_file_handler.setStream(stream):
            prev_stream.close()

    @hookimpl(wrapper=True, tryfirst=True)
    def pytest_sessionstart(self) -> Generator[None, None, None]:
        self.log_cli_handler.set_when("sessionstart")

        with CatchingLogs(self.log_cli_handler, level=self.log_cli_level):
            with CatchingLogs(self.log_file_handler, level=self.log_file_level):
                return (yield)

    @hookimpl(wrapper=True, tryfirst=True)
    def pytest_collection(self) -> Generator[None, None, None]:
        self.log_cli_handler.set_when("collection")

        with CatchingLogs(self.log_cli_handler, level=self.log_cli_level):
            with CatchingLogs(self.log_file_handler, level=self.log_file_level):
                return (yield)

    @hookimpl(wrapper=True)
    def pytest_runtestloop(self, session: Session) -> Generator[None, object, object]:
        if session.config.option.collectonly:
            return (yield)

        if self._log_cli_enabled() and self._config.getoption("verbose") < 1:
            self._config.option.verbose = 1

        with CatchingLogs(self.log_cli_handler, level=self.log_cli_level):
            with CatchingLogs(self.log_file_handler, level=self.log_file_level):
                return (yield)

    @hookimpl
    def pytest_runtest_logstart(self) -> None:
        self.log_cli_handler.reset()
        self.log_cli_handler.set_when(_PHASE_START)

    @hookimpl
    def pytest_runtest_logreport(self) -> None:
        self.log_cli_handler.set_when("logreport")

    @hookimpl(wrapper=True)
    def pytest_runtest_setup(self, item: nodes.Item) -> Generator[None, None, None]:
        self.log_cli_handler.set_when(_PHASE_SETUP)
        empty_val: _CapLogRecordType = {}
        item.stash[_CAPLOG_RECORDS_KEY] = empty_val
        yield from self._runtest_for(item, _PHASE_SETUP)

    @hookimpl(wrapper=True)
    def pytest_runtest_call(self, item: nodes.Item) -> Generator[None, None, None]:
        # CC-OFFNXT(G.NAM.05) hide redundant logs in logging facility
        __tracebackhide__ = True
        self.log_cli_handler.set_when(_PHASE_CALL)
        yield from self._runtest_for(item, _PHASE_CALL)

    @hookimpl(wrapper=True)
    def pytest_runtest_teardown(self, item: nodes.Item) -> Generator[None, None, None]:
        self.log_cli_handler.set_when(_PHASE_TEARDOWN)
        try:
            yield from self._runtest_for(item, _PHASE_TEARDOWN)
        finally:
            del item.stash[_CAPLOG_RECORDS_KEY]
            del item.stash[_CAPLOG_HANDLER_KEY]

    @hookimpl
    def pytest_runtest_logfinish(self) -> None:
        self.log_cli_handler.set_when(_PHASE_FINISH)

    @hookimpl(wrapper=True, tryfirst=True)
    def pytest_sessionfinish(self) -> Generator[None, None, None]:
        self.log_cli_handler.set_when(_PHASE_SESSION_FINISH)

        with CatchingLogs(self.log_cli_handler, level=self.log_cli_level):
            with CatchingLogs(self.log_file_handler, level=self.log_file_level):
                return (yield)

    @hookimpl
    def pytest_unconfigure(self) -> None:
        self.log_file_handler.close()

    def _log_cli_enabled(self) -> bool:
        """Return whether live logging is enabled."""
        return self._config.getoption("--log-cli-level") is not None or self._config.getini("log_cli")

    def _runtest_for(self, test_invocation_item: nodes.Item, when: str) -> Generator[None, None, None]:
        # CC-OFFNXT(G.NAM.05) hide redundant logs in logging facility
        __tracebackhide__ = True
        with CatchingLogs(
            self.report_handler,
            level=self.log_level,
        ) as report_handler:
            report_handler.reset()
            with CatchingLogs(
                self.caplog_handler,
                level=self.log_level,
            ) as caplog_handler:
                caplog_handler.reset()
                test_invocation_item.stash[_CAPLOG_HANDLER_KEY] = caplog_handler
                test_invocation_item.stash[_CAPLOG_RECORDS_KEY][when] = caplog_handler.records
                try:
                    yield
                finally:
                    log_content = report_handler.stream.getvalue().strip()
                    test_invocation_item.add_report_section(when, "log", log_content)


class _FileHandler(logging.FileHandler):
    """A logging FileHandler with pytest tweaks."""

    # override the logging.Handler.handleError method
    def handleError(self, _: logging.LogRecord) -> None:
        pass


class _RichLoggingStreamHandler(ExtRichHandler):

    def __init__(
        self,
        console: Console,
        capture_manager: CaptureManager | None,
    ) -> None:
        super().__init__(console=console)  # type: ignore[arg-type]
        self.capture_manager = capture_manager
        self.reset()
        self.set_when(None)
        self._test_outcome_was_written = False
        self._first_record_was_emitted = False
        self._section_name_was_shown = False
        self._when: str | None = None

    def set_when(self, when: str | None) -> None:
        self._section_name_was_shown = False
        self._when = when
        if self._when == _PHASE_START:
            self._test_outcome_was_written = False

    def reset(self) -> None:
        self._first_record_was_emitted = False

    def emit(self, log_record: logging.LogRecord) -> None:
        ctx_manager = self.capture_manager.global_and_fixture_disabled() if self.capture_manager else nullcontext()
        with ctx_manager:
            if not self._first_record_was_emitted:
                self.console.line()
                self._first_record_was_emitted = True
            elif self._when in (_PHASE_TEARDOWN, _PHASE_FINISH):
                if not self._test_outcome_was_written:
                    self._test_outcome_was_written = True
                    self.console.line()
            if not self._section_name_was_shown and self._when:
                text = Text.styled(f" cli-log {self._when} ", style="bold")
                text.align(align="center", width=self.console.width, character="-")
                self.console.print(text)
                self._section_name_was_shown = True
            super().emit(log_record)

    # CC-OFFNXT(G.NAM.01) This is override method of base class logging.Handler.handleError.
    def handleError(self, _: logging.LogRecord) -> None:
        pass


class _RichLoggingNullHandler(ExtRichHandler):
    def set_when(self, _: str):
        pass

    def reset(self):
        pass

    # CC-OFFNXT(G.NAM.01) This is override method of base class logging.Handler.handleError.
    def handleError(self, _: logging.LogRecord):
        pass
