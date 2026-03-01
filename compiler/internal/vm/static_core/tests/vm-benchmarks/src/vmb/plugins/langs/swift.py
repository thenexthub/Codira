#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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
from vmb.lang import LangBase


class Lang(LangBase):
    name = 'Swift'
    short_name = 'swift'
    __re_state = re.compile(
        # State class should not be generic
        r'^\s*(export)?\s*'
        r'class\s+(?P<class>\w+)\s*'
        r'({)?\s*$')
    __re_func = re.compile(
        r'^\s*func\s+(?P<func>[<>\w]+)'
        r'\(\s*\)'
        r'\s*(async)?\s*(throws)?\s*(?P<type>->\s*[<>\w]+)?\s*({)?\s*$')
    __re_param = re.compile(
        r'^\s*var\s*(?P<param>\w+)\s*'
        r'(:|\=)?\s*(?P<type>[<>\w]+)(\?)?\s*$')

    def __init__(self):
        super().__init__()
        self.src = {'.swift'}
        self.ext = '.swift'

    @property
    def re_state(self) -> re.Pattern:
        return self.__re_state

    @property
    def re_param(self) -> re.Pattern:
        return self.__re_param

    @property
    def re_func(self) -> re.Pattern:
        return self.__re_func

    def get_method_call(self, name: str, typ: str) -> str:
        if typ and typ != 'void':
            return f'try Consumer.consume(bench.{name}())'
        return f'bench.{name}();'
