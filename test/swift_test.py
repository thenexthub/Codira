# language/test/language_test.py - CodiraTest format for lit tests -*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
# -----------------------------------------------------------------------------
#
# This is a test format file for the 'lit' test runner.
#
# -----------------------------------------------------------------------------

import os

import lit
import lit.formats
import lit.util


class CodiraTest(lit.formats.ShTest, object):
    def __init__(self, coverage_mode=None, execute_external=True):
        super(CodiraTest, self).__init__(execute_external=execute_external)
        if coverage_mode == "FALSE":
            self.coverage_mode = None
        else:
            self.coverage_mode = coverage_mode
        self.skipped_tests = set()

    def before_test(self, test, litConfig):
        if self.coverage_mode:
            # FIXME: The compiler crashers run so fast they fill up the
            # merger's queue (and therefore the build bot's disk)
            if 'crasher' in test.getSourcePath():
                test.config.environment["TOOLCHAIN_PROFILE_FILE"] = os.devnull
                self.skipped_tests.add(test.getSourcePath())
                return

            if self.coverage_mode == "NOT_MERGED":
                execpath = test.getExecPath()
                profdir = os.path.join(os.path.dirname(execpath), "Output",
                                       os.path.basename(execpath) + '.profdir')
                if not os.path.exists(profdir):
                    os.makedirs(profdir)

                test.config.environment["TOOLCHAIN_PROFILE_FILE"] = \
                    os.path.join(profdir, "language-%p.profraw")
            else:
                test.config.environment["TOOLCHAIN_PROFILE_FILE"] = \
                    os.path.join(test.config.code_test_results_dir,
                                 "language-%4m.profraw")

    def after_test(self, test, litConfig, result):
        if test.getSourcePath() in self.skipped_tests:
            self.skipped_tests.remove(test.getSourcePath())
        return result

    def execute(self, test, litConfig):
        self.before_test(test, litConfig)
        result = super(CodiraTest, self).execute(test, litConfig)
        return self.after_test(test, litConfig, result)
