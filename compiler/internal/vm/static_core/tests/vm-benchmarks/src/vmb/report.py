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

# pylint: disable=too-many-locals
from __future__ import annotations
import sys
import json
import re
import logging
from datetime import datetime, timezone, timedelta
from typing import Union, Iterable, Optional, List, Set, Dict, Tuple, Any
from dataclasses import dataclass
from statistics import geometric_mean, mean
from pathlib import Path
from vmb.helpers import Jsonable, pad_left, read_list_file, create_file, Timer, check_file_exists
from vmb.unit import BenchUnit
from vmb.result import RunReport, TestResult, AotStatEntry, AotPasses, \
    MachineMeta, BUStatus
from vmb.result_tolerance import Tolerance, ToleranceSettings, ToleranceDefault
from vmb.cli import Args, Command


OptList = Optional[List[str]]
log = logging.getLogger('vmb')


@dataclass
class VMBDiff(Jsonable):
    time: str
    size: str
    rss: str
    status: str

    def __str__(self):
        return f"Time: {self.time}; Size: {self.size}; " \
               f"RSS: {self.rss}; {self.status}"


class VMBComparison(Jsonable):
    def __init__(self):
        self.title = ''
        self.summary = ''
        self.tests = {}
        self._regressions = 0
        self._perf_regressions = []
        self._name_len = 0

    def add(self, t, diff) -> None:
        self.tests[t] = diff

    def add_perf_regression(self, t: str, diff: str):
        self._perf_regressions.append((t, diff))

    def set_summary(self, t_diff: str, s_diff: str, r_diff: str,
                    new: int, fixed: int, new_fail: int, missed: int) -> None:
        self.summary = f'Time: {t_diff}; Size: {s_diff}; RSS: {r_diff}'
        # pylint: disable-next=protected-access
        if self._regressions > 0:
            self.summary += f' New Fails: {new_fail}; Missed: {missed}'
        if new + fixed > 0:
            self.summary += f' New: {new}; Fixed: {fixed}'

    def print(self, full: bool = False) -> None:
        caption(self.title)
        caption(self.summary)
        for name, diff in self.tests.items():
            if not full and diff.status not in ('new fail', 'missed'):
                continue
            print(f'{name:<{self._name_len}}; {diff}')

    def process_perf_regressions(self) -> int:
        if len(self._perf_regressions) == 0:
            log.passed('No performance degradation')
            return 0
        log.warning('Some tests have perfomance degradation')
        for r in self._perf_regressions:
            print(f'{r[0]:<{self._name_len}} {r[1]}', file=sys.stderr)
        return 1

    def compare_one_result(self, test: str, t1: TestResult, t2: TestResult,
                           status_diff: str,
                           tolerance: Optional[Tolerance] = None) -> None:
        time_t = tolerance.time if tolerance else ToleranceDefault.time
        size_t = tolerance.code_size if tolerance else ToleranceDefault.code_size
        rss_t = tolerance.rss if tolerance else ToleranceDefault.rss
        comp_t = tolerance.compile_time if tolerance else ToleranceDefault.compile_time
        time_diff, time_regr = cmp(t1.mean_time, t2.mean_time, tolerance=time_t)
        size_diff, size_regr = cmp(t1.code_size, t2.code_size, tolerance=size_t)
        rss_diff, rss_regr = cmp(t1.mem_bytes, t2.mem_bytes, tolerance=rss_t)
        comp_diff, comp_regr = cmp(t1.compile_time, t2.compile_time, tolerance=comp_t)
        self.add(test, VMBDiff(time_diff, size_diff, rss_diff, status_diff))
        for diff in (
                         (time_diff, time_regr, 'Time        ', time_t),
                         (size_diff, size_regr, 'Code Size   ', size_t),
                         (rss_diff, rss_regr, 'RSS         ', rss_t),
                         (comp_diff, comp_regr, 'Compile Time', comp_t),
        ):
            if diff[1]:
                self.add_perf_regression(test, f'{diff[2]} {diff[0]} (>{diff[3]:.1f}%)')


