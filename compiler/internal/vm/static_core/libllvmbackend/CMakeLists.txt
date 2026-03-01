# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

project(llvmbackend)

if (NOT PANDA_BUILD_LLVM_BACKEND)
    message(FATAL_ERROR "PANDA_BUILD_LLVM_BACKEND must be true")
endif()

# CC-OFFNXT(bc-40028) false positive
include(${PANDA_ROOT}/compiler/cmake/target.cmake)

if(ENABLE_COMPILER_COVERAGE)
    # Set coverage options
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

set(SOURCES
        llvm_ark_interface.cpp
        llvm_compiler.cpp
        llvm_irtoc_compiler.cpp
        llvm_logger.cpp
        llvm_options.cpp
        lowering/debug_data_builder.cpp
        lowering/gc_barriers.cpp
        lowering/irtoc_function_utils.cpp
        lowering/llvm_ir_constructor.cpp
        lowering/wrapped_module.cpp
        mir_compiler.cpp
        object_code/ark_aot_linker.cpp
        object_code/code_info_producer.cpp
        object_code/created_object_file.cpp
        object_code/dump.cpp
        target_machine_builder.cpp
        transforms/builtins.cpp
        transforms/gc_utils.cpp
        transforms/llvm_optimizer.cpp
        transforms/passes/aarch64_fixup_sdiv.cpp
        transforms/passes/ark_frame_lowering/frame_builder.cpp
        transforms/passes/ark_frame_lowering/frame_lowering.cpp
        transforms/passes/ark_gvn.cpp
        transforms/passes/ark_inlining.cpp
        transforms/passes/check_external.cpp
        transforms/passes/check_tail_calls.cpp
        transforms/passes/devirt.cpp
        transforms/passes/expand_atomics.cpp
        transforms/passes/fixup_poisons.cpp
        transforms/passes/gc_intrusion.cpp
        transforms/passes/gc_intrusion_check.cpp
        transforms/passes/gep_propagation.cpp
        transforms/passes/infer_flags.cpp
        transforms/passes/inline_devirt.cpp
        transforms/passes/inline_ir/cleanup_inline_module.cpp
        transforms/passes/inline_ir/discard_inline_module.cpp
        transforms/passes/inline_ir/inline_ir_utils.cpp
        transforms/passes/inline_ir/mark_always_inline.cpp
        transforms/passes/inline_ir/mark_inline_module.cpp
        transforms/passes/inline_ir/patch_return_handler_stack_adjustment.cpp
        transforms/passes/inline_ir/remove_unused_functions.cpp
        transforms/passes/insert_safepoints.cpp
        transforms/passes/intrinsics_lowering.cpp
        transforms/passes/loop_peeling.cpp
        transforms/passes/mem_barriers.cpp
        transforms/passes/panda_runtime_lowering.cpp
        transforms/passes/propagate_lenarray.cpp
        transforms/passes/prune_deopt.cpp
        transforms/runtime_calls.cpp
        utils.cpp
)

include(cmake/CommonDefines.cmake)

if (PANDA_LLVM_AOT)
    list(APPEND SOURCES
        llvm_aot_compiler.cpp
    )
endif()

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(PIPELINE_CFG ${CMAKE_CURRENT_LIST_DIR}/transforms/pipeline.cfg)
set(PIPELINE_GEN_INC ${GENERATED_DIR}/pipeline_gen.inc)
add_custom_command(
    OUTPUT ${PIPELINE_GEN_INC}
    COMMAND ruby ${CMAKE_CURRENT_LIST_DIR}/templates/pipeline.rb
    -d ${PIPELINE_CFG}
    -o ${PIPELINE_GEN_INC}
    DEPENDS ${PIPELINE_CFG}
)
add_custom_target(llvmbackend_pipeline_gen DEPENDS ${PIPELINE_CFG} ${PIPELINE_GEN_INC})

