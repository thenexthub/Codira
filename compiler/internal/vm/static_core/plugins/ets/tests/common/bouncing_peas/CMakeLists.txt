# Copyright (c)Huawei Device Co., Ltd.
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

run_int_jit_aot_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/bouncing_peas.ets ${CMAKE_CURRENT_BINARY_DIR} bouncing_peas_ets)
run_deterministic_test(${CMAKE_CURRENT_SOURCE_DIR}/bouncing_peas.ets ${CMAKE_CURRENT_BINARY_DIR} bouncing_peas_ets)

set(OUTPUT_ABC ${CMAKE_CURRENT_BINARY_DIR}/bouncing_peas_unit_native.abc)
compile_ets_code(
    ${CMAKE_CURRENT_SOURCE_DIR}/bouncing_peas_unit_native.ets
    ${OUTPUT_ABC}
    bouncing_peas_unit_ets_abc
)

set(OUTPUT_AN_ARK ${CMAKE_CURRENT_BINARY_DIR}/bouncing_peas_unit_native_ark.an)
set(OUTPUT_AN_LLVM ${CMAKE_CURRENT_BINARY_DIR}/bouncing_peas_unit_native_llvm.an)
compile_aot_ets_code(
    aot
    ${OUTPUT_ABC}
    ${OUTPUT_AN_ARK}
    bouncing_peas_unit_ets_an_ark
    bouncing_peas_unit_ets_abc
)
if (PANDA_LLVM_AOT)
    compile_aot_ets_code(
        llvm
        ${OUTPUT_ABC}
        ${OUTPUT_AN_LLVM}
        bouncing_peas_unit_ets_an_llvm
        bouncing_peas_unit_ets_abc
    )
endif()

function(add_bouncing_peas_test PEAS_NAME PEAS_SOURCE)
    panda_ets_add_gtest(
        NO_CORES
        NAME ${PEAS_NAME}
        SOURCES
            ${PEAS_SOURCE}
        LIBRARIES
            arkruntime
            arkbase
        DEPS_TARGETS
            bouncing_peas_unit_ets_abc
            etsstdlib
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )

    if(TARGET ${PEAS_NAME})
        panda_target_compile_definitions(${PEAS_NAME}
            PUBLIC STDLIB_ABC=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
            PUBLIC PATH_ABC=${OUTPUT_ABC}
            PUBLIC PATH_AN_ARK=${OUTPUT_AN_ARK}
            PUBLIC PATH_AN_LLVM=${OUTPUT_AN_LLVM}
        )

        panda_target_include_directories(${PEAS_NAME}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            PUBLIC ${PANDA_ROOT}
        )
        panda_target_compile_options(${PEAS_NAME} PUBLIC "-Wno-ignored-attributes")

        if (CMAKE_CROSSCOMPILING)
            panda_target_compile_definitions(${PEAS_NAME} PUBLIC HOST_CROSSCOMPILING)
        else()
            add_dependencies(${PEAS_NAME} bouncing_peas_unit_ets_an_ark)
	    if (PANDA_LLVM_AOT)
                add_dependencies(${PEAS_NAME} bouncing_peas_unit_ets_an_llvm)
            endif()
        endif()
    endif()
endfunction()

add_bouncing_peas_test(bouncing_peas_unit bouncing_peas_unit.cpp)
add_bouncing_peas_test(bouncing_peas_unit_napi bouncing_peas_unit_napi.cpp)
