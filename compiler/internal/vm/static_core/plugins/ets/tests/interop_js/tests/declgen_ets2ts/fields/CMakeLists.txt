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

create_declgen_ets2ts_test(fields
    ROOT_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}
    BINARY_ROOT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}
    TS_MAIN
        ${CMAKE_CURRENT_SOURCE_DIR}/main.ts
    ETS_VM_LAUNCHER
        ${CMAKE_CURRENT_SOURCE_DIR}/../ets_vm_launcher.ts
    TS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/fields.ts
    ETS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib.ets
    DECLS_EXPECTED
        ${CMAKE_CURRENT_SOURCE_DIR}/lib.expected
)
