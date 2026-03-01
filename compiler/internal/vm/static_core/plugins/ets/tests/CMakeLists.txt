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

if (NOT TARGET es2panda)
    return()
    message(WARNING "es2panda not found for ets tests")
endif()

add_custom_target(ets_gtests)
add_custom_target(ets_panda_gtests)
add_dependencies(ets_tests ets_gtests ets_panda_gtests)

include(cmake/native_gtest.cmake)
include(cmake/managed_helpers.cmake)

set(BOOT_PANDA_FILES ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc)

add_panda_assembly(TARGET mock_stdlib SOURCE integrational/mock_stdlib.pa)

function(panda_ets_add_gtest)
    if(NOT "TEST_GROUP" IN_LIST ARGV)
        list(APPEND ARGV "TEST_GROUP" "ets_panda_gtests")
    endif()
    panda_add_gtest(
        STASH_LIST ets_stash_list
        ${ARGV}
    )
endfunction()

panda_add_asm_file(
    FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/integrational/empty_program.pa
    TARGET ets_tests_empty_program
    DEPENDS mock_stdlib
    LANGUAGE_CONTEXT ets
    ADDITIONAL_STDLIBS $<TARGET_PROPERTY:mock_stdlib,FILE>
)

add_dependencies(tests ets_tests)

add_custom_target(ets_deterministic_tests)

function(run_deterministic_tests ETS_SRC TARGET)
    set(oneValueArgs ITER_NUM)

    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})

    if (NOT DEFINED ARG_ITER_NUM)
        SET(ARG_ITER_NUM 10)
    endif()

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)

    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/${SRC_FNAME}")
    file(MAKE_DIRECTORY "${TEST_DIR}")
    set(DEPENDS_ON "")

    set(FIRST_ABC_APP_TARGET ${TARGET}-${SRC_FNAME}-0-ets-es2panda)

    foreach(i RANGE ${ARG_ITER_NUM})
        set(APP_FILE_OUTPUT "${TEST_DIR}/${i}.abc")
        set(ABC_APP_TARGET ${TARGET}-${SRC_FNAME}-${i}-ets-es2panda)
        list(APPEND all_bin_abc ${APP_FILE_OUTPUT})
        list(APPEND DEPENDS_ON "${ABC_APP_TARGET}")
        compile_ets_code(${ETS_SRC} ${APP_FILE_OUTPUT} ${ABC_APP_TARGET})

        set(target_name ${TARGET}_${SRC_FNAME}_${i})
        list(GET all_bin_abc 0 first)
        add_custom_target(
            ${target_name}
            COMMAND bash -c "diff ${APP_FILE_OUTPUT} ${first}"
            WORKING_DIRECTORY ${TEST_DIR}
            DEPENDS ${ABC_APP_TARGET} ${FIRST_ABC_APP_TARGET}
        )
        list(APPEND dependencies ${target_name})
    endforeach()
    add_custom_target(${TARGET}
        COMMENT "Running ${ETS_SRC} determenistic compilation test"
        DEPENDS ${dependencies}
    )
    add_dependencies(ets_deterministic_tests ${TARGET})
endfunction()

add_dependencies(ets_tests ets_deterministic_tests)
add_custom_target(ets_code_verifier_tests)

function(run_ets_code_verifier ETS_SRC WORK_DIR TARGET)
    if (NOT TARGET verifier)
        return()
    endif()

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)
    set(OUTPUT_ABC ${WORK_DIR}/${SRC_FNAME}.abc)
    set(ABC_TARGET ${TARGET}-ets-es2panda)
    compile_ets_code(${ETS_SRC} ${OUTPUT_ABC} ${ABC_TARGET} OPT_LEVEL ${ARG_OPT_LEVEL})

    set(VERIFIER_ARGUMENTS
        --boot-panda-files=${BOOT_PANDA_FILES}
        --load-runtimes=ets
        ${OUTPUT_ABC}
    )

    add_custom_target(${TARGET}
        COMMENT "Running verifier for ets file: ${OUTPUT_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:verifier> ${VERIFIER_ARGUMENTS}
        DEPENDS verifier etsstdlib ${ABC_TARGET}
    )

    add_dependencies(ets_code_verifier_tests ${TARGET})
endfunction()

add_dependencies(ets_tests ets_code_verifier_tests)

