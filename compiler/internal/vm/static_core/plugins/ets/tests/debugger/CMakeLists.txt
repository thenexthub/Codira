# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(ets_debugger_tests)

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
    COMMAND .venv/bin/mypy ${CMAKE_CURRENT_SOURCE_DIR}/src
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
    COMMAND .venv/bin/flake8 --config ${CMAKE_CURRENT_SOURCE_DIR}/.flake8 ${CMAKE_CURRENT_SOURCE_DIR}/src
    DEPENDS ${PROJECT_NAME}_deps
    COMMENT "${PROJECT_NAME}: flake8"
    USES_TERMINAL
)

add_custom_target(${PROJECT_NAME}_black
    COMMAND .venv/bin/black --check --diff --config pyproject.toml  ${CMAKE_CURRENT_SOURCE_DIR}/src
    DEPENDS ${PROJECT_NAME}_deps
    COMMENT "${PROJECT_NAME}: black"
    USES_TERMINAL
)

add_custom_target(${PROJECT_NAME}_check
    DEPENDS
        ${PROJECT_NAME}_mypy
        ${PROJECT_NAME}_flake8
        ${PROJECT_NAME}_black
)

# NOTE(dslynko, #25138): fix and reenable expression evaluation tests
set(IGNORED_TESTS --ignore-glob=**/evaluate/*)

add_custom_target(${PROJECT_NAME}
    COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/.venv/bin/python3
        -X pycache_prefix=${CMAKE_CURRENT_BINARY_DIR}/.pycache
        -m pytest
        -o cache_dir=${CMAKE_CURRENT_BINARY_DIR}/.pytest_cache
        -o testpaths=${CMAKE_CURRENT_SOURCE_DIR}/src
        --ark-build-path=${CMAKE_BINARY_DIR}
        --rootdir=${CMAKE_CURRENT_SOURCE_DIR}/src
        -vv
        ${IGNORED_TESTS}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    DEPENDS
        ${PROJECT_NAME}_deps
        ${PROJECT_NAME}_check
        panda_bins
        arkinspector
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/.pycache
        ${CMAKE_CURRENT_BINARY_DIR}/.pytest_cache
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "${PROJECT_NAME}: Running tests"
    USES_TERMINAL
)

set_property(TARGET ${PROJECT_NAME}
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        .pycache
        .pytest_cache
)
