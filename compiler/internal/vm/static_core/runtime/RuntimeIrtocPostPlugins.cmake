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

set(PLUGINS_DEFINES_H ${PANDA_BINARY_ROOT}/runtime/asm_defines/plugins_defines.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/runtime/templates/plugins_defines.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${PLUGINS_DEFINES_H}
)

set(PLUGINS_ASM_DEFINES_DEF ${PANDA_BINARY_ROOT}/runtime/asm_defines/plugins_asm_defines.def)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/runtime/templates/plugins_asm_defines.def.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${PLUGINS_ASM_DEFINES_DEF}
)

add_custom_target(plugins_defines_h DEPENDS
    plugin_options_gen
    ${PLUGINS_DEFINES_H}
    arkruntime_obj_options
    messages_gen_pandaverification_messages
    ir_dyn_base_types_h
    source_languages_h
)

add_custom_target(plugins_asm_defines_def DEPENDS
    plugin_options_gen
    ${PLUGINS_ASM_DEFINES_DEF}
)

add_dependencies(panda_gen_files
    plugins_defines_h
    plugins_asm_defines_def
)

add_dependencies(asm_defines_process plugins_asm_defines_def)

panda_target_include_directories(asm_defines
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}/runtime/asm_defines
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}/compiler/generated
)