function(run_ets_code_impl TARGET WORK_DIR ETS_SOURCES)
    if (NOT TARGET es2panda)
        return()
    endif()

    set(oneValueArgs ENTRYPOINT ETSPATH OPT_LEVEL)
    set(multiValueArgs ARKTS_CONFIGS COMPILER_EXTRA_OPTIONS RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(ABC_TARGET ${TARGET}-abc)
    compile_ets_sources(${WORK_DIR} ${ABC_TARGET} ABC_FILES "${ETS_SOURCES}"
                        ARKTS_CONFIGS ${ARG_ARKTS_CONFIGS}
                        ETSPATH ${ARG_ETSPATH}
                        OPT_LEVEL ${ARG_OPT_LEVEL}
                        COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )

    list(GET ABC_FILES 0 ENTRY_ABC)
    string(REPLACE ";" ":" ABC_FILES "${ABC_FILES}")

    if(NOT DEFINED ARG_ENTRYPOINT)
        list(GET ETS_SOURCES 0 ENTRY_SOURCE)
        get_filename_component(ARG_ENTRYPOINT ${ENTRY_SOURCE} NAME_WE)
    endif()

    set(VERIFICATION_MODE "--verification-mode=on-the-fly")
    # issues:22179: remove this loop after fix
    foreach(option IN LISTS ARG_RUNTIME_EXTRA_OPTIONS)
        if(option STREQUAL "--verification-mode=disabled")
            set(VERIFICATION_MODE "")
        endif()
    endforeach()
    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${BOOT_PANDA_FILES}
        --panda-files=${ABC_FILES}
        --load-runtimes=ets
        --verification-mode=ahead-of-time
        ${VERIFICATION_MODE}
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${ENTRY_ABC}
        "${ARG_ENTRYPOINT}.ETSGLOBAL::main"
    )

    string(FIND "${RUNTIME_ARGUMENTS}" "--gc-type" GC_TYPE_SET)
    if(NOT PANDA_LLVM_INTERPRETER AND ${GC_TYPE_SET} EQUAL -1)
        add_custom_target(${TARGET}
            COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --gc-type=g1-gc
            COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --gc-type=stw
        )
    else()
        add_custom_target(${TARGET}
            COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
        )
    endif()
    add_dependencies(${TARGET} ark etsstdlib ${ABC_TARGET})
endfunction()

add_custom_target(ets_run_int_ets_code_tests)

