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


set(REGISTER_EXTENSIONS_H ${PANDA_BINARY_ROOT}/assembler/register_extensions.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/assembler/extensions/register_extensions.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${REGISTER_EXTENSIONS_H}
)

add_custom_target(assembler_extensions DEPENDS
    plugin_options_gen
    ${REGISTER_EXTENSIONS_H}
)

add_dependencies(arkassembler assembler_extensions)