@dataclass
class VMBReport(Jsonable):
    report: RunReport
    title: str = ''
    summary: str = ''
    time_gm: float = 0.0
    size_gm: float = 0.0
    rss_gm: float = 0.0
    total_cnt: int = 0
    excluded_cnt: int = 0
    fail_cnt: int = 0
    _name_len: int = 0

    @staticmethod
    def status_diff(status1: bool, status2: bool, is_flaky: bool) -> str:
        flaky = '(flaky)' if is_flaky else ''
        status_diff = f'both pass{flaky}'
        if not status1 and not status2:
            status_diff = f'both fail{flaky}'
        if status1 != status2:
            if status2:
                status_diff = f'fixed{flaky}'
            else:
                status_diff = f'new fail{flaky}'
        return status_diff

    @staticmethod
    def compare_vmb(r1: VMBReport, r2: VMBReport,
                    flaky: OptList = None,
                    tolerances: Optional[ToleranceSettings] = None
                    ) -> VMBComparison:
        if not flaky:
            flaky = []
        # create tmp dicts of tests for simple search of test by name
        results1 = {t.name: t for t in r1.report.tests}
        results2 = {t.name: t for t in r2.report.tests}
        tests1, tests2 = set(results1.keys()), set(results2.keys())
        comparison = VMBComparison()
        # pylint: disable-next=protected-access
        comparison._name_len = max(r1._name_len, r2._name_len)
        times1, times2, sizes1, sizes2, rss1, rss2 = [], [], [], [], [], []
        missed, new, fixed, new_fail = 0, 0, 0, 0
        # compare tests present in both reports
        common_tests = tests1.intersection(tests2)
        log.info('Comparison: %d common tests selected', len(common_tests))
        for t in common_tests:
            t1, t2 = results1[t], results2[t]
            status1, status2 = test_passed(t1), test_passed(t2)
            is_flaky = t in flaky
            status_diff = VMBReport.status_diff(status1, status2, is_flaky)
            if not is_flaky:
                if status2 and not status1:
                    fixed += 1
                elif status1 and not status2:
                    comparison._regressions += 1
                    new_fail += 1
            comparison.compare_one_result(t, t1, t2, status_diff,
                                          tolerance=tolerances[t] if tolerances else None)
            if status1 and status2:
                times1.append(t1.mean_time)
                times2.append(t2.mean_time)
                sizes1.append(t1.code_size)
                sizes2.append(t2.code_size)
                rss1.append(t1.mem_bytes)
                rss2.append(t2.mem_bytes)
        for t in tests1.difference(tests2):
            comparison._regressions += 1
            missed += 1
            comparison.add(t, VMBDiff('n/a', 'n/a', 'n/a', 'missed'))
        for t in tests2.difference(tests1):
            comparison.add(t, VMBDiff('n/a', 'n/a', 'n/a', 'new'))
            new += 1
        comparison.title = f'Comparison: {short_title(r1.title, r2.title)}'
        # GM should be recalculated because
        # of possible difference in test set for report1 and report2
        t_diff = diff_str_gm(times1, times2, tolerance=ToleranceDefault.time)
        s_diff = diff_str_gm(sizes1, sizes2, tolerance=ToleranceDefault.code_size)
        r_diff = diff_str_gm(rss1, rss2, tolerance=ToleranceDefault.rss)
        comparison.set_summary(t_diff, s_diff, r_diff, new, fixed, new_fail, missed)
        return comparison

    @classmethod
    def parse(cls, text: str,
              exclude: OptList = None,
              tags: Optional[Set[str]] = None,
              skip_tags: Optional[Set[str]] = None) -> VMBReport:
        obj = json.loads(text)
        rr = RunReport.from_obj(**obj)
        vmb_rep = VMBReport(report=rr)
        vmb_rep.recalc(exclude=exclude, tags=tags, skip_tags=skip_tags)
        return vmb_rep

    def get_exit_code(self) -> int:
        passed_cnt = self.total_cnt - self.excluded_cnt - self.fail_cnt
        return 0 if passed_cnt > 0 and self.fail_cnt == 0 else 1

    def recalc(self, exclude: OptList = None, tags: Optional[Set[str]] = None,
               skip_tags: Optional[Set[str]] = None) -> None:
        times, sizes, rsss = [], [], []
        if not exclude:
            exclude = []
        self.total_cnt, self.excluded_cnt, self.fail_cnt = 0, 0, 0
        if tags:
            tags = set(t.lower() for t in tags)
            self.report.tests = list(filter(lambda t: tags.intersection(t.tags), self.report.tests))
        if skip_tags:
            skips = set(t.lower() for t in skip_tags)
            log.debug("Skips: %s", skips)
            self.report.tests = list(filter(lambda t: not skips.intersection(t.tags), self.report.tests))
        for t in self.report.tests:
            name = t.name
            self.total_cnt += 1
            if name in exclude:
                self.excluded_cnt += 1
                continue
            self._name_len = max(self._name_len, len(name))
            if not test_passed(t):
                self.fail_cnt += 1
                continue
            t._status = BUStatus.PASS  # pylint: disable=protected-access
            times.append(mean(e.avg_time for e in t.execution_forks
                              if e.avg_time is not None))
            sizes.append(t.code_size)
            rsss.append(t.mem_bytes)
        self.time_gm = safe_geomean(times)
        self.size_gm = safe_geomean(sizes)
        self.rss_gm = safe_geomean(rsss)
        self.title = self.report.machine.name
        self.summary = f'{self.total_cnt} tests; ' \
                       f'{self.fail_cnt} failed; ' \
                       f'{self.excluded_cnt} excluded; ' \
                       f'Time(GM): {self.time_gm:.1f} ' \
                       f'Size(GM): {self.size_gm:.1f} ' \
                       f'RSS(GM): {self.rss_gm:.1f}'

    def print_full_time(self) -> None:
        s = datetime.strptime(self.report.run.start_time, Timer.tm_format)
        e = datetime.strptime(self.report.run.end_time, Timer.tm_format)
        print(f'Run took {e - s}')

    def compile_time(self) -> None:
        caption(f'{self.title}')
        self.print_full_time()
        compile_time = [t.compile_time for t in self.report.tests if t.compile_time]
        total_ns = sum(compile_time)
        total = len(compile_time)
        min_ns, max_ns = min(compile_time), max(compile_time)
        mean_ns = mean(compile_time)
        total_time = str(timedelta(microseconds=total_ns / 1000)).split('.', maxsplit=1)
        print(f'Compile: {total_time[0]} for {total} tests')
        print(f'Min: {min_ns / 1e9:.1f} Max: {max_ns / 1e9:.1f} Mean: {mean_ns / 1e9:.1f} (seconds)')

    def text_report(self, full: bool = True, exclude: OptList = None,
                    fmt: str = 'auto') -> None:
        self.recalc(exclude=exclude)
        caption(f'{self.title}')
        self.print_full_time()
        caption(f'{self.summary}')
        print()
        if self.fail_cnt > 0 or full or exclude:
            time_head = '    Time    ' if 'nano' == fmt else '  Time  '
            caption(f"{'Test':<{self._name_len}} | {time_head} | "
                    f'CodeSize |   RSS    | Status  |')
        for t in sorted(self.report.tests, key=lambda x: x.name):
            if test_passed(t) and not full:
                continue
            print(f'{pad_left(t.name, self._name_len)} | {t.to_str(fmt)} |')
        print()

    def csv_report(self, csv: Union[str, Path],
                   exclude: OptList = None) -> None:
        """Export results in CSV format."""
        self.recalc(exclude=exclude)
        csv_path = Path(csv)
        csv_path.parent.mkdir(parents=True, exist_ok=True)
        with create_file(csv_path) as f:
            f.write('Benchmark name,Time ns/op,Ext Time sec,'
                    "RSS max KB,Compile sec,Size bytes\n")
            for t in sorted(self.report.tests, key=lambda x: x.name):
                if test_passed(t):
                    # pylint: disable-next=protected-access
                    f.write(f'{t.name},{t.get_avg_time()},{t._ext_time},'
                            f"{t.mem_bytes},{t.compile_time},{t.code_size}\n")

    def json_report(self, js: Union[str, Path],
                    pretty: bool = True) -> None:
        js_path = Path(js)
        js_path.parent.mkdir(parents=True, exist_ok=True)
        indent = 4 if pretty else None
        with create_file(js_path) as f:
            f.write(self.report.js(indent=indent))


