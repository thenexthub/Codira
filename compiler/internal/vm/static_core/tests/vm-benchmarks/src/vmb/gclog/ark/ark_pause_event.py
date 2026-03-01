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

from dataclasses import dataclass
from vmb.gclog.event_parser import LogEntry


@dataclass
class ArkPauseEvent(LogEntry):
    gc_name: str
    timestamp: int
    freed_object_mem: int
    freed_large_object_mem: int
    mem_after: int
    mem_total: int
    pause_time: float
    pause_time_sum: float
    gc_time: float
    gc_cause: str
    gc_collection_type: str
    gc_counter: str
