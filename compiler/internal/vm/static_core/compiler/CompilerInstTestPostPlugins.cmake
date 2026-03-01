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

set(INST_GENERATOR_TEST_EXT_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/inst_generator_test_ext.inl.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/inst_generator_test_ext.inl.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INST_GENERATOR_TEST_EXT_INL_H}
)

add_custom_target(compiler_inst_generator_test_ext DEPENDS
    plugin_options_gen
    ${INST_GENERATOR_TEST_EXT_INL_H}
)

add_dependencies(panda_gen_files compiler_inst_generator_test_ext)
