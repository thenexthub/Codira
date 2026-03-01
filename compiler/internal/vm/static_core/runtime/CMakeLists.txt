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

cmake_minimum_required(VERSION 3.10)

project(arkruntime)

enable_language(ASM)

include(core/Core.cmake)
include(${PANDA_ROOT}/verification/Verification.cmake)

set(SOURCES
    assert_gc_scope.cpp
    bridge/bridge.cpp
    entrypoints/entrypoints.cpp
    exceptions.cpp
    class_linker.cpp
    class_linker_extension.cpp
    class_initializer.cpp
    tooling/backtrace/backtrace.cpp
    tooling/backtrace/symbol_extractor.cpp
    tooling/coverage_listener.cpp
    tooling/debugger.cpp
    tooling/evaluation/expression_loader.cpp
    tooling/inspector/debugger_arkapi.cpp
    tooling/inspector/types/profile_result.cpp
    tooling/pt_default_lang_extension.cpp
    tooling/pt_thread.cpp
    tooling/debug_inf.cpp
    tooling/tools.cpp
    tooling/sampler/sampling_profiler.cpp
    tooling/sampler/sample_writer.cpp
    tooling/sampler/samples_record.cpp
    tooling/sampler/thread_communicator.cpp
    tooling/sampler/stack_walker_base.cpp
    tooling/sampler/lock_free_queue.cpp
    field.cpp
    gc_task.cpp
    dprofiler/dprofiler.cpp
    hotreload/hotreload.cpp
    interpreter/interpreter.cpp
    intrinsics.cpp
    coretypes/line_string.cpp
    coretypes/string.cpp
    coretypes/string_flatten.cpp
    coretypes/array.cpp
    class.cpp
    class_helper.cpp
    locks.cpp
    panda_vm.cpp
    language_context.cpp
    mem/gc/epsilon/epsilon.cpp
    mem/gc/epsilon-g1/epsilon-g1.cpp
    mem/gc/epsilon/epsilon_barrier.cpp
    mem/gc/gc.cpp
    mem/gc/gc_adaptive_marking_stack.cpp
    mem/gc/gc_settings.cpp
    mem/gc/lang/gc_lang.cpp
    mem/gc/gc_queue.cpp
    mem/gc/gc_root.cpp
    mem/gc/gc_root_type.cpp
    mem/gc/gc_stats.cpp
    mem/gc/gc_trigger.cpp
    mem/gc/gc-hung/gc_hung.cpp
    mem/gc/card_table.cpp
    mem/gc/heap-space-misc/crossing_map.cpp
    mem/gc/heap-space-misc/crossing_map_singleton.cpp
    mem/gc/bitmap.cpp
    mem/gc/gc_scope.cpp
    mem/gc/gc_scoped_phase.cpp
    mem/gc/workers/gc_worker.cpp
    mem/gc/workers/gc_workers_task_pool.cpp
    mem/gc/workers/gc_workers_task_queue.cpp
    mem/gc/workers/gc_workers_thread_pool.cpp
    mem/gc/stw-gc/stw-gc.cpp
    mem/gc/gc_barrier_set.cpp
    mem/gc/cmc-gc-adapter/cmc-gc-adapter.cpp
    mem/gc/cmc-gc-adapter/cmc-allocator-adapter.cpp
    mem/refstorage/ref_block.cpp
    mem/refstorage/reference_storage.cpp
    mem/refstorage/global_object_storage.cpp
    mem/allocator.cpp
    mem/tlab.cpp
    mem/heap_manager.cpp
    mem/heap_verifier.cpp
    mem/rendezvous.cpp
    mem/runslots.cpp
    mem/region_space.cpp
    mem/object_helpers.cpp
    mem/mem_stats_additional_info.cpp
    mem/mem_stats_default.cpp
    mem/mem_stats.cpp
    mem/internal_allocator.cpp
    mem/panda_string.cpp
    mem/memory_manager.cpp
    mem/heap_space.cpp
    methodtrace/trace.cpp
    mark_word.cpp
    method.cpp
    monitor.cpp
    monitor_object_lock.cpp
    monitor_pool.cpp
    global_object_lock.cpp
    class_lock.cpp
    object_header.cpp
    regexp/ecmascript/regexp_parser.cpp
    regexp/ecmascript/regexp_executor.cpp
    regexp/ecmascript/regexp_opcode.cpp
    regexp/ecmascript/mem/dyn_chunk.cpp
    runtime.cpp
    runtime_controller.cpp
    string_table.cpp
    thread.cpp
    thread_proxy.cpp
    mt_thread_manager.cpp
    lock_order_graph.cpp
    time_utils.cpp
    timing.cpp
    compiler.cpp
    vtable_builder_base.cpp
    imtable_builder.cpp
    cframe.cpp
    stack_walker.cpp
    deoptimization.cpp
    object_accessor.cpp
    osr.cpp
    arch/asm_support.cpp
    signal_handler.cpp
    profilesaver/profile_saver.cpp
    profilesaver/profile_dump_info.cpp
    jit/libprofile/pgo_file_builder.cpp
    jit/profiling_loader.cpp
    jit/profiling_saver.cpp
    jit/profile_saver_worker.cpp
    cha.cpp
    runtime_helpers.cpp
    handle_scope.cpp
    file_manager.cpp
    mem/gc/generational-gc-base.cpp
    mem/gc/g1/g1-gc.cpp
    mem/gc/g1/g1-helpers.cpp
    mem/gc/g1/ref_updater.cpp
    mem/gc/g1/g1-allocator.cpp
    mem/gc/g1/g1_analytics.cpp
    plugins.cpp
    mem/gc/g1/update_remset_worker.cpp
    mem/gc/g1/update_remset_thread.cpp
    mem/gc/g1/update_remset_task_queue.cpp
    mem/gc/g1/g1_pause_tracker.cpp
    mem/gc/g1/hot_cards.cpp
    loadable_agent.cpp
    default_debugger_agent.cpp
    coroutines/coroutine.cpp
    coroutines/threaded_coroutine.cpp
    coroutines/stackful_coroutine.cpp
    coroutines/coroutine_manager.cpp
    coroutines/coroutine_worker.cpp
    coroutines/coroutine_worker_group.cpp
    coroutines/threaded_coroutine_manager.cpp
    coroutines/stackful_coroutine_manager.cpp
    coroutines/stackful_coroutine_worker.cpp
    coroutines/stackful_coroutine_state_info.cpp
    coroutines/native_stack_allocator/native_stack_allocator.cpp
    coroutines/coroutine_events.cpp
    coroutines/coroutine_stats.cpp
    coroutines/priority_queue.cpp
    fibers/fiber_context.cpp
    compiler_thread_pool_worker.cpp
    compiler_task_manager_worker.cpp
)

