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

project(arkbase)

include(CheckFunctionExists)
include(CheckSymbolExists)
include(cmake/mm_coverage.cmake)

set(ARKBASE_LTO_SOURCES
    ${PANDA_ROOT}/libarkbase/mem/arena_allocator.cpp
    ${PANDA_ROOT}/libarkbase/mem/base_mem_stats.cpp
    ${PANDA_ROOT}/libarkbase/mem/pool_map.cpp
    ${PANDA_ROOT}/libarkbase/os/time.cpp
    ${PANDA_ROOT}/libarkbase/utils/json_builder.cpp
    ${PANDA_ROOT}/libarkbase/utils/json_parser.cpp
    ${PANDA_ROOT}/libarkbase/utils/time.cpp
    ${PANDA_ROOT}/libarkbase/utils/type_converter.cpp
    ${PANDA_ROOT}/libarkbase/utils/utf.cpp
)

set(SOURCES
    ${PANDA_ROOT}/libarkbase/mem/alloc_tracker.cpp
    ${PANDA_ROOT}/libarkbase/mem/code_allocator.cpp
    ${PANDA_ROOT}/libarkbase/mem/pool_manager.cpp
    ${PANDA_ROOT}/libarkbase/mem/mem_config.cpp
    ${PANDA_ROOT}/libarkbase/utils/debug.cpp
    ${PANDA_ROOT}/libarkbase/utils/logger.cpp
    ${PANDA_ROOT}/libarkbase/utils/dfx.cpp
    ${PANDA_ROOT}/libarkbase/utils/terminate.cpp
    ${PANDA_ROOT}/libarkbase/utils/utils.cpp
    ${PANDA_ROOT}/libarkbase/trace/trace.cpp
    ${PANDA_ROOT}/libarkbase/os/filesystem.cpp
    ${PANDA_ROOT}/libarkbase/os/native_stack.cpp
    ${PANDA_ROOT}/libarkbase/os/property.cpp
    ${PANDA_ROOT}/libarkbase/os/dfx_option.cpp
    ${PANDA_ROOT}/libarkbase/taskmanager/task.cpp
    ${PANDA_ROOT}/libarkbase/taskmanager/task_manager.cpp
    ${PANDA_ROOT}/libarkbase/taskmanager/task_queue_set.cpp
    ${PANDA_ROOT}/libarkbase/taskmanager/task_scheduler.cpp
    ${PANDA_ROOT}/libarkbase/taskmanager/timer_thread.cpp
    ${PANDA_ROOT}/libarkbase/taskmanager/worker_thread.cpp
    ${PANDA_ROOT}/libarkbase/taskmanager/utils/task_time_stats.cpp
    ${ARKBASE_LTO_SOURCES}
)

set(ARKBASE_LTO_UNIX_SOURCES
    ${PANDA_ROOT}/platforms/unix/libarkbase/thread.cpp
)

set(UNIX_SOURCES_COMMON
    ${PANDA_ROOT}/platforms/unix/libarkbase/cpu_affinity.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/error.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/native_stack.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/property.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/file.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/filesystem.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/kill.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/library_loader_resolve_symbol.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/mem.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/mem_hooks.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/sighook.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/system_environment.cpp
    ${PANDA_ROOT}/platforms/unix/libarkbase/trace.cpp
    ${ARKBASE_LTO_UNIX_SOURCES}
)


# Handle pthread and futex mutexes
if(PANDA_USE_FUTEX)
    set(UNIX_SOURCES_COMMON ${UNIX_SOURCES_COMMON}
        ${PANDA_ROOT}/platforms/unix/libarkbase/futex/fmutex.cpp
        ${PANDA_ROOT}/platforms/unix/libarkbase/futex/mutex.cpp
    )
else()
    set(UNIX_SOURCES_COMMON ${UNIX_SOURCES_COMMON}
        ${PANDA_ROOT}/libarkbase/os/mutex.cpp
    )
endif()

