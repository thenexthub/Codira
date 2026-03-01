#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

import time
from collections.abc import Awaitable, Sequence
from contextlib import asynccontextmanager
from typing import Callable

import structlog
from asgi_correlation_id import CorrelationIdMiddleware
from fastapi import FastAPI, Request, Response
from fastapi.middleware.cors import CORSMiddleware
from prometheus_fastapi_instrumentator import Instrumentator

from arkts_playground.logs import configure_logging

from .config import get_settings
from .routers import compile_run, formatting, options
from .deps.runner import get_runner

access_logger = structlog.stdlib.get_logger("api.access")

EXCLUDED_PATHS = ["/metrics", "/syntax", "/versions", "/features", "/health", "/docs", "/openapi.json"]


def create_app(args: Sequence[str] | None = None) -> FastAPI:
    settings = get_settings(args)

    @asynccontextmanager
    async def lifespan(_app: FastAPI):
        configure_logging(settings.env)
        runner = get_runner()
        versions = await runner.get_versions()
        structlog.stdlib.get_logger(__file__).debug(
            "Playground runner started",
            arkts_version=versions[0],
            playground_version=versions[1],
            es2panda_version=versions[2],
        )
        yield

    app = FastAPI(lifespan=lifespan)

    instrumentator = Instrumentator(
        should_group_status_codes=True,
        should_ignore_untemplated=True,
        excluded_handlers=EXCLUDED_PATHS,
    )

    _ = instrumentator.instrument(app).expose(app, endpoint="/metrics")

    @app.middleware("http")
    async def logging_middleware(request: Request, call_next: Callable[[Request], Awaitable[Response]]) -> Response:
        structlog.contextvars.clear_contextvars()
        request_id = getattr(request.state, "correlation_id", None)
        _ = structlog.contextvars.bind_contextvars(request_id=request_id)

        start_time = time.perf_counter_ns()
        response = Response(status_code=500)
        try:
            response = await call_next(request)
        except Exception:
            structlog.stdlib.get_logger("api.error").exception("Uncaught exception")
            raise
        finally:
            duration = time.perf_counter_ns() - start_time
            status_code = response.status_code
            url = request.url.path
            client_host = request.client.host if request.client else "-"
            client_port = request.client.port if request.client else "-"
            access_logger.info(
                f'{client_host}:{client_port} - "{request.method} {url}" {status_code}',
                http={"url": str(request.url), "status_code": status_code, "method": request.method},
                duration=duration,
            )
            response.headers["X-Process-Time"] = str(duration / 1e9)
        return response

    app.add_middleware(CorrelationIdMiddleware)

    if settings.server.cors:
        app.add_middleware(
            CORSMiddleware,
            allow_origins=settings.server.cors_allow_origins,
            allow_credentials=settings.server.cors_allow_credentials,
            allow_methods=settings.server.cors_allow_methods,
            allow_headers=settings.server.cors_allow_headers,
        )

    app.include_router(formatting.router)
    app.include_router(compile_run.router)
    app.include_router(options.router)

    return app
