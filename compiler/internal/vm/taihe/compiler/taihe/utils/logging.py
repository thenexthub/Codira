# coding=utf-8
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

import logging
import os
from sys import stderr
from typing import ClassVar, TextIO

from typing_extensions import override


class AnsiStyle:
    RED = "\033[31m"
    GREEN = "\033[32m"
    BLUE = "\033[34m"
    YELLOW = "\033[33m"
    MAGENTA = "\033[35m"
    CYAN = "\033[36m"
    GRAY = "\033[90m"
    WHITE = "\033[97m"

    # Background colors
    BG_RED = "\033[41m"
    BG_GREEN = "\033[42m"
    BG_BLUE = "\033[44m"
    BG_GRAY = "\033[100m"

    # Styles
    RESET = "\033[39m"
    BRIGHT = "\033[1m"
    REVERSE = "\033[7m"
    RESET_ALL = "\033[0m"


def should_use_color(f: TextIO) -> bool:
    """Determine if we should use colored output."""
    # Check for explicit color disable
    if os.getenv("TAIHE_NO_COLOR") or os.getenv("TERM") == "dumb":
        return False

    # Check for force color
    if os.getenv("TAIHE_FORCE_COLOR"):
        return True

    # Check if we're writing to a terminal
    return f.isatty()


def setup_logger(verbosity: int = 0):
    """Sets up a console-based log handler for the system logger.

    Verbosity levels:
    - 40: ERROR
    - 30: WARNING
    - 20: INFO
    - 10: DEBUG
    """
    handler = logging.StreamHandler()
    handler.setFormatter(ColoredFormatter("%(message)s"))

    logger = logging.getLogger()
    logger.addHandler(handler)

    logger.setLevel(verbosity)


class ColoredFormatter(logging.Formatter):
    """A pretty, colored, console-based log handler."""

    COLORS: ClassVar = {
        logging.ERROR: f"{AnsiStyle.REVERSE}{AnsiStyle.RED}",
        logging.WARNING: f"{AnsiStyle.REVERSE}{AnsiStyle.YELLOW}",
        logging.INFO: f"{AnsiStyle.REVERSE}{AnsiStyle.GREEN}",
        logging.DEBUG: f"{AnsiStyle.REVERSE}{AnsiStyle.BLUE}",
    }

    PREFIXES: ClassVar = {
        logging.ERROR: "[!]",
        logging.WARNING: "[*]",
        logging.INFO: "[-]",
        logging.DEBUG: "[.]",
    }

    def __init__(self, fmt: str | None = None, use_color: bool | None = None):
        super().__init__(fmt)
        if use_color is None:
            use_color = should_use_color(stderr)
        self.use_color = use_color

    @override
    def format(self, record: logging.LogRecord):
        prefix = self.PREFIXES.get(record.levelno, "")
        message = super().format(record)

        if not self.use_color:
            return f"{prefix} {message}"

        color = self.COLORS.get(record.levelno, "")
        return f"{color}{prefix}{AnsiStyle.RESET_ALL} {message}"