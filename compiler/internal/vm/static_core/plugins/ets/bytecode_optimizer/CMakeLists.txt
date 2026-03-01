# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

project(plugin_ets_bytecode_optimizer)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

panda_isa_gen(
        TEMPLATES
            "ets_codegen_intrinsics_gen.inc.erb"
        SOURCE ${PANDA_ETS_PLUGIN_SOURCE}/bytecode_optimizer/templates
        API ${PANDA_ROOT}/assembler/asm_isapi.rb
        DESTINATION ${GENERATED_DIR}
        )

add_dependencies(arkbytecodeopt_obj isa_gen_${PROJECT_NAME})

panda_target_include_directories(arkbytecodeopt_obj PUBLIC
  ${GENERATED_DIR}
)
