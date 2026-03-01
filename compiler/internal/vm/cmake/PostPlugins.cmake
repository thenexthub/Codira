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

include(assembler/extensions/AssemblerExtPostPlugins.cmake)
include(bytecode_optimizer/templates/BytecodeOptPostPlugins.cmake)
include(runtime/RuntimeEnableRelayoutPostPlugins.cmake)
include(runtime/RuntimeLanguageContextPostPlugins.cmake)
include(runtime/RuntimeIntrinsicsPostPlugins.cmake)
include(runtime/RuntimeEntrypointsPostPlugins.cmake)
include(runtime/RuntimeIrtocPostPlugins.cmake)
include(runtime/RuntimeOptionsPostPlugins.cmake)
include(libpandabase/LibpandabasePostPlugins.cmake)
include(libpandafile/LibpandafilePostPlugins.cmake)
include(compiler/CompilerInstTemplatesPostPlugins.cmake)
include(compiler/CompilerIntrinsicsPostPlugins.cmake)
include(disassembler/DisassemblerPostPlugins.cmake)
include(isa/IsaPostPlugins.cmake)
include(irtoc/templates/IrtocPostPlugins.cmake)
include(verification/VerifierPostPlugins.cmake)