@dataclass
class AOTStatsDiff(Jsonable):
    methods: str
    time: str
    size: str


@dataclass
class AOTPassesDiff(Jsonable):
    runs: str
    ir_mem: str
    local_mem: str
    time: str


@dataclass
class GCDiff(Jsonable):
    bench_time: str
    throughput: str
    avg_time: str
    max_time: str


class GCComparison(Jsonable):
    def __init__(self):
        self.title = ''
        self.summary = ''
        self.tests = {}

    def add(self, t, diff):
        self.tests[t] = diff


class AOTStatsComparison(Jsonable):
    def __init__(self):
        self.title = ''
        self.summary = ''
        self.libs = {}

    def add(self, t, diff):
        self.libs[t] = diff


class AOTPassesComparison(Jsonable):
    def __init__(self):
        self.title = ''
        self.optimizations = {}

    def add(self, opt, summary, diff):
        self.optimizations[opt] = {
            'summary': summary,
            'libs': diff
        }


def safe_geomean(numbers: Iterable[Optional[float]]) -> float:
    if not numbers:
        return 0.0
    res = [x for x in numbers if x and x > 0]
    if not res:
        return 0.0
    return geometric_mean(res)


def caption(msg):
    print()
    print(msg)
    print('=' * len(msg))


def split_name(name):
    name = name[6:] if name.startswith('Panda-') else name
    m = re.search(r'(ECMA|ETS).*(AOT|INT|JIT).*-(Master|MR)$', name)
    if m:
        return f'{m.group(1)} {m.group(2)}', f'{m.group(3)}'
    return name, ''