set(PIPELINE_IRTOC_CFG ${CMAKE_CURRENT_LIST_DIR}/transforms/pipeline_irtoc.cfg)
set(PIPELINE_IRTOC_GEN_INC ${GENERATED_DIR}/pipeline_irtoc_gen.inc)
add_custom_command(
    OUTPUT ${PIPELINE_IRTOC_GEN_INC}
    COMMAND ruby ${CMAKE_CURRENT_LIST_DIR}/templates/pipeline.rb
    -d ${PIPELINE_IRTOC_CFG}
    -o ${PIPELINE_IRTOC_GEN_INC}
    -s "_IRTOC"
    DEPENDS ${PIPELINE_IRTOC_CFG}
)
add_custom_target(llvmbackend_pipeline_irtoc_gen DEPENDS ${PIPELINE_IRTOC_CFG} ${PIPELINE_IRTOC_GEN_INC})

panda_gen(DATA ${CMAKE_CURRENT_LIST_DIR}/transforms/passes/passes.yaml
    TEMPLATES llvm_passes.inl.erb
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION ${GENERATED_DIR}
    API ${CMAKE_CURRENT_LIST_DIR}/templates/llvm_pass.rb
    TARGET_NAME llvmbackend_passes_gen
)

set(OPTIONS_GEN_H ${GENERATED_DIR}/llvm_options_gen.h)
panda_gen_file(
        DATA ${CMAKE_CURRENT_LIST_DIR}/llvmbackend.yaml
        TEMPLATE ${PANDA_ROOT}/templates/options/options.h.erb
        OUTPUTFILE ${OPTIONS_GEN_H}
        API ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(llvmbackend_options_gen DEPENDS ${OPTIONS_GEN_H})

panda_gen(DATA ${PANDA_BINARY_ROOT}/runtime/intrinsics.yaml
    TEMPLATES intrinsics_gen.inl.erb
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION ${GENERATED_DIR}
    API ${CMAKE_CURRENT_LIST_DIR}/templates/intrinsics.rb
    REQUIRES ${PANDA_ROOT}/libarkbase/utils.rb
    EXTRA_DEPENDENCIES arkruntime_gen_intrinsics_yaml
)

panda_gen(DATA ${PANDA_BINARY_ROOT}/runtime/intrinsics.yaml
    TEMPLATES can_compile_intrinsics_gen.inl.erb
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION ${GENERATED_DIR}
    API ${CMAKE_CURRENT_LIST_DIR}/templates/intrinsics.rb
    REQUIRES ${PANDA_ROOT}/libarkbase/utils.rb
    EXTRA_DEPENDENCIES arkruntime_gen_intrinsics_yaml
    TARGET_NAME can_compile_intrinsics_gen_llvmbackend
)

panda_gen(DATA ${PANDA_BINARY_ROOT}/runtime/intrinsics.yaml
    TEMPLATES intrinsic_names_gen.inl.erb
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION ${GENERATED_DIR}
    API ${CMAKE_CURRENT_LIST_DIR}/templates/intrinsics.rb
    REQUIRES ${PANDA_ROOT}/libarkbase/utils.rb
    EXTRA_DEPENDENCIES arkruntime_gen_intrinsics_yaml
    TARGET_NAME intrinsic_names_gen_llvmbackend
)

panda_gen(DATA ${PANDA_BINARY_ROOT}/runtime/intrinsics.yaml
    TEMPLATES intrinsics_llvm_codegen.inl.erb
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION ${GENERATED_DIR}
    API ${CMAKE_CURRENT_LIST_DIR}/templates/intrinsics.rb
    REQUIRES ${PANDA_ROOT}/libarkbase/utils.rb
    EXTRA_DEPENDENCIES arkruntime_gen_intrinsics_yaml
    TARGET_NAME intrinsic_llvm_codegen_llvmbackend
)

panda_gen(DATA ${PANDA_BINARY_ROOT}/runtime/entrypoints.yaml
    TEMPLATES entrypoints_gen.inl.erb
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION ${GENERATED_DIR}
    API ${CMAKE_CURRENT_LIST_DIR}/templates/entrypoints.rb
    EXTRA_DEPENDENCIES entrypoints_yaml_merge
)

panda_gen(DATA ${PANDA_BINARY_ROOT}/runtime/entrypoints.yaml
    TEMPLATES entrypoints_llvm_ark_interface_gen.inl.erb
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/templates
    DESTINATION ${GENERATED_DIR}
    API ${CMAKE_CURRENT_LIST_DIR}/templates/entrypoints.rb
    EXTRA_DEPENDENCIES entrypoints_yaml_merge
    TARGET_NAME entrypoints_llvm_ark_interface_gen_llvmbackend
)

set(LOGGER_COMPONENTS_GEN_H ${GENERATED_DIR}/llvm_logger_components.inc)
panda_gen_file(
    DATA ${CMAKE_CURRENT_LIST_DIR}/llvmbackend.yaml
    TEMPLATE ${PANDA_ROOT}/templates/logger_components/logger_components.inc.erb
    OUTPUTFILE ${LOGGER_COMPONENTS_GEN_H}
    API ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(llvmbackend_logger_components_gen DEPENDS ${LOGGER_COMPONENTS_GEN_H})

panda_add_library(llvmbackend SHARED ${SOURCES})

add_custom_target(llvmbackend_gen_files COMMENT "Umbrella target for llvmbackend generated files")
add_dependencies(llvmbackend_gen_files
    llvmbackend_pipeline_gen
    llvmbackend_pipeline_irtoc_gen
    llvmbackend_options_gen
    llvmbackend_passes_gen
    entrypoints_gen_llvmbackend
    intrinsics_gen_llvmbackend
    can_compile_intrinsics_gen_llvmbackend
    intrinsic_names_gen_llvmbackend
    intrinsic_llvm_codegen_llvmbackend
    entrypoints_llvm_ark_interface_gen_llvmbackend
    llvmbackend_logger_components_gen
)

add_dependencies(llvmbackend llvmbackend_gen_files copy-libLLVM.so)
add_dependencies(panda_gen_files llvmbackend_gen_files)

panda_target_include_directories(llvmbackend SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
panda_target_include_directories(llvmbackend PRIVATE .)

panda_target_include_directories(llvmbackend
        PRIVATE ${PANDA_ROOT}/compiler
        PUBLIC ${GENERATED_DIR}
        PUBLIC ${PANDA_ROOT}/../common_runtime
)

panda_target_link_libraries(llvmbackend arkcompiler arkbase arkfile aot_builder ${LIB_LLVM})

if (NOT (PANDA_TARGET_MOBILE))
    panda_target_link_libraries(llvmbackend rt)
endif()

panda_target_compile_options(llvmbackend PUBLIC "-Wno-unused-parameter")

panda_target_compile_definitions(llvmbackend PUBLIC REQUIRED_LLVM_VERSION=${REQUIRED_LLVM_VERSION})

# LLVM_DEFINITIONS is defined in ${LLVM_TARGET_PATH}/lib/cmake/llvm/LLVMConfig.cmake.
separate_arguments(LLVM_DEFINITIONS)
panda_target_compile_definitions(llvmbackend PRIVATE ${LLVM_DEFINITIONS})

if (PANDA_LLVM_AOT AND PANDA_TARGET_AMD64 AND PANDA_COMPILER_TARGET_AARCH64 AND PANDA_WITH_TESTS)
    set(PANDA_LLVM_AOT_TESTS_SOURCES
        ${PANDA_ROOT}/compiler/tests/unit_test.cpp
        ${PANDA_ROOT}/compiler/tests/inst_generator.cpp
        tests/inst_generator_test.cpp
    )
    set(PANDA_LLVM_AOT_TESTS_LIBRARIES arkcompiler arkbase arkassembler arkruntime llvmbackend)

    panda_add_gtest(
        CONTAINS_MAIN
        NAME llvm_aot_unit_tests
        SOURCES
        ${PANDA_LLVM_AOT_TESTS_SOURCES}
        LIBRARIES
        ${PANDA_LLVM_AOT_TESTS_LIBRARIES}
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )

    panda_target_include_directories(llvm_aot_unit_tests SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
    panda_target_include_directories(llvm_aot_unit_tests PRIVATE .)
    panda_target_include_directories(llvm_aot_unit_tests PRIVATE ${PANDA_ROOT}/compiler
        PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_INCLUDE_DIRECTORIES>")
    panda_target_compile_options(llvm_aot_unit_tests PUBLIC "-Wno-unused-parameter")
endif()