if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    list(APPEND SOURCES
        fibers/arch/arm/get.S
        fibers/arch/arm/update.S
        fibers/arch/arm/switch.S
        bridge/arch/arm/interpreter_to_compiled_code_bridge_arm.S
        bridge/arch/arm/compiled_code_to_interpreter_bridge_arm.S
        bridge/arch/arm/compiled_code_to_interpreter_bridge_dyn_arm.S
        bridge/arch/arm/interpreter_to_compiled_code_bridge_dyn_arm.S
        bridge/arch/arm/compiled_code_to_runtime_bridge_arm.S
        bridge/arch/arm/expand_compiled_code_args_dyn_arm.S
        bridge/arch/arm/deoptimization_arm.S
        arch/arm/osr_arm.S
        arch/arm/interpreter_support.S)
elseif (PANDA_TARGET_ARM32_ABI_HARD)
    list(APPEND SOURCES
        fibers/arch/arm/get.S
        fibers/arch/arm/update.S
        fibers/arch/arm/switch.S
        bridge/arch/arm/interpreter_to_compiled_code_bridge_armhf.S
        bridge/arch/arm/compiled_code_to_interpreter_bridge_armhf.S
        bridge/arch/arm/compiled_code_to_interpreter_bridge_dyn_arm.S
        bridge/arch/arm/interpreter_to_compiled_code_bridge_dyn_arm.S
        bridge/arch/arm/compiled_code_to_runtime_bridge_arm.S
        bridge/arch/arm/expand_compiled_code_args_dyn_arm.S
        bridge/arch/arm/deoptimization_arm.S
        arch/arm/osr_arm.S
        arch/arm/interpreter_support.S)
elseif(PANDA_TARGET_ARM64)
    list(APPEND SOURCES
        fibers/arch/aarch64/get.S
        fibers/arch/aarch64/update.S
        fibers/arch/aarch64/switch.S
        bridge/arch/aarch64/interpreter_to_compiled_code_bridge_aarch64.S
        bridge/arch/aarch64/compiled_code_to_interpreter_bridge_aarch64.S
        bridge/arch/aarch64/compiled_code_to_interpreter_bridge_dyn_aarch64.S
        bridge/arch/aarch64/interpreter_to_compiled_code_bridge_dyn_aarch64.S
        bridge/arch/aarch64/compiled_code_to_runtime_bridge_aarch64.S
        bridge/arch/aarch64/expand_compiled_code_args_dyn_aarch64.S
        bridge/arch/aarch64/deoptimization_aarch64.S
        arch/aarch64/osr_aarch64.S
        arch/aarch64/interpreter_support.S)
elseif(PANDA_TARGET_X86)
    list(APPEND SOURCES
        bridge/arch/x86/interpreter_to_compiled_code_bridge_x86.S
        bridge/arch/x86/compiled_code_to_interpreter_bridge_x86.S
        bridge/arch/x86/deoptimization_x86.S
        arch/x86/osr_x86.S
        arch/x86/interpreter_support.S)
elseif(PANDA_TARGET_AMD64)
    list(APPEND SOURCES
        fibers/arch/amd64/get.S
        fibers/arch/amd64/update.S
        fibers/arch/amd64/switch.S
        bridge/arch/amd64/compiled_code_to_interpreter_bridge_amd64.S
        bridge/arch/amd64/compiled_code_to_interpreter_bridge_dyn_amd64.S
        bridge/arch/amd64/interpreter_to_compiled_code_bridge_amd64.S
        bridge/arch/amd64/interpreter_to_compiled_code_bridge_dyn_amd64.S
        bridge/arch/amd64/compiled_code_to_runtime_bridge_amd64.S
        bridge/arch/amd64/expand_compiled_code_args_dyn_amd64.S
        bridge/arch/amd64/deoptimization_amd64.S
        arch/amd64/common_amd64.S
        arch/amd64/osr_amd64.S
        arch/amd64/interpreter_support.S)
endif()

if (ENABLE_PERF_COUNTERS)
    list(APPEND SOURCES tooling/perf_counter.cpp)
endif()

if (PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS)
    list(APPEND SOURCES ${PANDA_ROOT}/platforms/mobile/runtime/thread.cpp)
else()
    list(APPEND SOURCES ${PANDA_ROOT}/platforms/common/runtime/thread.cpp)
endif()

if (PANDA_TARGET_MOBILE)
    list(APPEND SOURCES ${PANDA_ROOT}/platforms/mobile/runtime/verify_app_install.cpp)
else()
    list(APPEND SOURCES ${PANDA_ROOT}/platforms/common/runtime/verify_app_install.cpp)
endif()

if (PANDA_WITH_SAFEPOINT_CHECKER)
    list(APPEND SOURCES
        timer_table.cpp
        safepoint_timer.cpp
    )
    add_runtime_options(${CMAKE_CURRENT_SOURCE_DIR}/safepoint_checker_options.yaml)
endif()

set(INTERPRETER_IMPL_SOURCES
    interpreter/interpreter_impl.cpp
)

panda_add_library(arkruntime_interpreter_impl OBJECT ${INTERPRETER_IMPL_SOURCES})
panda_set_lib_32bit_property(arkruntime_interpreter_impl)

set(ISA_TEMPLATES
    interpreter-inl_gen.h.erb
    isa_constants_gen.h.erb
    irtoc_interpreter_utils.h.erb
    debug_test_interpreter-inl_gen.cpp.erb
)

