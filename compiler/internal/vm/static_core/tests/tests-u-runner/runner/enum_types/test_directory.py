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

from typing import List, Union

from dataclasses import dataclass


@dataclass
class TestDirectory:
    directory: str
    extension: str
    flags: List[str]
    __current: int = -1

    def __iter__(self) -> 'TestDirectory':
        self.__current = -1
        return self

    def __next__(self) -> Union[str, List[str]]:
        self.__current += 1
        if self.__current == 0:
            return self.directory
        if self.__current == 1:
            return self.extension
        if self.__current == 2:
            return self.flags
        raise StopIteration
