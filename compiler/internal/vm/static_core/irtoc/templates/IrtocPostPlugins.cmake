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

get_target_property(PLUGIN_FILES_LIST interpreter_irt_combine IRTOC_PLUGIN_FILES)
set(GENERATED_DIRECTORY ${PANDA_BINARY_ROOT}/irtoc/generated)
file(MAKE_DIRECTORY "${GENERATED_DIRECTORY}")
set(PLUGINS_TXT ${GENERATED_DIRECTORY}/plugins.txt)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/irtoc/templates/plugins.txt.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge ${PLUGIN_FILES_LIST}
    OUTPUTFILE ${PLUGINS_TXT}
)

add_custom_target(irtoc_plugins_txt
    DEPENDS plugin_options_gen ${PLUGINS_TXT}
)
add_dependencies(irtoc_plugins_txt interpreter_irt_combine)
add_dependencies(panda_gen_files irtoc_plugins_txt)
