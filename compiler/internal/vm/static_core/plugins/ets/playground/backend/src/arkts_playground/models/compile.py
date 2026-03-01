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

from typing import Any

from pydantic import BaseModel, Field

from .common import DisasmResponse, IrDumpOptions, IrDumpResponse, ResponseLog, VerificationMode


class CompileRequestModel(BaseModel):
    code: str
    options: dict[str, Any] = Field(default_factory=dict)
    disassemble: bool = False
    verifier: bool = False
    verification_mode: VerificationMode = VerificationMode.AHEAD_OF_TIME
    aot_mode: bool = False
    ir_dump: IrDumpOptions | None = None


class RunResponse(BaseModel):
    run: ResponseLog | None = None
    run_aot: ResponseLog | None = None
    compile: ResponseLog
    disassembly: DisasmResponse | None = None
    verifier: ResponseLog | None = None
    ir_dump: IrDumpResponse | None = None


class CompileResponse(BaseModel):
    compile: ResponseLog
    disassembly: DisasmResponse | None = None
    verifier: ResponseLog | None = None
    ir_dump: IrDumpResponse | None = None
