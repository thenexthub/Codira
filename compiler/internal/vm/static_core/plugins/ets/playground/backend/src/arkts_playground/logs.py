#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
import sys
import typing as t
from types import TracebackType

import structlog
from structlog.types import EventDict, Processor, WrappedLogger


def drop_color_message_key(_lg: WrappedLogger, _s: str, event_dict: EventDict) -> EventDict:
    # Uvicorn logs the message a second time in the extra `color_message`, but we don't
    # need it. This processor drops the key from the event dict if it exists.
    event_dict.pop("color_message", None)
    return event_dict


def configure_logging(env: t.Literal["production", "dev"], log_level: str = "DEBUG"):
    timestamper = structlog.processors.TimeStamper(
        fmt="%Y-%m-%dT%H:%M:%SZ"
    )

    shared_processors: list[Processor] = [
        structlog.contextvars.merge_contextvars,
        structlog.stdlib.add_log_level,
        structlog.stdlib.PositionalArgumentsFormatter(),
        structlog.stdlib.ExtraAdder(),
        drop_color_message_key,
        timestamper,
        structlog.processors.StackInfoRenderer(),
    ]
    processors: list[Processor] = []
    log_renderer: structlog.types.Processor

    if env == "dev":
        # Pretty printing when we run in dev environment
        processors = shared_processors + [
            structlog.dev.ConsoleRenderer(),
        ]
        log_renderer = structlog.dev.ConsoleRenderer()
    else:
        processors = shared_processors + [
            structlog.processors.dict_tracebacks,
            structlog.processors.JSONRenderer(),
        ]
        log_renderer = structlog.processors.JSONRenderer()

    structlog.configure(processors)

    formatter = structlog.stdlib.ProcessorFormatter(
        # These run ONLY on `logging` entries that do NOT originate within
        # structlog.
        foreign_pre_chain=shared_processors,
        # These run on ALL entries after the pre_chain is done.
        processors=[
            # Remove _record & _from_structlog.
            structlog.stdlib.ProcessorFormatter.remove_processors_meta,
            log_renderer,
        ],
    )

    # Reconfigure the root logger to use our structlog formatter, effectively emitting the logs via structlog
    handler = logging.StreamHandler()
    handler.setFormatter(formatter)
    root_logger = logging.getLogger()
    root_logger.handlers.clear()
    root_logger.addHandler(handler)
    root_logger.setLevel(log_level.upper())

    for _log in ["uvicorn", "uvicorn.error"]:
        # Make sure the logs are handled by the root logger
        logging.getLogger(_log).handlers.clear()
        logging.getLogger(_log).propagate = True

    # Uvicorn logs are re-emitted with more context. We effectively silence them here
    logging.getLogger("uvicorn.access").handlers.clear()
    logging.getLogger("uvicorn.access").propagate = False

    def handle_exception(exc_type: type[BaseException], exc_value: BaseException, exc_traceback: TracebackType | None):
        """
        Log any uncaught exception instead of letting it be printed by Python
        (but leave KeyboardInterrupt untouched to allow users to Ctrl+C to stop)
        See https://stackoverflow.com/a/16993115/3641865
        """
        if issubclass(exc_type, KeyboardInterrupt):
            sys.__excepthook__(exc_type, exc_value, exc_traceback)
            return

        root_logger.error(
            "Uncaught exception", exc_info=(exc_type, exc_value, exc_traceback)
        )

    sys.excepthook = handle_exception