set(WINDOWS_SOURCES
    ${PANDA_ROOT}/libarkbase/utils/logger.cpp
    ${PANDA_ROOT}/libarkbase/utils/debug.cpp
    ${PANDA_ROOT}/libarkbase/utils/dfx.cpp
    ${PANDA_ROOT}/libarkbase/utils/time.cpp
    ${PANDA_ROOT}/libarkbase/utils/utf.cpp
    ${PANDA_ROOT}/libarkbase/utils/utils.cpp
    ${PANDA_ROOT}/libarkbase/utils/json_builder.cpp
    ${PANDA_ROOT}/libarkbase/utils/json_parser.cpp
    ${PANDA_ROOT}/libarkbase/utils/type_converter.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/cpu_affinity.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/error.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/kill.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/library_loader.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/file.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/mem.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/mem_hooks.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/thread.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/filesystem.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/trace.cpp
    ${PANDA_ROOT}/platforms/windows/libarkbase/system_environment.cpp
    ${PANDA_ROOT}/libarkbase/os/dfx_option.cpp
    ${PANDA_ROOT}/libarkbase/os/filesystem.cpp
    ${PANDA_ROOT}/libarkbase/os/mutex.cpp
    ${PANDA_ROOT}/libarkbase/os/time.cpp
    ${PANDA_ROOT}/libarkbase/mem/alloc_tracker.cpp
    ${PANDA_ROOT}/libarkbase/mem/arena_allocator.cpp
    ${PANDA_ROOT}/libarkbase/mem/pool_manager.cpp
    ${PANDA_ROOT}/libarkbase/mem/pool_map.cpp
    ${PANDA_ROOT}/libarkbase/mem/base_mem_stats.cpp
    ${PANDA_ROOT}/libarkbase/mem/code_allocator.cpp
    ${PANDA_ROOT}/libarkbase/mem/mem_config.cpp
)

set(INCLUDE_DIRECTORIES)
set(LINK_LIBRARIES)

# Handle library load method
if(PANDA_TARGET_MOBILE)
    set(UNIX_SOURCES_COMMON ${UNIX_SOURCES_COMMON}
        ${PANDA_ROOT}/platforms/mobile/libarkbase/library_loader_load.cpp
    )
else()
    set(UNIX_SOURCES_COMMON ${UNIX_SOURCES_COMMON}
        ${PANDA_ROOT}/platforms/unix/libarkbase/library_loader_load.cpp
    )
endif()

# Native stacktraces
if (PANDA_TARGET_WINDOWS)
    # We are not interesting in native stacktraces on windows
    list(APPEND WINDOWS_SOURCES
        ${PANDA_ROOT}/libarkbase/os/stacktrace_stub.cpp
    )
elseif (PANDA_TARGET_OHOS)
    # Not yet supported
    list(APPEND SOURCES
        ${PANDA_ROOT}/libarkbase/os/stacktrace_stub.cpp
    )
elseif (PANDA_PRODUCT_BUILD)
    # Don't add libdwarf dependency for product build
    list(APPEND SOURCES
        ${PANDA_ROOT}/libarkbase/os/stacktrace_stub.cpp
    )
else()
    # Add common sources for native stacktraces
    list(APPEND UNIX_SOURCES_COMMON
        ${PANDA_ROOT}/libarkbase/os/stacktrace.cpp
        ${PANDA_ROOT}/libarkbase/os/debug_info.cpp
    )
    # Handle backtrace function
    if (PANDA_TARGET_MOBILE)
        # Android doesn't have 'backtrace' function but it has
        # libunwind. Use 'unw_backtrace' from libunwind to get stacktraces.
        if (PANDA_TARGET_32)
            list(APPEND INCLUDE_DIRECTORIES "/usr/armv7a-linux-androideabi/include")
            list(APPEND LINK_LIBRARIES
                "/usr/armv7a-linux-androideabi/lib/libdwarf.so"
                "/usr/armv7a-linux-androideabi/lib/libunwind.so")
        else()
            list(APPEND INCLUDE_DIRECTORIES "/usr/aarch64-linux-android/include")
            list(APPEND LINK_LIBRARIES
                "/usr/aarch64-linux-android/lib/libdwarf.so"
                "/usr/aarch64-linux-android/lib/libunwind.so")
        endif()
    else()
        list(APPEND LINK_LIBRARIES dwarf z)
    endif()

    set(CMAKE_REQUIRED_INCLUDES ${INCLUDE_DIRECTORIES})
    set(CMAKE_REQUIRED_LIBRARIES ${LINK_LIBRARIES})
    check_symbol_exists(dwarf_rnglists_get_rle_head "libdwarf/libdwarf.h" HAVE_DWARF_RNGLISTS_API)
    panda_promote_to_definitions(HAVE_DWARF_RNGLISTS_API)
