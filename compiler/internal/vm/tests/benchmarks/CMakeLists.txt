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
                PROPERTY panda_benchmark_run_interpreter
                BRIEF_DOCS "Run this benchmark with interpreter?"
                FULL_DOCS  "Run this benchmark with interpreter?")
define_property(SOURCE
                PROPERTY panda_benchmark_run_enforce_jit_compiler
                BRIEF_DOCS "Run this benchmark with enforced JIT compiler?"
                FULL_DOCS  "Run this benchmark with enforced JIT compiler?")
define_property(SOURCE
                PROPERTY panda_benchmark_run_gc_type
                BRIEF_DOCS "Type of GC?"
                FULL_DOCS  "Type of GC?")
define_property(SOURCE
                PROPERTY panda_benchmark_run_aot
                BRIEF_DOCS "Run with enabled AOT?"
                FULL_DOCS  "Run with enabled AOT?")
define_property(SOURCE
                PROPERTY panda_benchmark_run_default
                BRIEF_DOCS "Run in default mode?"
                FULL_DOCS  "Run in default mode?")

set(PANDA_BENCHMARKS)
set(PANDA_BENCHMARKS_DIR "${CMAKE_CURRENT_LIST_DIR}")
function(panda_add_benchmark
         name vmb_name stack_limit interpreter enforce_jit_compiler gc_type aot default)
    # Allow to write something like this: 64 * 1024 * 1024
    math(EXPR stack_limit "${stack_limit}")

    set(file_name "${PANDA_BENCHMARKS_DIR}/${name}.pa")

    set_source_files_properties(${file_name} PROPERTIES
        panda_benchmark_name            "${name}"
        panda_benchmark_vmb_name        "${vmb_name}"
        panda_benchmark_stack_limit     "${stack_limit}"
        panda_benchmark_run_interpreter ${interpreter}
        panda_benchmark_run_enforce_jit_compiler ${enforce_jit_compiler}
        panda_benchmark_run_gc_type   ${gc_type}
        panda_benchmark_run_aot       ${aot}
        panda_benchmark_run_default   ${default})

    list(APPEND PANDA_BENCHMARKS ${file_name})
    set (PANDA_BENCHMARKS ${PANDA_BENCHMARKS} PARENT_SCOPE)
endfunction()