set(GEN_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
file(MAKE_DIRECTORY ${GEN_INCLUDE_DIR})
panda_isa_gen(
    TEMPLATES ${ISA_TEMPLATES}
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/interpreter/templates
    DESTINATION ${GEN_INCLUDE_DIR}
)

panda_gen_file(
        DATA ${CMAKE_BINARY_DIR}/plugin_options.yaml
        TEMPLATE ${CMAKE_CURRENT_LIST_DIR}/interpreter/templates/core_any_intrinsics.inc.erb
        API ${PANDA_ROOT}/templates/plugin_options.rb
        EXTRA_DEPENDENCIES plugin_options_merge
        OUTPUTFILE ${GEN_INCLUDE_DIR}/core_any_intrinsics.inc
)
add_custom_target(core_any_intrinsics_inc DEPENDS ${GEN_INCLUDE_DIR}/core_any_intrinsics.inc)
add_dependencies(arkruntime_interpreter_impl core_any_intrinsics_inc)
add_dependencies(panda_gen_files core_any_intrinsics_inc)

set(ISA "${CMAKE_BINARY_DIR}/isa/isa.yaml")
set(ISA_API "${PANDA_ROOT}/isa/isapi.rb")
set(BRIDGE_DISPATCH_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/templates/bridge_dispatch.S.erb")
set(BRIDGE_DISPATCH_DYN_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/templates/bridge_dispatch_dyn.S.erb")
set(BRIDGE_ARCHS aarch64 arm armhf amd64 x86)
foreach(arch ${BRIDGE_ARCHS})
    panda_gen_file(
        DATA ${ISA}
        TEMPLATE ${BRIDGE_DISPATCH_TEMPLATE}
        OUTPUTFILE ${GEN_INCLUDE_DIR}/bridge_dispatch_${arch}.S
        API ${ISA_API}
        REQUIRES
            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_${arch}.rb
            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_common.rb
            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_static.rb
    )
    add_custom_target(bridge_dispatch_${arch} DEPENDS ${GEN_INCLUDE_DIR}/bridge_dispatch_${arch}.S ${ISA})
    add_dependencies(bridge_dispatch_${arch} isa_assert)

    panda_gen_file(
        DATA ${ISA}
        TEMPLATE ${BRIDGE_DISPATCH_DYN_TEMPLATE}
        OUTPUTFILE ${GEN_INCLUDE_DIR}/bridge_dispatch_dyn_${arch}.S
        API ${ISA_API}
        REQUIRES
            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_${arch}.rb
            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_common.rb
            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_dynamic.rb
    )
    add_custom_target(bridge_dispatch_dyn_${arch} DEPENDS ${GEN_INCLUDE_DIR}/bridge_dispatch_dyn_${arch}.S ${ISA})
    add_dependencies(bridge_dispatch_dyn_${arch} isa_assert)

endforeach()

set(GENERATOR ${ARKFILE_ROOT}/types.rb)
set(TEMPLATE ${CMAKE_CURRENT_LIST_DIR}/templates/shorty_values.h.erb)
set(DATAFILE ${ARKFILE_ROOT}/types.yaml)
set(DEPENDENCIES ${GENERATOR} ${TEMPLATE} ${DATAFILE})
set(OUTFILE "${CMAKE_CURRENT_BINARY_DIR}/include/shorty_values.h")
add_custom_command(OUTPUT "${OUTFILE}"
                   COMMENT "Generate shorty_values.h"
                   COMMAND ${PANDA_ROOT}/isa/gen.rb -d ${DATAFILE} -t ${TEMPLATE} -o "${OUTFILE}" -q ${GENERATOR}
                   DEPENDS ${DEPENDENCIES})
add_custom_target(shorty_values_gen_${PROJECT_NAME} ALL DEPENDS "${OUTFILE}")
add_dependencies(panda_gen_files shorty_values_gen_${PROJECT_NAME})

panda_target_compile_options(arkruntime_interpreter_impl PUBLIC -Wno-invalid-offsetof)

set_property(TARGET arkruntime_interpreter_impl PROPERTY POSITION_INDEPENDENT_CODE ON)

if (PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES)
    if (PANDA_TARGET_ARM64)
        panda_target_compile_options(arkruntime_interpreter_impl PUBLIC -ffixed-x20 -ffixed-x21 -ffixed-x22 -ffixed-x23 -ffixed-x24 -ffixed-x25 -ffixed-x28)
        panda_target_compile_definitions(arkruntime_interpreter_impl PUBLIC FFIXED_REGISTERS)
    endif()

    # Temporary disable clang tidy as it cannot recognize -ffixed-<reg> params that we used. Should be fixed in clang tidy 9
    set_target_properties(arkruntime_interpreter_impl PROPERTIES CXX_CLANG_TIDY "")
endif()

panda_target_include_directories(arkruntime_interpreter_impl
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${PANDA_ROOT}/compiler
    PUBLIC ${PANDA_BINARY_ROOT}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${VERIFIER_INCLUDE_DIR}
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${PANDA_ROOT}/../common_runtime
)

panda_target_include_directories(arkruntime_interpreter_impl
    SYSTEM PRIVATE "$<TARGET_PROPERTY:c_secshared,INCLUDE_DIRECTORIES>"
)

add_dependencies(arkruntime_interpreter_impl
    isa_gen_${PROJECT_NAME}
    ${PROJECT_NAME}_obj_options
    intrinsics_gen_${PROJECT_NAME}
    shorty_values_gen_${PROJECT_NAME}
    arkbase
    arkfile
    entrypoints_gen
)

add_dependencies(arkcompiler entrypoints_gen entrypoints_compiler_checksum_gen)

if (HOST_TOOLS)
    # Host tools should take bitcode compiled for target
    SET_SOURCE_FILES_PROPERTIES(${IRTOC_INT_LLVM_INL_MODULE} PROPERTIES
            EXTERNAL_OBJECT true
            GENERATED true)
    add_custom_target(irtoc_int_llvm_inl_module)
elseif (PANDA_LLVM_INTERPRETER AND PANDA_LLVM_INTERPRETER_INLINING)
    set(sources
            $<TARGET_PROPERTY:arkruntime_obj,SOURCE_DIR>/entrypoints/entrypoints.cpp
            $<TARGET_PROPERTY:arkruntime_obj,SOURCE_DIR>/object_header.cpp
            $<TARGET_PROPERTY:arkruntime_obj,SOURCE_DIR>/mem/heap_manager.cpp
            $<TARGET_PROPERTY:arkruntime_obj,SOURCE_DIR>/mem/tlab.cpp
            $<TARGET_PROPERTY:arkruntime_obj,SOURCE_DIR>/runtime.cpp
            $<TARGET_PROPERTY:arkruntime_obj,SOURCE_DIR>/class_linker.cpp
    )
    set(outputs "")
    panda_add_llvm_bc_lib(TARGET irtoc_int_llvm_inl_module
            SOURCES ${sources}
            INCLUDES $<TARGET_PROPERTY:arkruntime_obj,INCLUDE_DIRECTORIES>
            DEPENDS arkruntime_obj_options
            asm_defines
            compiler_options
            compiler_intrinsics
            cpu_features_gen
            cross_values
            entrypoints_compiler_checksum_gen
            intrinsics_gen_arkruntime
            intrinsics_gen_compiler
            instructions_gen_compiler
            isa_gen_compiler
            profiling_gen
            runtime_intrinsics_inl
            OUTPUT_VARIABLE outputs
    )
    set(IRTOC_INT_LLVM_INL_MODULE "${outputs}")
    # Create the file with a list of bitcode files
    file(GENERATE OUTPUT ${IRTOC_INT_LLVM_INL_MODULE_FILES_LIST} CONTENT "${outputs}")
else()
    add_custom_target(irtoc_int_llvm_inl_module)
endif()

if(NOT (CMAKE_CROSSCOMPILING OR PANDA_TARGET_OHOS))
    set(skip_fastpath_validation FALSE)
    if (IRTOC_TARGET STREQUAL "arm64" AND PANDA_LLVM_INTERPRETER)
        set(skip_fastpath_validation TRUE)
    endif()
    irtoc_compile(TARGET_NAME irtoc_fastpath
                  INPUT_FILES ${IRTOC_SOURCE_DIR}/scripts/common.irt
                              ${IRTOC_SOURCE_DIR}/scripts/resolvers.irt
                              ${IRTOC_SOURCE_DIR}/scripts/allocation.irt
                              ${IRTOC_SOURCE_DIR}/scripts/monitors.irt
                              ${IRTOC_SOURCE_DIR}/scripts/check_cast.irt
                              ${IRTOC_SOURCE_DIR}/scripts/gc.irt
                              ${IRTOC_SOURCE_DIR}/scripts/strings.irt
                              ${IRTOC_SOURCE_DIR}/scripts/string_builder.irt
                              ${IRTOC_SOURCE_DIR}/scripts/string_helpers.irt
                              ${IRTOC_SOURCE_DIR}/scripts/memcopy.irt
                              ${IRTOC_SOURCE_DIR}/scripts/arrays.irt
                  TARGET_VARIABLE IRTOC_FASTPATH_OBJ
                  TARGET_VARIABLE_LLVM IRTOC_FASTPATH_LLVM_OBJ
                  SKIP_VALIDATION ${skip_fastpath_validation})

    irtoc_compile(TARGET_NAME irtoc_interpreter
                  INPUT_FILES ${IRTOC_SOURCE_DIR}/scripts/interpreter.irt
                  TARGET_VARIABLE IRTOC_INTERPRETER_OBJ
                  LLVM_BACKEND_INLINE_MODULE_FILES ${IRTOC_INT_LLVM_INL_MODULE}
                  TARGET_VARIABLE_LLVM IRTOC_INTERPRETER_LLVM_OBJ)
    set(irtoc_fastpath_target irtoc_fastpath)
    set(irtoc_interpreter_target irtoc_interpreter)
else()
    ExternalProject_Get_Property(panda_host_tools binary_dir)
    set(IRTOC_FASTPATH_OBJ "${binary_dir}/irtoc/irtoc_fastpath/irtoc_fastpath.o")
    set(IRTOC_FASTPATH_LLVM_OBJ "${binary_dir}/irtoc/irtoc_fastpath/irtoc_fastpath_llvm.o")
    set(IRTOC_INTERPRETER_OBJ "${binary_dir}/irtoc/irtoc_interpreter/irtoc_interpreter.o")
    set(IRTOC_INTERPRETER_LLVM_OBJ "${binary_dir}/irtoc/irtoc_interpreter/irtoc_interpreter_llvm.o")

    SET_SOURCE_FILES_PROPERTIES(${IRTOC_FASTPATH_OBJ} PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    SET_SOURCE_FILES_PROPERTIES(${IRTOC_FASTPATH_LLVM_OBJ} PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    SET_SOURCE_FILES_PROPERTIES(${IRTOC_INTERPRETER_OBJ} PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    SET_SOURCE_FILES_PROPERTIES(${IRTOC_INTERPRETER_LLVM_OBJ} PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    add_dependencies(host_tools_depends irtoc_int_llvm_inl_module)
    set(irtoc_fastpath_target panda_host_tools-build)
    set(irtoc_interpreter_target panda_host_tools-build)
endif()

set(IRTOC_LLVM_OBJECTS "")

if (PANDA_LLVM_FASTPATH)
    list(APPEND IRTOC_LLVM_OBJECTS ${IRTOC_FASTPATH_LLVM_OBJ})
endif()

if (PANDA_LLVM_INTERPRETER)
    list(APPEND IRTOC_LLVM_OBJECTS ${IRTOC_INTERPRETER_LLVM_OBJ})
endif()

set(ARKRUNTIME_SOURCES
    ${SOURCES}
    ${CORE_VM_SOURCES}
    ${VERIFIER_SOURCES}
)

panda_add_library(arkruntime_obj OBJECT
    ${ARKRUNTIME_SOURCES}
)

set_target_properties(arkruntime_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (PANDA_WITH_HIDDEN_SYMBOLS)
    set_target_properties(arkruntime_obj PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

panda_add_library(arkruntime_interpreter_static STATIC
    $<TARGET_OBJECTS:arkruntime_interpreter_impl>
    ${IRTOC_FASTPATH_OBJ}
    ${IRTOC_INTERPRETER_OBJ}
    ${IRTOC_LLVM_OBJECTS}
)

panda_add_library(arkruntime_static STATIC
    $<TARGET_OBJECTS:arkruntime_obj>
    $<TARGET_OBJECTS:arkruntime_interpreter_impl>
    ${IRTOC_FASTPATH_OBJ}
    ${IRTOC_INTERPRETER_OBJ}
    ${IRTOC_LLVM_OBJECTS}
)
panda_target_link_libraries(arkruntime_static arkruntime_obj)

add_dependencies(arkruntime_interpreter_static
    intrinsics_gen_${PROJECT_NAME}
    isa_gen_pandaverification
    messages_gen_pandaverification
    messages_data_pandaverification
    arkstdlib
    asm_defines_generator
    entrypoints_gen
    profiling_gen
    ${irtoc_fastpath_target}
    ${irtoc_interpreter_target}
)

set(STACK_WALKER_HOOK_BRIDGE_SOURCE "")

if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    list(APPEND STACK_WALKER_HOOK_BRIDGE_SOURCE tests/arch/arm/stack_walker_hook_bridge.S)
elseif (PANDA_TARGET_ARM32_ABI_HARD)
    list(APPEND STACK_WALKER_HOOK_BRIDGE_SOURCE tests/arch/arm/stack_walker_hook_bridge.S)
elseif(PANDA_TARGET_ARM64)
    list(APPEND STACK_WALKER_HOOK_BRIDGE_SOURCE tests/arch/aarch64/stack_walker_hook_bridge.S)
elseif(PANDA_TARGET_AMD64)
    list(APPEND STACK_WALKER_HOOK_BRIDGE_SOURCE tests/arch/amd64/stack_walker_hook_bridge.S)
endif()

panda_add_library(arkruntime_stack_walker_obj OBJECT ${STACK_WALKER_HOOK_BRIDGE_SOURCE})
add_dependencies(arkruntime_stack_walker_obj arkruntime_obj)
panda_target_include_directories(arkruntime_stack_walker_obj
    PUBLIC ${PANDA_BINARY_ROOT}/runtime/asm_defines
    PUBLIC ${PANDA_BINARY_ROOT}/runtime/include
    PUBLIC ${PANDA_ROOT}/runtime
)

panda_target_sources(arkruntime_static PRIVATE ${STACK_WALKER_HOOK_BRIDGE_SOURCE})

panda_add_library(arkruntime SHARED $<TARGET_OBJECTS:arkruntime_obj> $<TARGET_OBJECTS:arkruntime_interpreter_impl>)
panda_target_link_libraries(arkruntime arkruntime_obj arkruntime_interpreter_static)
if (NOT PANDA_USE_PREBUILT_TARGETS)
    add_dependencies(arkruntime arkruntime_static)
endif ()

add_subdirectory(scheduler)
add_subdirectory(asm_defines)

add_dependencies(arkruntime_obj
    intrinsics_gen_${PROJECT_NAME}
    isa_gen_pandaverification
    messages_gen_pandaverification
    messages_data_pandaverification
    arkstdlib
    asm_defines_generator
    entrypoints_gen
    profiling_gen
    ${irtoc_fastpath_target}
    ${irtoc_interpreter_target}
)

if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    add_dependencies(arkruntime_obj bridge_dispatch_arm bridge_dispatch_dyn_arm)
    add_dependencies(panda_gen_files bridge_dispatch_arm bridge_dispatch_dyn_arm)
elseif (PANDA_TARGET_ARM32_ABI_HARD)
    add_dependencies(arkruntime_obj bridge_dispatch_armhf bridge_dispatch_dyn_arm)
    add_dependencies(panda_gen_files bridge_dispatch_armhf bridge_dispatch_dyn_arm)
elseif(PANDA_TARGET_ARM64)
    add_dependencies(arkruntime_obj bridge_dispatch_aarch64 bridge_dispatch_dyn_aarch64)
    add_dependencies(panda_gen_files bridge_dispatch_aarch64 bridge_dispatch_dyn_aarch64)
elseif(PANDA_TARGET_X86)
    add_dependencies(arkruntime_obj bridge_dispatch_x86)
    add_dependencies(panda_gen_files bridge_dispatch_x86)
elseif(PANDA_TARGET_AMD64)
    add_dependencies(arkruntime_obj bridge_dispatch_amd64 bridge_dispatch_dyn_amd64)
    add_dependencies(panda_gen_files bridge_dispatch_amd64 bridge_dispatch_dyn_amd64)
endif()

panda_target_include_directories(arkruntime_obj
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}/runtime/asm_defines
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${VERIFIER_INCLUDE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tooling/inspector
    PUBLIC ${PANDA_ROOT}/../common_runtime
)

panda_target_link_libraries(arkruntime_obj arkbase arkfile arkcompiler dprof arkaotmanager arktarget_options arkassembler)
if (NOT PANDA_TARGET_OHOS)
  panda_target_link_libraries(arkruntime_obj atomic)
endif()

set(CSA_TESTS_ARKRUNTIME_PATH ${GEN_INCLUDE_DIR}/debug_test_interpreter-inl_gen.cpp)
panda_add_library(csa_tests_arkruntime_interpreter_impl OBJECT EXCLUDE_FROM_ALL ${CSA_TESTS_ARKRUNTIME_PATH})
panda_target_include_directories(csa_tests_arkruntime_interpreter_impl
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${PANDA_ROOT}/compiler
    PUBLIC ${PANDA_ROOT}/../common_runtime
    PUBLIC ${PANDA_BINARY_ROOT}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}/compiler/generated
    PUBLIC ${VERIFIER_INCLUDE_DIR}
    PUBLIC ${PANDA_BINARY_ROOT}
)

panda_target_include_directories(csa_tests_arkruntime_interpreter_impl
    SYSTEM PRIVATE "$<TARGET_PROPERTY:c_secshared,INCLUDE_DIRECTORIES>"
)

add_dependencies(csa_tests_arkruntime_interpreter_impl arkruntime_static)

# Runtime uses CodeInfo from the compiler. Maybe in the future CodeInfo
# will be moved to another place, then this line should be removed.
panda_target_include_directories(arkruntime_obj PUBLIC ${PANDA_ROOT}/compiler)

# Disable warning about offsetof usage for non-standard layout types.
# In C++17 offsetof is conditionally implemented for such types, so
# compiler should issue error if it doesn't implemented offsetof for
# them. Also we use static asserts to ensure that offsetof works correcly
# for non-standard layout types.
panda_target_compile_options(arkruntime_obj PUBLIC -Wno-invalid-offsetof)

if(PANDA_WITH_TESTS)
  set(TEST_INTERPRETER_IMPL_SOURCES
    tests/interpreter/test_interpreter_impl.cpp
    )

  panda_add_library(arkruntime_test_interpreter_impl OBJECT ${TEST_INTERPRETER_IMPL_SOURCES})

  panda_target_compile_options(arkruntime_test_interpreter_impl PUBLIC -Wno-invalid-offsetof)
  panda_set_lib_32bit_property(arkruntime_test_interpreter_impl)

  if (PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES)
    if (PANDA_TARGET_ARM64)
      panda_target_compile_options(arkruntime_test_interpreter_impl PUBLIC -ffixed-x20 -ffixed-x21 -ffixed-x22 -ffixed-x23 -ffixed-x24 -ffixed-x25 -ffixed-x28)
      panda_target_compile_definitions(arkruntime_test_interpreter_impl PUBLIC FFIXED_REGISTERS)
    endif()

    # Temporary disable clang tidy as it cannot recognize -ffixed-<reg> params that we used. Should be fixed in clang tidy 9
    set_target_properties(arkruntime_test_interpreter_impl PROPERTIES CXX_CLANG_TIDY "")
  endif()

  panda_target_include_directories(arkruntime_test_interpreter_impl
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${PANDA_ROOT}/compiler
    PUBLIC ${PANDA_BINARY_ROOT}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${VERIFIER_INCLUDE_DIR}
    PUBLIC ${PANDA_ROOT}/../common_runtime
    )

  panda_target_include_directories(arkruntime_test_interpreter_impl
    SYSTEM PUBLIC "$<TARGET_PROPERTY:c_secshared,INCLUDE_DIRECTORIES>"
  )


  if (PANDA_WITH_TESTS)
      panda_target_include_directories(arkruntime_test_interpreter_impl
          SYSTEM PUBLIC "$<TARGET_PROPERTY:gtest,INCLUDE_DIRECTORIES>")
  endif()


  add_dependencies(arkruntime_test_interpreter_impl
    isa_gen_${PROJECT_NAME}
    intrinsics_gen_${PROJECT_NAME}
    ${PROJECT_NAME}_obj_options
    arkbase
    arkfile
    entrypoints_gen
    profiling_gen
    runtime_language_context
    runtime_intrinsics_inl
    source_languages_h
    )
endif()

if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
SET(INVOKE_HELPER tests/arch/arm/invokation_helper.S)
elseif (PANDA_TARGET_ARM32_ABI_HARD)
SET(INVOKE_HELPER tests/arch/arm/invokation_helper_hf.S)
elseif(PANDA_TARGET_ARM64)
SET(INVOKE_HELPER tests/arch/aarch64/invokation_helper.S)
elseif(PANDA_TARGET_X86)
#SET(INVOKE_HELPER tests/arch/x86/invokation_helper.S)
elseif(PANDA_TARGET_AMD64)
SET(INVOKE_HELPER tests/arch/amd64/invokation_helper.S)
endif()

function(add_gtests test_name)
    cmake_parse_arguments(ARG "NO_WARNING_FILTER" "TEST_GROUP;STASH_LIST" "" ${ARGN})
    set(FLAGS_LIST NO_CORES)
    if (ARG_NO_WARNING_FILTER)
        set(FLAGS_LIST "${FLAGS_LIST};NO_WARNING_FILTER")
    endif()
    panda_add_gtest(
        ${FLAGS_LIST}
        NAME ${test_name}
        SOURCES
            ${ARG_UNPARSED_ARGUMENTS}
        LIBRARIES
            arkruntime
            arkassembler
            arkdisassembler
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
        DEPS_TARGETS
            arkstdlib
            ark_aot
        PANDA_STD_LIB
            $<TARGET_PROPERTY:arkstdlib,FILE>
        TEST_GROUP
            ${ARG_TEST_GROUP}
        STASH_LIST
            ${ARG_STASH_LIST}
    )

    if(TARGET ${test_name})
        panda_target_include_directories(${test_name}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            PUBLIC ${PANDA_ROOT}
            PRIVATE ${GEN_INCLUDE_DIR}
        )
        panda_target_compile_options(${test_name} PUBLIC "-Wno-ignored-attributes")
    endif()

endfunction()

if (INTRUSIVE_TESTING)
    if (NOT PANDA_ENABLE_THREAD_SANITIZER)
        message(FATAL_ERROR "Intrusive testing requires enabled thread sanitizer, please set -DPANDA_ENABLE_THREAD_SANITIZER=true.")
    endif()
    if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14))
        message(FATAL_ERROR "Intrusive testing requires compiler clang++-14 or higher.")
    endif()
    set(IS_RUNTIME_INTRUSIVE_BUILD false)
    add_subdirectory(tests/intrusive-tests ${PANDA_BINARY_ROOT}/runtime/tests/intrusive_tests/tests)

    set(IS_RUNTIME_INTRUSIVE_BUILD true)
    add_subdirectory(${PANDA_ROOT}/runtime/tests/intrusive-tests ${CMAKE_BINARY_DIR}/runtime/tests/intrusive_tests)
endif()

function(add_multithread_gtests test_name)
    panda_add_gtest(
        NAME ${test_name}
        SOURCES
            ${ARGN}
        LIBRARIES
            arkruntime
            arkassembler
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )

    if(TARGET ${test_name})
        panda_target_include_directories(${test_name}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            PUBLIC ${PANDA_ROOT}
            PRIVATE ${GEN_INCLUDE_DIR}
        )
        panda_target_compile_options(${test_name} PUBLIC "-Wno-ignored-attributes")
    endif()

endfunction()

add_multithread_gtests (
    arkruntime_multithreaded_test
    tests/multithreaded_intern_string_table_test.cpp
)

add_gtests(
    arkruntime_bitmap_ClearRange_test
    tests/bitmap_clear_range_test.cpp
)

add_gtests(
    arkruntime_bitmap_OrderObjectAlignment_test
    tests/bitmap_order_object_alignment_test.cpp
)

add_gtests(
    arkruntime_bitmap_VisitorObjectAlignment_test
    tests/bitmap_visitor_object_alignment_test.cpp
)

add_gtests(
    arkruntime_bitmap_PageAlignment_test
    tests/bitmap_page_alignment_test.cpp
)

add_gtests(
    arkruntime_fibers_test
    tests/fibers_test.cpp
    NO_WARNING_FILTER
)

set(LAYOUT_TESTS_SOURCES
    tests/array_test.cpp
    tests/mark_word_test.cpp
    tests/compiler_thread_pool.cpp
    tests/mock_queue_thread_pool.cpp
    tests/string_table_test.cpp
    tests/string_test.cpp
    tests/compilation_status_test.cpp
    tests/osr_code_test.cpp
)
if (NOT PANDA_QEMU_BUILD OR NOT PANDA_TARGET_ARM32)
    # NOTE: not enough user space for class allocation after enabling AOT veifier by default.
    # Issue: #29519
    list(APPEND LAYOUT_TESTS_SOURCES tests/method_test.cpp)
endif()

add_gtests(
    arkruntime_core_layout_test
    ${LAYOUT_TESTS_SOURCES}
)

add_gtests(
    arkruntime_get_method_test
    tests/getmethod_test.cpp
)

add_gtests(
    arkruntime_memory_mem_leak_test
    tests/mem_leak_test.cpp
)

add_gtests(
    arkruntime_native_stack_allocator_test
    tests/native_stack_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_statistic_test
    tests/histogram_test.cpp
    tests/mem_stats_additional_info_test.cpp
    tests/mem_stats_gc_test.cpp
    tests/mem_stats_test.cpp
)

if (NOT ENABLE_UNIT_TESTS_FULL_COVERAGE)
    add_gtests(
        arkruntime_memory_management_bump_allocator_test
        tests/bump_allocator_test.cpp
    )
endif()

add_gtests(
    arkruntime_memory_management_frame_allocators_test
    tests/frame_allocator_test.cpp
    tests/stack_like_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_freelist_allocator_test
    tests/freelist_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_humongous_obj_allocator_test
    tests/humongous_obj_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_internal_allocator_test
    tests/internal_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_malloc-proxy-allocator-test
    tests/malloc-proxy-allocator-test.cpp
)

add_gtests(
    arkruntime_memory_management_pygote_space_allocator_stw_test
    tests/pygote_space_allocator_test_base.cpp
    tests/pygote_space_allocator_stw_test.cpp
)

add_gtests(
    arkruntime_memory_management_region_allocator_test
    tests/region_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_rem_set_test
    tests/rem_set_test.cpp
)

add_gtests(
    arkruntime_memory_management_runslots_allocator_test
    tests/runslots_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_test_2
    tests/card_table_test.cpp
    tests/crossing_map_test.cpp
    tests/panda_smart_pointers_test.cpp
    tests/panda_tl_containers_test.cpp
    tests/tlab_test.cpp
    tests/gc_task_test.cpp
    tests/epsilon_gcs_test.cpp
    tests/g1gc_test.cpp
    tests/gc_root_test.cpp
    tests/heap_verifier_test.cpp
    tests/collection_set_test.cpp
    tests/g1gc_fullgc_test.cpp
    tests/object_helpers_test.cpp
    tests/gc_log_test.cpp
    tests/explicit_gc_test.cpp
    tests/g1_pause_tracker_test.cpp
    tests/g1_analytics_test.cpp
)

add_gtests(
    arkruntime_sa_object_helpers_test
    tests/static_analyzer_test.cpp
)

add_gtests(
    arkruntime_gc_trigger_test
    tests/gc_trigger_test.cpp
)

add_gtests(
    arkruntime_memory_management_heap_space_test
    tests/heap_space_test.cpp
)

set(MULTITHREADING_TESTS_SRC
    tests/compiler_queue_test.cpp
    tests/monitor_test.cpp
)

if (NOT ENABLE_UNIT_TESTS_FULL_COVERAGE)
    list(APPEND MULTITHREADING_TESTS_SRC
        tests/thread_test.cpp
    )
endif()

add_gtests(
    arkruntime_multithreading_test
    ${MULTITHREADING_TESTS_SRC}
)

add_gtests(
    arkruntime_options_test
    tests/options_test.cpp
)

add_gtests(
    arkruntime_signal_test
    tests/signal_handler_test.cpp
)

add_gtests(
    arkruntime_regexp_test
    tests/regexp_test.cpp
    tests/dyn_buffer_test.cpp
)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND NOT PANDA_TARGET_ARM32)
    add_gtests(
        arkruntime_getclass_cache_collision_test
        tests/getclass_cache_collision_test.cpp
    )
endif()

# We run irtoc tests only in host mode, because irtoc tests are intended for testing only Irtoc language capabilities.
if (PANDA_WITH_TESTS OR HOST_TOOLS)
    if (NOT CMAKE_CROSSCOMPILING)
        irtoc_compile(TARGET_NAME irtoc_tests
            INPUT_FILES ${IRTOC_SOURCE_DIR}/scripts/tests.irt
            TARGET_VARIABLE IRTOC_TESTS_OBJ)
        set(irtoc_tests_target irtoc_tests)
    else()
        ExternalProject_Get_Property(panda_host_tools binary_dir)
        set(IRTOC_TESTS_OBJ "${binary_dir}/irtoc/irtoc_tests/irtoc_tests.o")

        SET_SOURCE_FILES_PROPERTIES(${IRTOC_TESTS_OBJ} PROPERTIES
            EXTERNAL_OBJECT true
            GENERATED true
        )
        # Due to cmake complain about absence of the irtoc file, we just fake it until it will be generated
        file(MAKE_DIRECTORY ${binary_dir}/irtoc/irtoc_tests)
        if (NOT EXISTS ${IRTOC_TESTS_OBJ})
            file(TOUCH ${IRTOC_TESTS_OBJ})
        endif()
        set(irtoc_tests_target panda_host_tools-build)
    endif()
    # In Host tools mode we only generate Irtoc files for irtoc_tests, we shouldn't create irtoc_tests itself.
    if (irtoc_tests_target AND NOT HOST_TOOLS)
        add_gtests(
            arkruntime_irtoc_tests
            tests/irtoc_test.cpp
            ${IRTOC_TESTS_OBJ}
        )
        add_dependencies(arkruntime_irtoc_tests ${irtoc_tests_target})
    endif()
endif()

if (NOT PANDA_PRODUCT_BUILD)
  add_gtests(
    arkruntime_intrinsics_blacklist_test
    tests/intrinsics_blacklist_test.cpp
    )
endif()

if (TARGET arkruntime_test_interpreter_impl)
    set(INTERPRETER_TESTS_SOURCES
        arkruntime_interpreter_test
        tests/c2i_bridge_test.cpp
        tests/class_linker_test.cpp
        tests/class_linker_test_extension.cpp
        tests/class_refflag_test.cpp
        tests/interpreter/test_interpreter.cpp
        tests/interpreter/test_runtime_interface.cpp
        tests/interpreter_test.cpp
        tests/interpreter_test_switch.cpp
        tests/invokation_helper.cpp
    )
    if (NOT PANDA_QEMU_BUILD OR NOT PANDA_TARGET_ARM32)
        # NOTE: not enough user space for class allocation after enabling AOT veifier by default.
        # Issue: #29519
        list(APPEND INTERPRETER_TESTS_SOURCES tests/exception_test.cpp)
    endif()

    add_gtests(
        ${INTERPRETER_TESTS_SOURCES}
        $<TARGET_OBJECTS:arkruntime_test_interpreter_impl>
        ${INVOKE_HELPER}
    )
endif()

# In interpreter_test.cpp tests ResolveCtorClass and
# ResolveField are extremely slow (that lead to timeout in arm32 debug builds).
# To avoid timeout these tests were moved to special targets
# and code generation that needed in tests is made in build time
add_gtests(
    interpreter_test_resolve_field
    tests/interpreter_test_resolve_field.cpp
)

add_gtests(
    arkruntime_interpreter_frame_test
    tests/interpreter_frame_test.cpp
    ${INVOKE_HELPER}
)

add_gtests(
    interpreter_test_resolve_ctor_class
    tests/interpreter_test_resolve_ctor_class.cpp
)

add_gtests(
    arkruntime_test_classhelper
    tests/class_helper_test.cpp
)

if (PANDA_WITH_TESTS)
    file(MAKE_DIRECTORY ${PANDA_BINARY_ROOT}/bin-gtests/pre-build)

    function(generate_gtests_prebuild test_name)
        set(SOURCE_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/tests/templates/${test_name}.pa.erb")
        set(GENERATED_OUTPUT "${PANDA_BINARY_ROOT}/bin-gtests/pre-build/${test_name}")

        if(CMAKE_CROSSCOMPILING)
            ExternalProject_Get_Property(panda_host_tools binary_dir)
            set(assembler_target panda_host_tools-build)
            set(assembler_bin    "${binary_dir}/assembler/ark_asm")
        else()
            set(assembler_target ark_asm)
            set(assembler_bin    $<TARGET_FILE:${assembler_target}>)
        endif()

        add_custom_command(OUTPUT ${GENERATED_OUTPUT}.abc
            COMMENT "Generating ${test_name}.abc"
            COMMAND erb ${SOURCE_TEMPLATE} > ${GENERATED_OUTPUT}.pa
            COMMAND ${assembler_bin} ${GENERATED_OUTPUT}.pa ${GENERATED_OUTPUT}.abc
            DEPENDS ${assembler_target}
        )

        add_custom_target(${test_name}_abc
            DEPENDS ${GENERATED_OUTPUT}.abc
        )

        add_dependencies(${test_name}_gtests ${test_name}_abc)
    endfunction()

    generate_gtests_prebuild(interpreter_test_resolve_field)
    generate_gtests_prebuild(interpreter_test_resolve_ctor_class)
endif()

add_gtests(
    arkruntime_utils_test
    tests/class_size_test.cpp
    tests/coverage_listener_test.cpp
    tests/debugger_test.cpp
    tests/frame_test.cpp
    tests/i2c_bridge_test.cpp
    tests/math_helpers_test.cpp
    tests/stack_walker_test.cpp
    tests/time_utils_test.cpp
    ${INVOKE_HELPER}
    ${PANDA_ROOT}/compiler/tests/panda_runner.cpp
    $<TARGET_OBJECTS:arkruntime_stack_walker_obj>
)

add_gtests(
    arkruntime_flattened_string_cache_test
    tests/flattened_string_cache_test.cpp
)

include(intrinsics.cmake)
include(icu.cmake)

add_subdirectory(tooling/inspector)

if(PANDA_WITH_TESTS)
    add_subdirectory(tests/tooling)
endif()

add_subdirectory(profiling)

panda_add_sanitizers(TARGET arkruntime_obj SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET arkruntime_interpreter_impl SANITIZERS ${PANDA_SANITIZERS_LIST})
if(PANDA_WITH_TESTS AND TARGET arkruntime_test_interpreter_impl)
    panda_add_sanitizers(TARGET arkruntime_test_interpreter_impl SANITIZERS ${PANDA_SANITIZERS_LIST})
endif()
