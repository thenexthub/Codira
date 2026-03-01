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

from src.arkts_playground.models.options import OptionsResponseModel
from tests.fixtures.overrides import FakeRunner, override_get_options


def test_formatting_api(playground_client, monkeypatch):
    expected = {"keywords": [], "typeKeywords": [], "builtins": [], "tokenizer": {"root": []}}
    monkeypatch.setattr("src.arkts_playground.routers.formatting.get_syntax", lambda: expected)
    resp = playground_client.get("/syntax")
    data = resp.json()

    expected_top_keys = {"keywords", "typeKeywords", "builtins", "tokenizer"}
    assert expected_top_keys.issubset(data.keys())


def test_options_api(playground_client, conf_data, monkeypatch):
    monkeypatch.setattr("src.arkts_playground.routers.options.get_options", override_get_options)
    resp = playground_client.get("/options")
    assert resp.json() == OptionsResponseModel(compileOptions=conf_data["options"]).model_dump(by_alias=True)


@pytest.mark.parametrize("code, disasm, run_compile, options", [
    (
            "let x = 1",
            False,
            False,
            {
                "--custom-opt": 1
            }
    ),
    (
            "let x = 2",
            True,
            True,
            {
                "--custom-opt": 12
            }
    ),
    (
            "let x = 3",
            False,
            True,
            {
                "--custom-opt": 123
            }
    ),
    (
            "let x = 4",
            True,
            False,
            {
                "--custom-opt": 1234
            }
    ),
])
def test_compile_api(playground_client, code, disasm, run_compile, options):
    resp = playground_client.post("/run" if run_compile else "/compile", json={
        "code": code,
        "disassemble": disasm,
        "options": options,
        "verifier": False
    })
    output_msg = f"testing output: {code}"
    output_with_opt = f"testing output: {code}, {list(options.keys())}"
    error_msg = f"testing error: {code}"
    result = {
        "compile": {
            "output": output_with_opt,
            "error": error_msg,
            "exit_code": 0
        },
        "disassembly": None,
        "verifier": None,
        "ir_dump": None
    }
    if run_compile:
        result["run"] = {
            "output": output_msg,
            "error": error_msg,
            "exit_code": 0
        }
        result["run_aot"] = None
    if disasm:
        result["disassembly"] = {
            "output": output_msg,
            "error": error_msg,
            "code": "disasm code",
            "exit_code": 0
        }
    assert resp.json() == result


def test_get_version_api(playground_client):
    resp = playground_client.get("/versions")
    assert resp.json() == {
        "arktsVersion": "arkts_ver",
        "backendVersion": "playground_ver",
        "es2pandaVersion": "es2panda_ver"
    }


@pytest.mark.parametrize("ast_mode, manual_flag, expected_status", [
    ("auto",   False, 200),
    ("manual", False, 403),
    ("manual", True,  200),
])
def test_ast_api_respects_mode(playground_client, monkeypatch, ast_mode, manual_flag, expected_status):
    async def fake_dump_ast(_self, code, _options=None, **_):
        return {
            "ast": "ast code",
            "output": f"testing output: {code}",
            "error": f"testing error: {code}",
            "exit_code": 0,
        }

    monkeypatch.setattr(FakeRunner, "dump_ast", fake_dump_ast, raising=False)

    class _Features:
        def __init__(self, mode: str):
            self.ast_mode = mode

    class _Settings:
        def __init__(self, mode: str):
            self.features = _Features(mode)

    monkeypatch.setattr(
        "src.arkts_playground.routers.compile_run.get_settings",
        lambda: _Settings(ast_mode),
        raising=True,
    )

    payload = {"code": "let x = 1", "options": {"--custom-opt": 1}}
    params = {"manual": 1} if manual_flag else None

    resp = playground_client.post("/ast", json=payload, params=params)
    assert resp.status_code == expected_status

    if expected_status == 200:
        assert resp.json() == {
            "ast": "ast code",
            "output": "testing output: let x = 1",
            "error": "testing error: let x = 1",
            "exit_code": 0,
        }
    else:
        assert resp.json() == {
            "detail": {
                "code": "AST_AUTO_DISABLED",
                "message": "AST is manual-only; call /ast?manual=1",
            }
        }


