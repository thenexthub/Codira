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

from __future__ import annotations
import csv
from dataclasses import dataclass
from typing import Union, Dict, Any, Optional
from pathlib import Path


class ToleranceDefault:
    """Global keeper for tolerance settings."""

    time: float = 0.5
    code_size: float = 0.5
    rss: float = 0.5
    compile_time: float = 0.5
    count: float = 0.5


@dataclass
class Tolerance:
    """Individual tolerance setting."""

    time: float = 0.5
    code_size: float = 0.5
    rss: float = 0.5
    compile_time: float = 0.5
    count: float = 0.5


class ToleranceSettings:
    """Emulates dictionary of tolerance settings for test."""

    def __init__(self) -> None:
        self._data: Dict[str, Tolerance] = {}

    def __str__(self):
        return str(self._data)

    def __getitem__(self, key: str) -> Optional[Tolerance]:
        return self._data.get(key)

    def __setitem__(self, key: str, value: Tolerance) -> None:
        self._data[key] = value

    @staticmethod
    def read_float(row: Dict[str, Any], field: str, default: float) -> float:
        try:
            ret = abs(float(row[field]))
        except Exception:  # pylint: disable=broad-exception-caught
            ret = abs(default)
        return ret

    @classmethod
    def from_csv(cls, csv_file: Union[str, Path]) -> ToleranceSettings:
        tols = ToleranceSettings()
        with open(csv_file, mode='r', encoding='utf-8', newline='\n') as f:
            reader = csv.DictReader(f, delimiter=',', skipinitialspace=True)
            for row in reader:
                name = row.get('name')
                if not name:
                    continue
                name = name.strip()
                time = ToleranceSettings.read_float(row, 'time', ToleranceDefault.time)
                size = ToleranceSettings.read_float(row, 'code_size', ToleranceDefault.code_size)
                rss = ToleranceSettings.read_float(row, 'rss', ToleranceDefault.rss)
                comp = ToleranceSettings.read_float(row, 'compile_time', ToleranceDefault.compile_time)
                tols._data[name] = Tolerance(time=time, code_size=size, rss=rss, compile_time=comp)
        return tols
