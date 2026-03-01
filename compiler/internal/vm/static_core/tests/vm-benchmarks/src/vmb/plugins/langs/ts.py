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
#

import os
import re
from typing import Optional, Tuple

from vmb.lang import LangBase


# pylint: disable=duplicate-code
class Lang(LangBase):
    name = 'TypeScript'
    short_name = 'ts'
    __re_state = re.compile(
        r'^\s*(export)?\s*'
        r'class\s+(?P<class>\w+)\s*'
        r'({)?\s*$')
    __re_func = re.compile(
        r'^\s*(public)?\s+(?P<func>\w+)\s*'
        r'\(\s*\)\s*:\s*(?P<type>\w+)\s*(throws)?\s*({)?\s*$')
    __re_func_void = re.compile(
        r'^\s*(public)?\s+(?P<func>\w+)\s*'
        r'\(\s*\)\s*(throws)?\s*({)?\s*$')
    __re_param = re.compile(
        r'(?P<param>\w+)\s*'
        r':\s*(?P<type>\w+)\s*'
        r'(\s*=\s*.+)?(;)?\s*$')  # possible initialization

    def __init__(self):
        super().__init__()
        self.src = {'.ts'}
        self.ext = '.ts'

    @property
    def re_state(self) -> re.Pattern:
        return self.__re_state

    @property
    def re_param(self) -> re.Pattern:
        return self.__re_param

    @property
    def re_func(self) -> re.Pattern:
        return self.__re_func

    def parse_func(self, line: str) -> Optional[Tuple[str, str]]:
        """Override LangBase to cope with omitted void."""
        m = re.search(self.re_func, line)  # type: ignore
        if m:
            return m.group("func"), m.group("type")
        m = re.search(self.__re_func_void, line)  # type: ignore
        ret = None
        if m:
            return m.group("func"), "void"
        return ret

    def get_method_call(self, name: str, typ: str) -> str:
        if typ and typ != 'void':
            return f'Consumer.consume_{typ}(bench.{name}());'
        return f'bench.{name}();'

    def get_import_line(self, lib: str, what: str) -> str:
        libfile = os.path.split(lib)[1]
        libname = os.path.splitext(libfile)[0]
        return f'import {what} from "./{libname}";\n'  # Note "./"