def test_features_api(playground_client, monkeypatch):
    class _Features:
        def __init__(self, mode: str):
            self.ast_mode = mode

    class _Settings:
        def __init__(self, mode: str):
            self.features = _Features(mode)

    monkeypatch.setattr(
        "src.arkts_playground.routers.compile_run.get_settings",
        lambda: _Settings("manual"),
        raising=True,
    )

    resp = playground_client.get("/features")
    assert resp.status_code == 200
    assert resp.json() == {"ast_mode": "manual"}


@pytest.mark.parametrize("verification_mode", ["disabled", "ahead-of-time", "on-the-fly"])
def test_run_api_with_verification_modes(playground_client, verification_mode):
    resp = playground_client.post("/run", json={
        "code": "let x = 1",
        "disassemble": False,
        "options": {},
        "verifier": False,
        "verification_mode": verification_mode,
    })
    assert resp.status_code == 200
    result = resp.json()

    expected_top_level_keys = {"compile", "run", "run_aot", "disassembly", "verifier", "ir_dump"}
    expected_compile_keys = {"exit_code", "output", "error"}
    expected_run_keys = {"exit_code", "output", "error"}

    assert set(result.keys()) == expected_top_level_keys, \
        f"Top-level keys mismatch: got {set(result.keys())}, expected {expected_top_level_keys}"

    assert result["compile"] is not None
    assert set(result["compile"].keys()) == expected_compile_keys, \
        f"Compile keys mismatch: got {set(result['compile'].keys())}, expected {expected_compile_keys}"
    assert isinstance(result["compile"]["exit_code"], int)
    assert isinstance(result["compile"]["output"], str)
    assert isinstance(result["compile"]["error"], str)
    assert result["compile"]["exit_code"] == 0

    assert result["run"] is not None
    assert set(result["run"].keys()) == expected_run_keys, \
        f"Run keys mismatch: got {set(result['run'].keys())}, expected {expected_run_keys}"
    assert isinstance(result["run"]["exit_code"], int)
    assert isinstance(result["run"]["output"], str)
    assert isinstance(result["run"]["error"], str)
    assert result["run"]["exit_code"] == 0

    assert result["disassembly"] is None
    assert result["verifier"] is None
    assert result["run_aot"] is None
    assert result["ir_dump"] is None


def test_run_api_full_json_schema_validation(playground_client):
    """Test that validates complete JSON structure without using 'in' operator"""
    resp = playground_client.post("/run", json={
        "code": "let x = 1",
        "disassemble": False,
        "options": {},
        "verifier": False,
        "verification_mode": "disabled",
    })
    assert resp.status_code == 200
    result = resp.json()

    expected_structure = {
        "compile": {
            "exit_code": int,
            "output": str,
            "error": str,
        },
        "run": {
            "exit_code": int,
            "output": str,
            "error": str,
        },
        "run_aot": type(None),
        "disassembly": type(None),
        "verifier": type(None),
        "ir_dump": type(None),
    }

    assert set(result.keys()) == set(expected_structure.keys()), \
        f"Top-level keys mismatch: {set(result.keys())} != {set(expected_structure.keys())}"

    assert result["compile"] is not None
    assert set(result["compile"].keys()) == set(expected_structure["compile"].keys()), \
        f"Compile keys mismatch: {set(result['compile'].keys())} != {set(expected_structure['compile'].keys())}"
    assert isinstance(result["compile"]["exit_code"], expected_structure["compile"]["exit_code"])
    assert isinstance(result["compile"]["output"], expected_structure["compile"]["output"])
    assert isinstance(result["compile"]["error"], expected_structure["compile"]["error"])
    assert result["compile"]["exit_code"] == 0

    assert result["run"] is not None
    assert set(result["run"].keys()) == set(expected_structure["run"].keys()), \
        f"Run keys mismatch: {set(result['run'].keys())} != {set(expected_structure['run'].keys())}"
    assert isinstance(result["run"]["exit_code"], expected_structure["run"]["exit_code"])
    assert isinstance(result["run"]["output"], expected_structure["run"]["output"])
    assert isinstance(result["run"]["error"], expected_structure["run"]["error"])
    assert result["run"]["exit_code"] == 0

    assert result["disassembly"] is None
    assert result["verifier"] is None
    assert result["run_aot"] is None
    assert result["ir_dump"] is None


def test_run_api_invalid_verification_mode(playground_client):
    resp = playground_client.post("/run", json={
        "code": "let x = 1",
        "disassemble": False,
        "options": {},
        "verifier": False,
        "verification_mode": "invalid-mode",
    })
    assert resp.status_code == 422  # Validation error
