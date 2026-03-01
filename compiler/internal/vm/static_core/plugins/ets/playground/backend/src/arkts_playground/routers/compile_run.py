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

from typing import Annotated

import structlog
from fastapi import APIRouter, Depends, HTTPException, Query

from ..config import get_settings
from ..deps.runner import Runner, get_runner
from ..models.ast import AstRequestModel, AstResponse
from ..models.common import VersionsResponse
from ..models.compile import CompileRequestModel, CompileResponse, RunResponse
from ..models.features import FeaturesResponse

logger = structlog.stdlib.get_logger(__name__)

router = APIRouter()


@router.post("/compile",
             response_model=CompileResponse,
             tags=["run"],
             description="Compile provided code and returns the result")
async def compile_arkts_code(body: CompileRequestModel,
                             runner: Annotated[Runner, Depends(get_runner)]) -> CompileResponse:
    ir_dump_opts = None
    if body.ir_dump:
        ir_dump_opts = {
            "compiler_dump": body.ir_dump.compiler_dump,
            "disasm_dump": body.ir_dump.disasm_dump,
        }
    result = await runner.compile_arkts(
        body.code,
        options=runner.parse_compile_options(body.options),
        disasm=body.disassemble,
        verifier=body.verifier,
        ir_dump=ir_dump_opts
    )
    return CompileResponse(**result)


@router.post("/run",
             response_model=RunResponse,
             tags=["run"],
             description="Compile and run provided code")
async def compile_run_arkts_code(body: CompileRequestModel,
                                 runner: Annotated[Runner, Depends(get_runner)]) -> RunResponse:
    logger.info("Run endpoint called", ir_dump_request=body.ir_dump)
    ir_dump_opts = None
    if body.ir_dump:
        ir_dump_opts = {
            "compiler_dump": body.ir_dump.compiler_dump,
            "disasm_dump": body.ir_dump.disasm_dump,
        }
        logger.debug("IR dump options parsed", ir_dump_opts=ir_dump_opts)
    result = await runner.compile_run_arkts(
        body.code,
        options=runner.parse_compile_options(body.options),
        disasm=body.disassemble,
        verification_mode=body.verification_mode,
        aot_mode=body.aot_mode,
        ir_dump=ir_dump_opts
    )
    return RunResponse(**result)


@router.get("/versions",
            response_model=VersionsResponse,
            tags=["run"],
            description="Return playground version and Ark compiler version")
async def get_versions(runner: Annotated[Runner, Depends(get_runner)]) -> VersionsResponse:
    playground_ver, ark_ver, es2panda_ver = await runner.get_versions()
    return VersionsResponse(backendVersion=playground_ver, arktsVersion=ark_ver, es2pandaVersion=es2panda_ver)


@router.post("/ast",
             response_model=AstResponse,
             tags=["run"],
             description="Parse ETS code and return AST dump from es2panda")
async def dump_ast(body: AstRequestModel,
                   runner: Annotated[Runner, Depends(get_runner)],
                   manual: bool = Query(False)) -> AstResponse:
    sett = get_settings()
    if sett.features.ast_mode == "manual" and not manual:
        raise HTTPException(
            status_code=403,
            detail={"code": "AST_AUTO_DISABLED",
                    "message": "AST is manual-only; call /ast?manual=1"}
        )
    result = await runner.dump_ast(
        body.code,
        options=runner.parse_compile_options(body.options),
    )
    return AstResponse(**result)


@router.get("/features",
             response_model=FeaturesResponse,
             tags=["run"])
async def get_features() -> FeaturesResponse:
    sett = get_settings()
    return FeaturesResponse(ast_mode=sett.features.ast_mode)
