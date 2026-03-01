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

project(irtoc)

if (TARGET arkcompiler)
    message(FATAL_ERROR "IRtoC core and front-end must be configured before defining target arkcompiler")
endif()

# These variables will be used in the IRtoC frontend and backend.
# Since it will be invoked from arbitrary folder,
# we need to use these variables to determine irtoc folders.

#
# IRtoC sources:
#

set(IRTOC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IRTOC_SOURCE_DIR ${IRTOC_SOURCE_DIR} PARENT_SCOPE)

#
# IRtoC build directory:
#

set(IRTOC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(IRTOC_BUILD_DIR ${IRTOC_BUILD_DIR} PARENT_SCOPE)

#
# IRtoC target:
#

if (NOT IRTOC_TARGET)
    if (PANDA_TARGET_AMD64)
        set(IRTOC_TARGET x86_64)
    elseif(PANDA_TARGET_ARM64)
        set(IRTOC_TARGET arm64)
    elseif(PANDA_TARGET_ARM32)
        set(IRTOC_TARGET arm32)
    else()
        message(FATAL_ERROR "Unsupported IRtoC target")
    endif()
endif()
set(IRTOC_TARGET ${IRTOC_TARGET} PARENT_SCOPE)

#
# IRtoC front-end:
#

set(IRTOC_LANG_SOURCES
    ${IRTOC_SOURCE_DIR}/lang/basic_block.rb
    ${IRTOC_SOURCE_DIR}/lang/cpp_function.rb
    ${IRTOC_SOURCE_DIR}/lang/function.rb
    ${IRTOC_SOURCE_DIR}/lang/ir_generator.rb
    ${IRTOC_SOURCE_DIR}/lang/instruction.rb
    ${IRTOC_SOURCE_DIR}/lang/instructions_data.rb
    ${IRTOC_SOURCE_DIR}/lang/irtoc.rb
    ${IRTOC_SOURCE_DIR}/lang/isa.rb
    ${IRTOC_SOURCE_DIR}/lang/options.rb
    ${IRTOC_SOURCE_DIR}/lang/output.rb
    ${IRTOC_SOURCE_DIR}/lang/regmap.rb
    ${IRTOC_SOURCE_DIR}/lang/regmask.rb
    ${IRTOC_SOURCE_DIR}/lang/validation.rb
)
set(IRTOC_LANG_SOURCES ${IRTOC_LANG_SOURCES} PARENT_SCOPE) # To be available in irtoc_generate

# Unit tests for IRtoC frontend. Available only in x86 mode, since there is no need to test in different targets.
if (PANDA_WITH_TESTS AND PANDA_TARGET_AMD64)
    add_custom_target(irtoc_unit_tests COMMENT "Irtoc frontend unit tests"
        COMMAND ruby ${IRTOC_SOURCE_DIR}/lang/tests/regmask_test.rb
        DEPENDS ${IRTOC_LANG_SOURCES}
                ${IRTOC_SOURCE_DIR}/lang/tests/regmask_test.rb)
    add_dependencies(core_tests irtoc_unit_tests)
endif()

add_custom_target(interpreter_irt_combine)
set_target_properties(interpreter_irt_combine PROPERTIES IRTOC_PLUGIN_FILES "")

function(add_irtoc_plugin YAML_FILE_PATH)
    get_target_property(YAML_FILES interpreter_irt_combine IRTOC_PLUGIN_FILES)
    list(APPEND YAML_FILES ${YAML_FILE_PATH})
    set_target_properties(interpreter_irt_combine PROPERTIES IRTOC_PLUGIN_FILES "${YAML_FILES}")
endfunction()

# Run IRtoC front-end to generate C++ code from IRtoC sources.
# Arguments:
#   [in] TARGET            - name of the target that will generate C++ code. Thereby user can make a dependency on it.
#   [in] INPUT_FILES       - input IRtoC scripts, usually files from `irtoc/scripts` folder.
#   [in] OUTPUT_FILE       - name of the output C++ file.
#   [in] IR_API            - IR API to be emitted during C++ code generation. See IRtoC sources for possible supported values.
#   [in] WORKING_DIRECTORY - working directory for the front-end.
function(irtoc_generate)
    set(noValues)
    set(singleValueArgs TARGET IR_API WORKING_DIRECTORY)
    set(multiValueArgs INPUT_FILES OUTPUT_FILES)
    cmake_parse_arguments(ARG "${noValues}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    get_directory_property(IRTOC_DEFINITIONS DIRECTORY ${IRTOC_SOURCE_DIR} COMPILE_DEFINITIONS)

    if (CMAKE_BUILD_TYPE STREQUAL "Release"
            OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"
            OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        list(APPEND IRTOC_DEFINITIONS NDEBUG)
    endif()

    list(TRANSFORM IRTOC_DEFINITIONS PREPEND "-D")

    string(REPLACE ";" ":" IRTOC_INPUT "${ARG_INPUT_FILES}")
    string(REPLACE ";" ":" IRTOC_OUTPUT "${ARG_OUTPUT_FILES}")

    set(ISAPI "${PANDA_ROOT}/isa/isapi.rb")
    set(ISA "${PANDA_BINARY_ROOT}/isa/isa.yaml")

    if (NOT DEFINED ARG_WORKING_DIRECTORY)
        set(ARG_WORKING_DIRECTORY ${IRTOC_BUILD_DIR})
    else()
        file(MAKE_DIRECTORY ${ARG_WORKING_DIRECTORY})
    endif()

    set(IRTOC_PLUGINS_FILE ${IRTOC_BUILD_DIR}/generated/plugins.txt)

    set(irtoc_cpu_features "")
    if (NOT "${PANDA_TARGET_CPU_FEATURES}" STREQUAL "")
        set(irtoc_cpu_features "--cpu-features=${PANDA_TARGET_CPU_FEATURES}")
    endif()

    add_custom_command(OUTPUT ${ARG_OUTPUT_FILES}
        COMMENT             "Run IRtoC generator for ${ARG_TARGET}"
        COMMAND             ruby ${IRTOC_SOURCE_DIR}/lang/irtoc.rb
                                --input ${IRTOC_INPUT}
                                --output ${IRTOC_OUTPUT}
                                --ir-api ${ARG_IR_API}
                                --ark_source_dir ${PANDA_ROOT}
                                --isa ${ISA}
                                ${IRTOC_DEFINITIONS}
                                --arch ${IRTOC_TARGET}
                                ${irtoc_cpu_features}
                                --plugins ${IRTOC_PLUGINS_FILE}
        WORKING_DIRECTORY   ${ARG_WORKING_DIRECTORY}
        DEPENDS             ${IRTOC_LANG_SOURCES} ${ARG_INPUT_FILES} ${ISA} ${ISAPI} ${IRTOC_SOURCE_DIR}/scripts/common.irt ${IRTOC_PLUGINS_FILE}
    )

    add_custom_target(${ARG_TARGET} DEPENDS ${ARG_OUTPUT_FILES} irtoc_plugins_txt)
    add_dependencies(${ARG_TARGET} isa_combine)
    add_dependencies(panda_gen_files ${ARG_TARGET})
endfunction()