def short_title(t1, t2):
    n1, m1 = split_name(t1)
    n2, m2 = split_name(t2)
    if n1 == n2:
        return f'{n1} {m1} vs {m2}'
    x1 = f'{n1} {m1}'.rstrip()
    x2 = f'{n2} {m2}'.rstrip()
    return f'{x1} vs {x2}'


def cmp(x: Optional[float], y: Optional[float], less_is_better: bool = True,
        tolerance: float = ToleranceDefault.time) -> Tuple[str, bool]:
    """Return comparison str representation and regression status."""
    if not x or not y:
        # cannot compare if one of the results is missing
        return 'n/a', False
    if x < 0 or y < 0:
        return 'n/a', False
    try:  # just to make linter happy
        diff_percent = (y / x - 1.0) * 100
    except ZeroDivisionError:
        return 'n/a', False
    if abs(diff_percent) < tolerance:
        return f'{x:.2e}->{y:.2e}(same)', False
    better = (less_is_better and diff_percent < 0) \
        or (not less_is_better and diff_percent > 0)
    return f'{x:.2e}->{y:.2e}' \
           f'({"better" if better else "worse"} {diff_percent:+.1f}%)', not better


def diff_str(x: Optional[float], y: Optional[float], less_is_better: bool = True,
             tolerance: float = ToleranceDefault.time) -> str:
    """Proxy to 'cmp'."""
    diff, _ = cmp(x, y, less_is_better, tolerance)
    return diff


