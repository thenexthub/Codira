#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Python CDP Heap Profiler.
"""

from dataclasses import dataclass


@dataclass
class TrackingHeapObjectsParams:
    track_allocations: bool


def enable():
    return {'method': 'HeapProfiler.enable'}


def disable():
    return {'method': 'HeapProfiler.disable'}


def start_tracking_heap_objects(params: TrackingHeapObjectsParams):
    return {'method': 'HeapProfiler.startTrackingHeapObjects',
            'params': {'trackAllocations': params.track_allocations}}


def stop_tracking_heap_objects():
    return {'method': 'HeapProfiler.stopTrackingHeapObjects'}


def start_sampling():
    return {'method': 'HeapProfiler.startSampling'}


def stop_sampling():
    return {'method': 'HeapProfiler.stopSampling'}


def take_heap_snapshot():
    return {'method': 'HeapProfiler.takeHeapSnapshot'}

def take_heap_snapshot_static():
    return {'method': 'DumpHeapSnapshot.start',
            'sessionId': ''}