# Arguments:
# * 1. Panda benchmark name
# * 2. VMB benchmark name; if empty, not a part of VMB yet
# * 3. Stack limit
# * 4. Run with interpreter (without compiler at all)?
# * 5. Run with enforced JIT compiler?
# * 6. Gc type (default - for default gc)
# * 7. Run in AOT mode?
# * 8. Run in default mode?
# TODO: Make all benchmarks into VMB
#                    1.name                     2.vmb_name            3.stack_limit       4.interpreter
#                    |                          |                     |                   |     5.enforce_jit_compiler
#                    |                          |                     |                   |     |     6.gc_type
#                    |                          |                     |                   |     |     |        7.aot
#                    |                          |                     |                   |     |     |        |    8.default
panda_add_benchmark("3d-morph"                 "Morph3d"              0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("access-binary-trees"      "AccessBinaryTrees"    0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("access-fannkuch"          "AccessFannkuch"       0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("access-nbody"             "AccessNBody"          0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("access-nsieve"            "AccessNSieve"         0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("bitops-3bit-bits-in-byte" "Bitops3BitBitsInByte" 0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("bitops-bits-in-byte"      "BitopsBitsInByte"     0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("bitops-bitwise-and"       "BitopsBitwiseAnd"     0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("bitops-nsieve-bits"       "BitopsNSieveBits"     0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("controlflow-recursive"    "ControlFlowRecursive" "384 * 1024 * 1024" TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("math-cordic"              "MathCordic"           0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("math-partial-sums"        "MathPartialSums"      0                   TRUE  TRUE  default  TRUE TRUE)
panda_add_benchmark("math-spectral-norm"       "MathSpectralNorm"     0                   TRUE  TRUE  default  TRUE TRUE)

add_custom_target(benchmarks-panda-assembly-interpreter
                  COMMENT "Running benchmarks in Panda Assembly in interpreter")
add_custom_target(benchmarks-panda-assembly-enforce-jit-compiler
                  COMMENT "Running benchmarks in Panda Assembly with enforced JIT")
add_custom_target(benchmarks-panda-assembly-aot
                  COMMENT "Running benchmarks in Panda Assembly with AOT")
add_custom_target(benchmarks-panda-assembly-default
                  COMMENT "Running benchmarks in Panda Assembly")

add_dependencies(benchmarks
    benchmarks-panda-assembly-interpreter
    benchmarks-panda-assembly-enforce-jit-compiler
    benchmarks-panda-assembly-aot
    benchmarks-panda-assembly-default
)

add_custom_target(benchmarks-panda-assembly-aot-stats
                  COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/extras/mem_usage_analysis.py ${COMPILER_STATS_DIR} --mode=default --output=${COMPILER_STATS_DIR}/../report.html
                  COMMENT "Gathering compiler's statistics in benchmarks-AOT")

function(add_benchmark
         common_target_basename name file interpreter enforce_jit_compiler stack_limit gc_type aot default)

    if (stack_limit)
        set(PANDA_RUN_PREFIX prlimit --stack=${stack_limit} ${PANDA_RUN_PREFIX})
    endif()

    if (NOT "${gc_type}" STREQUAL "default")
        set(ENABLE_GC_OPTION "--gc-type=${gc_type}")
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

    if (interpreter)
        panda_add_asm_file(TARGET "${name}-interpreter"
            FILE "${file}"
            LANGUAGE_CONTEXT "${source_language}"
            SUBDIR "${subdir_prefix}"
            ENTRY "${entry_point}"
            TIMEOUT "${benchmark_timeout}"
            SKIP_BUILD ${skip_build}
            DEPENDS ${deps}
            AOT_MODE FALSE
            RUNTIME_OPTIONS ${ENABLE_GC_OPTION} "--compiler-enable-jit=false" "--limit-standard-alloc=true")
        add_dependencies(${common_target_basename}-interpreter "${name}-interpreter")
    endif()

    set(CUR_COMPILER_OPTIONS "")
    if (PANDA_TARGET_ARM32)
        set(CUR_COMPILER_OPTIONS "--compiler-ignore-failures=true")
    endif()

    if(PANDA_COMPILER_ENABLE)
        list(APPEND deps arkcompiler)
        if (enforce_jit_compiler)
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
                RUNTIME_OPTIONS ${ENABLE_GC_OPTION} "--compiler-enable-jit=true" "--compiler-hotness-threshold=0" "--limit-standard-alloc=true")
            add_dependencies(${common_target_basename}-enforce-jit-compiler "${name}-enforce-jit-compiler")
        endif()

        if (aot AND (PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64))
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
                AOT_GC_OPTION ${ENABLE_GC_OPTION}
                RUNTIME_OPTIONS ${ENABLE_GC_OPTION} "--compiler-enable-jit=true")
            add_dependencies(${common_target_basename}-aot "${name}-aot")
            add_dependencies(${common_target_basename}-aot-stats "${name}-aot-stats")
        endif()

        if (default)
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
                RUNTIME_OPTIONS ${ENABLE_GC_OPTION} "--compiler-enable-jit=true")
            add_dependencies(${common_target_basename}-default "${name}-default")
        endif()
    endif()
endfunction()

foreach(benchmark ${PANDA_BENCHMARKS})
    get_source_file_property(name "${benchmark}" panda_benchmark_name)
    get_source_file_property(interpreter "${benchmark}" panda_benchmark_run_interpreter)
    get_source_file_property(gc_type "${benchmark}" panda_benchmark_run_gc_type)
    get_source_file_property(enforce_jit_compiler "${benchmark}" panda_benchmark_run_enforce_jit_compiler)
    get_source_file_property(stack_limit "${benchmark}" panda_benchmark_stack_limit)
    get_source_file_property(aot "${benchmark}" panda_benchmark_run_aot)
    get_source_file_property(default "${benchmark}" panda_benchmark_run_default)

    add_benchmark(benchmarks-panda-assembly "${name}" "${benchmark}"
        ${interpreter}
        ${enforce_jit_compiler}
        ${stack_limit}
        ${gc_type}
        ${aot}
        ${default})
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