def diff_str_gm(xs: Iterable[Optional[float]], ys: Iterable[Optional[float]],
                less_is_better: bool = True, tolerance: float = ToleranceDefault.time) -> str:
    xs_gm = safe_geomean(xs)
    ys_gm = safe_geomean(ys)
    return diff_str(xs_gm, ys_gm, less_is_better=less_is_better, tolerance=tolerance)


def test_passed(t: TestResult) -> bool:
    return (
        t._status != BUStatus.ERROR and  # pylint: disable=protected-access
        t.compile_status == 0 and
        t.execution_status == 0 and
        len(t.execution_forks) > 0
    )


def print_flaky(flaky):
    if flaky:
        caption('Flaky tests:')
        for t in flaky:
            print(t)


def get_common_aotstats(r1: VMBReport, r2: VMBReport
                        ) -> Tuple[AotStatEntry, AotStatEntry]:
    for lib in ('etsstdlib', f'lib{"core"}'):
        if lib in r1.report.ext_info and lib in r2.report.ext_info:
            return r1.report.ext_info[lib], r2.report.ext_info[lib]
    raise ValueError('Bad ext_info in aot report!')


def get_aot_passes(aot_stats: AotStatEntry) -> Dict[str, AotPasses]:
    """Return { optimization: { lib: [runs, ir_mem, loc_mem, time]...}."""
    passes = {}
    libs = aot_stats.keys()
    for lb in libs:
        lib = aot_stats[lb]
        passes[lb] = lib.aot_stats.passes
    all_optimisations = list(
        set().union(*[set(x.keys()) for x in passes.values()]))
    all_optimisations.sort()
    # number of runs, ir mem, local mem, time
    passes_stats = {opt: {lb: passes[lb][opt]
                          for lb in libs if opt in passes[lb]}
                    for opt in all_optimisations}
    return passes_stats


def compare_aot_stats(r_1: VMBReport, r_2: VMBReport) -> AOTStatsComparison:
    comparison = AOTStatsComparison()
    comparison.title = \
        f'Comparison AOT stats: {short_title(r_1.title, r_2.title)}'
    times1, sizes1, times2, sizes2 = [], [], [], []
    r1, r2 = get_common_aotstats(r_1, r_2)
    # only for libs present in both reports
    for lib in set(r1.keys()).intersection(set(r2.keys())):
        l1 = r1[lib]
        l2 = r2[lib]
        times1.append(l1.time)
        times2.append(l2.time)
        sizes1.append(l1.size)
        sizes2.append(l2.size)
        meth_diff = diff_str(l1.aot_stats.number_of_methods, l2.aot_stats.number_of_methods,
                             less_is_better=False, tolerance=ToleranceDefault.count)
        time_diff = diff_str(l1.time, l2.time, tolerance=ToleranceDefault.time)
        size_diff = diff_str(l1.size, l2.size, tolerance=ToleranceDefault.code_size)
        comparison.add(lib, AOTStatsDiff(meth_diff, time_diff, size_diff))
    gm_times1 = safe_geomean(times1)
    gm_times2 = safe_geomean(times2)
    t_diff = diff_str(gm_times1, gm_times2, tolerance=ToleranceDefault.time)
    gm_sizes1 = safe_geomean(sizes1)
    gm_sizes2 = safe_geomean(sizes2)
    s_diff = diff_str(gm_sizes1, gm_sizes2, tolerance=ToleranceDefault.code_size)
    comparison.summary = f'Time: {t_diff}; Size: {s_diff}'
    return comparison


