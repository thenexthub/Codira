# ==--- opt_bug_reducer_test.py ------------------------------------------===#
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
# ==----------------------------------------------------------------------===#


import os
import platform
import re
import shutil
import subprocess
import unittest


import bug_reducer.code_tools as language_tools


@unittest.skipUnless(platform.system() == 'Darwin',
                     'func_bug_reducer is only available on Darwin for now')
class FuncBugReducerTestCase(unittest.TestCase):

    verbose = False

    def setUp(self):
        self.file_dir = os.path.dirname(os.path.abspath(__file__))
        self.reducer = os.path.join(os.path.dirname(self.file_dir),
                                    'bug_reducer', 'bug_reducer.py')
        self.build_dir = os.path.abspath(
            os.environ['BUGREDUCE_TEST_LANGUAGE_OBJ_ROOT'])

        (root, _) = os.path.splitext(os.path.abspath(__file__))
        self.root_basename = ''.join(os.path.basename(root).split('_'))
        self.tmp_dir = os.path.join(
            os.path.abspath(os.environ['BUGREDUCE_TEST_TMP_DIR']),
            self.root_basename)
        subprocess.call(['mkdir', '-p', self.tmp_dir])

        self.module_cache = os.path.join(self.tmp_dir, 'module_cache')
        self.sdk = subprocess.check_output(['xcrun', '--sdk', 'macosx',
                                            '--toolchain', 'Default',
                                            '--show-sdk-path']).strip("\n")
        self.tools = language_tools.CodiraTools(self.build_dir)
        self.passes = ['--pass=-bug-reducer-tester']

        if os.access(self.tmp_dir, os.F_OK):
            shutil.rmtree(self.tmp_dir)
        os.makedirs(self.tmp_dir)
        os.makedirs(self.module_cache)

    def _get_test_file_path(self, module_name):
        return os.path.join(self.file_dir,
                            '{}_{}.code'.format(
                                self.root_basename, module_name))

    def _get_sib_file_path(self, filename):
        (root, ext) = os.path.splitext(filename)
        return os.path.join(self.tmp_dir, os.path.basename(root) + '.sib')

    def run_languagec_command(self, name):
        input_file_path = self._get_test_file_path(name)
        sib_path = self._get_sib_file_path(input_file_path)
        args = [self.tools.codec,
                '-module-cache-path', self.module_cache,
                '-sdk', self.sdk,
                '-Onone', '-parse-as-library',
                '-module-name', name,
                '-emit-sib',
                '-resource-dir', os.path.join(self.build_dir, 'lib', 'language'),
                '-o', sib_path,
                input_file_path]
        return self.run_check_call(args)

    def run_check_call(self, args):
        if FuncBugReducerTestCase.verbose:
            print('Cmd: {}'.format(' '.join(args)))
        result_code = subprocess.check_call(args)
        if FuncBugReducerTestCase.verbose:
            print('Result Code: {}'.format(result_code))
        return result_code

    def run_check_output(self, args):
        if FuncBugReducerTestCase.verbose:
            print('Cmd: {}'.format(' '.join(args)))
        raw_output = subprocess.check_output(args)
        if FuncBugReducerTestCase.verbose:
            print('Raw Output: {}'.format(raw_output))
        return raw_output

    def test_basic(self):
        name = 'testbasic'
        result_code = self.run_languagec_command(name)
        assert result_code == 0, "Failed initial compilation"
        args = [
            self.reducer,
            'fn',
            self.build_dir,
            self._get_sib_file_path(self._get_test_file_path(name)),
            '--sdk=%s' % self.sdk,
            '--module-cache=%s' % self.module_cache,
            '--module-name=%s' % name,
            '--work-dir=%s' % self.tmp_dir,
            ('--extra-silopt-arg='
             '-bug-reducer-tester-target-fn=__TF_test_target'),
            '--extra-silopt-arg=-bug-reducer-tester-failure-kind=opt-crasher'
        ]
        args.extend(self.passes)
        output = self.run_check_output(args).split("\n")
        self.assertTrue("*** Successfully Reduced file!" in output)
        self.assertTrue("*** Final Functions: " +
                        "$s9testbasic6foo413yyF" in output)
        re_end = 'testfuncbugreducer_testbasic_'
        re_end += '92196894259b5d6c98d1b77f19240904.sib'
        output_file_re = re.compile(r'\*\*\* Final File: .*' + re_end)
        output_matches = [
            1 for o in output if output_file_re.match(o) is not None]
        self.assertEqual(sum(output_matches), 1)
        # Make sure our final output command does not have -emit-sib in
        # the output. We want users to get sil output when they type in
        # the relevant command.
        self.assertEqual([], [o for o in output if '-emit-sib' in o])


if __name__ == '__main__':
    unittest.main()
