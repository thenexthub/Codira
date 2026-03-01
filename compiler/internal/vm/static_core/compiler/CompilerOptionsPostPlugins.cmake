# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

get_target_property(COMPILER_YAML_FILES compiler_options_gen COMPILER_OPTIONS_YAML_FILES)
string(REPLACE ";" "," COMPILER_YAML_FILES_STR "${COMPILER_YAML_FILES}")
set(GEN_COMPILER_OPTIONS_YAML ${CMAKE_CURRENT_BINARY_DIR}/compiler_options_gen.yaml)
add_custom_command(OUTPUT ${GEN_COMPILER_OPTIONS_YAML}
    COMMENT "Merge yaml files: ${COMPILER_YAML_FILES_STR}"
    COMMAND ${PANDA_ROOT}/templates/merge.rb -d "${COMPILER_YAML_FILES_STR}" -o "${GEN_COMPILER_OPTIONS_YAML}"
    DEPENDS ${COMPILER_YAML_FILES}
)
add_custom_target(compiler_options_merge DEPENDS ${GEN_COMPILER_OPTIONS_YAML})

set(COMPILER_OPTIONS_GEN_H ${CMAKE_CURRENT_BINARY_DIR}/compiler/generated/compiler_options_gen.h)
panda_gen_file(
    DATA ${GEN_COMPILER_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/templates/options/options.h.erb
    OUTPUTFILE ${COMPILER_OPTIONS_GEN_H}
    API ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(compiler_options DEPENDS ${COMPILER_OPTIONS_GEN_H})
add_dependencies(panda_gen_files compiler_options)
