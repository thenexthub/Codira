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

set(PLUGINS_INC ${PANDA_BINARY_ROOT}/runtime/include/plugins.inc)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/runtime/templates/plugins.inc.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${PLUGINS_INC}
)

set(PLUGINS_ENTRYPOINTS_H ${PANDA_BINARY_ROOT}/runtime/include/plugins_entrypoints_gen.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/runtime/entrypoints/plugins_entrypoints_gen.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${PLUGINS_ENTRYPOINTS_H}
)

set(PLUGINS_INTERPRETERS_INL_H ${PANDA_BINARY_ROOT}/runtime/include/plugins_interpreters-inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/runtime/templates/plugins_interpreters-inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${PLUGINS_INTERPRETERS_INL_H}
)

set(LANGUAGE_CONFIG_GEN_INC ${PANDA_BINARY_ROOT}/runtime/include/language_config_gen.inc)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/runtime/templates/language_config_gen.inc.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${LANGUAGE_CONFIG_GEN_INC}
)

add_custom_target(runtime_language_context DEPENDS
    plugin_options_gen
    ${PLUGINS_INC}
    ${PLUGINS_INTERPRETERS_INL_H}
    ${LANGUAGE_CONFIG_GEN_INC}
    ${PLUGINS_ENTRYPOINTS_H}
)

add_dependencies(panda_gen_files runtime_language_context)
add_dependencies(arkruntime_obj runtime_language_context)
add_dependencies(arkaotmanager runtime_language_context)
add_dependencies(arkcompiler runtime_language_context)