# NOTE(vpukhov): use a common format
function(run_int_ets_code TARGET WORK_DIR)
    set(oneValueArgs ENTRYPOINT ETSPATH OPT_LEVEL)
    set(multiValueArgs SOURCES ARKTS_CONFIGS COMPILER_EXTRA_OPTIONS RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_target(${TARGET})
    add_dependencies(ets_run_int_ets_code_tests ${TARGET})

    set(arkopts_cpp
        --compiler-enable-jit=false
        --interpreter-type=cpp
        --profilesaver-enabled=true
        ${ARG_RUNTIME_EXTRA_OPTIONS}
    )
    run_ets_code_impl(${TARGET}-cpp ${WORK_DIR} "${ARG_SOURCES}"
                      ARKTS_CONFIGS ${ARG_ARKTS_CONFIGS}
                      ETSPATH ${ARG_ETSPATH}
                      ENTRYPOINT ${ARG_ENTRYPOINT}
                      OPT_LEVEL ${ARG_OPT_LEVEL}
                      RUNTIME_EXTRA_OPTIONS "${arkopts_cpp}"
                      COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )
    add_dependencies(${TARGET} ${TARGET}-cpp)

    set(arkopts_irtoc
        --compiler-enable-jit=false
        --interpreter-type=irtoc
        --profilesaver-enabled=true
        ${ARG_RUNTIME_EXTRA_OPTIONS}
    )
    run_ets_code_impl(${TARGET}-irtoc ${WORK_DIR} "${ARG_SOURCES}"
                      ARKTS_CONFIGS ${ARG_ARKTS_CONFIGS}
                      ETSPATH ${ARG_ETSPATH}
                      ENTRYPOINT ${ARG_ENTRYPOINT}
                      OPT_LEVEL ${ARG_OPT_LEVEL}
                      RUNTIME_EXTRA_OPTIONS "${arkopts_irtoc}"
                      COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )
    add_dependencies(${TARGET} ${TARGET}-irtoc)

    if (PANDA_LLVM_INTERPRETER)
        set(arkopts_llvm
            --compiler-enable-jit=false
            --interpreter-type=llvm
            --profilesaver-enabled=true
            ${ARG_RUNTIME_EXTRA_OPTIONS}
        )
        run_ets_code_impl(${TARGET}-llvmirtoc ${WORK_DIR} "${ARG_SOURCES}"
                          ARKTS_CONFIGS ${ARG_ARKTS_CONFIGS}
                          ETSPATH ${ARG_ETSPATH}
                          ENTRYPOINT ${ARG_ENTRYPOINT}
                          OPT_LEVEL ${ARG_OPT_LEVEL}
                          RUNTIME_EXTRA_OPTIONS "${arkopts_llvm}"
                          COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
        add_dependencies(${TARGET} ${TARGET}-llvmirtoc)
    endif()
endfunction()

add_dependencies(ets_tests ets_run_int_ets_code_tests)
add_custom_target(ets_run_jit_ets_code_tests)

function(run_jit_ets_code ETS_SRC WORK_DIR TARGET)
    set(oneValueArgs ENTRYPOINT OPT_LEVEL)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS COMPILER_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(arkopts
        --compiler-enable-jit=true
        --no-async-jit=true
        --compiler-hotness-threshold=0
        --compiler-ignore-failures=false
        ${ARG_RUNTIME_EXTRA_OPTIONS}
    )
    run_ets_code_impl(${TARGET} ${WORK_DIR} ${ETS_SRC}
                      ENTRYPOINT ${ARG_ENTRYPOINT}
                      OPT_LEVEL ${ARG_OPT_LEVEL}
                      RUNTIME_EXTRA_OPTIONS "${arkopts}"
                      COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )
endfunction()

add_dependencies(ets_tests ets_run_jit_ets_code_tests)
add_custom_target(ets_run_jit_osr_ets_code_tests)

function(run_jit_osr_ets_code ETS_SRC WORK_DIR TARGET)
    set(oneValueArgs ENTRYPOINT OPT_LEVEL)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS COMPILER_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(arkopts
        --compiler-enable-jit=true
        --compiler-hotness-threshold=2
        --compiler-enable-osr=true
        --compiler-ignore-failures=true
        ${ARG_RUNTIME_EXTRA_OPTIONS}
    )
    run_ets_code_impl(${TARGET} ${WORK_DIR} ${ETS_SRC}
                      ENTRYPOINT ${ARG_ENTRYPOINT}
                      OPT_LEVEL ${ARG_OPT_LEVEL}
                      RUNTIME_EXTRA_OPTIONS "${arkopts}"
                      COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )
endfunction()

add_dependencies(ets_tests ets_run_jit_osr_ets_code_tests)

function(compile_aot_ets_code PAOC_MODE ETS_ABC OUTPUT_AN TARGET ABC_TARGET)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})

    set(AOT_RUNTIME_ARGUMENTS
        --boot-panda-files=${BOOT_PANDA_FILES}
        --load-runtimes=ets
        --compiler-ignore-failures=false
        --paoc-panda-files=${ETS_ABC}
        --paoc-output=${OUTPUT_AN}
        --paoc-mode=${PAOC_MODE}
        ${ARG_RUNTIME_EXTRA_OPTIONS}
    )

    add_custom_target(${TARGET}
        COMMENT "Compile ark AOT(${PAOC_MODE}) for ets file: ${ETS_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark_aot> ${AOT_RUNTIME_ARGUMENTS}
        DEPENDS ark_aot etsstdlib ${ABC_TARGET}
    )
endfunction()

add_custom_target(ets_run_aot_ets_code_tests)

function(run_aot_ets_code PAOC_MODE ETS_SRC WORK_DIR TARGET)
    if (CMAKE_CROSSCOMPILING)
        return()
    endif()

    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS COMPILER_EXTRA_OPTIONS )
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)
    set(OUTPUT_ABC ${WORK_DIR}/${SRC_FNAME}-${PAOC_MODE}.abc)
    set(ABC_TARGET ${TARGET}-ets-es2panda)
    compile_ets_code(${ETS_SRC} ${OUTPUT_ABC} ${ABC_TARGET}
                     OPT_LEVEL ${ARG_OPT_LEVEL}
                     COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )

    set(OUTPUT_AN ${WORK_DIR}/${SRC_FNAME}-${PAOC_MODE}.an)
    set(TARGET_AOT ${TARGET}-${PAOC_MODE}-files)
    compile_aot_ets_code(${PAOC_MODE} ${OUTPUT_ABC} ${OUTPUT_AN} ${TARGET_AOT} ${ABC_TARGET} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS})

    if(NOT ARG_ENTRYPOINT)
        get_filename_component(ARG_ENTRYPOINT ${ETS_SRC} NAME_WE)
    endif()

    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${BOOT_PANDA_FILES}
        --load-runtimes=ets
        --aot-file=${OUTPUT_AN}
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${OUTPUT_ABC}
        "${ARG_ENTRYPOINT}.ETSGLOBAL::main"
    )

    add_custom_target(${TARGET}
        COMMENT "Running ark AOT(${PAOC_MODE}) for ets file: ${OUTPUT_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
        DEPENDS ark etsstdlib ${TARGET_AOT}
    )

    add_dependencies(ets_run_aot_ets_code_tests ${TARGET})
