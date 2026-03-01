#!/usr/bin/env python3
# coding=utf-8
#
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

import os
import re
import shutil
import subprocess
import time
import unittest
from pathlib import Path


class TestCoverageScript(unittest.TestCase):
    def setUp(self):
        """SetUp for the Tests"""
        self.panda_bin_root = os.getenv("PANDA_BINARY_ROOT")
        self.panda_root = os.getenv("PANDA_ROOT")
        print(f"panda_bin_root: {self.panda_bin_root}")
        print(f"panda_root: {self.panda_root}")
        self.src_path = "./coverage_spec"
        if self.panda_bin_root is None:
            raise EnvironmentError("PANDA_BINARY_ROOT environment variable is not set")
        if self.panda_root is None:
            raise EnvironmentError("PANDA_ROOT environment variable is not set")

        self.es2panda_path = os.path.join(self.panda_bin_root, "bin/es2panda")
        self.ark_link_path = os.path.join(self.panda_bin_root, "bin/ark_link")
        self.ark_disasm_path = os.path.join(self.panda_bin_root, "bin/ark_disasm")
        self.ark_path = os.path.join(self.panda_bin_root, "bin/ark")
        self.etsstdlib_path = os.path.join(self.panda_bin_root, "plugins/ets/etsstdlib.abc")
        self.static_core_path = Path(self.panda_root)
        self.script_path = os.path.join(
            self.panda_root,
            "runtime/tooling/coverage/coverage.py"
        )
        self.output_path = os.path.join(self.panda_bin_root, "runtime/tooling/coverage")
        ark_path = Path(self.ark_path).resolve()
        self.assertTrue(ark_path.exists(), f"Error: ark executable not found at {self.ark_path}")

        if os.path.isdir(self.output_path):
            try:
                shutil.rmtree(self.output_path)
            except Exception as e:
                print(f"delete directory Error: {e}")
        pass

    def tearDown(self):
        """clean up"""
        pass

    def wait_for_csv_content(self, csv_path, max_wait_time = 10, check_interval = 1):
        """
        Wait for CSV file to be generated and check its content
        Args:
            csv_path: Path to the CSV file
            max_wait_time: Maximum waiting time in seconds
            check_interval: Check interval in seconds
        Raises:
            AssertionError: If no valid content is detected within timeout
        """
        start_time = time.time()
        while time.time() - start_time < max_wait_time:
            if not os.path.exists(csv_path):
                time.sleep(check_interval)
                print(f"CSV file {csv_path} not exists, waiting for {check_interval} seconds")
                continue

            # Check file size
            if os.path.getsize(csv_path) == 0:
                time.sleep(check_interval)
                print(f"CSV file {csv_path} is empty, waiting for {check_interval} seconds")
                continue

            # Check if file has content
            try:
                with open(csv_path, 'r', encoding='utf-8') as f:
                    lines = [line.strip() for line in f if line.strip()]
                    if len(lines) > 0:
                        print(f"CSV file {csv_path} has been generated and contains content")
                        return
            except Exception as e:
                print(f"Error reading CSV file: {e}")

            time.sleep(check_interval)

        # Timeout, report failure
        error_msg = f"Failed to detect valid content in CSV file {csv_path} within {max_wait_time} seconds"
        print(error_msg)
        raise AssertionError(error_msg)

    def test_host_output(self):
        """test the coverage script"""
        self.assertTrue(os.path.exists(self.es2panda_path), f"{self.es2panda_path} not exists")
        self.assertTrue(os.path.exists(self.ark_disasm_path), f"{self.ark_disasm_path} not exists")
        self.assertTrue(os.path.exists(self.ark_path), f"{self.ark_path} not exists")

        # execute coverage.py
        ohos_ets_api_path = os.path.join(self.static_core_path, "plugins/ets/sdk/api/")
        ohos_ets_arkts_path = os.path.join(self.static_core_path, "plugins/ets/sdk/arkts/")
        std_path = os.path.join(self.static_core_path, "plugins/ets/stdlib/std/")
        escompat_path = os.path.join(self.static_core_path, "plugins/ets/stdlib/escompat/")

        subprocess.run(
            [
                "python", self.script_path, "gen_ast", f"--src={self.src_path}", f"--output={self.output_path}",
                f"--es2panda={self.es2panda_path}", f"--ark-link={self.ark_link_path}", "--mode=host-multi",
                "--abc-link-name=coverage_spec.abc",
                f"--ets-arkts-path={ohos_ets_arkts_path}", f"--ets-api-path={ohos_ets_api_path}",
                f"--std-path={std_path}", f"--escompat-path={escompat_path}"
            ],
            check=True
        )
        subprocess.run(
            [
                "python", self.script_path, "gen_pa", f"--src={self.output_path}/abc",
                f"--output={self.output_path}/pa",
                f"--ark_disasm-path={self.ark_disasm_path}"
            ],
            check=True
        )

        ast_output = Path(f"{self.output_path}/runtime_info/coverageBytecodeInfo.csv")
        ast_output.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            f"{self.ark_path}", f"--boot-panda-files={self.etsstdlib_path}",
            "--load-runtimes=ets", f"{self.output_path}/abc/coverage_spec.abc",
            "coverage_spec.coverage_spec.ETSGLOBAL::main",
            "--code-coverage-enabled=true", f"--code-coverage-output={ast_output}"
        ]
        print(*cmd)
        subprocess.run(cmd, check=True)
        self.wait_for_csv_content(ast_output)

        subprocess.run(
            [
                "python", self.script_path, "gen_report", f"--src={self.src_path}", f"--workdir={self.output_path}",
                "--mode=host-multi",
                "-a"
            ],
            check=True
        )

        self.assertTrue(os.path.exists(f"{self.output_path}/report/index.html"),
                        "The report/index.html file is not generated")
        self.assertTrue(os.path.exists(f"{self.output_path}/result.info"), "The result.info file is not generated")

        cmd = ['lcov', '--summary', f"{self.output_path}/result.info", '--rc', 'lcov_branch_coverage=1']
        print(*cmd)
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=True
        )
        print(result.stdout)
        summary = self.parse_lcov_summary(result.stdout)

        # Check if all required coverage data exists
        required_categories = ['lines', 'functions', 'branches']
        for category in required_categories:
            self.assertIn(category, summary, f"{category} coverage data not found")

        # Line coverage: > 70%
        lines_percent = summary['lines']['percentage'] * 100
        self.assertGreater(lines_percent, 70.0,
                           f"Line coverage {lines_percent:.1f}% is not greater than 70%")

        # Function coverage: >= 90%
        functions_percent = summary['functions']['percentage'] * 100
        self.assertGreaterEqual(functions_percent, 90.0,
                                f"Function coverage {functions_percent:.1f}% is not greater than 90%")

        # Branch coverage: >= 55%
        branches_percent = summary['branches']['percentage'] * 100
        self.assertGreaterEqual(branches_percent, 55.0,
                                f"Branch coverage {branches_percent:.1f}% is not greater than 55%")

    def parse_lcov_summary(self, output):
        summary = {}

        pattern = r'(\w+)\.+:\s*([\d.]+)%\s*\((\d+)\s+of\s+(\d+)\s+\w+\)'

        for line in output.split('\n'):
            match = re.search(pattern, line)
            if match:
                category = match.group(1)
                percentage = float(match.group(2)) / 100
                covered = int(match.group(3))
                total = int(match.group(4))

                summary[category] = {
                    'percentage': percentage,
                    'covered': covered,
                    'total': total
                }

        return summary


if __name__ == "__main__":
    unittest.main()
