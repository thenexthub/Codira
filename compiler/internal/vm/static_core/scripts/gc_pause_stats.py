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
import statistics
from typing import Tuple, List


def min_handle(times: list) -> float:
    return min(times, default=0.0)


def max_handle(times: list) -> float:
    return max(times, default=0.0)


def avg_handle(times: list) -> float:
    if len(times) == 0:
        return 0.0
    return statistics.mean(times)


def median_handle(times: list) -> float:
    if len(times) == 0:
        return 0.0
    return statistics.median(times)


def stdev_handle(times: list) -> float:
    if len(times) < 2:
        return 0.0
    return statistics.stdev(times)


def stdev_mean_percent(times: list) -> float:
    if len(times) < 2:
        return 0.0
    m = statistics.mean(times)
    if m <= 0.0:
        return 0.0
    return statistics.stdev(times) / m * 100.0


# TODO(ipetrov): Process remark pause
class StatsInfo:
    """Saved sequence of time"""
    STATS_MAP = {
        "count": len,
        "avg": avg_handle,
        "median": median_handle,
        "max": max_handle,
        "min": min_handle,
        "stdev": stdev_handle,
        "stdev / avg * 100%": stdev_mean_percent,
        "sum": sum
    }
    LIST_OF_STATS = list(STATS_MAP.keys())

    def __init__(self):
        self.times = list()

    def __len__(self) -> int:
        """Returns: count of times"""
        return StatsInfo.STATS_MAP.get("count")(self.times)

    def add(self, gc_time: float) -> None:
        """Add new time value to sequence"""
        self.times.append(gc_time)

    def get_stats(self) -> dict:
        """Returns: dictionary of all stats"""
        return {stat_name: stat_func(
            self.times) for stat_name, stat_func in StatsInfo.STATS_MAP.items()}


class GCTypeStats:
    """Class for one gc type with several gc causes"""

    def __init__(self, gc_type: str):
        self.times = {gc_type: StatsInfo()}
        self.gc_type = gc_type
    
    def __len__(self) -> int:
        return len(self.times.get(self.gc_type))

    def add(self, gc_type_cause: str, ms_time: float) -> None:
        self.times.get(self.gc_type).add(ms_time)
        if gc_type_cause != self.gc_type:
            self.times.setdefault(
                gc_type_cause, StatsInfo()).add(ms_time)

    def get_stats(self) -> list:
        sorted_info = sorted(list(self.times.items()),
                             key=lambda item: len(item[1]), reverse=True)
        if len(sorted_info) == 2:
            i = 0
            if sorted_info[0][0] == self.gc_type:
                i = 1
            return [(sorted_info[i][0], sorted_info[i][1].get_stats())]
        return list((st_info[0], st_info[1].get_stats()) for st_info in sorted_info)


class GCStatsTable:

    TOTAL = "Total"
    WO_FULL = "Total without FULL"

    def __init__(self, name: str) -> None:
        self.name = name
        self.stats = {
            GCStatsTable.TOTAL: GCTypeStats(GCStatsTable.TOTAL),
            GCStatsTable.WO_FULL: GCTypeStats(GCStatsTable.WO_FULL)
        }

    def add(self, gc_type: str, gc_type_cause: str, gc_time: float) -> None:
        self.stats.get(GCStatsTable.TOTAL).add(
            GCStatsTable.TOTAL, gc_time)
        if gc_type != "FULL":
            self.stats.get(GCStatsTable.WO_FULL).add(
                GCStatsTable.WO_FULL, gc_time)
        self.stats.setdefault(gc_type, GCTypeStats(
            gc_type)).add(gc_type_cause, gc_time)

    def save_table(self, saving_file: str) -> int:
        if len(self.stats.get("Total")) == 0:
            print(f"{self.name}: No gc logs", file=sys.stderr)
            return 0
        with open(saving_file, 'a') as file:
            file.write(f"<details><summary>{self.name}</summary>\n\n")
            file.write("| Parameter |")
            gc_stats_list = self.__sorted_gc_stats()
            for gc_type in gc_stats_list:
                file.write(f" {gc_type[0]} |")
            file.write("\n|:----|")
            for _ in range(len(gc_stats_list)):
                file.write(":---:|")
            for stat_type in StatsInfo.LIST_OF_STATS:
                file.write(f"\n| {stat_type} |")
                for trigger_stat in gc_stats_list:
                    file.write(f" {trigger_stat[1].get(stat_type)} |")
            file.write("\n\n</details>\n\n")
        return 1
    
    def __sorted_gc_stats(self) -> list:
        stats_list = list(x[1] for x in self.stats.items() if (
            x[0] != GCStatsTable.TOTAL and x[0] != GCStatsTable.WO_FULL))
        stats_list = sorted(stats_list,
                            key=lambda x: len(x), reverse=True)
        result = [self.stats.get(GCStatsTable.TOTAL).get_stats(
        )[0], self.stats.get(GCStatsTable.WO_FULL).get_stats()[0]]
        for gc_type_stat in stats_list:
            result += gc_type_stat.get_stats()
        return result


