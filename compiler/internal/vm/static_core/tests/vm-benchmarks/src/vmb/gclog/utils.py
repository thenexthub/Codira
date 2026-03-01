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

from datetime import datetime
from typing import List


def percentile(data_sorted_asc: List[float], p: int) -> float:
    n = int(round(p / 100 * len(data_sorted_asc) + 0.5))
    if n > 1:
        return data_sorted_asc[n - 2]
    return data_sorted_asc[0]


def mem_to_bytes(size: int, units: str) -> int:
    multipliers = {
        "B": 1,
        "KB": 1024,
        "MB": 1024 * 1024,
        "GB": 1024 * 1024 * 1024
    }

    return int(size * multipliers.get(units, -1))


def time_to_ns(t: float, units: str) -> float:
    multipliers = {
        "ns": 1,
        "us": 1000,
        "ms": 1000 * 1000,
        "s": 1000 * 1000 * 1000,
    }

    return int(t * multipliers.get(units, float("nan")))


def ark_datetime_to_nanos(ts: str) -> int:
    today = datetime.today()
    dt, ms = ts.split('.')
    x = datetime.strptime(str(today.year) + ' ' + dt, '%Y %b %d %H:%M:%S')
    return (int(x.timestamp() * 1000) + int(ms)) * 1000 * 1000
