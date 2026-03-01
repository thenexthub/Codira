#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#
#

import re
from vmb.lang import LangBase


# pylint: disable=duplicate-code
class Lang(LangBase):

    name = 'ArkTS'
    short_name = 'ets'
    __re_state = re.compile(
        r'^\s*(export)?\s*'
        r'class\s+(?P<class>\w+)\s*'
        r'({)?\s*$')
    __re_func = re.compile(
        r'^\s*(public\s+)?(override\s+)?'
        r'(?P<func>\w+)\s*'
        r'\(\s*\)\s*:?\s*(?P<type>[<>\w]+)?\s*(throws)?\s*({)?\s*$')
    __re_param = re.compile(
        r'^\s*(public\s+)?(static\s+)?'
        r'(?P<param>\w+)\s*'
        r':\s*(?P<type>\w+)\s*'
        r'(\s*=\s*.+)?(;)?\s*$')  # possible initialization

    def __init__(self) -> None:
        super().__init__()
        self.src = {'.ets'}
        self.ext = '.ets'

    @property
    def re_state(self) -> re.Pattern:
        return self.__re_state

    @property
    def re_param(self) -> re.Pattern:
        return self.__re_param

    @property
    def re_func(self) -> re.Pattern:
        return self.__re_func