def compare_aot_passes(r_1: VMBReport, r_2: VMBReport) -> AOTPassesComparison:
    r1, r2 = get_common_aotstats(r_1, r_2)
    passes1 = get_aot_passes(r1)
    passes2 = get_aot_passes(r2)
    opts1 = set(passes1.keys())
    opts2 = set(passes2.keys())
    opts = opts1.intersection(opts2)
    comparison = AOTPassesComparison()
    comparison.title = \
        f'Comparison AOT passes: {short_title(r_1.title, r_2.title)}'
    for opt in opts:
        opt1 = passes1[opt]
        opt2 = passes2[opt]
        time1, ir1, loc1 = [], [], []
        time2, ir2, loc2 = [], [], []
        opt_diff = {}
        for lib in set(opt1.keys()).intersection(set(opt2.keys())):
            runs_diff = diff_str(opt1[lib].runs, opt2[lib].runs,
                                 less_is_better=False, tolerance=ToleranceDefault.count)
            ir_diff = diff_str(opt1[lib].ir_mem, opt2[lib].ir_mem, tolerance=ToleranceDefault.rss)
            local_diff = diff_str(opt1[lib].local_mem, opt2[lib].local_mem, tolerance=ToleranceDefault.rss)
            time_diff = diff_str(opt1[lib].time, opt2[lib].time, tolerance=ToleranceDefault.time)
            opt_diff[lib] = AOTPassesDiff(
                runs_diff, ir_diff, local_diff, time_diff)
            time1.append(opt1[lib].time)
            time2.append(opt2[lib].time)
            ir1.append(opt1[lib].ir_mem)
            ir2.append(opt2[lib].ir_mem)
            loc1.append(opt1[lib].local_mem)
            loc2.append(opt2[lib].local_mem)
        tm_gm = diff_str_gm(time1, time2, tolerance=ToleranceDefault.time)
        ir_gm = diff_str_gm(ir1, ir2, tolerance=ToleranceDefault.rss)
        loc_gm = diff_str_gm(loc1, loc2, tolerance=ToleranceDefault.rss)
        comparison.add(
            opt,
            f'Time: {tm_gm}; IR size: {ir_gm}; Local size: {loc_gm}',
            opt_diff)
    return comparison


def get_gc_stats(name: str, results: List[TestResult]) -> Dict[str, float]:
    stats = None
    score, tp, avg, mx = 0.0, 0.0, 0.0, 0.0
    for t in results:
        if t.name == name:
            score = float(mean(float(f.avg_time) for f in t.execution_forks
                          if f.avg_time is not None))
            stats = t.gc_stats
            break
    if stats:
        total = stats.gc_total_time
        total_sum = stats.gc_total_time_sum
        vm = stats.gc_vm_time
        avg = stats.gc_avg_time
        mx = stats.gc_max_time
        total = total if total > 0 else 0.0
        vm = vm if vm > total else 0.0
        avg = avg if avg > 0 else 0.0
        mx = mx if mx > 0 else 0.0
        total_sum = total_sum if total_sum > 0 else 0.0
        tp = (vm - total_sum) / vm if vm > 0 else 0.0
    return {
        'bench_time': score,
        'throughput': tp,
        'avg_time': avg,
        'max_time': mx
    }


