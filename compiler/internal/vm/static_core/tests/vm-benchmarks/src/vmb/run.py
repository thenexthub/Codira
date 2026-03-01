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

import sys
import logging
from pathlib import Path
from typing import List, Tuple
from subprocess import TimeoutExpired
from vmb.platform import PlatformBase
from vmb.unit import BenchUnit
from vmb.result import BUStatus
from vmb.tool import VmbToolExecError
from vmb.hook import HookRegistry
from vmb.cli import Args
from vmb.result import ExtInfo
from vmb.helpers import Timer, create_file
from vmb.report import test_passed

log = logging.getLogger('vmb')


class VmbRunner:
    def __init__(self, args: Args) -> None:
        self.platform = PlatformBase.create(args)
        log.info('Using platform: %s', self.platform.name)
        # add hooks specifically requested by platform
        args.hooks = args.hooks.union(self.platform.required_hooks)
        # add some hooks depending on args
        if args.get('enable_gc_logs', False):
            args.hooks.add('gclog')
        if args.get('cpumask', False):
            args.hooks.add('cpumask')
        if args.get('custom_script', False):
            args.hooks.add('run_custom_script')
        if args.get('safepoint_checker', False):
            args.hooks.add('safepoint_checker')
        try:
            if 'socperf_clean' in args.hooks:
                # this one should run before all
                args.hooks.remove('socperf_clean')
                self.hooks = HookRegistry().register_all_by_name({'socperf_clean'}, args)
            self.hooks = HookRegistry().register_all_by_name(args.hooks, args)
        except (RuntimeError, ValueError) as e:
            log.fatal(e)
            sys.exit(1)
        self.abort_on_fail = args.abort_on_fail
        self.dry_run = args.dry_run
        self.no_run = args.no_run
        self.skip_cleanup = args.skip_cleanup
        self.exclude_list = args.exclude_list
        self.fail_logs = args.fail_logs
        self.tests_per_batch = args.tests_per_batch

    @staticmethod
    def save_failure_lists(bus: List[BenchUnit], lst: str = 'failures.lst') -> None:
        lst_path = Path(lst).resolve().with_suffix('.lst')  # force suffix
        tst_path = Path(lst).resolve().with_suffix('.txt')
        # Note: paths are absolute
        names, srcs = tuple(zip(*[(str(bu.name), str(bu.doclet_src)) for bu in bus if not test_passed(bu.result)]))
        if not names:
            return
        with create_file(lst_path) as f:
            f.write("\n".join(set(srcs)) + "\n")
        with create_file(tst_path) as f:
            f.write("\n".join(names) + "\n")
        log.warning('Failure lists created. Re-run with:\n--test-list=%s %s',
                    str(tst_path), str(lst_path))

    def process_error(self, bu: BenchUnit, e: Exception) -> None:
        msg = str(e)
        if isinstance(e, VmbToolExecError):
            msg = e.out
            if BUStatus.COMPILATION_FAILED == bu.status:
                bu.result.compile_status = 1
                log.error('%s: compilation failed', bu.name)
            else:
                bu.status = BUStatus.EXECUTION_FAILED
        if isinstance(e, RuntimeError):
            bu.status = BUStatus.ERROR
        elif isinstance(e, TimeoutExpired):
            bu.status = BUStatus.TIMEOUT
        if self.fail_logs:
            bu.save_fail_log(self.fail_logs, msg)
        log.error(e)

    def run_one_unit(self, bu: BenchUnit) -> None:
        timer_unit = Timer()
        if bu.name in self.exclude_list:
            log.warning('Excluding bench unit: %s', bu.name)
            bu.status = BUStatus.SKIPPED
            return
        try:
            self.hooks.run_before_unit(bu)
            timer_unit.start()
            self.platform.run_unit(bu)  # do actual work
            self.hooks.run_after_unit(bu)
            if BUStatus.PASS == bu.status:
                log.passed('%s: %f', bu.name, bu.result.get_avg_time())
            elif BUStatus.COMPILATION_FAILED == bu.status:
                bu.result.compile_status = 1
                log.error('%s: compilation failed', bu.name)
            elif len(bu.result.execution_forks) == 0 and not (self.dry_run or self.no_run):
                raise VmbToolExecError('No benchmark iterations!')
            elif not (self.dry_run or self.no_run):
                log.error('%s: failed', bu.name)
        except (VmbToolExecError, TimeoutExpired, RuntimeError) as e:
            self.process_error(bu, e)
            if self.abort_on_fail:
                log.fatal('Aborting on first fail...')
                return
        except KeyboardInterrupt as e:
            raise KeyboardInterrupt() from e
        finally:
            if not (self.dry_run or self.skip_cleanup):
                self.platform.cleanup(bu)
            timer_unit.finish()
            elapsed = timer_unit.elapsed().total_seconds()
            bu.result.full_time = elapsed
            log.debug('%s total time: %f', bu.name, elapsed)

    def run_suite_batch(self, bench_units: List[BenchUnit]) -> List[BenchUnit]:
        tests_per_batch = self.tests_per_batch if self.tests_per_batch > 0 else 5
        current = 0
        total = len(bench_units)
        groups = list(range(0, total, tests_per_batch))
        batch = 0
        for i in groups:
            batch += 1
            log.info('Starting bench group %d/%d', batch, len(groups))
            end = i + tests_per_batch
            try:
                self.platform.run_batch(bench_units[i:end])
            except (VmbToolExecError, TimeoutExpired, RuntimeError) as e:
                log.error('Batch run failed in pre-phase!')
                log.error(str(e))
                for bu in bench_units[i:end]:
                    bu.status = BUStatus.ERROR
                continue
            except KeyboardInterrupt:
                log.warning('Aborting batch run...')
                break
            for bu in bench_units[i:end]:
                current += 1
                log.info('Starting bench (%d/%d): %s', current, total, bu.name)
                try:
                    self.run_one_unit(bu)
                except KeyboardInterrupt:
                    log.warning('Aborting run...')
                    break
        return bench_units

    def run_suite_serial(self, bench_units: List[BenchUnit]) -> List[BenchUnit]:
        total = len(bench_units)
        current = 0
        for bu in bench_units:
            current += 1
            log.info('Starting bench (%d/%d): %s', current, total, bu.name)
            try:
                self.run_one_unit(bu)
            except KeyboardInterrupt:
                log.warning('Aborting run...')
                break
        return bench_units

    def run(self, bench_units: List[BenchUnit]) -> Tuple[List[BenchUnit], ExtInfo, Timer]:
        log.info("Starting RUN phase...")
        if self.no_run:
            log.warning('%s\nDummy run! No command actually executed!\n%s', '=' * 40, '=' * 40)
        timer_suite = Timer()
        self.hooks.run_before_suite(self.platform)
        # the point is to run platform init (if any) after hooks
        self.platform.lazy_setup()
        # run suite in serial or batch mode
        # if platform expose 'run_batch' method run it in batch mode
        # otherwise - one by one
        run_batch = getattr(self.platform, 'run_batch', None)
        bus = self.run_suite_batch(bench_units) \
            if run_batch and callable(run_batch) \
            else self.run_suite_serial(bench_units)
        self.hooks.run_after_suite(self.platform)
        timer_suite.finish()
        elapsed = timer_suite.elapsed()
        if self.dry_run:
            log.passed('Dry run finished in %s', elapsed)
        else:
            log.passed('Run took %s', elapsed)
        return bus, self.platform.ext_info, timer_suite


if __name__ == '__main__':
    arg = Args()
    runner = VmbRunner(arg)
    runner.run(PlatformBase.search_units(arg.paths))
