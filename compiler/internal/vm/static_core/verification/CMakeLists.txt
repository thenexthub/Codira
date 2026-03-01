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

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

include(${PANDA_ROOT}/verification/Verification.cmake)
project(pandaverification)

set(ABS_INT_EXTRA_HEADERS)
set(ABS_INT_TEMPLATES
    abs_int_inl_compat_checks
)
foreach(TEMPLATE_NAME ${ABS_INT_TEMPLATES})
    set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gen/include/${TEMPLATE_NAME}.h")
    panda_gen_file(
        DATA "${PANDA_ROOT}/verification/verification.yaml"
        API "${PANDA_ROOT}/verification/verification.rb"
        TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/gen/templates/${TEMPLATE_NAME}.h.erb"
        OUTPUTFILE ${OUTPUT}
    )
    list(APPEND ABS_INT_EXTRA_HEADERS ${OUTPUT})
endforeach()

add_custom_target(pandaverification_gen DEPENDS ${ABS_INT_EXTRA_HEADERS})
add_dependencies(panda_gen_files pandaverification_gen)

set(ISA_TEMPLATES
    cflow_iterate_inl_gen.h.erb
    abs_int_inl_gen.h.erb
    job_fill_gen.h.erb
    handle_gen.h.erb
)
set(GEN_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/include")
file(MAKE_DIRECTORY ${GEN_INCLUDE_DIR})
panda_isa_gen(
    TEMPLATES ${ISA_TEMPLATES}
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/gen/templates
    DESTINATION ${GEN_INCLUDE_DIR}
    EXTRA_DEPENDENCIES ${ABS_INT_EXTRA_HEADERS}
)

panda_gen_messages(
    YAML_FILE ${PANDA_ROOT}/verification/messages.yaml
    GENERATED_HEADER ${GEN_INCLUDE_DIR}/verifier_messages.h
)

panda_gen(
    TARGET_NAME messages_data_${PROJECT_NAME}
    DATA ${PANDA_ROOT}/verification/messages.yaml
    API ${PANDA_ROOT}/templates/messages.rb
    SOURCE ${PANDA_ROOT}/verification/gen/templates
    TEMPLATES verifier_messages_data_gen.cpp.erb
)

if ((PANDA_TARGET_X86 OR PANDA_TARGET_AMD64) AND PANDA_WITH_TESTS)
    panda_add_gtest(
        NO_CORES
        NAME arkverification_tests
        SOURCES
            ${VERIFIER_TESTS_SOURCES}
        LIBRARIES
            arkruntime
            arkbase
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )
    if(TARGET arkverification_tests)
        panda_target_include_directories(arkverification_tests PUBLIC ${PANDA_ROOT})
        panda_target_compile_options(arkverification_tests PUBLIC "-Wno-ignored-attributes")
        panda_add_sanitizers(TARGET arkverification_tests SANITIZERS ${PANDA_SANITIZERS_LIST})
    endif()

    panda_add_gtest(
        RAPIDCHECK_ON
        NO_CORES
        NAME pandaverification_rapidcheck_gtest_tests
        SOURCES
            ${VERIFIER_RAPIDCHECK_TESTS_SOURCES}
        LIBRARIES
            arkbase
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )
    if(TARGET pandaverification_rapidcheck_gtest_tests)
        panda_target_include_directories(pandaverification_rapidcheck_gtest_tests PUBLIC ${PANDA_ROOT})
        panda_target_include_directories(pandaverification_rapidcheck_gtest_tests
            SYSTEM PRIVATE "${PANDA_THIRD_PARTY_SOURCES_DIR}/rapidcheck/extras/gtest/include")
        add_verification_includes(TARGET pandaverification_rapidcheck_gtest_tests)
        panda_target_compile_options(pandaverification_rapidcheck_gtest_tests PUBLIC "-Wno-ignored-attributes")
        panda_add_sanitizers(TARGET pandaverification_rapidcheck_gtest_tests SANITIZERS ${PANDA_SANITIZERS_LIST})
    endif()
endif()
