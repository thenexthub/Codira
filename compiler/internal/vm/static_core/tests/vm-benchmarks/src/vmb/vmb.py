#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from __future__ import annotations
import sys
import logging
from pathlib import Path
from vmb.helpers import ColorFormatter, \
    PASS_LOG_LEVEL, TRACE_LOG_LEVEL, \
    log_pass, log_trace
from vmb.generate import generate_main
from vmb.run import VmbRunner
from vmb.report import report_main
from vmb.log import log_main
from vmb.cli import Args, Command, LOG_LEVELS
from vmb.platform import PlatformBase

__all__ = ['main', 'VERSION']

VERSION = '1.0.5'
log = logging.getLogger("vmb")
# Inject new log level above info
logging.addLevelName(PASS_LOG_LEVEL, "PASS")
logging.addLevelName(TRACE_LOG_LEVEL, "TRAC")
logging.Logger.passed = log_pass  # type: ignore
logging.Logger.trace = log_trace  # type: ignore


def print_list(args: Args) -> None:
    vmb_root = Path(__file__).parent.resolve()
    for p in ('hooks', 'tools', 'langs', 'platforms'):
        print(f'\n=== {p} ===')
        for f in sorted(vmb_root.joinpath('plugins', p).glob('*.py')):
            print(f"    {f.with_suffix('').name}")
        if args.extra_plugins:
            for f in sorted(args.extra_plugins.joinpath(p).glob('*.py')):
                print(f"    {f.with_suffix('').name} (extra)")


def main() -> None:
    """Entry point for all the commands."""
    args = Args()
    lvl = dict(zip(LOG_LEVELS,
                   (logging.FATAL,
                    PASS_LOG_LEVEL,
                    logging.ERROR,
                    logging.WARN,
                    logging.INFO,
                    logging.DEBUG,
                    # pylint: disable-next=no-member
                    TRACE_LOG_LEVEL)))[args.log_level]
    log.setLevel(lvl)
    log_handler = logging.StreamHandler()
    log_handler.setLevel(lvl)
    if args.get('no_color', False):
        log_handler.setFormatter(
            logging.Formatter('[%(levelname)s] %(message)s'))
    else:
        # pylint: disable-next=no-member
        log_handler.setFormatter(ColorFormatter(timestamps=args.timestamps))
    log.addHandler(log_handler)

    if args.command == Command.VERSION:
        print(VERSION)
        return

    if args.command == Command.LIST:
        print_list(args)
        return

    if args.command == Command.REPORT:
        report_main(args)
        return

    if args.command == Command.LOG:
        log_main(args)
        return

    if args.command == Command.GEN:
        generate_main(args)
        return

    runner = VmbRunner(args)
    if args.command == Command.ALL:
        # if there is no override, use all langs, exposed by platform
        if not args.langs:
            args.langs = runner.platform.langs
        bench_units = generate_main(args, settings=runner.platform.template)
    else:
        bench_units = PlatformBase.search_units(args.paths)

    bus, ext_info, timer = runner.run(bench_units)
    if not bus:
        log.error('No tests run!')
        sys.exit(1)
    if args.fail_list:
        VmbRunner.save_failure_lists(bus, args.fail_list)
    report_main(args, bus=bus, ext_info=ext_info, timer=timer)


if __name__ == '__main__':
    main()
