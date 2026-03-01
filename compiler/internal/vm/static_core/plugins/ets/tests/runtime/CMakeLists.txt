# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

add_custom_target(ets_tests_runtime_gtests)

panda_ets_add_gtest(
    NO_CORES
    NAME ets_vm_test
    SOURCES ets_vm_test.cpp
    LIBRARIES arkbase arkfile arkruntime
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB $<TARGET_PROPERTY:mock_stdlib,FILE>
    DEPS_TARGETS mock_stdlib
)

panda_ets_add_gtest(
    NO_CORES
    NAME static_object_accessor_test
    SOURCES static_object_accessor_test.cpp
    LIBRARIES arkbase arkfile arkruntime
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
    DEPS_TARGETS etsstdlib
)

# NOTE(@srokashevich, #29164): enable with tsan after fix
if (NOT PANDA_ENABLE_THREAD_SANITIZER)
    panda_add_gtest_with_ani(ets_vm_init_prealloc_test
        CPP_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/prealloc_test/ets_vm_init_prealloc_test.cpp
        ETS_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/prealloc_test/ets_vm_init_prealloc_test.ets
        INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
        LIBRARIES arkbase arkfile arkruntime
        TEST_GROUP ets_tests_runtime_gtests
    )
endif()

panda_add_gtest_with_ani(entrypoint_return_test
    CPP_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/entrypoint_return_test.cpp
    ETS_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/entrypoint_return_test.ets
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    LIBRARIES arkbase arkfile arkruntime
    TEST_GROUP ets_tests_runtime_gtests
)

panda_add_gtest_with_ani(ets_vm_oom_test
    CPP_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/ets_vm_outofmemory_test.cpp
    ETS_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/ets_vm_outofmemory_test.ets
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    LIBRARIES arkbase arkfile arkruntime
    TEST_GROUP ets_tests_runtime_gtests
)

if (NOT PANDA_QEMU_BUILD)
    panda_add_gtest_with_ani(ets_vm_postfork_aot_load_test
        CPP_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/ets_vm_postfork_aot_load_test.cpp
        ETS_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/ets_vm_postfork_aot_load_test.ets
        INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
        LIBRARIES arkbase arkfile arkruntime
        TEST_GROUP ets_tests_runtime_gtests
    )
endif()

panda_ets_add_gtest(
    NO_CORES
    NAME static_type_converter_test
    SOURCES static_type_converter_test.cpp
    LIBRARIES arkbase arkfile arkruntime
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
    DEPS_TARGETS etsstdlib
)

# Test checks IterCounter overflow. Cpp part set pre-overflowed value 
# to IterCounter, then ets part check overflow. It requires cpp part,
# because it takes a long time to overflow IterCouner without hacking.
panda_add_gtest_with_ani(map_iter_overflow_test
    CPP_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/map_iter_overflow_test.cpp
    ETS_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/map_iter_overflow_test.ets
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    LIBRARIES arkbase arkfile arkruntime
    TEST_GROUP ets_tests_runtime_gtests
)

panda_add_gtest_with_ani(is_assignable_from_union_test
    CPP_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/is_assignable_from_union_test.cpp
    ETS_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/is_assignable_from_union_test.ets
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    LIBRARIES arkbase arkfile arkruntime
    TEST_GROUP ets_tests_runtime_gtests
)

add_dependencies(ets_tests_runtime_gtests
    ets_vm_test_gtests
    static_object_accessor_test_gtests
    static_type_converter_test_gtests)

add_subdirectory(types)
add_subdirectory(tooling)
