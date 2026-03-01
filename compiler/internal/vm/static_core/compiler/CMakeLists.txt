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

project(compiler)

include(cmake/coverage.cmake)

if(PANDA_COMPILER_ENABLE)
    include(cmake/benchmark_coverage.cmake)
    include(cmake/ir_builder_coverage.cmake)
endif()

include(cmake/target.cmake)

include_directories(
    .
    ${PANDA_ROOT}
    ${PANDA_ROOT}/../common_runtime
)

include_directories(AFTER ${PANDA_BINARY_ROOT}/cross_values)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(IRTOC_BUILDER_GEN ${GENERATED_DIR}/irtoc_builder.cpp)
irtoc_generate(
    TARGET irtoc_generate_inst_builder
    IR_API ir-builder
    INPUT_FILES ${IRTOC_SOURCE_DIR}/scripts/interpreter.irt
    OUTPUT_FILES ${IRTOC_BUILDER_GEN}
    WORKING_DIRECTORY ${IRTOC_BUILD_DIR}/irtoc_inst_builder
)

add_dependencies(irtoc_generate_inst_builder asm_defines)

set(IRTOC_IR_INLINE_GEN ${GENERATED_DIR}/irtoc_ir_inline.h)
irtoc_generate(
    TARGET irtoc_generate_ir_inline
    IR_API ir-inline
    INPUT_FILES ${IRTOC_SOURCE_DIR}/scripts/interpreter.irt
    OUTPUT_FILES ${IRTOC_IR_INLINE_GEN}
    WORKING_DIRECTORY ${IRTOC_BUILD_DIR}/irtoc_ir_inline
)
add_dependencies(irtoc_generate_ir_inline asm_defines)

set(COMPILER_TEMPLATES_DIR ${CMAKE_CURRENT_LIST_DIR}/optimizer/templates)

add_custom_target(inst_templates_gen)
set_target_properties(inst_templates_gen PROPERTIES INST_TEMPLATES_YAML_FILES ${CMAKE_CURRENT_LIST_DIR}/optimizer/ir_builder/inst_templates.yaml)
set(INST_TEMPLATES_GEN_YAML ${GENERATED_DIR}/inst_templates.yaml)
set_target_properties(inst_templates_gen PROPERTIES INST_TEMPLATES_GEN_YAML ${INST_TEMPLATES_GEN_YAML})

function(add_inst_templates YAML_FILE_PATH)
    get_target_property(YAML_FILES inst_templates_gen INST_TEMPLATES_YAML_FILES)
    list(APPEND YAML_FILES ${YAML_FILE_PATH})
    set_target_properties(inst_templates_gen PROPERTIES INST_TEMPLATES_YAML_FILES "${YAML_FILES}")
endfunction()

set(INST_BUILDER_GEN ${GENERATED_DIR}/inst_builder_gen.cpp)
configure_file(${COMPILER_TEMPLATES_DIR}/inst_builder_gen.cpp.erb ${GENERATED_DIR}/inst_builder_gen.cpp.erb @ONLY)

panda_isa_gen(
        TEMPLATES
            "inst_builder_gen.cpp.erb"
        SOURCE ${GENERATED_DIR}
        REQUIRES ${PANDA_ROOT}/assembler/asm_isapi.rb
        DESTINATION ${GENERATED_DIR}
        EXTRA_DEPENDENCIES inst_templates_merge ${INST_TEMPLATES_GEN_YAML}
        )

