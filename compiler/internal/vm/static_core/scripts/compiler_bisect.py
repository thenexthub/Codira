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

import subprocess
import sys
import shutil
import glob
import re
import random
import os
from collections import namedtuple
from typing import List, Set, Dict, Optional, Tuple, Iterable

HELP_TEXT = """
Script to help find minimal subsets of compiled methods, inlined methods and compiler passes
to reproduce compiler bugs.

Usage:
  Write a shell command or a bash script which launches a test and exits with non-zero code
  when the test fails. Use `$GEN_OPTIONS` variable in your script/command where
  compiler options are needed (probably next to other arguments of `ark`/`ark_aot`),
  and this script will set it to different values and run your script to find minimal subsets
  of options which lead to failure.

  Run `./compiler_bisect.py [COMMAND] [ARGS]...`
  Or put command in a script and do `./compiler_bisect.py ./script.sh`

Example usage:
    cd $ARK_BUILD
    $ARK_ROOT/scripts/compiler_bisect.py bin/ark --boot-panda-files=plugins/ets/etsstdlib.abc --load-runtimes=ets \
    --compiler-enable-jit=true --no-async-jit=true --compiler-hotness-threshold=0 --compiler-ignore-failures=false \
    \$GEN_OPTIONS test.abc ETSGLOBAL::main

  Or put command in `run.sh` and do `./compiler_bisect.py ./run.sh`

  Note that you need escaped \$GEN_OPTIONS when passing command with arguments, and not escaped
  $GEN_OPTIONS inside your script
"""


class Colors:
    """ANSI color codes for terminal output"""
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'


class PassConfig:
    """Configuration for compiler passes and their logging"""
    PASS_LOGS: Dict[str, Optional[str]] = {
        'lowering': 'lowering',
        'code-sink': 'code-sink',
        'balance-expressions': 'balance-expr',
        'branch-elimination': 'branch-elim',
        'checks-elimination': 'checks-elim',
        'deoptimize-elimination': 'deoptimize-elim',
        'licm': 'licm-opt',
        'licm-conditions': 'licm-cond-opt',
        'loop-unswitch': 'loop-unswitch',
        'loop-idioms': None,
        'loop-peeling': None,
        'loop-unroll': None,
        'lse': 'lse-opt',
        'cse': 'cse-opt',
        'vn': 'vn-opt',
        'memory-coalescing': 'memory-coalescing',
        'inlining': 'inlining',
        'if-conversion': 'ifconversion',
        'adjust-refs': None,
        'scalar-replacement': 'pea',
        'simplify-string-builder': 'simplify-sb',
        'enable-fast-interop': None,
        'interop-intrinsic-optimization': 'interop-intrinsic-opt',
        'peepholes': 'peephole',
        'move-constants': None,
        'if-merging': 'if-merging',
        'redundant-loop-elimination': 'rle-opt',
        'scheduling': 'scheduler',
        'optimize-memory-barriers': None,
    }

    @classmethod
    def get_pass_list(cls) -> List[str]:
        """Get list of all available passes"""
        return list(cls.PASS_LOGS.keys())


