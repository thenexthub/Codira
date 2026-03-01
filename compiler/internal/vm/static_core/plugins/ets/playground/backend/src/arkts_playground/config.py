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

import typing as t
from argparse import ArgumentParser
from collections.abc import Sequence
from functools import lru_cache

import yaml
from pydantic import BaseModel, Field
from pydantic_settings import BaseSettings

from .models.options import OptionModel


class SyntaxModel(BaseModel):
    keywords: list[str]
    type_keywords: list[str] = Field(alias="typeKeywords")
    builtins: list[str]
    tokenizer: dict[str, list[tuple[str, str | dict[str, t.Any]]]]


class Server(BaseModel):
    uvicorn_config: dict[str, str | int]
    cors: bool = False
    cors_allow_origins: list[str] = []
    cors_allow_methods: list[str] = []
    cors_allow_headers: list[str] = []
    cors_allow_credentials: bool = False


class Binary(BaseModel):
    build: str
    timeout: int = 120
    # Path to icu dat file f.e 'icudt72l.dat'
    icu_data: str = ""


class Features(BaseModel):
    ast_mode: t.Literal["auto", "manual"] = "auto"


class PlaygroundSettings(BaseSettings):
    server: Server
    binary: Binary
    options: list[OptionModel]
    syntax: SyntaxModel
    features: t.Annotated[Features, Field(default_factory=Features)]
    env: t.Literal["production", "dev"] = "production"


@lru_cache
def get_settings(args: Sequence[str] | None = None) -> PlaygroundSettings:
    parser = ArgumentParser()
    _ = parser.add_argument("--config", "-c", type=str)
    parsed_args = parser.parse_args(args)
    with open(t.cast(str, parsed_args.config), "r", encoding="utf-8") as f:
        conf = yaml.safe_load(f)
    return PlaygroundSettings(**conf)


@lru_cache
def get_options(args: Sequence[str] | None = None) -> list[OptionModel]:
    return get_settings(args).options


@lru_cache
def get_syntax(args: Sequence[str] | None = None) -> SyntaxModel:
    return get_settings(args).syntax


@lru_cache
def get_features(args: Sequence[str] | None = None) -> Features:
    return get_settings(args).features