endfunction()

add_dependencies(ets_tests ets_run_aot_ets_code_tests)

function(run_deterministic_test ETS_SRC WORK_DIR TARGET)
    # deterministic binary
    run_deterministic_tests(${ETS_SRC} ${TARGET}-determ)
    if(TARGET ${TARGET}-determ)
        add_dependencies(${TARGET} ${TARGET}-determ)
    endif()
endfunction()

function(run_int_jit_aot_ets_code ETS_SRC WORK_DIR TARGET)
    set(oneValueArgs OPT_LEVEL)
    set(noValues SKIP_ARM32_COMPILER)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS COMPILER_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "${noValues}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_target(${TARGET})

    # int
    run_int_ets_code(${TARGET}-ets-int ${WORK_DIR}
                     SOURCES "${ETS_SRC}"
                     OPT_LEVEL ${ARG_OPT_LEVEL}
                     RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS}
                     COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )
    if(TARGET ${TARGET}-ets-int)
        add_dependencies(${TARGET} ${TARGET}-ets-int)
    endif()
    # Compiler doesn't support some instruction in arm32 mode(launch)
    if (ARG_SKIP_ARM32_COMPILER AND PANDA_TARGET_ARM32)
        return()
    endif()
    # jit
    run_jit_ets_code(${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-jit
                     OPT_LEVEL ${ARG_OPT_LEVEL}
                     RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS}
                     COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )
    if(TARGET ${TARGET}-ets-jit)
        add_dependencies(${TARGET} ${TARGET}-ets-jit)
    endif()
    # aot
    run_aot_ets_code(aot ${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-aot
                     OPT_LEVEL ${ARG_OPT_LEVEL}
                     RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS}
                     COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
    )
    if(TARGET ${TARGET}-ets-aot)
        add_dependencies(${TARGET} ${TARGET}-ets-aot)
    endif()
    # llvm-aot
    if (PANDA_LLVM_AOT)
        run_aot_ets_code(llvm ${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-llvmaot
                         OPT_LEVEL ${ARG_OPT_LEVEL}
                         RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS}
                         COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
        if(TARGET ${TARGET}-ets-llvmaot)
            add_dependencies(${TARGET} ${TARGET}-ets-llvmaot)
        endif()
    endif()
    # jit-osr
    if (PANDA_TARGET_ARM64)
        run_jit_osr_ets_code(${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-jit-osr
                             OPT_LEVEL ${ARG_OPT_LEVEL}
                             RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS}
                             COMPILER_EXTRA_OPTIONS ${ARG_COMPILER_EXTRA_OPTIONS}
        )
        if(TARGET ${TARGET}-ets-jit-osr)
            add_dependencies(${TARGET} ${TARGET}-ets-jit-osr)
        endif()
    endif()
endfunction()

function(run_int_jit_aot_ets_code_foreach_gc PARENT_TARGET ETS_SRC WORK_DIR TARGET)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})

    set(gc_types g1-gc stw)

    foreach(gc_type ${gc_types})
        set(extra_options RUNTIME_EXTRA_OPTIONS "--gc-type=${gc_type};${ARG_RUNTIME_EXTRA_OPTIONS}")
        run_int_jit_aot_ets_code(${ETS_SRC} ${WORK_DIR}-${gc_type} ${TARGET}-${gc_type} ${extra_options})
        add_dependencies(${PARENT_TARGET} ${TARGET}-${gc_type})
    endforeach()
endfunction()

set (COMPILER_OPTIONS_OSR "--compiler-ignore-failures=true" ${COMPILER_OPTIONS})

