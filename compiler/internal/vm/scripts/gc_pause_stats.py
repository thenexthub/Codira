#!/usr/bin/env python3
# -- coding: utf-8 --
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

import sys
import os
import itertools
from typing import NamedTuple


class GCPauseStats(NamedTuple):
    """Data class with constants for gc stats"""
    GC_TYPES = ["YOUNG", "MIXED", "TENURED", "FULL"]
    PAUSE_DETECT_STR = ", paused "
    TOTAL_DETECT_STR = " total "
    LIST_OF_STATS = ["count", "min", "max", "avg", "sum"]


def sort_one_gc_stat(stats: dict, gc_type: str) -> list:
    """Sort one type of gc stats for pretty table"""
    stats_list = list()
    for trig_type in stats:
        if trig_type.find(gc_type) != -1:
            stats_list.append(trig_type)
    return sorted(stats_list, key=lambda x: stats.get(x)["count"], reverse=True)


def sort_gc_stats(stats: dict) -> list:
    """Sort gc stats for pretty table"""
    stats_info = list()
    for gc_type in GCPauseStats.GC_TYPES:
        if gc_type in stats:
            stats_info.append(sort_one_gc_stat(stats, gc_type))
    stats_info.sort(key=lambda x: stats.get(x[0])["count"], reverse=True)
    return list(itertools.chain(*stats_info))


def save_pause_stats(gc_log_path: str, file_name: str, stats: dict) -> None:
    """Save md table in the file"""
    with open(file_name, 'a') as file:
        file.write(f"GC logs: {gc_log_path}\n\n")
        file.write("| Parameter |")
        gc_stats_list = ["Total"] + sort_gc_stats(stats)
        for gc_type in gc_stats_list:
            file.write(f" {gc_type} |")
        file.write("\n|:----|")
        for _ in range(len(stats)):
            file.write(":---:|")
        for stat_type in GCPauseStats.LIST_OF_STATS:
            file.write(f"\n| {stat_type} |")
            for trigger_stat in gc_stats_list:
                file.write(f" {stats.get(trigger_stat).get(stat_type)} |")
        file.write("\n\n")


def get_ms_time(line: str) -> float:
    """Return time in ms"""
    times = [("ms", 1.0), ("us", 0.001), ("s", 1000.0)]
    i = line.find(GCPauseStats.PAUSE_DETECT_STR)
    j = line.find(GCPauseStats.TOTAL_DETECT_STR, i)
    time_str = line[i + len(GCPauseStats.PAUSE_DETECT_STR):j]
    for time_end in times:
        if time_str.endswith(time_end[0]):
            return float(time_str[:-len(time_end[0])]) * time_end[1]
    raise ValueError("Could not detect time format")


def get_full_type(line: str, cause_start: int, cause_len: int) -> str:
    """Get gc type with cause"""
    cause_end = cause_start + cause_len
    while line[cause_start] != '[':
        cause_start -= 1
    while line[cause_end] != ']':
        cause_end += 1
    return line[cause_start + 1: cause_end]


def get_gc_type(line: str) -> (str, str):
    """Get gc type type and gc type with cause"""
    for cause in GCPauseStats.GC_TYPES:
        i = line.find(cause)
        if i != -1:
            return cause, get_full_type(line, i, len(cause))
    raise ValueError("Unsupported gc cause")


def update_stats(stats: dict, gc_type: str, time_value: float):
    """Update info about the gc type"""
    trigger_info = stats.setdefault(gc_type, {
        "max": 0.0,
        "min": 0.0,
        "avg": 0.0,
        "sum": 0.0,
        "count": 0
    })
    count_v = trigger_info.get("count") + 1
    sum_v = trigger_info.get("sum") + time_value
    avg_v = sum_v / count_v
    if count_v == 1:
        min_v = time_value
    else:
        min_v = min(trigger_info.get("min"), time_value)
    max_v = max(trigger_info.get("max"), time_value)
    trigger_info.update({
        "max": max_v,
        "min": min_v,
        "avg": avg_v,
        "sum": sum_v,
        "count": count_v
    })
    stats.update({gc_type: trigger_info})


def detect_str(line: str) -> (int, int):
    """Detect gc info string from log lines"""
    # Find for mobile and host logs
    for detect_string in [" I Ark gc  : ", " I/gc: "]:
        i = line.find(detect_string)
        if i != -1:
            return (i, len(detect_string))
    return (-1, 0)


def update_group_stats(gc_pause_stats: dict, gc_type: str, full_gc_type: str, time_v: float):
    """Update group (Total, full on short gc type) of stats"""
    update_stats(gc_pause_stats, "Total", time_v)
    update_stats(gc_pause_stats, gc_type, time_v)
    update_stats(gc_pause_stats, full_gc_type, time_v)


def process_one_log(gc_log_path: str, result_file_path: str, all_stats: dict) -> None:
    """Process one log file"""
    gc_pause_stats = {"Total": {
        "max": 0.0,
        "min": 0.0,
        "avg": 0.0,
        "sum": 0.0,
        "count": 0
    }
    }
    with open(gc_log_path, 'r') as log_file:
        for f_line in log_file.readlines():
            ii = detect_str(f_line)
            if ii[0] != -1 and f_line.find(GCPauseStats.PAUSE_DETECT_STR) != -1:
                gc_info_str = f_line[ii[0] + ii[1]:]
                time_v = get_ms_time(gc_info_str)
                cause_s, full_cause_s = get_gc_type(gc_info_str)
                update_group_stats(gc_pause_stats, cause_s,
                                   full_cause_s, time_v)
                update_group_stats(all_stats, cause_s,
                                   full_cause_s, time_v)
    save_pause_stats(gc_log_path, result_file_path, gc_pause_stats)


def main() -> None:
    """Script's entrypoint"""
    if len(sys.argv) < 3:
        print("Incorrect parameters count", file=sys.stderr)
        print("Usage: ", file=sys.stderr)
        print(
            f"  python3 {sys.argv[0]} <gc_log_1...> <results_path>", file=sys.stderr)
        print(f"    gc_log_num   -- Path to gc logs or application logs with gc logs", file=sys.stderr)
        print(
            f"    results_path -- Path to result file with pause stats", file=sys.stderr)
        print(
            f"Example: python3 {sys.argv[0]} gc_log.txt result.md", file=sys.stderr)
        exit(2)
    gc_log_paths = list()
    all_gc_stats = {"Total": {
        "max": 0.0,
        "min": 0.0,
        "avg": 0.0,
        "sum": 0.0,
        "count": 0
    }
    }
    result_file_path = os.path.abspath(sys.argv[-1])

    with open(result_file_path, 'w') as result_file:
        result_file.write("_Generated by gc pause stats script_\n\n")
        result_file.write("All times in ms\n\n")

    for log_path in list(map(os.path.abspath, sys.argv[1:-1])):
        if os.path.isfile(log_path):
            gc_log_paths.append(log_path)
        else:
            print(f"{log_path}: No such log file", file=sys.stderr)

    for log_path in gc_log_paths:
        process_one_log(log_path, result_file_path, all_gc_stats)
    if len(gc_log_paths) > 1:
        save_pause_stats(
            f"All {len(gc_log_paths)} logs", result_file_path, all_gc_stats)


if __name__ == "__main__":
    main()
