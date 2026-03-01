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

set(INTRINSICS_CODEGEN_EXT_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_codegen_ext.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_codegen_ext.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_CODEGEN_EXT_INL_H}
)

set(INTRINSICS_IR_BUILD_STATIC_CALL_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_ir_build_static_call.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_ir_build_static_call.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_IR_BUILD_STATIC_CALL_INL}
)

set(INTRINSICS_IR_BUILD_VIRTUAL_CALL_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_ir_build_virtual_call.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_ir_build_virtual_call.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_IR_BUILD_VIRTUAL_CALL_INL}
)

set(INTRINSICS_GRAPH_CHECKER_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_graph_checker.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_graph_checker.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_GRAPH_CHECKER_INL}
)

set(INTRINSICS_IR_BUILD_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_ir_build.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_ir_build.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_IR_BUILD_INL_H}
)

set(INTRINSICS_CAN_ENCODE_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_can_encode.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_can_encode.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_CAN_ENCODE_INL}
)

set(INTRINSICS_LSE_HEAP_INV_ARGS_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_lse_heap_inv_args.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_lse_heap_inv_args.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_LSE_HEAP_INV_ARGS_INL}
)

set(IR_DYN_BASE_TYPES_H ${PANDA_BINARY_ROOT}/compiler/generated/ir-dyn-base-types.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/ir-dyn-base-types.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${IR_DYN_BASE_TYPES_H}
)

add_custom_target(ir_dyn_base_types_h DEPENDS ${IR_DYN_BASE_TYPES_H})

set(SOURCE_LANGUAGES_H ${PANDA_BINARY_ROOT}/compiler/generated/source_languages.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/source_languages.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${SOURCE_LANGUAGES_H}
)

add_custom_target(source_languages_h DEPENDS ${SOURCE_LANGUAGES_H})

set(CODEGEN_LANGUAGE_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/codegen_language_extensions.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/codegen_language_extensions.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${CODEGEN_LANGUAGE_EXTENSIONS_H}
)

set(COMPILER_INTERFACE_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/compiler_interface_extensions.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/compiler_interface_extensions.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${COMPILER_INTERFACE_EXTENSIONS_H}
)

set(IRTOC_INTERFACE_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/irtoc_interface_extensions.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/irtoc_interface_extensions.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${IRTOC_INTERFACE_EXTENSIONS_H}
)

set(IRTOC_INTERFACE_EXTENSIONS_INCLUDES_H ${PANDA_BINARY_ROOT}/compiler/generated/irtoc_interface_extensions_includes.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/irtoc_interface_extensions_includes.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${IRTOC_INTERFACE_EXTENSIONS_INCLUDES_H}
)

set(INST_BUILDER_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/inst_builder_extensions.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/inst_builder_extensions.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${INST_BUILDER_EXTENSIONS_H}
)

set(INTRINSICS_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_extensions.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics_extensions.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${INTRINSICS_EXTENSIONS_H}
)

set(INTRINSICS_INLINE_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_inline.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_inline.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_INLINE_INL_H}
)
set(INTRINSICS_INLINE_NATIVE_METHOD_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_inline_native_method.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_inline_native_method.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_INLINE_NATIVE_METHOD_INL}
)

set(INTRINSICS_INLINING_EXPANSION_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_inlining_expansion.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_inlining_expansion.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_INLINING_EXPANSION_INL_H}
)

set(INTRINSICS_INLINING_EXPANSION_SWITCH_CASE_INL
    ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_inlining_expansion_switch_case.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_inlining_expansion_switch_case.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_INLINING_EXPANSION_SWITCH_CASE_INL}
)

set(INTRINSICS_PEEPHOLE_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_peephole.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_peephole.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_PEEPHOLE_INL_H}
)

set(LI_MEMMOVE_INTRINSIC_ID_SWITCH_CASE_INL
    ${PANDA_BINARY_ROOT}/compiler/generated/loop_idioms_memmove_intrinsic_id_switch_case.inl)
panda_gen_file(
        DATA ${GEN_PLUGIN_OPTIONS_YAML}
        TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/loop_idioms_memmove_intrinsic_id_switch_case.inl.erb
        API ${PANDA_ROOT}/templates/plugin_options.rb
        EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
        OUTPUTFILE ${LI_MEMMOVE_INTRINSIC_ID_SWITCH_CASE_INL}
)

set(LI_CUSTOMIZE_MEMMOVE_INTRINSIC_SWITCH_CASE_INL
    ${PANDA_BINARY_ROOT}/compiler/generated/loop_idioms_customize_memmove_intrinsic_switch_case.inl)
panda_gen_file(
        DATA ${GEN_PLUGIN_OPTIONS_YAML}
        TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/loop_idioms_customize_memmove_intrinsic_switch_case.inl.erb
        API ${PANDA_ROOT}/templates/plugin_options.rb
        EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
        OUTPUTFILE ${LI_CUSTOMIZE_MEMMOVE_INTRINSIC_SWITCH_CASE_INL}
)

set(INTRINSIC_STRING_FLAT_CHECK_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsic_string_flat_check.inl)
panda_gen_file(
        DATA ${GEN_PLUGIN_OPTIONS_YAML}
        TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsic_string_flat_check.inl.erb
        API ${PANDA_ROOT}/templates/plugin_options.rb
        EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
        OUTPUTFILE ${INTRINSIC_STRING_FLAT_CHECK_INL}
)

set(PIPELINE_INCLUDES_H ${PANDA_BINARY_ROOT}/compiler/generated/pipeline_includes.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/pipeline_includes.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${PIPELINE_INCLUDES_H}
)

add_custom_target(compiler_intrinsics DEPENDS
    plugin_options_gen
    ${INTRINSICS_CODEGEN_EXT_INL_H}
    ${INTRINSICS_IR_BUILD_STATIC_CALL_INL}
    ${INTRINSICS_IR_BUILD_VIRTUAL_CALL_INL}
    ${INTRINSICS_IR_BUILD_INL_H}
    ${INTRINSICS_CAN_ENCODE_INL}
    ${INTRINSICS_LSE_HEAP_INV_ARGS_INL}
    ${IR_DYN_BASE_TYPES_H}
    ${SOURCE_LANGUAGES_H}
    ${CODEGEN_LANGUAGE_EXTENSIONS_H}
    ${COMPILER_INTERFACE_EXTENSIONS_H}
    ${IRTOC_INTERFACE_EXTENSIONS_H}
    ${IRTOC_INTERFACE_EXTENSIONS_INCLUDES_H}
    ${INST_BUILDER_EXTENSIONS_H}
    ${INTRINSICS_EXTENSIONS_H}
    ${INTRINSICS_INLINE_INL_H}
    ${INTRINSICS_INLINING_EXPANSION_INL_H}
    ${INTRINSICS_INLINING_EXPANSION_SWITCH_CASE_INL}
    ${INTRINSICS_GRAPH_CHECKER_INL}
    ${INTRINSICS_INLINE_NATIVE_METHOD_INL}
    ${LI_MEMMOVE_INTRINSIC_ID_SWITCH_CASE_INL}
    ${LI_CUSTOMIZE_MEMMOVE_INTRINSIC_SWITCH_CASE_INL}
    ${INTRINSICS_PEEPHOLE_INL_H}
    ${PIPELINE_INCLUDES_H}
)

add_dependencies(arkcompiler compiler_intrinsics)
add_dependencies(panda_gen_files compiler_intrinsics)
