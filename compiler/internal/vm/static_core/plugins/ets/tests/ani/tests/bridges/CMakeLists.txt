# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

set(ANI_LIB_TESTS "${CMAKE_CURRENT_BINARY_DIR}/lib")
file(MAKE_DIRECTORY ${ANI_LIB_TESTS})

set(CREATE_ANI_TEST_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_test_files.sh")
set(QEMU_ARM32 false)

if(PANDA_TARGET_ARM32 AND PANDA_QEMU_BUILD)
    set(QEMU_ARM32 true)
endif()

set(J2_files
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/types.cpp.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/types.ets.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/arrays.cpp.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/arrays.ets.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/returns.cpp.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/returns.ets.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/types_ref.ets.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/types_ref.cpp.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/returns_ref.ets.j2
    ${CMAKE_CURRENT_SOURCE_DIR}/j2/returns_ref.cpp.j2
)

set(Scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_test_files.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_test_files.sh
)

set(Prefixes "types" "arrays" "returns")
set(Types "boolean" "byte" "char" "double" "float" "int" "long" "short" "string" "ref")

set(FILES "")
foreach(pref IN LISTS Prefixes)
    foreach(type IN LISTS Types)
        if(pref STREQUAL "types")
            foreach(i RANGE 1 11)
                list(APPEND FILES ${CMAKE_CURRENT_BINARY_DIR}/tests/${pref}_${type}_${i}/${pref}_${type}_${i}.cpp)
                list(APPEND FILES ${CMAKE_CURRENT_BINARY_DIR}/tests/${pref}_${type}_${i}/${pref}_${type}_${i}.ets)
            endforeach()
        elseif (NOT "${pref}_${type}" STREQUAL "arrays_ref")
            list(APPEND FILES ${CMAKE_CURRENT_BINARY_DIR}/tests/${pref}_${type}/${pref}_${type}.cpp)
            list(APPEND FILES ${CMAKE_CURRENT_BINARY_DIR}/tests/${pref}_${type}/${pref}_${type}.ets)
        endif()
    endforeach()
endforeach()

add_custom_command(
    OUTPUT ${FILES}
    COMMAND ${CREATE_ANI_TEST_SCRIPT} ${PANDA_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/tests ${QEMU_ARM32}
    DEPENDS ${J2_files} ${Scripts}
)

add_custom_target(ani_tests_bridges_generate_test_files DEPENDS ${FILES})

set(PANDA_RUN_PREFIX
    LD_LIBRARY_PATH=${ANI_LIB_TESTS}
    ${PANDA_RUN_PREFIX}
)

add_custom_target(ani_tests_bridges_tests COMMENT "Common target to run ANI bridges tests")

function(add_tests pref suff)
    #  NOTE(mgroshev): #29029
    if(${pref} STREQUAL "types_double")
        set(RT_OPTION "--compiler-inlining=false")
    endif()
    set(test_name ${pref}_${suff})
    panda_add_library(${test_name} SHARED ${CMAKE_CURRENT_BINARY_DIR}/tests/${test_name}/${test_name}.cpp)
    add_dependencies(${test_name} ani_tests_bridges_generate_test_files)

    panda_target_include_directories(${test_name} PRIVATE ${PLUGIN_SOURCE}/runtime/ani)
    panda_target_compile_options(${test_name} PRIVATE -Wno-attributes)

    set_target_properties(${test_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${ANI_LIB_TESTS})
    panda_target_link_libraries(${test_name} arkruntime gtest)

    if(NOT TARGET ani_tests_bridges_${pref})
        add_custom_target(ani_tests_bridges_${pref})
        add_dependencies(ani_tests_bridges_${pref} ${test_name})
        add_dependencies(ani_tests_bridges_tests ani_tests_bridges_${pref})
    endif()

    # Create targets ani_tests_bridges_${pref}_${suff}-ets-aot, ani_tests_bridges_${pref}_${suff}-ets-jit (etc.)
    # ani_tests_bridges_${pref} target depends on them
    add_ets_test(ani_tests_bridges_${pref}
        FILE ${test_name}.ets
        TEST_NAME ${suff}
        MODE "INT" "JIT" "AOT" "LLVMAOT" "JITOSR"
        ENTRYPOINT "EtsAniTests"
        OPTIONS ${RT_OPTION} "--compiler-regex='.*ETSGLOBAL::.*'"
        TEST_PATH ${CMAKE_CURRENT_BINARY_DIR}/tests/${test_name}/${test_name}.ets
    )

    if(TARGET ani_tests_bridges_${pref}_${suff}-ets-aot)
        add_dependencies(ani_tests_bridges_${pref}_${suff}-ets-aot ${test_name})
    endif()
    if(TARGET ani_tests_bridges_${pref}_${suff}-ets-int-cpp)
        add_dependencies(ani_tests_bridges_${pref}_${suff}-ets-int-cpp ${test_name})
    endif()
    if(TARGET ani_tests_bridges_${pref}_${suff}-ets-int-irtoc)
        add_dependencies(ani_tests_bridges_${pref}_${suff}-ets-int-irtoc ${test_name})
    endif()
    if(TARGET ani_tests_bridges_${pref}_${suff}-ets-int-llvmirtoc)
        add_dependencies(ani_tests_bridges_${pref}_${suff}-ets-int-llvmirtoc ${test_name})
    endif()
    if(TARGET ani_tests_bridges_${pref}_${suff}-ets-jit)
        add_dependencies(ani_tests_bridges_${pref}_${suff}-ets-jit ${test_name})
    endif()
    if(TARGET ani_tests_bridges_${pref}_${suff}-ets-llvmaot)
        add_dependencies(ani_tests_bridges_${pref}_${suff}-ets-llvmaot ${test_name})
    endif()
    if(TARGET ani_tests_bridges_${pref}_${suff}-ets-jit-osr)
        add_dependencies(ani_tests_bridges_${pref}_${suff}-ets-jit-osr ${test_name})
    endif()
endfunction(add_tests)

# 26982 64-bit pointer support
if(NOT PANDA_TARGET_ARM64 OR PANDA_32_BIT_MANAGED_POINTER)
    foreach(pref IN LISTS Prefixes)
        foreach(type IN LISTS Types)
            if(pref STREQUAL "types")
                foreach(i RANGE 1 11)
                    add_tests(${pref}_${type} ${i})
                endforeach()
            elseif(NOT "${pref}_${type}" STREQUAL "arrays_ref")
                add_tests(${pref} ${type})
            endif()
        endforeach()
    endforeach()
endif()

add_dependencies(ani_tests ani_tests_bridges_tests)
