#!/usr/bin/env python3

import argparse
import os
import subprocess


def perform_build(args, languagebuild_path):
    languagebuild_args = [
        languagebuild_path,
        "--package-path",
        args.package_path,
        "--build-path",
        args.build_path,
        "--configuration",
        args.configuration,
        "-Xlanguagec",
        "-I",
        "-Xlanguagec",
        os.path.join(args.toolchain, 'usr', 'include', 'language'),
        "-Xlanguagec",
        "-L",
        "-Xlanguagec",
        os.path.join(args.toolchain, 'usr', 'lib', 'language', 'macosx'),
        "-Xlanguagec",
        "-llanguageRemoteMirror"
    ]
    if args.verbose:
        languagebuild_args.append("--verbose")
    print(' '.join(languagebuild_args))
    subprocess.call(languagebuild_args)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", "-v", action="store_true")
    parser.add_argument("--package-path", type=str, required=True)
    parser.add_argument("--build-path", type=str, required=True)
    parser.add_argument("--toolchain", type=str, required=True)
    parser.add_argument("--configuration", type=str, choices=['debug', 'release'],
                        default='release')

    args = parser.parse_args()

    # Create our bin directory so we can copy in the binaries.
    bin_dir = os.path.join(args.build_path, "bin")
    if not os.path.isdir(bin_dir):
        os.makedirs(bin_dir)

    languagebuild_path = os.path.join(args.toolchain, "usr", "bin", "language-build")
    perform_build(args, languagebuild_path)


if __name__ == "__main__":
    main()