class CommandRunner:
    """Handles execution of test commands with different options"""

    def __init__(self, args: List[str]):
        self.args = args
        self.env = os.environ.copy()

    def get_run_options(
            self,
            compiled_methods: Optional[Iterable[str]],
            noinline_methods: Iterable[str],
            passes: Optional[Iterable[str]],
            dump: bool,
            verbose: bool
    ) -> List[str]:
        """Generate compiler options based on the specified parameters.

        Args:
            compiled_methods: Methods to compile (None means all methods)
            noinline_methods: Methods to exclude from inlining
            passes: Optimization passes to enable (None means all passes)
            dump: Whether to enable compiler dump
            verbose: Whether to enable verbose output

        Returns:
            List of compiler option strings
        """
        pass_list = PassConfig.get_pass_list()
        # Generate compiler regex pattern
        compiler_regex = '.*' if compiled_methods is None else '(' + '|'.join(compiled_methods) + ')'


        inline_exclude = ','.join(noinline_methods)

        # Determine which passes to use
        passes = pass_list if passes is None else passes

        # NOLINTNEXTLINE
        options = [f'--compiler-regex={compiler_regex}', f'--compiler-inlining-blacklist={inline_exclude}']
        options += [f'--compiler-{opt}={str(opt in passes).lower()}' for opt in pass_list]
        # Add dump options if needed
        if dump or verbose:
            options.append('--compiler-dump')

        # Add verbose options if needed
        if verbose:
            options.extend([
                '--compiler-disasm-dump:single-file',
                '--log-debug=compiler'
            ])

            # Generate compiler log options
            compiler_log = set(PassConfig.PASS_LOGS.get(opt) for opt in passes) - {None}
            if any('loop' in opt for opt in passes):
                compiler_log.add('loop-transform')
            if compiler_log:
                options.append('--compiler-log=' + ','.join(compiler_log))

        return options


    def run_test(
            self,
            compiled_methods: Optional[Iterable[str]],
            noinline_methods: Iterable[str],
            passes: Optional[Iterable[str]],
            dump: bool = False,
            verbose: bool = False,
            expect_fail: Optional[bool] = None
    ) -> subprocess.CompletedProcess:
        """Execute the compiler with specified options and return the result.

        Args:
            compiled_methods: Methods to compile (None for all methods)
            noinline_methods: Methods to exclude from inlining
            passes: Optimization passes to enable (None for all passes)
            dump: Whether to enable compiler dump
            verbose: Whether to enable verbose output
            expect_fail: Whether failure is expected (None for no expectation)

        Returns:
            Completed process result from subprocess.run()

        Raises:
            RuntimeError: If command execution fails
        """
        options = self.get_run_options(compiled_methods, noinline_methods, passes, dump, verbose)
        options_str = ' '.join(options)
        self.env["GEN_OPTIONS"] = options_str

        shutil.rmtree('ir_dump', ignore_errors=True)

        ## Prepare command
        cmd = self._prepare_command(options)

        if verbose or (expect_fail is not None):
            self.env['SHELLOPTS'] = 'xtrace'

        try:
            res = subprocess.run(
                cmd,
                env=self.env,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
        except Exception as e:
            raise RuntimeError(f"Command execution failed: {e}") from e

        # Process and display results
        self._handle_results(res, options_str, cmd, verbose, expect_fail)

        return res

    def _is_shell_script(self, path: str) -> bool:
        """Check if a file is a shell script"""
        try:
            with open(path, 'r') as f:
                first_line = f.readline()
                return first_line.startswith('#!') and 'sh' in first_line
        except (IOError, FileNotFoundError):
            return False

    def _prepare_command(self, options: List[str]) -> List[str]:
        """Prepare the command to be executed."""
        if len(sys.argv) > 1 and (sys.argv[1].endswith('.sh') or self._is_shell_script(sys.argv[1])):
            return ['bash', sys.argv[1]]

        cmd = []
        for arg in sys.argv[1:]:
            if arg == '$GEN_OPTIONS':
                cmd.extend(options)  # More efficient than +=
            else:
                cmd.append(arg)
        return cmd

    def _handle_results(
            self,
            result: subprocess.CompletedProcess,
            options_str: str,
            cmd: List[str],
            verbose: bool,
            expect_fail: Optional[bool]
    ) -> None:
        """Handle and display the command results."""
        # Determine status color
        status_color = Colors.WARNING if result.returncode else Colors.OKGREEN
        print_color(status_color, "Return value:", result.returncode)

        # Check if we need to force verbose output
        if expect_fail is not None and (result.returncode != 0) != expect_fail:
            verbose = True

        # Show detailed output if verbose
        if verbose:
            print(f"GEN_OPTIONS='{options_str}'")
            print('Command:', ' '.join(cmd))
            if result.stdout:
                print("Standard Output:")
                print(result.stdout)
            if result.stderr:
                print("Standard Error:")
                print(result.stderr)


class OptionSet:
    """Manages a set of options for bisection"""

    def __init__(self, values: Set[str]):
        self.all_values = set(values)
        self.current_values = list(values)

    @property
    def negative_values(self) -> Set[str]:
        """Get values not in current set"""
        return self.all_values - set(self.current_values)


class BisectionEngine:
    """Handles the bisection process to find minimal failing options"""

    Options = namedtuple('Options', ['compiled', 'inline', 'passes'])

    def __init__(self, runner: CommandRunner):
        self.runner = runner
        self.opts = None
        self.current_option = ''

    def initialize_options(
            self,
            compiled_methods: Set[str],
            inline_methods: Set[str],
            passes: Set[str]
    ) -> None:
        """Initialize the options for bisection.

        Args:
            compiled_methods: Set of methods to compile
            inline_methods: Set of methods to inline
            passes: Set of compiler passes to enable
        """
        self.opts = self.Options(
            compiled=OptionSet(compiled_methods),
            inline=OptionSet(inline_methods),
            passes=OptionSet(passes)
        )

    def set_option(self, name, value):
        self.opts = self.opts._replace(**{name: value})

    def run_with_options(self, verbose: bool = False) -> bool:
        """Run test with current options and return whether it failed.

        Args:
            verbose: Whether to show verbose output

        Returns:
            bool: True if test failed, False otherwise
        """
        result = self.runner.run_test(self.opts.compiled.current_values, self.opts.inline.negative_values,
                                      self.opts.passes.current_values, verbose=verbose)
        return result.returncode != 0

    def bisect_option(self, option_name: str) -> None:
        """Perform bisection on a specific option set to find minimal failing subset.

        Args:
            option_name: Name of option to bisect ('compiled', 'inline', or 'passes')
        """
        self.current_option = option_name
        option_set = getattr(self.opts, option_name)
        print_color(Colors.OKCYAN, f'Bisecting {option_name}...')

        while True:
            current_values = option_set.current_values
            if len(current_values) <= 0:
                break

            # First try splitting the set in half
            if len(current_values) > 3:
                mid = len(current_values) // 2
                halves = [current_values[:mid], current_values[mid:]]

                if any(self._try_option_values(option_set, half) for half in halves):
                    continue

            # Then try removing individual elements
            random.shuffle(current_values)
            for i in range(len(current_values)):
                reduced_values = current_values[:i] + current_values[i + 1:]
                if self._try_option_values(option_set, reduced_values):
                    break
            else:
                break  # No further reduction possible

        print_color(
            Colors.OKGREEN + Colors.BOLD,
            f'Minimal {option_name} set: {option_set.current_values}'
        )

    def _try_option_values(self, option_set: OptionSet, new_values: List[str]) -> bool:
        """Temporarily try new values for an option set.

        Args:
            option_set: The option set to modify
            new_values: New values to try

        Returns:
            bool: True if the new values should be kept
        """
        print_color(Colors.OKCYAN, f'Testing {self.current_option}={new_values}')
        old_values = option_set.current_values
        option_set.current_values = new_values
        should_keep = self.run_with_options()

        if not should_keep:
            option_set.current_values = old_values
        return should_keep


def parse_methods() -> Tuple[Set[str], Set[str]]:
    """Parse compiled and inlined methods from IR dumps"""
    compiled = set()
    inlined = set()

    for dump in glob.glob('ir_dump/*IrBuilder.ir'):
        try:
            method = re.match(r'ir_dump/\d+_pass_\d+_(.*)_IrBuilder.ir', dump).group(1)
        except Exception as e:
            raise RuntimeError(f"Failed to parse IR dump {dump}: {e}")
        parts = re.split('(?<!_)_(?!_*$)', method)

        if parts[-1] in ['_ctor_', '_cctor_']:
            parts[-1] = f'<{parts[-1].strip("_")}>'

        dest = compiled
        if parts[0] == 'inlined':
            dest = inlined
            parts.pop(0)

        qname = '::'.join(('.'.join(parts[:-1]), parts[-1]))
        dest.add(qname)
    return compiled, inlined


def print_color(color: str, *args, **kwargs) -> None:
    """Print colored text to terminal with proper ANSI code handling.

    Args:
        color: ANSI color code from Colors class
        *args: Positional arguments to print
        **kwargs: Keyword arguments for print()

    Example:
        print_color(Colors.OKGREEN, "Success!", file=sys.stderr)
    """
    print(color, end='')
    print(*args, **kwargs)
    print(Colors.ENDC, end='')


def exit_with_error(message: str = '', usage: bool = False) -> None:
    """Exit with colored error message and optional usage help.

    Args:
        message: Error message to display
        usage: Whether to show usage information
    """
    buffer = []
    if message:
        buffer.append(f"{Colors.FAIL}{message}")
    if usage:
        buffer.extend([
            f"{Colors.FAIL}Usage: {sys.argv[0]} COMMAND [ARG]...",
            f"COMMAND or ARGS should use $GEN_OPTIONS variable.",
            f"{sys.argv[0]} -h for help"
        ])
    print('\n'.join(buffer), file=sys.stderr)
    sys.exit(1)


def main():
    if len(sys.argv) < 2:
        exit_with_error(usage=True)
    if sys.argv[1] in ('-h', '--help'):
        print(HELP_TEXT)
        sys.exit(0)

    # Initialize components
    runner = CommandRunner(sys.argv[1:])
    bisector = BisectionEngine(runner)

    # Verify baseline cases
    print_color(Colors.OKCYAN, 'Running without compiled methods')
    if runner.run_test([], [], [], expect_fail=False).returncode != 0:
        exit_with_error("Script failed without compiled methods")

    print_color(Colors.OKCYAN, 'Running with all methods compiled and optimizations enabled')
    if runner.run_test(None, [], None, dump=True, expect_fail=True).returncode == 0:
        exit_with_error("Script didn't fail with default args")

    compiled, _ = parse_methods()
    bisector.initialize_options(compiled, set(), PassConfig.get_pass_list())

    # Perform bisection on each option type
    bisector.bisect_option('compiled')

    runner.run_test(bisector.opts.compiled.current_values, [], None, dump=True)
    _, inlined = parse_methods()
    bisector.set_option('inline', OptionSet(inlined))
    bisector.bisect_option('inline')
    bisector.bisect_option('passes')

    # Print final results
    print_color(Colors.OKGREEN + Colors.BOLD, 'Found minimal failing configuration:')
    if bisector.run_with_options(verbose=True):
        print_color(Colors.OKGREEN + Colors.BOLD, 'Options:')
        for name, opt in bisector.opts._asdict().items():
            print_color(Colors.OKGREEN + Colors.BOLD, f" {name}: {opt.current_values} ")
    else:
        exit_with_error("Didn't fail with found options")


if __name__ == "__main__":
    main()