endif()

if(PANDA_TARGET_UNIX)
    set(SOURCES ${SOURCES} ${UNIX_SOURCES_COMMON})
    set(ARKBASE_LTO_SOURCES ${ARKBASE_LTO_SOURCES} ${ARKBASE_LTO_UNIX_SOURCES})
    if(NOT PANDA_TARGET_MACOS)
        set(SOURCES ${SOURCES}
            ${PANDA_ROOT}/platforms/unix/libarkbase/exec.cpp
            ${PANDA_ROOT}/platforms/unix/libarkbase/pipe.cpp)
    endif()
elseif (PANDA_TARGET_WINDOWS)
    set(SOURCES ${WINDOWS_SOURCES})
else()
    message(FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} is not supported")
endif ()

if (PANDA_TARGET_ARM64)
  list(APPEND SOURCES ${PANDA_ROOT}/libarkbase/arch/aarch64/cpu_features.cpp)
elseif (PANDA_TARGET_AMD64 OR PANDA_TARGET_ARM32)
  list(APPEND SOURCES ${PANDA_ROOT}/libarkbase/arch/default/cpu_features.cpp)
else()
  message(FATAL_ERROR "Arch ${CMAKE_SYSTEM_PROCESSOR} is not supported")
endif()

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

if(PANDA_TARGET_MOBILE)
    set(COHERENCY_LINE_SIZE_H_INPUT "${PANDA_ROOT}/platforms/mobile/libarkbase/coherency_line_size.h")
else()
    set(COHERENCY_LINE_SIZE_H_INPUT "${PANDA_ROOT}/platforms/common/libarkbase/coherency_line_size.h")
endif()

set(COHERENCY_LINE_SIZE_H_OUTPUT "${GENERATED_DIR}/coherency_line_size.h")

add_custom_command(OUTPUT ${COHERENCY_LINE_SIZE_H_OUTPUT}
    COMMAND cmake -E copy ${COHERENCY_LINE_SIZE_H_INPUT} ${COHERENCY_LINE_SIZE_H_OUTPUT}
    DEPENDS ${COHERENCY_LINE_SIZE_H_INPUT})

add_custom_target(coherency_line_size_h DEPENDS ${COHERENCY_LINE_SIZE_H_OUTPUT})

panda_add_library(arkbase_obj OBJECT ${SOURCES})
add_dependencies(arkbase_obj coherency_line_size_h)


# Version
set(ARK_VERSION_DATE)
execute_process(
    COMMAND date "+%Y-%m-%d %H:%M:%S"
    OUTPUT_VARIABLE ARK_VERSION_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(Git)
if(Git_FOUND)
    message(STATUS "Git found: ${GIT_EXECUTABLE}")
    set(IS_GIT_REPOSITORY)
    execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PANDA_ROOT} rev-parse
                    RESULT_VARIABLE IS_GIT_REPOSITORY)
    if (IS_GIT_REPOSITORY EQUAL 0)
        set(ARK_VERSION_GIT_HASH)
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
                        WORKING_DIRECTORY ${PANDA_ROOT}
                        OUTPUT_VARIABLE ARK_VERSION_GIT_HASH
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
else()
    message("-- Git not found")
endif()

set(ARK_VERSION_H ${GENERATED_DIR}/ark_version.h)
configure_file(templates/ark_version.h.in ${ARK_VERSION_H})

