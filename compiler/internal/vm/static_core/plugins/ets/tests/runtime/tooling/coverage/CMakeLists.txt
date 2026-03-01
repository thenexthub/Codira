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

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_command(
    OUTPUT .venv
    COMMAND ${Python3_EXECUTABLE} -m venv .venv
    COMMAND .venv/bin/pip install -U pip poetry
    USES_TERMINAL
)

add_custom_target(${PROJECT_NAME}_venv
    DEPENDS .venv
    COMMENT "${PROJECT_NAME}: create venv"
)
set_property(TARGET ${PROJECT_NAME}_venv
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        .venv
)

function(add_coverage_script_test test_name)
    set(PROJECT_TARGET_NAME test-coverage-script-${test_name})

    add_custom_target(${PROJECT_TARGET_NAME}
        COMMENT "Running coverage script test: ${test_name}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        COMMAND PANDA_BINARY_ROOT=${PANDA_BINARY_ROOT} PANDA_ROOT=${PANDA_ROOT} python3 test_coverage.py
        VERBATIM
    )

    add_dependencies(${PROJECT_TARGET_NAME} es2panda ark ark_link etsstdlib ark_disasm ${PROJECT_NAME}_deps ${PROJECT_NAME}_check)
    add_dependencies(ets_tests ${PROJECT_TARGET_NAME})
endfunction()

set(PROJECT_NAME coverage_script)

add_custom_command(
    OUTPUT poetry.lock pyproject.toml
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml pyproject.toml
    COMMAND .venv/bin/poetry install --no-root --no-interaction
    COMMAND .venv/bin/poetry show --why --no-interaction
    DEPENDS .venv ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml
    USES_TERMINAL
)

add_custom_target(${PROJECT_NAME}_deps
    DEPENDS poetry.lock pyproject.toml
    COMMENT "${PROJECT_NAME}: install dependinces"
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml
)

add_custom_target(${PROJECT_NAME}_mypy
    COMMAND .venv/bin/mypy ${CMAKE_CURRENT_SOURCE_DIR}/*.py
    DEPENDS ${PROJECT_NAME}_deps
    BYPRODUCTS .mypy_cache
    COMMENT "${PROJECT_NAME}: mypy"
    USES_TERMINAL
)
set_property(TARGET ${PROJECT_NAME}_mypy
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        .mypy_cache
)

add_custom_target(${PROJECT_NAME}_flake8
    COMMAND .venv/bin/flake8 --config ${CMAKE_CURRENT_SOURCE_DIR}/.flake8 ${CMAKE_CURRENT_SOURCE_DIR}/*.py
    DEPENDS ${PROJECT_NAME}_deps
    COMMENT "${PROJECT_NAME}: flake8"
    USES_TERMINAL
)

add_custom_target(${PROJECT_NAME}_check
    DEPENDS
        ${PROJECT_NAME}_mypy
        ${PROJECT_NAME}_flake8
)

add_coverage_script_test(host_output)
