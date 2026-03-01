#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

from pathlib import Path
import os
import subprocess
import argparse

SRC_PATH = os.path.realpath(os.path.dirname(__file__))


class RunStats:
    def __init__(self):
        self.failed_exec = []
        self.failed_aot_compile = []
        self.failed_compile = []
        self.passed = []
        self.time = {}

    def record_failure_compile(self, path: Path):
        self.failed_compile.append(path)

    def record_failure_aot_compile(self, path: Path):
        self.failed_aot_compile.append(path)

    def record_failure_exec(self, path: Path):
        self.failed_exec.append(path)

    def record_success(self, path: Path):
        self.passed.append(path)

    def record_time(self, path: Path, time: float):
        self.time[path] = time


class Logger:
    def __init__(self, args):
        self.log_level = args.log_level

    def info(self, message):
        if self.log_level == "info" or self.log_level == "debug":
            print(message)

    def debug(self, message):
        if self.log_level == "debug":
            print(message)

    def silence(self, message):
        print(message)

# =======================================================
# Main runner class


class EtsBenchmarksRunner:
    def __init__(self, args, logger, ark_opts, aot_opts):
        self.logger = logger
        self.mode = args.mode
        self.interpreter_type = args.interpreter_type
        self.is_device = args.target == "device"
        self.host_build_dir = args.host_builddir if self.is_device else os.path.join(
            args.bindir, "..")
        self.host_output_dir = os.path.join(
            self.host_build_dir, "plugins", "ets", "tests", "micro-benchmarks")
        self.device_output_dir = os.path.join(
            args.bindir, "..", "micro-benchmarks") if self.is_device else None
        self.current_bench_name = None
        self.source_dir = os.path.join(
            SRC_PATH, "..", "..", "micro-benchmarks")
        self.wrapper_asm_filepath = os.path.join(
            self.source_dir, "wrapper", "test_wrapper.pa")
        if self.is_device:
            self.stdlib_path = os.path.join(args.libdir, "etsstdlib.abc")
        else:
            self.stdlib_path = os.path.join(
                args.bindir, "..", "plugins", "ets", "etsstdlib.abc")
        self.ark_asm = os.path.join(args.bindir, "ark_asm")
        self.ark_aot = os.path.join(args.bindir, "ark_aot")
        self.ark = os.path.join(args.bindir, "ark")
        self.prefix = [
            "hdc", "shell", f"LD_LIBRARY_PATH={args.libdir}"] if self.is_device else []
        self.ark_opts = ark_opts
        self.aot_opts = aot_opts

    def dump_output_to_file(self, pipe, file_ext):
        dumpfile = os.fdopen(os.open(os.path.join(self.host_output_dir,
                                     self.current_bench_name, f"test.{file_ext}"),
                                     os.O_RDWR | os.O_CREAT, 0o755), "w")
        dumpfile.write(pipe.decode('ascii'))
        dumpfile.close()

    def dump_stdout(self, pipe, tp):
        self.dump_output_to_file(pipe, f"{tp}_out")

    def dump_stderr(self, pipe, tp):
        self.dump_output_to_file(pipe, f"{tp}_err")

    def compile_test(self, asm_filepath, bin_filepath):
        cmd = self.prefix + [self.ark_asm,
                             str(asm_filepath), str(bin_filepath)]
        proc = subprocess.Popen(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate(timeout=5400)
        self.dump_stdout(stdout, "asm")
        if proc.returncode == 0:
            return True
        self.logger.debug(f"{self.current_bench_name} FAIL (compile)")
        self.logger.debug("How to reproduce: " + " ".join(cmd) + "\n")
        self.dump_stderr(stderr, "asm")
        return False

    def compile_aot_test(self, bin_filepath, aot_filepath):
        cmd = self.prefix + [
            self.ark_aot, "--paoc-mode=aot", f"--boot-panda-files={self.stdlib_path}",
            "--load-runtimes=ets", "--compiler-ignore-failures=false"
        ] + self.aot_opts + [
            "--paoc-panda-files", bin_filepath,
            "--paoc-output", aot_filepath
        ]
        proc = subprocess.Popen(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate(timeout=5400)
        self.dump_stdout(stdout, "aot")
        if proc.returncode == 0:
            return True
        self.logger.debug(f"{self.current_bench_name} FAIL (aot)")
        self.logger.debug("How to reproduce: " + " ".join(cmd) + "\n")
        self.dump_stderr(stderr, "aot")
        return False

    def run_test(self, bin_filepath):
        additional_opts = []
        if self.mode == "aot":
            additional_opts += ["--aot-file",
                                bin_filepath.replace(".abc", ".an")]
        # NOTE(ipetrov, #14164): return limit standard allocation after fix in taskmanager
        cmd = self.prefix + [self.ark, f"--boot-panda-files={self.stdlib_path}", "--load-runtimes=ets",
                             "--compiler-ignore-failures=false"] \
            + self.ark_opts + additional_opts + \
            [bin_filepath, "_GLOBAL::main"]
        proc = subprocess.Popen(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate(timeout=5400)
        self.dump_stdout(stdout, "ark")
        if proc.returncode == 0:
            self.logger.debug(f"{self.current_bench_name} PASS")
            return True, float(stdout.decode('ascii').split("\n")[0])
        self.logger.debug(f"{self.current_bench_name} FAIL (execute)")
        self.logger.debug("How to reproduce: " + " ".join(cmd) + "\n")
        self.dump_stderr(stderr, "ark")
        return False, 0

    def generate_benchmark(self, filename):
        base_asm_file_path = os.path.join(self.source_dir, filename)
        current_output_dir = os.path.join(
            self.host_output_dir, self.current_bench_name)
        tmp_asm_file_path = os.path.join(current_output_dir, "test.pa")
        subprocess.run(["mkdir", "-p", current_output_dir])
        fd_read = os.open(self.wrapper_asm_filepath, os.O_RDONLY, 0o755)
        fd_read_two = os.open(base_asm_file_path, os.O_RDONLY, 0o755)

        with os.fdopen(fd_read, "r") as file_to_read, \
                os.fdopen(fd_read_two, "r") as file_to_read_two, \
                os.fdopen(os.open(tmp_asm_file_path, os.O_WRONLY |
                                  os.O_CREAT, 0o755), "w") as file_to_write:
            file_to_write.write(file_to_read.read() +
                                file_to_read_two.read())

        if self.is_device:
            device_current_output_dir = os.path.join(
                self.device_output_dir, self.current_bench_name)
            subprocess.run(["hdc", "shell", f"mkdir -p {device_current_output_dir}"])
            subprocess.run(
                ["hdc", "file", "send", tmp_asm_file_path, f"{device_current_output_dir}/"])
            tmp_asm_file_path = os.path.join(
                device_current_output_dir, "test.pa")
        return tmp_asm_file_path

    def run_separate_bench(self, filename, stats):
        bench_name = Path(filename).stem
        self.current_bench_name = f"{bench_name}_{self.mode}_{self.interpreter_type}"

        tmp_asm_file_path = self.generate_benchmark(filename)

        bin_file_path = tmp_asm_file_path.replace(".pa", ".abc")
        res = self.compile_test(tmp_asm_file_path, bin_file_path)
        if not res:
            stats.record_time(bench_name, 0)
            stats.record_failure_compile(bench_name)
            return
        if self.mode == "aot":
            aot_file_path = tmp_asm_file_path.replace(".pa", ".an")
            res = self.compile_aot_test(bin_file_path, aot_file_path)
            if not res:
                stats.record_time(bench_name, 0)
                stats.record_failure_aot_compile(bench_name)
                return
        res, exec_time = self.run_test(bin_file_path)
        stats.record_time(bench_name, exec_time)
        if res:
            stats.record_success(bench_name)
        else:
            stats.record_failure_exec(bench_name)

    def run(self, args):
        stats = RunStats()

        if self.is_device:
            subprocess.run(["hdc", "shell", f"mkdir -p {self.device_output_dir}"])
        else:
            subprocess.run(["mkdir", "-p", self.host_output_dir])

        if args.test_name is not None:
            self.run_separate_bench(args.test_name + ".pa", stats)
            return stats

        if args.test_list is not None:
            with open(args.test_list, "r") as testlist_file:
                testlist = [line.strip() for line in testlist_file]
            for test_name in testlist:
                self.run_separate_bench(test_name + ".pa", stats)
            return stats

        for dirpath, dirnames, filepathes in os.walk(self.source_dir):
            for filepath in filepathes:
                if Path(filepath).suffix != ".pa":
                    continue
                self.run_separate_bench(filepath, stats)
            break
        return stats

# =======================================================
# Helpers


def dump_time_stats(logger, stats):
    max_width = max([len(x) for x in stats.time.keys()])
    keys = list(stats.time.keys())
    keys.sort()
    for name in keys:
        if name in stats.passed:
            logger.info(name.split(".")[0].ljust(
                max_width + 5) + str(round(stats.time[name], 3)))
        else:
            logger.info(f"{name.split('.')[0].ljust(max_width + 5)}None")


def dump_pass_rate(logger, skiplist_name, passed_tests, failed_tests):
    if os.path.isfile(skiplist_name):
        with open(skiplist_name, "r") as skiplist:
            skipset = set([line.strip() for line in skiplist])
    else:
        skipset = set()
    new_failed = set(failed_tests) - skipset
    new_passed = skipset.intersection(set(passed_tests)) - set(failed_tests)
    if len(new_passed) != 0:
        logger.info("\nNew passed:")
        for name in list(new_passed):
            logger.info(name)
    if len(new_failed) != 0:
        logger.info("\nNew failures:")
        for name in list(new_failed):
            logger.info(name)
        return False
    return True


def parse_results(logger, stats, mode, int_type):
    all_failed_tests = stats.failed_exec + \
        stats.failed_aot_compile + stats.failed_compile
    if len(all_failed_tests) == 0:
        return True
    return dump_pass_rate(logger, f"skiplist_{mode}_{int_type}.txt", stats.passed, all_failed_tests)


# =======================================================
# Entry

parser = argparse.ArgumentParser(description='Run ETS micro-benchmarks.')
parser.add_argument("--bindir", required=True,
                    help="Directory with compiled binaries (eg.: ark_asm, ark_aot, ark)")
parser.add_argument("--sourcedir",
                    help="Directory with source asm files (for arm64)")
parser.add_argument("--libdir",
                    help="Directory with etsstdlib (for arm64)")
parser.add_argument("--host-builddir",
                    help="Host build directory (for arm64)")
parser.add_argument("--mode", choices=["int", "jit", "aot"], default="int",
                    help="Running mode. Default: '%(default)s'")
parser.add_argument("--interpreter-type", choices=["cpp", "irtoc", "llvm"], default="cpp",
                    help="Type of interpreter. Default: '%(default)s'")
parser.add_argument("--target", choices=["host", "device"], default="host",
                    help="Launch target. Default: '%(default)s'")
parser.add_argument("--aot-options", metavar="LIST",
                    help="Comma separated list of aot options for ARK_AOT")
parser.add_argument("--runtime-options", metavar="LIST",
                    help="Comma separated list of runtime options for ARK")
parser.add_argument("--test-name",
                    help="Run a specific benchmark.")
parser.add_argument("--test-list",
                    help="List with benchmarks to be launched.")
parser.add_argument("--log-level", choices=["silence", "info", "debug"], default="info",
                    help="Log level. Default: '%(default)s'")


def main():
    args = parser.parse_args()
    if args.test_list and not os.path.isfile(args.test_list):
        logger.silence(f"ERROR: wrong path to testlist '{args.test_list}' ")
        exit(1)

    if args.target == "host" and not Path(args.bindir).is_dir():
        logger.silence(f"ERROR: Path '{args.bindir}' must be a directory")
        exit(1)

    # =======================================================
    # Prepare ARK and AOT options

    ark_opts = []
    aot_opts = []
    if args.runtime_options:
        ark_opts = ("--" + args.runtime_options.replace(" ",
                    "").replace(",", " --")).split(" ")
    if args.aot_options:
        aot_opts = ("--" + args.aot_options.replace(" ",
                    "").replace(",", " --")).split(" ")
    if args.test_list and not os.path.isfile(args.test_list):
        print_silence(f"ERROR: wrong path to testlist '{args.test_list}' ")
        exit(1)

    found_int_type_opt = False
    found_jit_opt = False
    for elem in ark_opts:
        if elem.find("compiler-enable-jit=true") != -1:
            args.mode = "jit"
            found_jit_opt = True
        if elem.find("interpreter-type") == -1:
            continue

        found_int_type_opt = True
        if elem.find("interpreter-type=cpp") != -1:
            args.interpreter_type = "cpp"
        if elem.find("interpreter-type=irtoc") != -1:
            args.interpreter_type = "irtoc"
        if elem.find("interpreter-type=llvm") != -1:
            args.interpreter_type = "llvm"

    if not found_jit_opt:
        ark_opts += ["--compiler-enable-jit=" +
                     ("true" if args.mode == "jit" else "false")]
    if not found_int_type_opt:
        ark_opts += [f"--interpreter-type={args.interpreter_type}"]

    # =======================================================
    # Run benchmarks

    logger = Logger(args)
    runner = EtsBenchmarksRunner(args, logger, ark_opts, aot_opts)

    stats = runner.run(args)

    # =======================================================
    # Prepare and output results

    if not stats:
        logger.info("\nFAIL!")
        exit(1)

    all_tests_amount = len(stats.failed_exec) + len(stats.failed_aot_compile) + \
        len(stats.failed_compile) + len(stats.passed)
    logger.info("\n=====")
    if args.mode == "aot":
        logger.info(f"TESTS {all_tests_amount} | "
                    f"FAILED (compile) {len(stats.failed_compile)} | "
                    f"FAILED (aot compile) {len(stats.failed_aot_compile)} | "
                    f"FAILED (exec) {len(stats.failed_exec)}\n")
    else:
        logger.info(f"TESTS {all_tests_amount} | "
                    f"FAILED (compile) {len(stats.failed_compile)} | "
                    f"FAILED (exec) {len(stats.failed_exec)}\n")
    dump_time_stats(logger, stats)

    if not parse_results(logger, stats, args.mode, args.interpreter_type):
        logger.info("\nFAIL!")
        exit(1)
    else:
        logger.info("\nSUCCESS!")


if __name__ == "__main__":
    main()