panda_gen(DATA ${PANDA_BINARY_ROOT}/runtime/intrinsics.yaml
        TEMPLATES
            intrinsics_enum.inl.erb
            get_intrinsics.inl.erb
            entrypoints_bridge_asm_macro.inl.erb
            intrinsics_ir_build.inl.erb
            intrinsics_flags.inl.erb
            get_intrinsics_names.inl.erb
            generate_operations_intrinsic_inst.inl.erb
            generate_operations_intrinsic_graph.inl.erb
            intrinsic_codegen_test.inl.erb
            intrinsic_flags_test.inl.erb
            can_encode_builtin.inl.erb
            intrinsics_codegen.inl.h.erb
            intrinsics_codegen.inl.erb
            intrinsics_inline.inl.erb
            intrinsics_peephole.inl.erb
            intrinsic_string_flat_check.inl.erb
        SOURCE ${COMPILER_TEMPLATES_DIR}/intrinsics
        DESTINATION ${GENERATED_DIR}
        API ${COMPILER_TEMPLATES_DIR}/intrinsics/compiler_intrinsics.rb
        REQUIRES ${PANDA_ROOT}/libarkbase/utils.rb
        EXTRA_DEPENDENCIES arkruntime_gen_intrinsics_yaml
        )

panda_gen(DATA ${CMAKE_CURRENT_LIST_DIR}/optimizer/ir/instructions.yaml
        TEMPLATES
            opcodes.h.erb
            inst_flags.inl.erb
            arch_info_gen.h.erb
            inst_checker_gen.h.erb
            IR-instructions.md.erb
            codegen_arm64_gen.inc.erb
            savestate_optimization_call_visitors.inl.erb
        SOURCE ${COMPILER_TEMPLATES_DIR}
        DESTINATION ${GENERATED_DIR}
        API ${COMPILER_TEMPLATES_DIR}/instructions.rb
        )

