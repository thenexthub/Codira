#!/usr/bin/env python3
# -- coding: utf-8 --
# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

import argparse
import stat
import sys
import os
import statistics
from typing import Tuple, List, Dict, Optional


def fatal(msg: str) -> None:
    """Print message and exit from the script with code 1"""
    print(msg, file=sys.stderr)
    sys.exit(1)


def parse_arguments() -> argparse.Namespace:
    """Parse script arguments"""
    argparser = argparse.ArgumentParser(
        description="Collect gc phase puase statistics"
    )
    argparser.add_argument("gc_log_file", type=str,
                           help="File with detailed info gc logs")
    argparser.add_argument("result_file", type=str,
                           help="Result markdown file with statistics")
    argparser.add_argument("--phase", nargs='+', type=str, required=True,
                           dest="phase", help="GC phase in detailed info gc logs")
    return argparser.parse_args()


class PhaseStats:
    TIMES: List[Tuple[str, float]] = [
        ("ms", 1.0), ("us", 0.001), ("ns", 0.000001), ("s", 1000.0)]

    def __init__(self):
        self.phase_thread_times: List[float] = []
        self.phase_cpu_times: List[float] = []

    @staticmethod
    def str_to_time(time_str: str) -> float:
        for possible_time in PhaseStats.TIMES:
            if time_str.endswith(possible_time[0]):
                return float(time_str[:-len(possible_time[0])]) * possible_time[1]
        raise ValueError("Unsupported time format: " + time_str)

    @staticmethod
    def get_times(times_str: str) -> Tuple[float, float]:
        times = times_str.split("/")
        if len(times) != 2:
            raise ValueError("Incorrect time format in log: " + times_str)
        return PhaseStats.str_to_time(times[0]), PhaseStats.str_to_time(times[1])

    def add_times(self, thread_time: float, cpu_time: float) -> None:
        self.phase_thread_times.append(thread_time)
        self.phase_cpu_times.append(cpu_time)

    def get_count(self) -> int:
        return len(self.phase_thread_times)

    def get_thread_min(self) -> float:
        return min(self.phase_thread_times, default=0.0)

    def get_cpu_min(self) -> float:
        return min(self.phase_cpu_times, default=0.0)

    def get_thread_max(self) -> float:
        return max(self.phase_thread_times, default=0.0)

    def get_cpu_max(self) -> float:
        return max(self.phase_cpu_times, default=0.0)

    def get_thread_avg(self) -> float:
        if len(self.phase_thread_times) == 0:
            return 0.0
        return statistics.mean(self.phase_thread_times)

    def get_cpu_avg(self) -> float:
        if len(self.phase_cpu_times) == 0:
            return 0.0
        return statistics.mean(self.phase_cpu_times)

    def get_thread_median(self) -> float:
        if len(self.phase_thread_times) == 0:
            return 0.0
        return statistics.median(self.phase_thread_times)

    def get_cpu_median(self) -> float:
        if len(self.phase_cpu_times) == 0:
            return 0.0
        return statistics.median(self.phase_cpu_times)


def change_gc_mode(log_line: str) -> Optional[str]:
    collect_types = ["YOUNG", "MIXED", "TENURED", "FULL"]
    for collect_type in collect_types:
        if log_line.find(f" [{collect_type} ") != -1:
            return f"({collect_type})"
    return None


def process_log(phases_list: List[str], gc_log_file: str, result_md_file: str) -> None:
    phases: Dict[str, PhaseStats] = dict()
    mode: str = ""
    with open(gc_log_file, 'r') as log_file:
        for log_line in log_file.readlines():
            new_mode = change_gc_mode(log_line)
            if new_mode is not None:
                mode = new_mode
            for phase in phases_list:
                start_phase = log_line.find(" " + phase + " ")
                if start_phase == -1:
                    continue
                thread_time, cpu_time = PhaseStats.get_times(
                    log_line[start_phase + len(phase) + 1:].strip())
                phases.setdefault(phase, PhaseStats()).add_times(
                    thread_time, cpu_time)
                phases.setdefault(
                    phase + f" {mode}", PhaseStats()).add_times(thread_time, cpu_time)
                break

    with os.fdopen(os.open(result_md_file, os.O_WRONLY | os.O_CREAT, stat.S_IWUSR | stat.S_IRUSR), 'a') as result_file:
        title_str = "| stats |"
        delim_table_str = "|----|"
        count_str = "| count |"
        avg_str = "| avg |"
        median_str = "| median |"
        max_str = "| max |"
        min_str = "| min |"
        for phase in sorted(phases.keys()):
            title_str += f" {phase} |"
            delim_table_str = f"{delim_table_str}:----:|"
            count_str += f" {phases.get(phase).get_count()} / {phases.get(phase).get_count()} |"
            avg_str += f" {phases.get(phase).get_thread_avg()} / {phases.get(phase).get_cpu_avg()} |"
            median_str += f" {phases.get(phase).get_thread_median()} / {phases.get(phase).get_cpu_median()} |"
            max_str += f" {phases.get(phase).get_thread_max()} / {phases.get(phase).get_cpu_max()} |"
            min_str += f" {phases.get(phase).get_thread_min()} / {phases.get(phase).get_cpu_min()} |"
        result_file.write(f"<details><summary>{gc_log_file}</summary>\n\n")
        result_file.write(title_str + '\n')
        result_file.write(delim_table_str + '\n')
        result_file.write(count_str + '\n')
        result_file.write(avg_str + '\n')
        result_file.write(median_str + '\n')
        result_file.write(max_str + '\n')
        result_file.write(min_str + '\n')
        result_file.write("\n</details>\n")


def main() -> None:
    """Main function for gc phase time stats script"""
    args = parse_arguments()
    if not os.path.isfile(args.gc_log_file):
        fatal(args.gc_log_file + ": file with gc logs does not exist")
    with os.fdopen(os.open(args.result_file, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o664), 'w') as result_file:
        result_file.write("\n_Generated by phase pause stats script_\n\n")
    process_log(args.phase, args.gc_log_file, args.result_file)


if __name__ == "__main__":
    main()
