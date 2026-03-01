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

import re
import os
from vmb.lang import LangBase


class Lang(LangBase):
    name = 'ECMAScript'
    short_name = 'js'
    # matches `function funcName () {` or `class ClassName {`
    __re_state = re.compile(
        r'^\s*(function|class)\s+'
        r'(?P<class>\w+)\s*(\(\s*\))?\s*({)?\s*$')
    # matches `this.funcName = function() {` or `funcName() {`
    __re_func = re.compile(
        r'^\s*(this\.)?(?P<func>\w+)\s*(=\s*)?'
        r'(?P<type>)(function)?\s*\(\s*\)\s*({)?\s*([}\s;]*)?$')  # `type` is a placeholder
    __re_param = re.compile(
        r'^\s*(?P<type>this)\.'  # `type` is a placeholder
        r'(?P<param>\w+)\s*'
        r'(\s*=\s*.+)?(;)?\s*$')  # possible initialization

    def __init__(self):
        super().__init__()
        self.src = {'.mjs', '.js'}
        self.ext = '.mjs'

    @property
    def re_state(self) -> re.Pattern:
        return self.__re_state

    @property
    def re_param(self) -> re.Pattern:
        return self.__re_param

    @property
    def re_func(self) -> re.Pattern:
        return self.__re_func

    def get_import_line(self, lib: str, what: str) -> str:
        libfile = os.path.split(lib)[1]
        libname = os.path.splitext(libfile)[0]
        return f'import {what} from "./{libname}.mjs";\n'  # Note "./" and ".mjs"

    def get_method_call(self, name: str, typ: str) -> str:
        if typ and typ != 'void':
            return f'Consumer.consume{typ}(bench.{name}());'
        return f'bench.{name}();'
