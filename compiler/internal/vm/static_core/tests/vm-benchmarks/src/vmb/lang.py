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
import logging
from typing import Optional, Tuple, Dict, Any
from abc import ABC, abstractmethod

log = logging.getLogger('vmb')


class LangBase(ABC):
    """Base class for lang plugns."""

    name = ''
    short_name = ''
    _re_import = re.compile(
        r'^\s*(import\s+)?(?P<what>[*\w{},\s]+)\s+from\s+'
        r'([\'"])?(?P<lib>[./\w]+)([\'"])?\s*(;\s*)?$')

    def __init__(self):
        self.src = set()
        self.ext = ''

    @property
    def re_import(self):
        """Regexp for import module."""
        return self._re_import

    @property
    @abstractmethod
    def re_state(self) -> re.Pattern:
        raise NotImplementedError

    @property
    @abstractmethod
    def re_param(self) -> re.Pattern:
        raise NotImplementedError

    @property
    @abstractmethod
    def re_func(self) -> re.Pattern:
        raise NotImplementedError

    def get_import_line(self, lib: str, what: str) -> str:
        libfile = os.path.split(lib)[1]
        libname = os.path.splitext(libfile)[0]
        return f'import {what} from "{libname}";\n'

    def get_method_call(self, name: str, typ: str) -> str:
        if typ and typ != 'void':
            return f'Consumer.consume(bench.{name}());'
        return f'bench.{name}();'

    def get_custom_fields(
            # pylint: disable-next=unused-argument
            self, values: Dict[str, Any], custom_values: Dict[str, Any]
    ) -> Dict[str, str]:
        """Help in adding custom fields in extra-plugins."""
        return {}

    def parse_state(self, line: str) -> str:
        m = re.search(self.re_state, line)
        if m:
            return m.group("class")
        return ''

    def parse_param(self, line: str) -> Optional[Tuple[str, str]]:
        m = re.search(self.re_param, line)
        ret = None
        if m:
            # actually type is not needed
            return m.group("param"), m.group("type")
        return ret

    def parse_func(self, line: str) -> Optional[Tuple[str, str]]:
        m = re.search(self.re_func, line)  # type: ignore
        ret = None
        if m:
            return m.group("func"), m.group("type")
        return ret

    def parse_import(self, line: str) -> Optional[Tuple[str, str]]:
        m = re.search(self.re_import, line)  # type: ignore
        ret = None
        if m:
            lib = m.group('lib')
            what = m.group('what')
            return lib, self.get_import_line(lib, what)
        return ret
