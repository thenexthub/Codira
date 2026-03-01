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
from typing import Optional, Dict


class FwTimeParser:
    PATTERN_TEXT = (
        r'(?P<fw_start>^\d+)(\s+\w+\s+-)? Startup execution started:\s+'
        r'(?P<vm_start>\d+).*'
        r'(?P<fw_end>^\d+)(\s+\w+\s+-)? Benchmark result:.*'
    )

    PATTERN = re.compile(PATTERN_TEXT, re.DOTALL | re.MULTILINE)

    def parse_text(self, text: str) -> Optional[Dict[str, float]]:
        m = self.PATTERN.search(text)
        ret = None
        if m:
            return {
                # to ns
                'fw_start_time': int(m.group('fw_start')) * 1000.0 * 1000.0,
                # to ns
                'fw_end_time': int(m.group('fw_end')) * 1000.0 * 1000.0,
                'vm_start_time': int(m.group('vm_start'))
            }
        return ret