function(compile_stdlib TARGET_ARCH)
    add_custom_target(ets-compile-stdlib-aot-${TARGET_ARCH}
        COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with default options"
        COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=aot -compiler-options="${COMPILER_OPTIONS}" -paoc-output=etsstdlib_aot_${TARGET_ARCH}.an
        DEPENDS ark_aot etsstdlib
    )
    add_custom_target(ets-compile-stdlib-jit-${TARGET_ARCH}
        COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with jit mode"
        COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=jit -compiler-options="${COMPILER_OPTIONS}" -paoc-output=etsstdlib_jit_${TARGET_ARCH}.an
        DEPENDS ark_aot etsstdlib
    )
    add_custom_target(ets-compile-stdlib-aot-${TARGET_ARCH}-no-inline
        COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} without inlining"
        COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=aot -compiler-options="${COMPILER_OPTIONS_NO_INLINE}" -paoc-output=etsstdlib_aot_${TARGET_ARCH}_no_inline.an
        DEPENDS ark_aot etsstdlib
    )
    add_custom_target(ets-compile-stdlib-aot-${TARGET_ARCH}-stw
        COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with gc stw"
        COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=aot -compiler-options="${COMPILER_OPTIONS_GC_STW}" -paoc-output=etsstdlib_aot_${TARGET_ARCH}_stw.an
        DEPENDS ark_aot etsstdlib
    )
    add_dependencies(ets-compile-stdlib ets-compile-stdlib-aot-${TARGET_ARCH} ets-compile-stdlib-jit-${TARGET_ARCH} ets-compile-stdlib-aot-${TARGET_ARCH}-no-inline ets-compile-stdlib-aot-${TARGET_ARCH}-stw)
    if(PANDA_LLVM_AOT AND NOT PANDA_THREAD_SAFETY AND NOT PANDA_ENABLE_THREAD_SANITIZER AND NOT PANDA_ENABLE_ADDRESS_SANITIZER)
        add_custom_target(ets-compile-stdlib-llvm-${TARGET_ARCH}
            COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with llvm"
            COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=llvm -compiler-options="${COMPILER_OPTIONS_LLVM}" -paoc-output=etsstdlib_llvmaot_${TARGET_ARCH}.an
            DEPENDS ark_aot etsstdlib
        )
        add_custom_target(ets-compile-stdlib-llvm-pre-opt-2-${TARGET_ARCH}
            COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with llvm pre-opt"
            COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=llvm -compiler-options="${COMPILER_OPTIONS_LLVM_PRE_OPT}" -paoc-output=etsstdlib_llvmaot_pre_opt_2_${TARGET_ARCH}.an
            DEPENDS ark_aot etsstdlib
        )
        add_dependencies(
            ets-compile-stdlib
            ets-compile-stdlib-llvm-${TARGET_ARCH}
            ets-compile-stdlib-llvm-pre-opt-2-${TARGET_ARCH}
        )
    endif()
    if (TARGET_ARCH STREQUAL "arm64")
        add_custom_target(ets-compile-stdlib-osr-${TARGET_ARCH}
            COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with osr mode"
            COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=osr -compiler-options="${COMPILER_OPTIONS_OSR}" -paoc-output=etsstdlib_osr_${TARGET_ARCH}.an
            DEPENDS ark_aot etsstdlib
        )
        add_dependencies(ets-compile-stdlib ets-compile-stdlib-osr-${TARGET_ARCH})
    endif()
endfunction()

run_deterministic_tests(GEN_STD_LIB
                        ets-stdlib-deterministic
                        ITER_NUM 3)
#add_dependencies(ets_tests ets-stdlib-deterministic)

if(NOT CMAKE_CROSSCOMPILING)
    if (NOT (PANDA_ENABLE_ADDRESS_SANITIZER OR PANDA_ENABLE_THREAD_SANITIZER) OR
        NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR
        PANDA_CI_TESTING_MODE STREQUAL "Nightly")
            panda_set_flag(PANDA_TEST_COMPILE_STDLIB)
    endif()
endif()