def compare_gc_stats(r1: VMBReport, r2: VMBReport) -> GCComparison:
    comparison = GCComparison()
    comparison.title = \
        f'Comparison GC stats: {short_title(r1.title, r2.title)}'
    sc1, tp1, avg1, max1, sc2, tp2, avg2, max2 = [], [], [], [], [], [], [], []
    # only for tests present in both reports
    gc_tests_1 = [t.name for t in r1.report.tests
                  if t.gc_stats and t.execution_status == 0]
    gc_tests_2 = [t.name for t in r2.report.tests
                  if t.gc_stats and t.execution_status == 0]
    for t in set(gc_tests_1).intersection(set(gc_tests_2)):
        t1 = get_gc_stats(t, r1.report.tests)
        t2 = get_gc_stats(t, r2.report.tests)
        sc1.append(t1.get('bench_time'))
        sc2.append(t2.get('bench_time'))
        tp1.append(t1.get('throughput'))
        tp2.append(t2.get('throughput'))
        avg1.append(t1.get('avg_time'))
        avg2.append(t2.get('avg_time'))
        max1.append(t1.get('max_time'))
        max2.append(t2.get('max_time'))
        score_diff = diff_str(t1.get('bench_time'), t2.get('bench_time'), tolerance=ToleranceDefault.time)
        through_diff = diff_str(t1.get('throughput'), t2.get('throughput'),
                                less_is_better=False, tolerance=ToleranceDefault.time)
        avg_diff = diff_str(t1.get('avg_time'), t2.get('avg_time'), tolerance=ToleranceDefault.time)
        max_diff = diff_str(t1.get('max_time'), t2.get('max_time'), tolerance=ToleranceDefault.time)
        comparison.add(t, GCDiff(score_diff, through_diff, avg_diff, max_diff))
    gm_sc1 = safe_geomean(sc1)
    gm_sc2 = safe_geomean(sc2)
    sc_diff = diff_str(gm_sc1, gm_sc2, tolerance=ToleranceDefault.time)
    gm_tp1 = safe_geomean(tp1)
    gm_tp2 = safe_geomean(tp2)
    tp_diff = diff_str(gm_tp1, gm_tp2, less_is_better=False, tolerance=ToleranceDefault.time)
    gm_avg1 = safe_geomean(avg1)
    gm_avg2 = safe_geomean(avg2)
    avg_diff = diff_str(gm_avg1, gm_avg2, tolerance=ToleranceDefault.time)
    gm_max1 = safe_geomean(max1)
    gm_max2 = safe_geomean(max2)
    max_diff = diff_str(gm_max1, gm_max2, tolerance=ToleranceDefault.time)
    comparison.summary = \
        f'Time: {sc_diff}; Throughput: {tp_diff}; ' \
        f'AvgTime: {avg_diff}; MaxTime: {max_diff}'
    return comparison


def compare_meta(r1: VMBReport, r2: VMBReport) -> None:
    rep1 = {t.name: t.full_time for t in r1.report.tests}
    rep2 = {t.name: t.full_time for t in r2.report.tests}
    tests1, tests2 = set(rep1.keys()), set(rep2.keys())
    # pylint: disable-next=protected-access
    name_len = max(r1._name_len, r2._name_len, len('name'))
    caption('Full test time comparison (seconds)')
    print(f'{"name":<{name_len}}| r1 | r2 |')
    print('=' * (name_len + 11))
    for t in tests1.intersection(tests2):
        t1, t2 = rep1[t], rep2[t]
        print(f'{t:<{name_len}}|{t1:>4.0f}|{t2:>4.0f}|{cmp(t1, t2)}')


def set_tolerance_defaults(args) -> None:
    if args.tolerance > 0:  # overall option set
        ToleranceDefault.time = args.tolerance
        ToleranceDefault.code_size = args.tolerance
        ToleranceDefault.rss = args.tolerance
        ToleranceDefault.compile_time = args.tolerance
        ToleranceDefault.count = args.tolerance
        return
    # read user settings for default tolerances
    ToleranceDefault.time = abs(args.tolerance_time)
    ToleranceDefault.code_size = abs(args.tolerance_code_size)
    ToleranceDefault.rss = abs(args.tolerance_rss)
    ToleranceDefault.compile_time = abs(args.tolerance_compile_time)
    ToleranceDefault.count = abs(args.tolerance_count)


