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
# Building and running benchmarks in Panda Assembly from the Panda source tree.

if(NOT DEFINED PANDA_ROOT)
    message(FATAL_ERROR "Not a Panda build")
endif()

define_property(SOURCE
                PROPERTY panda_benchmark_name
                BRIEF_DOCS "Short benchmark name"
                FULL_DOCS  "Short benchmark name")
define_property(SOURCE
                PROPERTY panda_benchmark_vmb_name
                BRIEF_DOCS "Short VMB name"
                FULL_DOCS  "Short VMB name")
define_property(SOURCE
                PROPERTY panda_benchmark_stack_limit
                BRIEF_DOCS "Stack limit for this benchmark"
                FULL_DOCS "Stack limit for this benchmark in bytes. Set 0 to use default value")
define_property(SOURCE
                PROPERTY panda_benchmark_pool_limit
                BRIEF_DOCS "Object pool limit for this benchmark"
                FULL_DOCS "Object pool limit for this benchmark in bytes. Set 0 to use default value")

set(PANDA_BENCHMARKS)
set(PANDA_BENCHMARKS_DIR "${CMAKE_CURRENT_LIST_DIR}")
function(panda_add_benchmark
         name vmb_name stack_limit pool_limit)
    # Allow to write something like this: 64 * 1024 * 1024
    math(EXPR stack_limit "${stack_limit}")
    math(EXPR pool_limit "${pool_limit}")

    set(file_name "${PANDA_BENCHMARKS_DIR}/${name}.pa")

    set_source_files_properties(${file_name} PROPERTIES
        panda_benchmark_name            "${name}"
        panda_benchmark_vmb_name        "${vmb_name}"
        panda_benchmark_stack_limit     "${stack_limit}"
        panda_benchmark_pool_limit      "${pool_limit}"
    )

    list(APPEND PANDA_BENCHMARKS ${file_name})
    set (PANDA_BENCHMARKS ${PANDA_BENCHMARKS} PARENT_SCOPE)
endfunction()

# Arguments:
# * 1. Panda benchmark name
# * 2. VMB benchmark name; if empty, not a part of VMB yet
# * 3. Stack limit
# * 4. Object pool size
# TODO: Make all benchmarks into VMB
#                   1.name                     2.vmb_name             3.stack_limit       4.pool_limit
panda_add_benchmark("3d-morph"                 "Morph3d"              0                   0)
panda_add_benchmark("access-binary-trees"      "AccessBinaryTrees"    0                   0)
panda_add_benchmark("access-fannkuch"          "AccessFannkuch"       0                   0)
panda_add_benchmark("access-nbody"             "AccessNBody"          0                   0)
panda_add_benchmark("access-nsieve"            "AccessNSieve"         0                   0)
panda_add_benchmark("bitops-3bit-bits-in-byte" "Bitops3BitBitsInByte" 0                   0)
panda_add_benchmark("bitops-bits-in-byte"      "BitopsBitsInByte"     0                   0)
panda_add_benchmark("bitops-bitwise-and"       "BitopsBitwiseAnd"     0                   0)
panda_add_benchmark("bitops-nsieve-bits"       "BitopsNSieveBits"     0                   0)
panda_add_benchmark("controlflow-recursive"    "ControlFlowRecursive" "384 * 1024 * 1024" 0)
panda_add_benchmark("math-cordic"              "MathCordic"           0                   0)
panda_add_benchmark("math-partial-sums"        "MathPartialSums"      0                   0)
panda_add_benchmark("math-spectral-norm"       "MathSpectralNorm"     0                   0)

add_custom_target(benchmarks-panda-assembly-interpreter
                  COMMENT "Running benchmarks in Panda Assembly in interpreter")
add_custom_target(benchmarks-panda-assembly-irtoc
                  COMMENT "Running benchmarks in Panda Assembly in Irtoc interpreter")
add_custom_target(benchmarks-panda-assembly-enforce-jit-compiler
                  COMMENT "Running benchmarks in Panda Assembly with enforced JIT")
add_custom_target(benchmarks-panda-assembly-aot
                  COMMENT "Running benchmarks in Panda Assembly with AOT")
add_custom_target(benchmarks-panda-assembly-default
                  COMMENT "Running benchmarks in Panda Assembly")

add_dependencies(benchmarks
    benchmarks-panda-assembly-interpreter
    benchmarks-panda-assembly-irtoc
    benchmarks-panda-assembly-enforce-jit-compiler
    benchmarks-panda-assembly-aot
    benchmarks-panda-assembly-default
)

add_custom_target(benchmarks-panda-assembly-aot-stats
                  COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/extras/mem_usage_analysis.py ${COMPILER_STATS_DIR} --mode=default --output=${COMPILER_STATS_DIR}/../report.html
                  COMMENT "Gathering compiler's statistics in benchmarks-AOT")

