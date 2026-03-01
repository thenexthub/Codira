#!/usr/bin/env python
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

import pytest
import yaml
from fastapi.testclient import TestClient

from src.arkts_playground.app import create_app
from src.arkts_playground.deps.runner import get_runner
from tests.fixtures.overrides import override_get_runner


@pytest.fixture(scope="session", name="ark_build")
def ark_build_fixture(tmp_path_factory):
    build_path = tmp_path_factory.mktemp("build")
    bin_path = build_path / "bin"
    plugin_path = build_path / "plugins" / "ets"
    plugin_path.mkdir(parents=True)
    bin_path.mkdir()
    paths = [
        bin_path / "es2panda",
        bin_path / "ark_disasm",
        bin_path / "ark",
        plugin_path / "etsstdlib.abc",
        bin_path / "verifier",
        build_path / "icudt72l.dat",
        bin_path / "ark_aot"
    ]
    for p in paths:
        p.write_text("")
    return build_path, *paths


@pytest.fixture(scope="session", name="conf_data")
def conf_data_fixture():
    return {
        "server": {
            "uvicorn_config": {
                "host": "localhost",
                "port": 8000
            }
        },
        "options": [
            {
                "flag": "--opt-level",
                "values": [0, 1, 2],
                "default": 2
            }
        ],
        "binary": {
            "build": "str"
        },
        "syntax": {
            "keywords": [],
            "typeKeywords": [],
            "builtins": [],
            "tokenizer": {
                "root": []
            }
        }
    }


@pytest.fixture(scope="session", name="conf_path")
def config_path_fixture(tmp_path_factory, conf_data):
    p = tmp_path_factory.mktemp("data") / "conf.yaml"
    with open(str(p), mode="w", encoding="utf-8") as f:
        yaml.dump(stream=f, data=conf_data)
    return p


@pytest.fixture(scope="session", name="playground_client")
def client_fixture(conf_path):
    app = create_app(("-c", str(conf_path)))
    app.dependency_overrides[get_runner] = override_get_runner
    return TestClient(app)
