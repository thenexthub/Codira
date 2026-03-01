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

from enum import Enum

from pydantic import BaseModel, Field


class VerificationMode(str, Enum):
    DISABLED = "disabled"
    AHEAD_OF_TIME = "ahead-of-time"
    ON_THE_FLY = "on-the-fly"


class ResponseLog(BaseModel):
    output: str | None = None
    error: str | None = None
    exit_code: int | None = None


class DisasmResponse(BaseModel):
    output: str | None = None
    code: str | None = None
    error: str | None = None
    exit_code: int | None = None


class VersionsResponse(BaseModel):
    backend_version: str | None = Field(alias="backendVersion")
    arkts_version: str | None = Field(alias="arktsVersion")
    es2panda_version: str | None = Field(alias="es2pandaVersion")


class IrDumpOptions(BaseModel):
    """Options for IR dump generation"""
    compiler_dump: bool = False
    disasm_dump: bool = False

    def is_enabled(self) -> bool:
        """IR dump is enabled if at least one dump type is selected"""
        return self.compiler_dump or self.disasm_dump


class IrDumpResponse(BaseModel):
    """Response containing IR dump data"""
    output: str | None = None
    error: str | None = None
    compiler_dump: str | None = None
    disasm_dump: str | None = None
    exit_code: int | None = None