if(PANDA_TEST_COMPILE_STDLIB)
    set (ARK_AOT_RUNNER ${CMAKE_SOURCE_DIR}/plugins/ets/compiler/tools/paoc_compile_stdlib.sh)
    if (NOT PANDA_CI_TESTING_MODE STREQUAL "Nightly")
       set (COMPILER_OPTIONS "--compiler-check-final=true")
       if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
          set (COMPILER_OPTIONS ${COMPILER_OPTIONS} "--compiler-inst-graph-coloring-limit=2000")
       endif()
    endif()
    set (COMPILER_OPTIONS_NO_INLINE "--compiler-inlining=false" ${COMPILER_OPTIONS})
    set (COMPILER_OPTIONS_GC_STW "--gc-type=stw" ${COMPILER_OPTIONS})
    if (PANDA_LLVM_AOT)
        set (COMPILER_LLVMAOT_FIXUP "--llvm-gc=true") # Investigate in #26733
        set (COMPILER_OPTIONS_LLVM ${COMPILER_LLVMAOT_FIXUP} ${COMPILER_OPTIONS})
        set (COMPILER_OPTIONS_LLVM_PRE_OPT "--llvm-pre-opt=2" ${COMPILER_LLVMAOT_FIXUP} ${COMPILER_OPTIONS})
        # NOTE(zdenis): False positives come from libLLVM.so
        if (NOT PANDA_ENABLE_THREAD_SANITIZER)
            set (COMPILER_OPTIONS_LLVM "--llvmaot-threads=4" "--llvmaot-methods-per-module=512" ${COMPILER_OPTIONS_LLVM})
            set (COMPILER_OPTIONS_LLVM_PRE_OPT "--llvmaot-threads=4" "--llvmaot-methods-per-module=512" ${COMPILER_OPTIONS_LLVM_PRE_OPT})
        endif()
    endif ()
    add_custom_target(ets-compile-stdlib)
    if (PANDA_COMPILER_TARGET_X86_64)
        compile_stdlib(x86_64)
    endif()
    if (PANDA_COMPILER_TARGET_AARCH64)
        compile_stdlib(arm64)
    endif()
    add_dependencies(ets_tests ets-compile-stdlib)
endif()


if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(abc2program)
    add_subdirectory(static_linker)
endif()

add_subdirectory(common)
add_subdirectory(equals)
add_subdirectory(lookup_by_name)
add_subdirectory(ets_test_suite)
add_subdirectory(runtime)
add_subdirectory(ani)
add_subdirectory(any_never)
add_subdirectory(compiler)
add_subdirectory(libani_helpers)
add_subdirectory(verifier-tests-ets)
if(NOT (PANDA_ENABLE_ADDRESS_SANITIZER OR PANDA_ENABLE_THREAD_SANITIZER))
    add_subdirectory(micro-benchmarks)
endif()

