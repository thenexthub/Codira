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

set(DISASM_PLUGINS_INC ${PANDA_BINARY_ROOT}/disassembler/disasm_plugins.inc)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/disassembler/templates/disasm_plugins.inc.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${DISASM_PLUGINS_INC}
)

set(GET_LANGUAGE_SPECIFIC_METADATA_INC ${PANDA_BINARY_ROOT}/disassembler/get_language_specific_metadata.inc)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/disassembler/templates/get_language_specific_metadata.inc.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${GET_LANGUAGE_SPECIFIC_METADATA_INC}
)


add_custom_target(arkdisassembler_plugin_gen DEPENDS
    plugin_options_gen
    ${DISASM_PLUGINS_INC}
    ${GET_LANGUAGE_SPECIFIC_METADATA_INC}
)

add_dependencies(arkdisassembler arkdisassembler_plugin_gen)