def compare_reports(args):
    if len(args.paths) != 2:
        print('Need 2 reports for comparison')
        sys.exit(1)
    check_file_exists(args.paths[0])
    check_file_exists(args.paths[1])
    with open(args.paths[0], 'r', encoding="utf-8") as f1:
        r1 = VMBReport.parse(f1.read(), tags=args.tags, skip_tags=args.skip_tags)
    with open(args.paths[1], 'r', encoding="utf-8") as f2:
        r2 = VMBReport.parse(f2.read(), tags=args.tags, skip_tags=args.skip_tags)
    if args.compare_meta:
        compare_meta(r1, r2)
        return
    flaky = read_list_file(args.flaky_list) if args.flaky_list else []
    set_tolerance_defaults(args)
    # read fine tuning if any
    tolerances = ToleranceSettings.from_csv(args.tolerance_list) \
        if args.tolerance_list else None
    cmp_vmb = VMBReport.compare_vmb(r1, r2, flaky=flaky, tolerances=tolerances)
    cmp_vmb.print(full=args.full)
    if args.json:
        cmp_vmb.save(args.json)
    if args.aot_stats_json:
        cmp_stats = compare_aot_stats(r1, r2)
        cmp_stats.save(args.aot_stats_json)
        caption(cmp_stats.title)
        print(cmp_stats.summary)
    if args.aot_passes_json:
        cmp_passes = compare_aot_passes(r1, r2)
        cmp_passes.save(args.aot_passes_json)
    if args.gc_stats_json:
        cmp_gc = compare_gc_stats(r1, r2)
        cmp_gc.save(args.gc_stats_json)
    if args.status_by_compare:
        sys.exit(cmp_vmb.process_perf_regressions())
    print_flaky(flaky)
    sys.exit(  # pylint: disable-next=protected-access
        0 if len(cmp_vmb.tests) > 0 and cmp_vmb._regressions == 0
        else 1)


def exit_with_dry_run_status(bus: List[BenchUnit]) -> None:
    if not bus:
        log.fatal('No tests run!')
        sys.exit(1)
    failed = [bu for bu in bus if bu.status == BUStatus.COMPILATION_FAILED]
    for bu in failed:
        log.error("%s : Compilation failed", bu.name)
    sys.exit(1 if failed else 0)


def report_main(args: Args,
                bus: Optional[List[BenchUnit]] = None,
                ext_info: Optional[Any] = None,
                timer: Optional[Timer] = None) -> None:
    # split into 2 func
    if ext_info is None:
        ext_info = {}
    # if called after run
    if bus:
        if args.dry_run:
            exit_with_dry_run_status(bus)
        name = args.get('name', None)
        if not name:
            name = '-'.join([args.platform, args.mode])
        info = MachineMeta(name=name)
        # skipping not run tests for the backward compatibility
        rep = RunReport(ext_info=ext_info, machine=info,
                        tests=[bu.result for bu in bus
                               if bu.status not in (BUStatus.NOT_RUN,
                                                    BUStatus.SKIPPED)])
        if timer:
            rep.run.start_time = Timer.format(timer.begin)
            rep.run.end_time = Timer.format(timer.end)
        else:
            rep.run.end_time = Timer.format(datetime.now(timezone.utc))
        vmb_rep = VMBReport(report=rep)
        if args.report_json:
            compact = args.get('report_json_compact', False)
            vmb_rep.json_report(args.report_json, pretty=not compact)
        if args.report_csv:
            vmb_rep.csv_report(args.report_csv)
        vmb_rep.text_report(full=True, exclude=args.exclude_list, fmt=args.number_format)
        sys.exit(vmb_rep.get_exit_code())
    # if called standalone
    if not args.paths:
        print('Report files (paths) missed')
        Args.print_help(Command.REPORT)
    if args.compare or args.compare_meta:
        compare_reports(args)
    else:
        flaky = read_list_file(args.flaky_list) if args.flaky_list else []
        check_file_exists(args.paths[0])
        with open(args.paths[0], 'r', encoding="utf-8") as f:
            r = VMBReport.parse(f.read(), exclude=flaky, tags=args.tags, skip_tags=args.skip_tags)
        if args.compile_time:
            r.compile_time()
            sys.exit(0)
        if not args.status_only:
            r.text_report(full=args.full, exclude=flaky, fmt=args.number_format)
            print_flaky(flaky)
        exit_code = r.get_exit_code()
        if exit_code == 0:
            log.passed('Run has no unexpected failures.')
        else:
            log.error('Run has %d unexpected failures.', r.fail_cnt)
        sys.exit(exit_code)


if __name__ == '__main__':
    report_main(Args())