add_custom_target(ark_version_gen DEPENDS ${ARK_VERSION_H})

set(EVENTS_GEN_H ${GENERATED_DIR}/events_gen.h)
panda_gen_file(
        DATA ${PANDA_ROOT}/libarkbase/events/events.yaml
        TEMPLATE ${PANDA_ROOT}/libarkbase/events/events_gen.h.erb
        OUTPUTFILE ${EVENTS_GEN_H}
        API ${PANDA_ROOT}/libarkbase/events/events.rb
)
add_custom_target(events_gen DEPENDS ${EVENTS_GEN_H})
add_dependencies(panda_gen_files events_gen)

if (NOT (PANDA_TARGET_WINDOWS OR PANDA_TARGET_MOBILE OR PANDA_TARGET_MACOS OR PANDA_TARGET_OHOS OR PANDA_ENABLE_FUZZBENCH))
    list(APPEND LINK_LIBRARIES stdc++fs)
endif()

if(NOT PANDA_TARGET_WINDOWS)
    list(APPEND LINK_LIBRARIES dl)
endif()

if(PANDA_NEED_LIBATOMIC)
    list(APPEND LINK_LIBRARIES atomic)
endif()

if(NOT (PANDA_TARGET_MOBILE OR PANDA_TARGET_OHOS))
    list(APPEND LINK_LIBRARIES pthread)
endif()

list(APPEND LINK_LIBRARIES c_secshared)

check_function_exists(gettid HAVE_GETTID)

panda_promote_to_definitions(HAVE_GETTID)

option(PANDA_ARKBASE_LTO "Link part of arkbase library into every shared object if using LTO" ON)

