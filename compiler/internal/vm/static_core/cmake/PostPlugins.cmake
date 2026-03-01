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

get_target_property(YAML_FILES plugin_options_gen PLUGIN_OPTIONS_YAML_FILES)

set(GEN_PLUGIN_OPTIONS_YAML ${PANDA_BINARY_ROOT}/plugin_options.yaml)
add_custom_command(OUTPUT ${GEN_PLUGIN_OPTIONS_YAML}
    COMMENT "Merge yaml files: ${YAML_FILES}"
    COMMAND ${PANDA_ROOT}/templates/concat_yamls.sh "${GEN_PLUGIN_OPTIONS_YAML}" ${YAML_FILES}
    DEPENDS ${YAML_FILES}
)
add_custom_target(plugin_options_merge DEPENDS ${GEN_PLUGIN_OPTIONS_YAML})

get_target_property(MERGE_PLUGINS merge_plugins PLUGINS)
foreach(plugin_file ${MERGE_PLUGINS})

    string(REGEX REPLACE "[/\\.]" "_" plugin_target ${plugin_file})
    set(plugin_target_files "${plugin_target}__files")

    get_target_property(PLUGIN_FILES ${plugin_target_files} PLUGIN_FILES)
    get_target_property(PLUGINS_BINARY_DIR ${plugin_target_files} GENERATED_DIR)
    file(MAKE_DIRECTORY ${PLUGINS_BINARY_DIR})

    if (PLUGIN_FILES)
        add_custom_command(OUTPUT ${PLUGINS_BINARY_DIR}/${plugin_file}
            COMMAND cat ${PLUGIN_FILES} > ${PLUGINS_BINARY_DIR}/${plugin_file}
            DEPENDS ${PLUGIN_FILES}
        )
    else()
        add_custom_command(OUTPUT ${PLUGINS_BINARY_DIR}/${plugin_file}
            COMMAND touch ${PLUGINS_BINARY_DIR}/${plugin_file}
        )
    endif()


    add_custom_target(${plugin_target} DEPENDS ${PLUGINS_BINARY_DIR}/${plugin_file})
    add_dependencies(merge_plugins ${plugin_target})
endforeach()

add_dependencies(panda_gen_files merge_plugins)

include(assembler/extensions/AssemblerExtPostPlugins.cmake)
include(bytecode_optimizer/templates/BytecodeOptPostPlugins.cmake)
include(runtime/RuntimeLanguageContextPostPlugins.cmake)
include(runtime/RuntimeIntrinsicsPostPlugins.cmake)
include(runtime/RuntimeEntrypointsPostPlugins.cmake)
include(runtime/RuntimeIrtocPostPlugins.cmake)
include(runtime/RuntimeOptionsPostPlugins.cmake)
include(libarkbase/LibpandabasePostPlugins.cmake)
include(libarkfile/LibpandafilePostPlugins.cmake)
include(compiler/CompilerInstTemplatesPostPlugins.cmake)
include(compiler/CompilerIntrinsicsPostPlugins.cmake)
include(compiler/CompilerOptionsPostPlugins.cmake)
include(compiler/CompilerInstTestPostPlugins.cmake)
include(disassembler/DisassemblerPostPlugins.cmake)
include(abc2program/Abc2ProgramPostPlugins.cmake)
include(isa/IsaPostPlugins.cmake)
include(irtoc/templates/IrtocPostPlugins.cmake)
include(verification/VerifierPostPlugins.cmake)
if (PANDA_BUILD_LLVM_BACKEND)
    include(libllvmbackend/LLVMBackendPostPlugins.cmake)
endif()
