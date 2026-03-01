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

create_declgen_ets2ts_test(export_default
    ROOT_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}
    BINARY_ROOT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}
    TS_MAIN
        ${CMAKE_CURRENT_SOURCE_DIR}/main.ts
    ETS_VM_LAUNCHER
        ${CMAKE_CURRENT_SOURCE_DIR}/../ets_vm_launcher.ts
    TS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/export_default.ts
    ETS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib1.ets
        ${CMAKE_CURRENT_SOURCE_DIR}/lib2.ets
        ${CMAKE_CURRENT_SOURCE_DIR}/lib3.ets
        ${CMAKE_CURRENT_SOURCE_DIR}/lib4.ets
        ${CMAKE_CURRENT_SOURCE_DIR}/lib5.ets
        ${CMAKE_CURRENT_SOURCE_DIR}/lib6.ets
    DECLS_EXPECTED
        ${CMAKE_CURRENT_SOURCE_DIR}/lib1.expected
        ${CMAKE_CURRENT_SOURCE_DIR}/lib2.expected
        ${CMAKE_CURRENT_SOURCE_DIR}/lib3.expected
        ${CMAKE_CURRENT_SOURCE_DIR}/lib4.expected
        ${CMAKE_CURRENT_SOURCE_DIR}/lib5.expected
        ${CMAKE_CURRENT_SOURCE_DIR}/lib6.expected
)
