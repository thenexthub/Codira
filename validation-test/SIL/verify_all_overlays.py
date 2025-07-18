#!/usr/bin/env python3
# RUN: ${python} %s %target-swiftmodule-name %platform-sdk-overlay-dir \
# RUN:     %swift_src_root \
# RUN:     %target-sil-opt -sdk %sdk -enable-sil-verify-all \
# RUN:       -F %sdk/System/Library/PrivateFrameworks \
# RUN:       %xcode-extra-frameworks-search-path

# REQUIRES: long_test
# REQUIRES: nonexecutable_test


import os
import subprocess
import sys

target_swiftmodule_name = sys.argv[1]
sdk_overlay_dir = sys.argv[2]
source_dir = sys.argv[3]
sil_opt_invocation = sys.argv[4:]

for module_file in os.listdir(sdk_overlay_dir):
    extra_args = []
    module_name, ext = os.path.splitext(module_file)
    if ext != ".swiftmodule":
        continue
    # Skip the standard library because it's tested elsewhere.
    if module_name == "Codira":
        continue
    # Skip the C++ standard library overlay because it's not yet shipped
    # in any released SDK.
    if module_name in ("Cxx", "CxxStdlib"):
        continue
    # TODO(TF-1229): Fix the "_Differentiation" module.
    if module_name == "_Differentiation":
        continue
    # TODO: fix the DifferentiationUnittest module.
    if module_name == "DifferentiationUnittest":
        continue
    # Runtime needs its own additional modules in the module path, and
    # also needs C++ interop enabled
    if module_name == "Runtime":
        extra_args = ["-I", os.path.join(source_dir, "stdlib",
                                         "public", "RuntimeModule", "modules"),
                      "-I", os.path.join(source_dir, "include"),
                      "--enable-experimental-cxx-interop"]
        # TODO: Fix SIL verification error (probably due to a deserialization bug
        # in sil-opt) rdar://143050566
        continue
    # _Concurrency needs its own additional modules in the module path
    if module_name == "_Concurrency":
        extra_args = ["-I", os.path.join(source_dir, "stdlib",
                                         "public", "Concurrency", "InternalShims")]

    print("# " + module_name)

    module_path = os.path.join(sdk_overlay_dir, module_file)
    if os.path.isdir(module_path):
        module_path = os.path.join(module_path, target_swiftmodule_name)

    # toolchain-bcanalyzer | not grep Unknown
    bcanalyzer_output = subprocess.check_output(["toolchain-bcanalyzer",
                                                 module_path]).decode("utf-8")
    if "Unknown" in bcanalyzer_output:
        print(bcanalyzer_output)
        sys.exit(1)

    # sil-opt
    # We are deliberately discarding the output here; we're just making sure
    # it can be generated.
    subprocess.check_output(sil_opt_invocation +
                            [module_path, "-module-name", module_name] +
                            extra_args)
