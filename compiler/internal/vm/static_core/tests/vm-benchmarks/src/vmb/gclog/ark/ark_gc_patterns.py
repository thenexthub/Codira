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


class ArkGcPatterns:
    THREAD_ID = r'\[TID (?P<thread_id>[\da-fA-F]+)\]'
    TIMESTAMP = r'(?P<timestamp>\w{3} \d{1,2} \d{1,2}:\d{1,2}:\d{1,2}.\d{3})'
    GC_NAME = r'(?P<gc_name>[\w\- ]+) GC'
    GC_COUNTER = r'\[(?P<gc_counter>\w+)\]'
    GC_CAUSE = r'\[(?P<gc_collection_type>\w+) ' \
               r'\((?P<gc_cause>\w+(?:\s+\w+)?)\)\]'
    OBJS_FREED = r'(?P<obj_mem_freed>\d+)' \
                 r'(?P<obj_mem_units>(?:B|KB|MB)),'
    LOBJS_FREED = r'(?P<large_obj_mem_freed>\d+)' \
                  r'(?P<large_obj_mem_units>(?:B|KB|MB)) LOS objects,'
    HEAP_FREE = r'(?P<heap_free_mem>\d+)% free,'
    MEM_AFTER_TOTAL = r'(?P<mem_after>\d+)(?P<mem_after_units>(?:B|KB|MB))\/' \
                      r'(?P<mem_total>\d+)(?P<mem_total_units>(?:B|KB|MB)),'
    PAUSE_TIME = r'phase: \w+ paused: (?P<pause_time>\d+(?:.\d+)?)' \
                 r'(?P<pause_time_units>(?:s|ms|us))'
    PAUSES = r'(?P<pauses>(phase: \w+ paused: \d+(?:.\d+)?(?:s|ms|us) )+)'
    GC_TOTAL_TIME = r'total: (?P<gc_time>\d+(?:.\d+)?)' \
                    r'(?P<gc_time_units>(?:s|ms|us))'
    SYSLOG_GC_ENTRY = r'^[0-9\:\.\- ]+\s+[A-Z]\s+Ark gc\s+:\s+'
