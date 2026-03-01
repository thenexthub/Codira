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

class FakeRunner:
    @staticmethod
    def parse_compile_options(options: dict) -> list:
        if not options:
            return []
        return list(options.keys())

    @staticmethod
    async def compile_run_arkts(code: str, options: list, disasm: bool = False, verifier: bool = False, **kwargs):
        _ = kwargs
        res = {
            "compile": {
                "output": f"testing output: {code}, {options}",
                "error": f"testing error: {code}",
                "exit_code": 0
            },
            "run": {
                "output": f"testing output: {code}",
                "error": f"testing error: {code}",
                "exit_code": 0
            },
            "disassembly": None,
            "verifier": None,
            "ir_dump": None,
            "run_aot": None
        }
        if disasm:
            res["disassembly"] = {
                "output": f"testing output: {code}",
                "error": f"testing error: {code}",
                "code": "disasm code",
                "exit_code": 0
            }
        if verifier:
            res["verifier"] = {
                "output": f"Verifier testing output: {code}",
                "error": f"Verifier testing error: {code}",
                "exit_code": 0
            }
        return res

    @staticmethod
    async def compile_arkts(code: str, options: list, **kwargs):
        disasm = kwargs.get("disasm", False)
        verifier = kwargs.get("verifier", False)
        res = {
            "compile": {
                "output": f"testing output: {code}, {options}",
                "error": f"testing error: {code}",
                "exit_code": 0
            },
            "disassembly": None,
            "verifier": None,
            "ir_dump": None
        }
        if disasm:
            res["disassembly"] = {
                "output": f"testing output: {code}",
                "error": f"testing error: {code}",
                "code": "disasm code",
                "exit_code": 0
            }
        if verifier:
            res["verifier"] = {
                "output": f"Verifier testing output: {code}",
                "error": f"Verifier testing error: {code}",
                "exit_code": 0
            }
        return res

    async def get_versions(self):
        return "playground_ver", "arkts_ver", "es2panda_ver"


def override_get_runner():
    return FakeRunner()


def override_get_options():
    return [
        {
            "flag": "--opt-level",
            "values": [0, 1, 2],
            "default": 2
        }
    ]