function(add_benchmark
         common_target_basename name file stack_limit pool_limit)

    if (stack_limit)
        set(PANDA_RUN_PREFIX prlimit --stack=${stack_limit} ${PANDA_RUN_PREFIX})
    endif()

    if (pool_limit)
        set(RUNTIME_OPTIONS "--heap-size-limit=${pool_limit}")
    endif()

    set(source_language "core")
    set(entry_point "_GLOBAL::main")
    set(skip_build FALSE)

    string(TOLOWER "${CMAKE_BUILD_TYPE}" cm_build_type)
    if ("${cm_build_type}" STREQUAL "")
        set(cm_build_type "debug")
    endif()
    if ("${cm_build_type}" STREQUAL "debug")
        set(benchmark_timeout "5s")
    endif()
    if ("${cm_build_type}" STREQUAL "fastverify")
        set(benchmark_timeout "5s")
    endif()

    set(subdir_prefix "benchmarks/${common_target_basename}")

    panda_add_asm_file(TARGET "${name}-interpreter"
        FILE "${file}"
        LANGUAGE_CONTEXT "${source_language}"
        SUBDIR "${subdir_prefix}"
        ENTRY "${entry_point}"
        TIMEOUT "${benchmark_timeout}"
        SKIP_BUILD ${skip_build}
        DEPENDS ${deps}
        AOT_MODE FALSE
        RUNTIME_OPTIONS ${RUNTIME_OPTIONS} "--compiler-enable-jit=false" "--limit-standard-alloc=true" "--interpreter-type=cpp")
    add_dependencies(${common_target_basename}-interpreter "${name}-interpreter")

    if (PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64)
        panda_add_asm_file(TARGET "${name}-irtoc"
            FILE "${file}"
            LANGUAGE_CONTEXT "${source_language}"
            SUBDIR "${subdir_prefix}"
            ENTRY "${entry_point}"
            TIMEOUT "${benchmark_timeout}"
            SKIP_BUILD ${skip_build}
            DEPENDS ${deps}
            AOT_MODE FALSE
            RUNTIME_OPTIONS ${RUNTIME_OPTIONS} "--gc-type=g1-gc" "--compiler-enable-jit=false" "--limit-standard-alloc=true" "--interpreter-type=irtoc")
        add_dependencies(${common_target_basename}-irtoc "${name}-irtoc")
        if (PANDA_LLVM_INTERPRETER)
            panda_add_asm_file(TARGET "${name}-llvmirtoc"
                FILE "${file}"
                LANGUAGE_CONTEXT "${source_language}"
                SUBDIR "${subdir_prefix}"
                ENTRY "${entry_point}"
                TIMEOUT "${benchmark_timeout}"
                SKIP_BUILD ${skip_build}
                DEPENDS ${deps}
                AOT_MODE FALSE
                RUNTIME_OPTIONS "--gc-type=g1-gc" "--compiler-enable-jit=false" "--limit-standard-alloc=true" "--interpreter-type=llvm")
            add_dependencies(${common_target_basename}-irtoc "${name}-llvmirtoc")
        endif()
    endif()

    set(CUR_COMPILER_OPTIONS "")
    if (PANDA_TARGET_ARM32)
        set(CUR_COMPILER_OPTIONS "--compiler-ignore-failures=true")
    endif()

    if(PANDA_COMPILER_ENABLE)
        list(APPEND deps arkcompiler)
        panda_add_asm_file(TARGET "${name}-enforce-jit-compiler"
            FILE "${file}"
            LANGUAGE_CONTEXT "${source_language}"
            SUBDIR "${subdir_prefix}"
            ENTRY "${entry_point}"
            TIMEOUT "${benchmark_timeout}"
            SKIP_BUILD ${skip_build}
            DEPENDS ${deps}
            AOT_MODE FALSE
            COMPILER_OPTIONS ${CUR_COMPILER_OPTIONS}
            RUNTIME_OPTIONS ${RUNTIME_OPTIONS} "--compiler-enable-jit=true" "--compiler-hotness-threshold=0" "--limit-standard-alloc=true")
        add_dependencies(${common_target_basename}-enforce-jit-compiler "${name}-enforce-jit-compiler")

        if (PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64)
            panda_add_asm_file(TARGET "${name}-aot"
                FILE "${file}"
                LANGUAGE_CONTEXT "${source_language}"
                SUBDIR "${subdir_prefix}"
                ENTRY "${entry_point}"
                TIMEOUT "${benchmark_timeout}"
                SKIP_BUILD ${skip_build}
                DEPENDS ${deps}
                AOT_MODE TRUE
                AOT_STATS TRUE
                AOT_COMPILER "aot"
                RUNTIME_OPTIONS ${RUNTIME_OPTIONS} "--compiler-enable-jit=true")
            add_dependencies(${common_target_basename}-aot "${name}-aot")
            add_dependencies(${common_target_basename}-aot-stats "${name}-aot-stats")

            if (PANDA_LLVM_AOT)
                panda_add_asm_file(TARGET "${name}-llvmaot"
                    FILE "${file}"
                    LANGUAGE_CONTEXT "${source_language}"
                    SUBDIR "${subdir_prefix}"
                    ENTRY "${entry_point}"
                    TIMEOUT "${benchmark_timeout}"
                    SKIP_BUILD ${skip_build}
                    DEPENDS ${deps}
                    AOT_MODE TRUE
                    AOT_COMPILER "llvm"
                    RUNTIME_OPTIONS ${RUNTIME_OPTIONS} "--compiler-enable-jit=true")
                add_dependencies(${common_target_basename}-aot "${name}-llvmaot")
            endif()
        endif()

        panda_add_asm_file(TARGET "${name}-default"
            FILE "${file}"
            LANGUAGE_CONTEXT "${source_language}"
            SUBDIR "${subdir_prefix}"
            ENTRY "${entry_point}"
            TIMEOUT "${benchmark_timeout}"
            SKIP_BUILD ${skip_build}
            DEPENDS ${deps}
            AOT_MODE FALSE
            COMPILER_OPTIONS ${CUR_COMPILER_OPTIONS}
            RUNTIME_OPTIONS ${RUNTIME_OPTIONS} "--compiler-enable-jit=true")
        add_dependencies(${common_target_basename}-default "${name}-default")
    endif()
