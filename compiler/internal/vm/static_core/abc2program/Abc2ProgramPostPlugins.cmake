# Copyright (c) 2024 Huawei Device Co., Ltd.
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

set(ABC2PROGRAM_PLUGINS_INC ${PANDA_BINARY_ROOT}/abc2program/abc2program_plugins.inc)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/abc2program/templates/abc2program_plugins.inc.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${ABC2PROGRAM_PLUGINS_INC}
)

set(GET_LANGUAGE_SPECIFIC_METADATA_INC ${PANDA_BINARY_ROOT}/abc2program/get_language_specific_metadata.inc)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/abc2program/templates/get_language_specific_metadata.inc.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${GET_LANGUAGE_SPECIFIC_METADATA_INC}
)

add_custom_target(abc2program_plugin_gen DEPENDS
    plugin_options_gen
    ${ABC2PROGRAM_PLUGINS_INC}
    ${GET_LANGUAGE_SPECIFIC_METADATA_INC}
)

add_dependencies(abc2program abc2program_plugin_gen)
add_dependencies(panda_gen_files abc2program_plugin_gen)