class GCStatsCollector:
    GC_TYPES = ["YOUNG", "MIXED", "TENURED", "FULL"]
    PAUSE_DETECT_STR: str = "COMMON_PAUSE paused: "
    REMARK_DETECT_STR: str = "REMARK_PAUSE paused: "
    TIMES: List[Tuple[str, float]] = [("ms", 1.0), ("us", 0.001), ("ns", 0.000001), ("s", 1000.0)]

    all_table = GCStatsTable("All logs")

    def __init__(self, result_file_name: str) -> None:
        self.result_file_path = os.path.abspath(result_file_name)
        with open(self.result_file_path, 'w') as result_file:
            result_file.write("_Generated by gc pause stats script_\n\n")
            result_file.write("All times in ms\n\n")

    @staticmethod
    def detect_str(line: str):
        """Detect gc info string from log lines"""
        # Find for mobile and host logs
        for detect_string in [" I Ark gc  : ", " I/gc: "]:
            i = line.find(detect_string)
            if i != -1:
                return (i, len(detect_string))
        return (-1, 0)

    @staticmethod
    def get_full_type(line: str, cause_start: int, cause_len: int) -> str:
        """Get gc type with cause"""
        cause_end = cause_start + cause_len
        while line[cause_start] != '[':
            cause_start -= 1
        while line[cause_end] != ']':
            cause_end += 1
        return line[cause_start + 1: cause_end]

    @staticmethod
    def get_gc_type(line: str):
        """Get gc type type and gc type with cause"""
        for cause in GCStatsCollector.GC_TYPES:
            i = line.find(cause)
            if i != -1:
                return cause, GCStatsCollector.get_full_type(line, i, len(cause))
        raise ValueError("Unsupported gc cause")

    @staticmethod
    def get_ms_time(line: str) -> float:
        """Return time in ms"""
        i = line.find(GCStatsCollector.PAUSE_DETECT_STR)
        j = line.find(" ", i + len(GCStatsCollector.PAUSE_DETECT_STR))
        time_str = line[i + len(GCStatsCollector.PAUSE_DETECT_STR):j]
        for time_end in GCStatsCollector.TIMES:
            if time_str.endswith(time_end[0]):
                return float(time_str[:-len(time_end[0])]) * time_end[1]
        raise ValueError("Could not detect time format")

    def save_table(self, table: GCStatsTable) -> int:
        return table.save_table(self.result_file_path)

    def process_one_log(self, gc_log_path: str) -> None:
        """Process one log file"""
        table = GCStatsTable(gc_log_path)

        with open(gc_log_path, 'r') as log_file:
            for f_line in log_file.readlines():
                ii = GCStatsCollector.detect_str(f_line)
                if ii[0] != -1 and f_line.find(GCStatsCollector.PAUSE_DETECT_STR) != -1:
                    gc_info_str = f_line[ii[0] + ii[1]:]
                    time_v = GCStatsCollector.get_ms_time(gc_info_str)
                    gc_type, gc_type_cuase = GCStatsCollector.get_gc_type(
                        gc_info_str)
                    table.add(gc_type, gc_type_cuase, time_v)
                    self.all_table.add(gc_type, gc_type_cuase, time_v)
        return self.save_table(table)


def get_logs_from_dir(dir_name: str) -> list:
    gc_log_paths = list()
    for file_or_dir in os.listdir(dir_name):
        name = os.path.join(dir_name, file_or_dir)
        if os.path.isfile(name):
            gc_log_paths.append(name)
    return gc_log_paths


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

    stats_collector = GCStatsCollector(sys.argv[-1])

    for log_path in list(map(os.path.abspath, sys.argv[1:-1])):
        if os.path.isfile(log_path):
            gc_log_paths.append(log_path)
        elif os.path.isdir(log_path):
            gc_log_paths += get_logs_from_dir(log_path)
        else:
            print(f"{log_path}: No such log file or directory", file=sys.stderr)

    count_nonempty_logs = 0
    for log_path in gc_log_paths:
        count_nonempty_logs += stats_collector.process_one_log(log_path)
    if count_nonempty_logs > 1:
        stats_collector.all_table.name = f"All {count_nonempty_logs} logs"
        stats_collector.save_table(stats_collector.all_table)


if __name__ == "__main__":
    main()
