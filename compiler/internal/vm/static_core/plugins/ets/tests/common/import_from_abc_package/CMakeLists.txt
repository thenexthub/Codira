# Copyright (c) 2026 Huawei Device Co., Ltd.
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
if (PANDA_TARGET_ARM32)
    return()
endif()

file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" CURRENT_SOURCE_DIR)
file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}" CURRENT_BINARY_DIR)
file(REAL_PATH "${CMAKE_BINARY_DIR}" CMAKE_BINARY_ROOT)

set(DEFAULT_CACHE_DIR "${CMAKE_BINARY_ROOT}/plugins/ets/stdlib/decls")
file(MAKE_DIRECTORY "${DEFAULT_CACHE_DIR}")
set(LIB_SOURCE_ABC ${CURRENT_BINARY_DIR}/lib.abc)
set(LIB_SOURCE_ABC_TARGET compile_lib_abc_package)

set(PACKAGE_CONFIG ${CURRENT_BINARY_DIR}/arktsconfig_package.json)
configure_file(${CURRENT_SOURCE_DIR}/lib/arktsconfig.in.json ${PACKAGE_CONFIG})

compile_ets_code("" ${LIB_SOURCE_ABC} ${LIB_SOURCE_ABC_TARGET}
                 ARKTS_CONFIG "${PACKAGE_CONFIG}"
                 COMPILER_EXTRA_OPTIONS "--simultaneous;--emit-declaration")

set(ENTRY_CONFIG ${CURRENT_BINARY_DIR}/arktsconfig_entry.json)
configure_file(${CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ENTRY_CONFIG})

run_int_ets_code(import_from_abc_package ${CURRENT_BINARY_DIR}
                 SOURCES ${CURRENT_SOURCE_DIR}/entry.ets
                 ARKTS_CONFIGS ${ENTRY_CONFIG}
                 ENTRYPOINT "Entry.entry"
                 RUNTIME_EXTRA_OPTIONS --panda-files=${LIB_SOURCE_ABC})

add_dependencies(import_from_abc_package-cpp-abc-entry-ets-es2panda ${LIB_SOURCE_ABC_TARGET})
add_dependencies(import_from_abc_package-irtoc-abc-entry-ets-es2panda ${LIB_SOURCE_ABC_TARGET})

if (PANDA_LLVM_INTERPRETER)
    add_dependencies(import_from_abc_package-llvmirtoc-abc-entry-ets-es2panda ${LIB_SOURCE_ABC_TARGET})
endif()