endfunction()

foreach(benchmark ${PANDA_BENCHMARKS})
    get_source_file_property(name "${benchmark}" panda_benchmark_name)
    get_source_file_property(stack_limit "${benchmark}" panda_benchmark_stack_limit)
    get_source_file_property(pool_limit "${benchmark}" panda_benchmark_pool_limit)

    add_benchmark(benchmarks-panda-assembly "${name}" "${benchmark}"
        ${stack_limit}
        ${pool_limit}
    )
endforeach()

# Build-time fix-up for applying most recent changes in Panda Assembly
# benchmarks to the source tree of VM Benchmarks aka VMB. To be used in CI only.
# Currently VMB owns almost all benchmarks and infrastructure for running them,
# but Panda Assembly benchmarks are maintained within Panda source tree.
# This function generates two shell scripts which copy all necessary files
# between the source trees. Dedicated targets are created for
# invoking the scripts. Path to the root of VMB repository is expected to be
# passed via VMB_REPO_ROOT environment variable during build time.
# Targets:
# * update-vmb:     Update all benchmarks.
# * update-vmb-jit: Update all benchmarks which can be JIT-compiled.
#                   Benchmarks that cannot be JIT-compiled will be
#                   copied as VMBenchmarkName.pa.disabled.
function(panda_update_vmb)
    set(vmb_benchmark_path "\$VMB_REPO_ROOT/benchmarks/components/SimpleLanguage/SimpleLanguage")
    set(script_name_all "${CMAKE_CURRENT_BINARY_DIR}/update-vmb.sh")
    set(script_name_jit "${CMAKE_CURRENT_BINARY_DIR}/update-vmb-jit.sh")
    # NB! Please do not use ';' character in the body of the update script.
    set(script_all
        "#!/bin/sh"
        "set -e"
        "[ -d \"\$VMB_REPO_ROOT\" ] || exit 1"
    )
    set(script_jit
        "#!/bin/sh"
        "set -e"
        "[ -d \"\$VMB_REPO_ROOT\" ] || exit 1"
    )

    foreach(benchmark ${PANDA_BENCHMARKS})
        get_source_file_property(vmb_name "${benchmark}" panda_benchmark_vmb_name)
        get_source_file_property(enforce_jit_compiler "${benchmark}" panda_benchmark_run_enforce_jit_compiler)

        if(NOT "${vmb_name}" STREQUAL "")
            list(APPEND script_all
                "cp \"${benchmark}\" \"${vmb_benchmark_path}/${vmb_name}/pa/${vmb_name}.pa\"")

            if(enforce_jit_compiler)
                list(APPEND script_jit
                    "cp \"${benchmark}\" \"${vmb_benchmark_path}/${vmb_name}/pa/${vmb_name}.pa\"")
            else()
                list(APPEND script_jit
                    "cp \"${benchmark}\" \"${vmb_benchmark_path}/${vmb_name}/pa/${vmb_name}.pa.disabled\"")
                list(APPEND script_jit
                    "rm -f \"${vmb_benchmark_path}/${vmb_name}/pa/${vmb_name}.pa\"")
            endif()
        endif()
    endforeach()

    string(REPLACE ";" "\n" script_all "${script_all}")
    string(REPLACE ";" "\n" script_jit "${script_jit}")
    file(GENERATE OUTPUT ${script_name_all} CONTENT "${script_all}")
    file(GENERATE OUTPUT ${script_name_jit} CONTENT "${script_jit}")
    add_custom_target(update-vmb
        COMMENT "Update VMB, all benchmarks"
        COMMAND sh ${script_name_all}
    )
    add_custom_target(update-vmb-jit
        COMMENT "Update VMB, only JITtable benchmarks"
        COMMAND sh ${script_name_jit}
    )
endfunction()

panda_update_vmb()