function(panda_ets_napi_add_gtest)
    if (CMAKE_CROSSCOMPILING)
        return()
    endif()
    set(prefix ARG)
    set(noValues MULTI_CORES)
    set(singleValues NAME CPP_SOURCE INCLUDE_DIRS TSAN_EXTRA_OPTIONS)
    set(multiValues ETS_SOURCE)
    cmake_parse_arguments(${prefix}
                        "${noValues}"
                        "${singleValues}"
                        "${multiValues}"
                        ${ARGN})

    set(PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")
    set(ARG_NAME ets_test_${ARG_NAME})

    add_custom_target(ets_gtests_sources_dependencies_${ARG_NAME})
    panda_ets_add_gtest(
        NO_CORES
        NAME ${ARG_NAME}
        SOURCES ${ARG_CPP_SOURCE}
        INCLUDE_DIRS ${ARG_INCLUDE_DIRS}
        LIBRARIES arkruntime arkassembler
        SANITIZERS ${PANDA_SANITIZERS_LIST}
        OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        DEPS_TARGETS ets_gtests_sources_dependencies_${ARG_NAME}
        PANDA_STD_LIB ${PANDA_STD_LIB}
    )

    add_dependencies(ets_gtests ${ARG_NAME}_gtests)

    set(es2panda_target_test es2panda)
    set(es2panda_bin  $<TARGET_FILE:${es2panda_target_test}>)
    set(ETS_COMPILE_ARGUMENTS --gen-stdlib=false --extension=ets --opt-level=0)

    foreach(c ${ARG_ETS_SOURCE})
        STRING(REPLACE ".ets" "" CLEAR_NAME ${c})
        if(NOT TARGET es2panda_${CLEAR_NAME})
            add_custom_target(es2panda_${CLEAR_NAME}
                COMMAND ${es2panda_bin} ${ETS_COMPILE_ARGUMENTS} --output ${CMAKE_CURRENT_BINARY_DIR}/${CLEAR_NAME}.abc ${CMAKE_CURRENT_SOURCE_DIR}/${c}
                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                DEPENDS es2panda
            )
        endif()
        add_dependencies(ets_gtests_sources_dependencies_${ARG_NAME} es2panda_${CLEAR_NAME})
    endforeach()

    add_dependencies(${ARG_NAME} etsstdlib)
    add_dependencies(ets_common_gtests ${ARG_NAME})
endfunction()

add_custom_target(ets_common_gtests)
add_dependencies(ets_gtests ets_common_gtests)
add_subdirectory(mock)
add_subdirectory(native)

if(PANDA_COMPILER_ENABLE)
    add_subdirectory(checked)
    add_dependencies(ets_tests ets_checked_tests)
endif()


if (PANDA_ETS_INTEROP_JS)
    add_subdirectory(interop_js)
endif()

add_subdirectory(debugger)

# This section should be located at the end of this file to include all targets created here!
# Targets to divide ets_tests subtargets by execution time to ensure that
# each group is not executed more that 1 hour.
# All this ets_timeXXX_tests targets should contain all ets_tests subtargets.
# It's not very important what subtarget where is included
add_custom_target(ets_time01_tests)
add_custom_target(ets_time02_tests)
add_custom_target(ets_time03_tests)
add_custom_target(ets_time04_tests)
add_custom_target(ets_time05_tests)
add_custom_target(ets_time06_tests)
add_custom_target(ets_time07_tests)
add_custom_target(ets_time08_tests)
add_custom_target(ets_time09_tests)

add_dependencies(ets_tests
    ets_time01_tests
    ets_time02_tests
    ets_time03_tests
    ets_time04_tests
    ets_time05_tests
    ets_time06_tests
    ets_time07_tests
    ets_time08_tests
    ets_time09_tests)

function(add_target_to_group group test_target)
    if (TARGET ${test_target})
        add_dependencies(${group} ${test_target})
    endif()
endfunction()

add_target_to_group(ets_time01_tests ets_deterministic_tests)
add_target_to_group(ets_time01_tests ets_run_jit_osr_ets_code_tests)
add_target_to_group(ets_time01_tests ets_tests_lookup_by_name)
if(NOT PANDA_ENABLE_THREAD_SANITIZER OR PANDA_CI_TESTING_MODE STREQUAL "Nightly")
    add_target_to_group(ets_time01_tests es2panda_tests)
endif()
if(PANDA_COMPILER_ENABLE)
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR
       NOT PANDA_ENABLE_THREAD_SANITIZER OR
       PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        add_target_to_group(ets_time01_tests ets_checked_tests)
    endif()
endif()
add_target_to_group(ets_time01_tests ets_gtests)

add_target_to_group(ets_time02_tests etsnative_symbols_test)
add_target_to_group(ets_time02_tests ets_code_verifier_tests)
add_target_to_group(ets_time02_tests ets_run_jit_ets_code_tests)
add_target_to_group(ets_time02_tests ets_tests_equals)
add_target_to_group(ets_time02_tests ets_modules_tests)
add_target_to_group(ets_time02_tests ets_app_linker_context_tests)
add_target_to_group(ets_time02_tests ets_veriifier_tests)

if (PANDA_USE_CUSTOM_SIGNAL_STACK)
    add_target_to_group(ets_time01_tests sampler_asm_test_suite)
    add_target_to_group(ets_time02_tests sampler_test_suite)
endif()

add_target_to_group(ets_time03_tests ets_run_aot_ets_code_tests)
if(PANDA_TEST_COMPILE_STDLIB)
    add_target_to_group(ets_time03_tests ets-compile-stdlib)
endif()

# NOTE(vshirunova): revise group division: now time4 and time5 are the same
add_target_to_group(ets_time04_tests ets_run_int_ets_code_tests)

add_target_to_group(ets_time05_tests ets_run_int_ets_code_tests)
# ---- end of groups by time

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR
   NOT (PANDA_ENABLE_THREAD_SANITIZER OR PANDA_ENABLE_ADDRESS_SANITIZER) OR
   PANDA_CI_TESTING_MODE STREQUAL "Nightly")
    add_target_to_group(ets_time06_tests ani_tests)
endif()
add_target_to_group(ets_time06_tests ets_panda_gtests)
add_target_to_group(ets_time07_tests sampler_ani_test_suite)
add_target_to_group(ets_time08_tests es2panda_gtests)
add_target_to_group(ets_time09_tests compiler_tests)