set(COMPILER_EVENTS_GEN_H ${GENERATED_DIR}/compiler_events_gen.h)
panda_gen_file(
    DATA ${CMAKE_CURRENT_LIST_DIR}/compiler.yaml
    TEMPLATE ${PANDA_ROOT}/templates/events/events.h.erb
    OUTPUTFILE ${COMPILER_EVENTS_GEN_H}
    API ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(compiler_events_gen DEPENDS ${COMPILER_EVENTS_GEN_H})

set(COMPILER_LOGGER_COMPONENTS_GEN_H ${GENERATED_DIR}/compiler_logger_components.inc)
panda_gen_file(
    DATA ${CMAKE_CURRENT_LIST_DIR}/compiler.yaml
    TEMPLATE ${PANDA_ROOT}/templates/logger_components/logger_components.inc.erb
    OUTPUTFILE ${COMPILER_LOGGER_COMPONENTS_GEN_H}
    API ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(compiler_logger_components_gen DEPENDS ${COMPILER_LOGGER_COMPONENTS_GEN_H})

set(CPU_FEATURES_GEN_H ${GENERATED_DIR}/cpu_features.inc)
panda_gen_file(
    DATA ${CMAKE_CURRENT_LIST_DIR}/compiler.yaml
    TEMPLATE ${PANDA_ROOT}/templates/cpu_features.inc.erb
    OUTPUTFILE ${CPU_FEATURES_GEN_H}
    API ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(cpu_features_gen DEPENDS ${CPU_FEATURES_GEN_H})

add_dependencies(panda_gen_files
    compiler_events_gen
    compiler_logger_components_gen
    cpu_features_gen
)

set(OPTIMIZER_SOURCES
        optimizer/pass.cpp
        optimizer/pass_manager.cpp
        optimizer/pass_manager_statistics.cpp
        optimizer/pipeline.cpp
        optimizer/analysis/alias_analysis.cpp
        optimizer/analysis/alias_visitor.cpp
        optimizer/analysis/bounds_analysis.cpp
        optimizer/analysis/countable_loop_parser.cpp
        optimizer/analysis/catch_inputs.cpp
        optimizer/analysis/dominators_tree.cpp
        optimizer/analysis/linear_order.cpp
        optimizer/analysis/liveness_analyzer.cpp
        optimizer/analysis/liveness_use_table.cpp
        optimizer/analysis/live_in_analysis.cpp
        optimizer/analysis/live_registers.cpp
        optimizer/analysis/loop_analyzer.cpp
        optimizer/analysis/monitor_analysis.cpp
        optimizer/analysis/object_type_propagation.cpp
        optimizer/analysis/rpo.cpp
        optimizer/analysis/reg_alloc_verifier.cpp
        optimizer/analysis/types_analysis.cpp
        optimizer/ir/analysis.cpp
        optimizer/ir/basicblock.cpp
        optimizer/ir/dump.cpp
        optimizer/ir/graph.cpp
        optimizer/ir/inst.cpp
        optimizer/ir/locations.cpp
        optimizer/ir/visualizer_printer.cpp
        optimizer/ir/graph_checker.cpp
        optimizer/ir/graph_cloner.cpp
        optimizer/ir/loop_unswitcher.cpp
        optimizer/optimizations/adjust_arefs.cpp
        optimizer/optimizations/balance_expressions.cpp
        optimizer/optimizations/branch_elimination.cpp
        optimizer/optimizations/checks_elimination.cpp
        optimizer/optimizations/code_sink.cpp
        optimizer/optimizations/const_folding.cpp
        optimizer/optimizations/deoptimize_elimination.cpp
        optimizer/optimizations/escape.cpp
        optimizer/optimizations/cleanup.cpp
        optimizer/optimizations/if_conversion.cpp
        optimizer/optimizations/if_merging.cpp
        optimizer/optimizations/licm.cpp
        optimizer/optimizations/licm_conditions.cpp
        optimizer/optimizations/condition_chain_manager.cpp
        optimizer/optimizations/condition_chain_cache.cpp
        optimizer/optimizations/locations_builder.cpp
        optimizer/optimizations/loop_idioms.cpp
        optimizer/optimizations/loop_peeling.cpp
        optimizer/optimizations/loop_unswitch.cpp
        optimizer/optimizations/loop_unroll.cpp
        optimizer/optimizations/lse.cpp
        optimizer/optimizations/lower_boxed_boolean.cpp
        optimizer/optimizations/memory_barriers.cpp
        optimizer/optimizations/memory_coalescing.cpp
        optimizer/optimizations/native_call_optimization.cpp
        optimizer/optimizations/object_type_check_elimination.cpp
        optimizer/optimizations/optimize_string_concat.cpp
        optimizer/optimizations/peepholes.cpp
        optimizer/optimizations/phi_type_resolving.cpp
        optimizer/optimizations/redundant_loop_elimination.cpp
        optimizer/optimizations/reserve_string_builder_buffer.cpp
        optimizer/optimizations/savestate_optimization.cpp
        optimizer/optimizations/scheduler.cpp
        optimizer/optimizations/select_optimization.cpp
        optimizer/optimizations/simplify_string_builder.cpp
        optimizer/optimizations/string_builder_utils.cpp
        optimizer/optimizations/try_catch_resolving.cpp
        optimizer/optimizations/inline_intrinsics.cpp
        optimizer/optimizations/vn.cpp
        optimizer/optimizations/cse.cpp
        optimizer/optimizations/string_flat_check.cpp
        tools/debug/jit_writer.cpp
        compiler_logger.cpp
        compiler_options.cpp
)
set(COMPILER_SOURCES
        optimizer/ir/aot_data.cpp
        optimizer/ir_builder/ir_builder.cpp
        optimizer/ir_builder/inst_builder.cpp
        optimizer/optimizations/inlining.cpp
        optimizer/optimizations/lowering.cpp
        optimizer/optimizations/move_constants.cpp
        optimizer/optimizations/regalloc/cleanup_empty_blocks.cpp
        optimizer/optimizations/regalloc/reg_alloc_base.cpp
        optimizer/optimizations/regalloc/interference_graph.cpp
        optimizer/optimizations/regalloc/reg_alloc.cpp
        optimizer/optimizations/regalloc/reg_alloc_stat.cpp
        optimizer/optimizations/regalloc/reg_alloc_graph_coloring.cpp
        optimizer/optimizations/regalloc/reg_map.cpp
        optimizer/optimizations/regalloc/reg_alloc_linear_scan.cpp
        optimizer/optimizations/regalloc/spill_fills_resolver.cpp
        optimizer/optimizations/regalloc/split_resolver.cpp
        optimizer/optimizations/regalloc/reg_alloc_resolver.cpp
        optimizer/code_generator/encode.cpp
        ${OPTIMIZER_SOURCES}
        ${INST_BUILDER_GEN}
        ${IRTOC_IR_INLINE_GEN}
)
if (NOT PANDA_TARGET_MACOS)
    list(APPEND COMPILER_SOURCES
        compile_method.cpp
        optimizer/code_generator/disassembly.cpp
        optimizer/code_generator/codegen.cpp
        optimizer/code_generator/codegen_native.cpp
        optimizer/code_generator/encode_visitor.cpp
        optimizer/code_generator/spill_fill_encoder.cpp
        optimizer/code_generator/slow_path.cpp
        optimizer/code_generator/method_properties.cpp
        code_info/code_info.cpp
        code_info/code_info_builder.cpp
    )
endif()

panda_add_library(arkcompiler ${PANDA_DEFAULT_LIB_TYPE} ${COMPILER_SOURCES})

add_dependencies(arkcompiler isa_gen_${PROJECT_NAME})
add_dependencies(arkcompiler instructions_gen_${PROJECT_NAME})
add_dependencies(arkcompiler compiler_events_gen)
add_dependencies(arkcompiler compiler_logger_components_gen)
add_dependencies(arkcompiler compiler_options)
add_dependencies(arkcompiler irtoc_generate_inst_builder)
add_dependencies(arkcompiler irtoc_generate_ir_inline)
add_dependencies(arkcompiler intrinsics_gen_compiler)
add_dependencies(arkcompiler intrinsics_gen_arkruntime)
add_dependencies(arkcompiler entrypoints_gen)
add_dependencies(arkcompiler entrypoints_compiler_checksum_gen)
add_dependencies(arkcompiler cross_values)
add_dependencies(arkcompiler cpu_features_gen)
add_dependencies(arkcompiler asm_defines_generator)
add_dependencies(arkcompiler profiling_gen)
add_dependencies(arkcompiler merge_plugins)

if (PANDA_TARGET_MOBILE OR PANDA_TARGET_OHOS)
  add_dependencies(host_tools_depends arkcompiler)
endif()

panda_target_link_libraries(arkcompiler arkbase arkfile)
if (NOT PANDA_TARGET_WINDOWS AND NOT PANDA_TARGET_MACOS)
  panda_target_link_libraries(arkcompiler arkencoder)
endif()

panda_target_include_directories(arkcompiler
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${PANDA_ROOT}/runtime
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}/runtime/include
    PUBLIC ${PANDA_BINARY_ROOT}/cross_values
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${GENERATED_DIR}
)

# Disable warning about offsetof usage for non-standard layout types.
# In C++17 offsetof is conditionally implemented for such types, so
# compiler should issue error if it doesn't implemented offsetof for
# them. Also we use static asserts to ensure that offsetof works correcly
# for non-standard layout types.
panda_target_compile_options(arkcompiler PUBLIC -Wno-invalid-offsetof)

set(PANDA_COMPILER_TESTS_SOURCES
    tests/analysis_test.cpp
    tests/adjust_arefs_test.cpp
    tests/alias_analysis_test.cpp
    tests/aot_test.cpp
    tests/balance_expressions_test.cpp
    tests/basicblock_test.cpp
    tests/bounds_analysis_test.cpp
    tests/branch_elimination_test.cpp
    tests/call_input_types_test.cpp
    tests/catch_inputs_test.cpp
    tests/checks_elimination_test.cpp
    tests/cleanup_empty_blocks_test.cpp
    tests/cleanup_test.cpp
    tests/codegen_runner_test.cpp
    tests/code_info_test.cpp
    tests/code_sink_test.cpp
    tests/const_folding_test.cpp
    tests/cse_test.cpp
    tests/deoptimize_elimination_test.cpp
    tests/escape_analysis_test.cpp
    tests/dom_tree_test.cpp
    tests/graph_cloner_test.cpp
    tests/graph_comparator_test.cpp
    tests/graph_creation_test.cpp
    tests/if_conversion_test.cpp
    tests/if_merging_test.cpp
    tests/inlining_test.cpp
    tests/inst_test.cpp
    tests/iterators_test.cpp
    tests/licm_test.cpp
    tests/licm_conditions_test.cpp
    tests/loop_unswitch_test.cpp
    tests/liveness_analyzer_test.cpp
    tests/live_registers_test.cpp
    tests/loop_analyzer_test.cpp
    tests/loop_peeling_test.cpp
    tests/loop_idioms_test.cpp
    tests/lse_test.cpp
    tests/memory_barriers_test.cpp
    tests/memory_coalescing_test.cpp
    tests/method_properties_test.cpp
    tests/monitor_analysis_test.cpp
    tests/move_constants_test.cpp
    tests/ref_set_test.cpp
    tests/osr_test.cpp
    tests/panda_runner.cpp
    tests/peepholes_test.cpp
    tests/redundant_loop_elimination_test.cpp
    tests/reg_alloc_common_test.cpp
    tests/reg_alloc_graph_coloring_test.cpp
    tests/reg_alloc_interference_graph_test.cpp
    tests/reg_alloc_resolver_test.cpp
    tests/rpo_test.cpp
    tests/savestate_optimization_tests.cpp
    tests/scheduler_test.cpp
    tests/select_optimization_test.cpp
    tests/split_resolver_test.cpp
    tests/try_catch_resolving_test.cpp
    tests/unit_test.cpp
    tests/vn_test.cpp
    tests/class_hash_table_test.cpp
    tests/linear_order_test.cpp
    tests/profiling_merger_test.cpp
    tests/profiling_runner_test.cpp
    tests/lower_boxed_boolean.cpp
)
set_source_files_properties(tests/loop_unroll_test.cpp PROPERTIES COMPILE_FLAGS "-Wno-shadow -Wno-deprecated-declarations")

# Disable profiling_round_trip_test for AARCH32 and X86
if(NOT (PANDA_TARGET_ARM32 OR PANDA_TARGET_X86))
    list(APPEND PANDA_COMPILER_TESTS_SOURCES
        tests/profiling_round_trip_test.cpp
    )
endif()

# Distinguish 'PANDA_COMPILER_TARGET_..' and 'PANDA_TARGET_..' because for PANDA_TARGET_AMD64 tests
# are being executed for Arch::AARCH64
if(PANDA_COMPILER_TARGET_AARCH64 OR PANDA_TARGET_ARM32)
    if(PANDA_COMPILER_TARGET_AARCH64)
        list(APPEND PANDA_COMPILER_TESTS_SOURCES
            tests/aarch64/codegen_test.cpp
        )
    endif()
    list(APPEND PANDA_COMPILER_TESTS_SOURCES
        tests/life_intervals_test.cpp
        tests/loop_unroll_test.cpp
        tests/reg_alloc_linear_scan_test.cpp
        tests/reg_alloc_verifier_test.cpp
    )
    list(APPEND PANDA_COMPILER_TESTS_SOURCES
        tests/lowering_test.cpp)
endif()

if(PANDA_TARGET_AMD64 OR PANDA_NIGHTLY_TEST_ON)
    list(APPEND PANDA_COMPILER_TESTS_SOURCES
    tests/ir_builder_test.cpp)
endif()

set(PANDA_COMPILER_TESTS_LIBRARIES arkcompiler arkbase arkassembler arkruntime arkaotmanager aot_builder)

set(ENCODER_TESTS_SOURCES
    tests/encoder_operands.cpp
    tests/constructor_test.cpp
    tests/fast_divisor_test.cpp
    )

if(PANDA_COMPILER_ENABLE)
    list(APPEND ENCODER_TESTS_SOURCES tests/asm_printer_test.cpp)
    set_source_files_properties(tests/asm_printer_test.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

if (PANDA_TARGET_ARM32 AND PANDA_COMPILER_TARGET_AARCH32)
    # Append to PANDA_COMPILER_TESTS_SOURCES modified version of inst_generator_test
    list(APPEND ENCODER_TESTS_SOURCES
        tests/aarch32/callconv32_test.cpp
        tests/aarch32/encoder32_test_1.cpp
        tests/aarch32/encoder32_test_2.cpp
        tests/aarch32/register32_test.cpp
        )
endif (PANDA_TARGET_ARM32 AND PANDA_COMPILER_TARGET_AARCH32)

if (PANDA_TARGET_ARM64)
    list(APPEND ENCODER_TESTS_SOURCES
        tests/aarch64/callconv64_test.cpp
        tests/aarch64/encoder64_test_1.cpp
        tests/aarch64/encoder64_test_2.cpp
        tests/aarch64/register64_test.cpp
        )
endif (PANDA_TARGET_ARM64)

if (PANDA_TARGET_X86)
    list(APPEND ENCODER_TESTS_SOURCES
        tests/x86/asmjit_test.cpp
        )
    list(APPEND PANDA_COMPILER_TESTS_LIBRARIES asmjit)
endif (PANDA_TARGET_X86)

if (PANDA_TARGET_AMD64)
    list(APPEND ENCODER_TESTS_SOURCES
        tests/amd64/asmjit_test.cpp
        tests/amd64/callconv64_test.cpp
        tests/amd64/encoder64_test_1.cpp
        tests/amd64/encoder64_test_2.cpp
        tests/amd64/register64_test.cpp
        )
    if (PANDA_COMPILER_TARGET_AARCH64)
        list(APPEND ENCODER_TESTS_SOURCES
            tests/encoders_test.cpp
        )
        set_source_files_properties(tests/encoders_test.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
    endif()
    list(APPEND PANDA_COMPILER_TESTS_LIBRARIES asmjit)
endif (PANDA_TARGET_AMD64)


if (NOT (PANDA_TARGET_MOBILE OR PANDA_TARGET_OHOS OR PANDA_ENABLE_FUZZBENCH))
    list(APPEND PANDA_COMPILER_TESTS_LIBRARIES stdc++fs)
endif()

if(NOT PANDA_MINIMAL_VIXL AND PANDA_COMPILER_ENABLE)
    panda_add_gtest(
        CONTAINS_MAIN
        NAME compiler_unit_tests
        SOURCES
            ${PANDA_COMPILER_TESTS_SOURCES}
            $<TARGET_OBJECTS:arkruntime_stack_walker_obj>
        LIBRARIES
            ${PANDA_COMPILER_TESTS_LIBRARIES}
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
        DEPS_TARGETS
            arkstdlib
    )
endif()

#  AMD64 and X86 - for unit tests
if(NOT PANDA_MINIMAL_VIXL AND PANDA_TARGET_AMD64 AND PANDA_WITH_TESTS)
    set(PANDA_CODEGEN_TESTS_SOURCES
        tests/unit_test.cpp
        tests/spill_fill_encoder_test.cpp
        )
    if(PANDA_COMPILER_TARGET_AARCH64)
        list(APPEND PANDA_CODEGEN_TESTS_SOURCES
            tests/codegen/codegen_test_1.cpp
            tests/codegen/codegen_test_2.cpp
        )
    endif()
    panda_add_gtest(
        CONTAINS_MAIN
        NAME compiler_codegen_tests
        SOURCES
            ${PANDA_CODEGEN_TESTS_SOURCES}
        LIBRARIES
            ${PANDA_COMPILER_TESTS_LIBRARIES}
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )

    if (PANDA_WITH_TESTS AND PANDA_COMPILER_TARGET_AARCH64 AND
        # Limit tests for sanitizer build
        NOT PANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER AND NOT PANDA_ENABLE_ADDRESS_SANITIZER AND NOT PANDA_ENABLE_THREAD_SANITIZER)

        set(PANDA_INST_GEN_TESTS_SOURCES
            tests/unit_test.cpp
            tests/inst_generator.cpp
            tests/inst_generator_test.cpp
            )
        set_source_files_properties(tests/inst_generator_test.cpp PROPERTIES COMPILE_FLAGS "-Wno-shadow -Wno-deprecated-declarations")
        panda_add_gtest(
            CONTAINS_MAIN
            NAME compiler_inst_gen_tests
            SOURCES
                ${PANDA_INST_GEN_TESTS_SOURCES}
            LIBRARIES
                ${PANDA_COMPILER_TESTS_LIBRARIES}
            SANITIZERS
                ${PANDA_SANITIZERS_LIST}
            DEPS_TARGETS
                compiler_inst_generator_test_ext
        )

        if (PANDA_CI_TESTING_MODE STREQUAL "Nightly")
            set(PANDA_INTRINSIC_CODEGEN_TESTS_SOURCES
                tests/unit_test.cpp
                tests/inst_generator.cpp
                tests/intrinsic_codegen_test.cpp
                )

            if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
                set_source_files_properties(tests/intrinsic_codegen_test.cpp PROPERTIES COMPILE_FLAGS "-Wno-shadow -Wno-deprecated-declarations -Wno-bitwise-instead-of-logical")
            else()
                set_source_files_properties(tests/intrinsic_codegen_test.cpp PROPERTIES COMPILE_FLAGS "-Wno-shadow -Wno-deprecated-declarations")
            endif()

            panda_add_gtest(
                CONTAINS_MAIN
                NAME compiler_intrinsic_codegen_arm64_tests
                SOURCES
                    ${PANDA_INTRINSIC_CODEGEN_TESTS_SOURCES}
                LIBRARIES
                    ${PANDA_COMPILER_TESTS_LIBRARIES}
                SANITIZERS
                    ${PANDA_SANITIZERS_LIST}
            )
            panda_target_compile_options(compiler_intrinsic_codegen_arm64_tests PUBLIC  "-DINTRINSIC_CODEGEN_TEST_ARM64")

            panda_add_gtest(
                CONTAINS_MAIN
                NAME compiler_intrinsic_codegen_amd64_tests
                SOURCES
                    ${PANDA_INTRINSIC_CODEGEN_TESTS_SOURCES}
                LIBRARIES
                    ${PANDA_COMPILER_TESTS_LIBRARIES}
                SANITIZERS
                    ${PANDA_SANITIZERS_LIST}
            )
            panda_target_compile_options(compiler_intrinsic_codegen_amd64_tests PUBLIC  "-DINTRINSIC_CODEGEN_TEST_AMD64")

            panda_add_gtest(
                CONTAINS_MAIN
                NAME compiler_intrinsic_codegen_arm32_tests
                SOURCES
                    ${PANDA_INTRINSIC_CODEGEN_TESTS_SOURCES}
                LIBRARIES
                    ${PANDA_COMPILER_TESTS_LIBRARIES}
                SANITIZERS
                    ${PANDA_SANITIZERS_LIST}
            )
            panda_target_compile_options(compiler_intrinsic_codegen_arm32_tests PUBLIC  "-DINTRINSIC_CODEGEN_TEST_ARM32")
        endif()
    endif()
endif()

if(NOT PANDA_MINIMAL_VIXL AND PANDA_WITH_TESTS AND PANDA_COMPILER_ENABLE)
  add_dependencies(compiler_unit_tests ark_aot)
  # TODO(igorban): Enable build in other platforms
  if (PANDA_TARGET_AMD64 OR PANDA_TARGET_ARM64)
    add_dependencies(compiler_unit_tests ark_aotdump)
  endif()

  panda_target_include_directories(compiler_unit_tests SYSTEM
    PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>"
  )

  panda_target_include_directories(compiler_unit_tests
    PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_INCLUDE_DIRECTORIES>"
  )

  if(PANDA_TARGET_AMD64 AND PANDA_COMPILER_TARGET_AARCH64 AND
    # Limit tests for sanitizer build
    NOT PANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER AND NOT PANDA_ENABLE_ADDRESS_SANITIZER AND NOT PANDA_ENABLE_THREAD_SANITIZER)
    panda_target_include_directories(compiler_inst_gen_tests SYSTEM
        PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>"
    )
    panda_target_include_directories(compiler_inst_gen_tests
        PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_INCLUDE_DIRECTORIES>"
    )
  endif()
  if(PANDA_TARGET_AMD64)
    panda_target_include_directories(compiler_codegen_tests SYSTEM
        PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>"
    )
    panda_target_include_directories(compiler_codegen_tests
        PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_INCLUDE_DIRECTORIES>"
    )
  endif()
endif()

# Encoder gtests are separate from other, because they are test the library, not whole compiler
# Please, do not build them together - to do not make additional dependencies!
if(NOT PANDA_MINIMAL_VIXL AND PANDA_WITH_TESTS)
    panda_add_gtest(
        NAME encoder_unit_tests
        SOURCES
            ${ENCODER_TESTS_SOURCES}
        LIBRARIES
            ${PANDA_COMPILER_TESTS_LIBRARIES}
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )
    panda_target_compile_options(encoder_unit_tests PUBLIC "-Wno-unused-variable" "-Wno-uninitialized" "-Wno-shadow")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
        panda_target_compile_options(encoder_unit_tests PUBLIC "-Wno-bitwise-instead-of-logical")
    endif()
endif()

panda_add_sanitizers(TARGET arkcompiler SANITIZERS ${PANDA_SANITIZERS_LIST})

# Enable documentation
add_doxygen(
    NAME    "compiler"
    PATH    "."
)

# Support mobile execution
if(NOT PANDA_MINIMAL_VIXL AND PANDA_WITH_TESTS AND NOT (PANDA_TARGET_MOBILE OR PANDA_TARGET_OHOS) AND PANDA_COMPILER_ENABLE)
    set(ASM_TEST_LIST
        "mov"
        "neg"
        "abs"
        "not"
        "add"
        "sub"
        "mul"
        "shl"
        "shr"
        "ashr"
        "and"
        "or"
        "xor"
        )

    if (PANDA_TARGET_AMD64)
        set(LIBGENERATED_DIR ${CMAKE_BINARY_DIR}/bin-gtests/asm_output/amd64/)
    elseif (PANDA_TARGET_ARM64)
        set(LIBGENERATED_DIR ${CMAKE_BINARY_DIR}/bin-gtests/asm_output/aarch64/)
    elseif(PANDA_TARGET_ARM32)
        set(LIBGENERATED_DIR ${CMAKE_BINARY_DIR}/bin-gtests/asm_output/aarch32/)
    else()
        set(LIBGENERATED_FILES "")
    endif()

    # Main target
    add_custom_target(asm_test)

    # Pseudo-target for generate asm
    add_custom_target(asm_generate)
    add_dependencies(asm_generate encoder_unit_tests_gtests)

    foreach(TEST ${ASM_TEST_LIST})
        set(TEST_ASM ${LIBGENERATED_DIR}/${TEST}.S)
        add_custom_command(
                        OUTPUT ${TEST_ASM}
                        COMMAND echo " Pseudo-command for generate asm ${TEST_ASM}" > /dev/null
                        DEPENDS asm_generate)
        set_property(SOURCE ${TEST_ASM} PROPERTY GENERATED TRUE)
        add_custom_target(asm_${TEST} DEPENDS ${TEST_ASM})
        add_dependencies(asm_test asm_${TEST})
        list(APPEND LIBGENERATED_FILES ${LIBGENERATED_DIR}/${TEST}.S)
    endforeach()

    enable_language(ASM)

    set(ASM_CALL_TEST
        tests/asm_caller.cpp
        ${LIBGENERATED_FILES}
    )

    panda_add_gtest(
        NAME compiler_asm_tests
        SOURCES
            ${ASM_CALL_TEST}
        LIBRARIES
            ${PANDA_COMPILER_TESTS_LIBRARIES}
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )
    if(PANDA_WITH_TESTS)
      add_dependencies(asm_test compiler_asm_tests)
    endif()
    # PANDA_TARGET_MOBILE
endif()

add_subdirectory(tools/paoc ark_aot)

if (NOT PANDA_MINIMAL_VIXL AND (PANDA_TARGET_AMD64 OR PANDA_TARGET_ARM64))
    add_subdirectory(tools/aotdump aotdump)
endif()
add_subdirectory(aot)
add_subdirectory(aot/aot_builder aot_builder)

declare_plugin_file("create_pipeline.inl")
