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

get_target_property(RUNTIME_YAML_FILES runtime_options_gen RUNTIME_OPTIONS_YAML_FILES)
string(REPLACE ";" "," RUNTIME_YAML_FILES_STR "${RUNTIME_YAML_FILES}")
set(GEN_RUNTIME_OPTIONS_YAML ${CMAKE_CURRENT_BINARY_DIR}/runtime_options_gen.yaml)
add_custom_command(OUTPUT ${GEN_RUNTIME_OPTIONS_YAML}
    COMMENT "Merge yaml files: ${RUNTIME_YAML_FILES_STR}"
    COMMAND ${PANDA_ROOT}/templates/merge.rb -d "${RUNTIME_YAML_FILES_STR}" -o "${GEN_RUNTIME_OPTIONS_YAML}"
    DEPENDS ${RUNTIME_YAML_FILES}
)
add_custom_target(runtime_options_merge DEPENDS ${GEN_RUNTIME_OPTIONS_YAML})

panda_gen_options(
    TARGET arkruntime_obj
    YAML_FILE "${GEN_RUNTIME_OPTIONS_YAML}"
    GENERATED_HEADER runtime_options_gen.h
)
add_dependencies(arkruntime_obj_options arkbase_obj_options)