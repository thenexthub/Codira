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

if (NOT PANDA_TARGET_MOBILE_WITH_MANAGED_LIBS)

set(TEST_SOURCE_SUBDIR "sources")
set(TEST_BINARY_SUBDIR "bin")

add_custom_target(panda_cli_create_binary_dir_for_testing
    COMMENT "Creating the binary directory ${CMAKE_CURRENT_BINARY_DIR}/${TEST_BINARY_SUBDIR} for panda cli testing"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/bin)

add_custom_target(panda_cli_options_test_suite COMMENT "Running panda options cli tests")

set (panda_cli_options_suite
    "boot"
    "pfile"
    "app"
)

foreach(t ${panda_cli_options_suite})
    add_panda_assembly(TARGET panda_cli_options_test_suite_${t}
        SOURCE "${t}.pa"
        INDIR "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE_SUBDIR}/panda-options"
        OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_BINARY_SUBDIR}"
        TARGETNAME "${t}")
    add_dependencies(panda_cli_options_test_suite_${t} panda_cli_create_binary_dir_for_testing)
    add_dependencies(panda_cli_options_test_suite panda_cli_options_test_suite_${t})
endforeach()

add_dependencies(panda_cli_options_test_suite ark_asm)

add_custom_target(panda_cli_test_suites
    COMMENT "Build Pandasm-based test for the panda cli"
    DEPENDS
        panda_cli_options_test_suite
    )

set (PANDA_CLI_TESTS_SOURCES
    run/panda-options/panda_options.cpp
)

set(PANDA_CLI_TESTS_LIBRARIES arkassembler arkruntime)

panda_add_gtest(
    CONTAINS_MAIN
    NAME panda_cli_run_tests
    SOURCES
        ${PANDA_CLI_TESTS_SOURCES}
    LIBRARIES
        ${PANDA_CLI_TESTS_LIBRARIES}
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    INCLUDE_DIRS
        "run"
    DEPS_TARGETS
        arkstdlib
)

add_dependencies(panda_cli_run_tests ark panda_cli_test_suites)
    
endif()
