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

separate_arguments(GENERATED_CROSS_VALUES_HEADERS)

get_target_property(ENTRYPOINT_YAML_FILES entrypoints_yaml_gen ENTRYPOINT_YAML_FILES)

set(GEN_ENTRYPOINTS_YAML ${PANDA_BINARY_ROOT}/runtime/entrypoints.yaml)
add_custom_command(OUTPUT ${GEN_ENTRYPOINTS_YAML}
    COMMENT "Merge yaml files: ${ENTRYPOINT_YAML_FILES}"
    COMMAND ${PANDA_ROOT}/templates/concat_yamls.sh "${GEN_ENTRYPOINTS_YAML}" ${ENTRYPOINT_YAML_FILES}
    DEPENDS ${ENTRYPOINT_YAML_FILES}
)
add_custom_target(entrypoints_yaml_merge DEPENDS ${GEN_ENTRYPOINTS_YAML})

panda_gen(DATA ${GEN_ENTRYPOINTS_YAML}
    TARGET_NAME entrypoints_gen
    TEMPLATES
    entrypoints_gen.h.erb
    entrypoints_gen.S.erb
    entrypoints_compiler.inl.erb
    SOURCE ${PANDA_ROOT}/runtime/entrypoints
    DESTINATION ${PANDA_BINARY_ROOT}/runtime/include
    REQUIRES
        ${PANDA_ROOT}/templates/common.rb
    API
        ${PANDA_ROOT}/runtime/entrypoints/entrypoints.rb
    EXTRA_DEPENDENCIES entrypoints_yaml_merge
)

panda_gen(DATA ${GEN_ENTRYPOINTS_YAML}
    TARGET_NAME entrypoints_compiler_checksum_gen
    TEMPLATES
    entrypoints_compiler_checksum.inl.erb
    SOURCE ${PANDA_ROOT}/runtime/entrypoints
    DESTINATION ${PANDA_BINARY_ROOT}/runtime/include
    REQUIRES
        ${PANDA_ROOT}/templates/common.rb
    API
        ${PANDA_ROOT}/runtime/entrypoints/entrypoints.rb
    EXTRA_DEPENDENCIES cross_values_generator ${GENERATED_CROSS_VALUES_HEADERS}
    EXTRA_ARGV ${PANDA_BINARY_ROOT}/cross_values
)
