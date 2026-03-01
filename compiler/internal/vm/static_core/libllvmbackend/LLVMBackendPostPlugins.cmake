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

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/libllvmbackend/generated)

set(LLVM_BACKEND_EMIT_INTRINSIC_GEN ${GENERATED_DIR}/emit_intrinsic_llvm_ir_constructor_gen.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/libllvmbackend/templates/emit_intrinsic_llvm_ir_constructor_gen.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${LLVM_BACKEND_EMIT_INTRINSIC_GEN}
)

add_custom_target(llvmbackend_emit_intrinsic_gen DEPENDS
    plugin_options_gen
    ${LLVM_BACKEND_EMIT_INTRINSIC_GEN}
)

set(LLVM_BACKEND_ENTRY_GEN ${GENERATED_DIR}/llvm_ir_constructor_gen.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/libllvmbackend/templates/llvm_ir_constructor_gen.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${LLVM_BACKEND_ENTRY_GEN}
)

set(LLVM_BACKEND_ENTRY_GEN_H ${GENERATED_DIR}/llvm_ir_constructor_gen.h.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/libllvmbackend/templates/llvm_ir_constructor_gen.h.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${LLVM_BACKEND_ENTRY_GEN_H}
)

add_custom_target(llvmbackend_ir_constructor_gen DEPENDS
    plugin_options_gen
    ${LLVM_BACKEND_ENTRY_GEN}
    ${LLVM_BACKEND_ENTRY_GEN_H}
)

set(LLVM_BACKEND_GET_INTRINSIC_ID_GEN_H ${GENERATED_DIR}/get_intrinsic_id_llvm_ark_interface_gen.h.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/libllvmbackend/templates/get_intrinsic_id_llvm_ark_interface_gen.h.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${LLVM_BACKEND_GET_INTRINSIC_ID_GEN_H}
)

set(LLVM_BACKEND_GET_INTRINSIC_ID_GEN ${GENERATED_DIR}/get_intrinsic_id_llvm_ark_interface_gen.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/libllvmbackend/templates/get_intrinsic_id_llvm_ark_interface_gen.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES} plugin_options_merge
    OUTPUTFILE ${LLVM_BACKEND_GET_INTRINSIC_ID_GEN}
)

add_custom_target(llvmbackend_get_intrinsic_id_gen DEPENDS
    plugin_options_gen
    ${LLVM_BACKEND_GET_INTRINSIC_ID_GEN_H}
    ${LLVM_BACKEND_GET_INTRINSIC_ID_GEN}
)

add_dependencies(llvmbackend llvmbackend_emit_intrinsic_gen)
add_dependencies(llvmbackend llvmbackend_ir_constructor_gen)
add_dependencies(llvmbackend llvmbackend_get_intrinsic_id_gen)

add_dependencies(panda_gen_files
    llvmbackend_emit_intrinsic_gen
    llvmbackend_ir_constructor_gen
    llvmbackend_get_intrinsic_id_gen
)