# If LTO is enabled, static libraries contain LLVM IR.
# For an external symbol X defined in one of ARKBASE_LTO_SOURCES,
# the expected layout with PANDA_ARKBASE_LTO=OFF is:
# * libarkbase.so: X - defined, strong, default visibility
# * every other library: X - undefined (if referenced at all)
# With PANDA_ARKBASE_LTO=ON, the expected layout is as follows:
# * libarkbase.so: X - defined, weak, default visibility
# * every other library: X - defined, strong, hidden (if any)
if(PANDA_ENABLE_LTO AND PANDA_ARKBASE_LTO)
    panda_add_library(arkbase_lto STATIC ${ARKBASE_LTO_SOURCES})
    # arkbase_lto: do not export the same symbol from multiple non-arkbase libraries
    panda_target_compile_options(arkbase_lto PRIVATE "-fvisibility=hidden")
    # arkbase: do not conflict with (strong hidden) symbols from
    # other shared objects (due to linked arkbase_lto)
    panda_target_compile_definitions(arkbase_obj PRIVATE WEAK_SYMBOLS_FOR_LTO)
    # Force loading of the entire contents of libarkbase_lto.a to make sure
    # LLD knows about strong symbols. Use INTERFACE to not replace public weak
    # symbols with strong hidden ones in libarkbase.so itself.
    set_property(TARGET arkbase_obj APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                -Wl,--push-state,--whole-archive arkbase_lto -Wl,--pop-state)
    # Sanity check: arkbase_lto should not contain any *unexpected* writable
    # sections (especially .data and .bss).
    set(LTO_TEST_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/test_arkbase_lto_writable_sections.o")
    add_custom_target(test_arkbase_lto_writable_sections
        # Codegen LLVM IR from arkbase_lto to a single object file.
        # Technically, using ld.lld is enough here, but ask C compiler
        # to find the correct LLD version for us, then prevent it from
        # passing some conflicting options to LLD.
        COMMAND ${CMAKE_C_COMPILER} --target=${CMAKE_C_COMPILER_TARGET}
                -fuse-ld=lld -r -Wl,-r -nostdlib -o "${LTO_TEST_OBJECT}"
                -Wl,--whole-archive "$<TARGET_FILE:arkbase_lto>"
        # Dump section headers ...
        COMMAND "${CMAKE_READELF}" --sections --wide "${LTO_TEST_OBJECT}"
                > "${LTO_TEST_OBJECT}.sections"
        # ... and filter those containing 'W' in the 4th field from the end of line
        COMMAND grep -E "W[^ ]*( +[^ ]+){3}$" "${LTO_TEST_OBJECT}.sections"
                > "${LTO_TEST_OBJECT}.sections-writable"
        # Then check that only *.rel.ro.* and .init_array sections are found.
        COMMAND sed "/\.rel\.ro\./ d; / \.init_array / d; q1" "${LTO_TEST_OBJECT}.sections-writable"
        COMMENT "Testing for unexpected mutable state in arkbase_lto"
        VERBATIM
    )
    add_dependencies(core_tests test_arkbase_lto_writable_sections)

    set(ARKBASE_TARGETS arkbase_obj arkbase_lto)
else()
    set(ARKBASE_TARGETS arkbase_obj)
endif()

foreach(TARGET ${ARKBASE_TARGETS})
    add_dependencies(${TARGET} ark_version_gen events_gen)
    panda_target_link_libraries(${TARGET} ${LINK_LIBRARIES})
    panda_target_include_directories(${TARGET}
        PUBLIC ${PANDA_ROOT}
        PUBLIC ${PANDA_BINARY_ROOT}
        PUBLIC ${INCLUDE_DIRECTORIES}
    )
endforeach()

set(ARKBASE_TESTS_SOURCES
    tests/list_test.cpp
    tests/bit_helpers_test.cpp
    tests/bit_memory_region_test.cpp
    tests/bit_table_test.cpp
    tests/bit_utils_test.cpp
    tests/bit_vector_test.cpp
    tests/bloom_filter_test.cpp
    tests/string_helpers_test.cpp
    tests/type_converter_tests.cpp
    tests/dfx_test.cpp
    tests/leb128_test.cpp
    tests/utf_test.cpp
    tests/arena_test.cpp
    tests/arena_allocator_test.cpp
    tests/expected_test.cpp
    tests/code_allocator_test.cpp
    tests/small_vector_test.cpp
    tests/span_test.cpp
    tests/pandargs_test.cpp
    tests/pool_map_test.cpp
    tests/hash_test.cpp
    tests/math_helpers_test.cpp
    tests/serializer_test.cpp
    tests/base_mem_stats_test.cpp
    tests/unique_fd_test.cpp
    tests/mmap_test.cpp
    tests/mmap_mem_pool_test.cpp
    tests/native_bytes_from_mallinfo_test.cpp
    tests/json_builder_test.cpp
    tests/json_parser_test.cpp
    tests/alloc_tracker_test.cpp
    tests/regmask_test.cpp
    tests/ring_buffer_test.cpp
    tests/ringbuf/lock_free_ring_buffer_test.cpp
    tests/base_thread_test.cpp
    tests/mem_hooks_test.cpp
)

if (NOT ENABLE_UNIT_TESTS_FULL_COVERAGE)
    set(ARKBASE_TESTS_SOURCES
        ${ARKBASE_TESTS_SOURCES}
        tests/logger_test.cpp
    )
endif()

if (PANDA_TARGET_UNIX)
    list(APPEND ARKBASE_TESTS_SOURCES
            tests/unix_signal_test.cpp
        )
endif()

if (PANDA_USE_FUTEX)
    list(APPEND ARKBASE_TESTS_SOURCES
            tests/futex_test.cpp
        )
endif()

if (PANDA_CI_TESTING_MODE STREQUAL "Nightly" OR NOT PANDA_TARGET_ARM32)
    panda_add_gtest(
        NO_CORES
        NO_WARNING_FILTER
        NAME arkbase_tests
        SOURCES
            ${ARKBASE_TESTS_SOURCES}
        LIBRARIES
            arkbase_static
            gmock
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )
    if (TARGET arkbase_tests)
        panda_target_compile_options(arkbase_tests PUBLIC "-Wno-unused")
    endif()
endif()

panda_add_gtest(
    NO_CORES
    NAME arkbase_mem_range_tests
    SOURCES
        tests/mem_range_test.cpp
    LIBRARIES
        arkbase_static
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)
if (TARGET arkbase_mem_range_tests)
    panda_target_compile_options(arkbase_mem_range_tests PUBLIC "-Wno-unused")
endif()

panda_add_gtest(
    NO_CORES
    NAME arkbase_task_manager_tests
    SOURCES
        tests/taskmanager/task_test.cpp
        tests/taskmanager/task_scheduler_test.cpp
        tests/taskmanager/task_sheduler_logging_test.cpp
    LIBRARIES
        arkbase_static
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)

panda_add_gtest(
    NO_CORES
    NAME arkbase_task_manager_utils_tests
    SOURCES
        tests/taskmanager/task_utils_test.cpp
    LIBRARIES
        arkbase_static
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)

panda_add_gtest(
    NO_CORES
    NAME arkbase_task_manager_gc_cases_tests
    SOURCES
        tests/taskmanager/task_manager_gc_corner_case_test.cpp
    LIBRARIES
        arkbase_static
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)

panda_add_gtest(
    NO_CORES
    NAME lock_holder_tests
    SOURCES
        tests/lock_holder_test.cpp
    LIBRARIES
        arkbase_static
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)

if (DEFINED ENV{GENMC_PATH})
    set(GENMC_TESTS
        ${PANDA_ROOT}/libarkbase/tests/genmc/mutex_test_genmc.cpp
        ${PANDA_ROOT}/libarkbase/tests/genmc/mutex_test_2.cpp
        ${PANDA_ROOT}/libarkbase/tests/genmc/mutex_test_3.cpp
        ${PANDA_ROOT}/libarkbase/tests/genmc/mutex_test_4.cpp
        ${PANDA_ROOT}/libarkbase/tests/genmc/condvar_test_1.cpp
        # Takes too much time #8866
        # ${PANDA_ROOT}/libarkbase/tests/genmc/condvar_test_2.cpp
        ${PANDA_ROOT}/libarkbase/tests/genmc/condvar_test_3.cpp
    )

    foreach(genmc_test ${GENMC_TESTS})
        get_filename_component(test_name ${genmc_test} NAME_WE)
        add_custom_target(genmc_${test_name}
            COMMAND $ENV{GENMC_PATH} "-unroll=10" -- -I"${PANDA_ROOT}" ${genmc_test}
        )
        list(APPEND dep_files genmc_${test_name})
    endforeach()

    add_custom_target(genmc_tests
        DEPENDS ${dep_files}
    )
endif()

if(PANDA_TARGET_32)
    # This test is considered passed as soon as it's built,
    # so it's enough to add building it to the umbrella tests target.
    panda_add_executable(memory_literals_test tests/memory_literals_test.cpp)
    panda_target_link_libraries(memory_literals_test arkbase_static)
    set_target_properties(memory_literals_test
        PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            EXCLUDE_FROM_DEFAULT_BUILD TRUE
            RUNTIME_OUTPUT_DIRECTORY "${PANDA_BINARY_ROOT}/bin-gtests"
    )
    add_dependencies(core_tests memory_literals_test)
    add_test(
        NAME memory_literals_test
        COMMAND ${PANDA_RUN_PREFIX} ${PANDA_BINARY_ROOT}/bin-gtests/memory_literals_test
    )
endif()

panda_gen_options(TARGET arkbase_obj YAML_FILE utils/logger_options.yaml GENERATED_HEADER logger_options.h)

panda_add_sanitizers(TARGET arkbase_obj SANITIZERS ${PANDA_SANITIZERS_LIST})

set_target_properties(arkbase_obj PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

panda_add_library(arkbase_static STATIC $<TARGET_OBJECTS:arkbase_obj>)
panda_target_link_libraries(arkbase_static arkbase_obj)

panda_add_library(arkbase ${PANDA_DEFAULT_LIB_TYPE} $<TARGET_OBJECTS:arkbase_obj>)
panda_target_link_libraries(arkbase arkbase_obj)
