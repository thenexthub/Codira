#!/usr/bin/env python3

# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors


"""
Utility script used to run the black code formatter over all the Python scripts in the
project sources.
"""


import argparse
import os
import subprocess
import sys
from pathlib import Path


_LANGUAGE_PATH = Path(__file__).resolve().parents[1]

_KNOWN_SCRIPT_PATHS = [
    _LANGUAGE_PATH / "benchmark/scripts/Benchmark_Driver",
    _LANGUAGE_PATH / "benchmark/scripts/Benchmark_DTrace.in",
    _LANGUAGE_PATH / "benchmark/scripts/Benchmark_GuardMalloc.in",
    _LANGUAGE_PATH / "benchmark/scripts/Benchmark_QuickCheck.in",
    _LANGUAGE_PATH / "benchmark/scripts/Benchmark_RuntimeLeaksRunner.in",
    _LANGUAGE_PATH / "benchmark/scripts/run_smoke_bench",
    _LANGUAGE_PATH / "docs/scripts/ns-html2rst",
    _LANGUAGE_PATH / "test/Driver/Inputs/fake-toolchain/ld",
    _LANGUAGE_PATH / "utils/80+-check",
    _LANGUAGE_PATH / "utils/backtrace-check",
    _LANGUAGE_PATH / "utils/build-script",
    _LANGUAGE_PATH / "utils/check-incremental",
    _LANGUAGE_PATH / "utils/coverage/coverage-build-db",
    _LANGUAGE_PATH / "utils/coverage/coverage-generate-data",
    _LANGUAGE_PATH / "utils/coverage/coverage-query-db",
    _LANGUAGE_PATH / "utils/coverage/coverage-touch-tests",
    _LANGUAGE_PATH / "utils/dev-scripts/blockifyasm",
    _LANGUAGE_PATH / "utils/dev-scripts/split-cmdline",
    _LANGUAGE_PATH / "utils/gyb",
    _LANGUAGE_PATH / "utils/line-directive",
    _LANGUAGE_PATH / "utils/PathSanitizingFileCheck",
    _LANGUAGE_PATH / "utils/recursive-lipo",
    _LANGUAGE_PATH / "utils/round-trip-syntax-test",
    _LANGUAGE_PATH / "utils/rth",
    _LANGUAGE_PATH / "utils/run-test",
    _LANGUAGE_PATH / "utils/scale-test",
    _LANGUAGE_PATH / "utils/submit-benchmark-results",
    _LANGUAGE_PATH / "utils/language_build_support/tests/mock-distcc",
    _LANGUAGE_PATH / "utils/symbolicate-linux-fatal",
    _LANGUAGE_PATH / "utils/update-checkout",
    _LANGUAGE_PATH / "utils/viewcfg",
]


_INSTALL_BLACK_MESSAGE = """\
The black Python package is required for formatting, but it was not found on
your system.

You can install it using:

    python3 -m pip install black

For more help, see https://black.readthedocs.io.
"""


def _get_python_sources():
    """Returns a list of path objects for all known Python sources in the Codira
    project.
    """

    return list(_LANGUAGE_PATH.rglob("*.py")) + _KNOWN_SCRIPT_PATHS


def _is_package_installed(name):
    """Runs the pip command to check if a package is installed.
    """

    command = [
        sys.executable,
        "-m",
        "pip",
        "show",
        "--quiet",
        name,
    ]

    with open(os.devnull, "w") as devnull:
        status = subprocess.call(command, stderr=devnull)

    return not status


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "paths",
        type=Path,
        metavar="PATH",
        nargs="*",
        help="Source path to format.",
    )

    parser.add_argument(
        "--check",
        action="store_true",
        help="Don't write the files back, just return the status.",
    )

    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Emit messages to stderr about files that were not changed.",
    )

    parser.add_argument(
        "--diff",
        action="store_true",
        help="Don't write the files back, just output a diff for each file on stdout.",
    )

    parser.add_argument(
        "-S",
        "--skip-string-normalization",
        action="store_true",
        help="Don't normalize string quotes or prefixes.",
    )

    return parser.parse_args()


def main():
    args = parse_args()

    if not _is_package_installed("black"):
        print(_INSTALL_BLACK_MESSAGE)
        return 1

    command = [
        sys.executable,
        "-m",
        "black",
        "--target-version",
        "py38",
    ]

    if args.check:
        command.append("--check")
    if args.verbose:
        command.append("--verbose")
    if args.diff:
        command.append("--diff")
    if args.skip_string_normalization:
        command.append("--skip-string-normalization")

    requested_paths = [path.resolve() for path in args.paths]

    # Narrow down the set of paths to format to only those paths which are either
    # included in the set of requested paths or are subpaths of the requested paths.
    format_paths = {
        known_path
        for path in requested_paths
        for known_path in _get_python_sources()
        if path == known_path or path in known_path.parents
    }

    # Add requested paths that exists, but aren't included in the format set.
    for path in requested_paths:
        if path not in format_paths and path.exists():
            format_paths.add(path)

    command += sorted([str(path) for path in format_paths])

    return subprocess.call(command)


if __name__ == "__main__":
    sys.exit(main())
