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

if (PANDA_TARGET_ARM32)
    return()
endif()

set(EXPORT_FILE_ETS ${CMAKE_CURRENT_SOURCE_DIR}/dir/export_file.ets)
set(EXPORT_FILE_CACHE ${CMAKE_CURRENT_BINARY_DIR}/dir/export_file.etscache)
set(EXPORT_FILE_ABC ${CMAKE_CURRENT_BINARY_DIR}/dir/export_file.abc)
set(EXPORT_FILE_CACHE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dir)
set(EXPORT_FILE_ABC_TARGET compile_export_file_abc)

make_directory(${EXPORT_FILE_CACHE_DIRECTORY})

compile_ets_code(${EXPORT_FILE_ETS} ${EXPORT_FILE_ABC} ${EXPORT_FILE_ABC_TARGET}
                 COMPILER_EXTRA_OPTIONS --generate-decl:enabled=true --generate-decl:path=${EXPORT_FILE_CACHE})

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

run_int_ets_code(import_from_declaration_cache ${CMAKE_CURRENT_BINARY_DIR}
                 SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/file.ets
                 ARKTS_CONFIGS ${ETS_CONFIG}
                 RUNTIME_EXTRA_OPTIONS --panda-files=${EXPORT_FILE_ABC})

add_dependencies(import_from_declaration_cache-cpp-abc-file-ets-es2panda ${EXPORT_FILE_ABC_TARGET})
add_dependencies(import_from_declaration_cache-irtoc-abc-file-ets-es2panda ${EXPORT_FILE_ABC_TARGET})

if (PANDA_LLVM_INTERPRETER)
    add_dependencies(import_from_declaration_cache-llvmirtoc-abc-file-ets-es2panda ${EXPORT_FILE_ABC_TARGET})
endif()
