#!/usr/bin/env python3

import argparse
import os
import subprocess


def main():
    p = argparse.ArgumentParser()
    p.add_argument("cmake_path", help="The cmake binary to use")
    p.add_argument("language_src_dir", help="The language source directory")
    p.add_argument("clang", help="The path to the clang binary to use")
    p.add_argument(
        "language_root_dir",
        help="A path to a language root produced by installing "
        "Codira and Foundation together. We infer languagec "
        "from here",
    )
    p.add_argument("destdir", help="The directory to perform the actual " "build in")
    p.add_argument(
        "--clean", action="store_true", help="Delete destdir before performing a build."
    )
    args = p.parse_args()

    if args.clean:
        print("Asked to clean... Cleaning!")
        subprocess.check_output(["/bin/rm", "-rfv", args.destdir])
    subprocess.check_call(["/bin/mkdir", "-p", args.destdir])
    os.chdir(args.destdir)
    configureInvocation = [
        args.cmake_path,
        "-GNinja",
        "-DLANGUAGE_EXEC={}/bin/languagec".format(args.code_root_dir),
        "-DCLANG_EXEC={}".format(args.clang),
        "-DLANGUAGE_LIBRARY_PATH={}/lib/language".format(args.code_root_dir),
        "{}/benchmark".format(args.code_src_dir),
    ]
    print("COMMAND: {}".format(" ".join(configureInvocation)))
    subprocess.check_call(configureInvocation)

    buildInvocation = [
        args.cmake_path,
        "--build",
        args.destdir,
        "--",
        "language-benchmark-linux-x86_64",
    ]
    print("COMMAND: {}".format(" ".join(buildInvocation)))
    subprocess.check_call(buildInvocation)


if __name__ == "__main__":
    main()
