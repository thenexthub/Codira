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


set(CODEGEN_VISITORS_INC ${PANDA_BINARY_ROOT}/bytecode_optimizer/generated/codegen_visitors.inc)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/bytecode_optimizer/templates/codegen_visitors.inc.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${CODEGEN_VISITORS_INC}
)

add_custom_target(bytecode_optimizer_codegen_visitors_inc DEPENDS
    plugin_options_gen
    ${CODEGEN_VISITORS_INC}
)

set(REG_ENCODER_VISITORS_INC ${PANDA_BINARY_ROOT}/bytecode_optimizer/generated/reg_encoder_visitors.inc)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/bytecode_optimizer/templates/reg_encoder_visitors.inc.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${REG_ENCODER_VISITORS_INC}
)

add_custom_target(bytecode_optimizer_reg_encoder_visitors_inc DEPENDS
    plugin_options_gen
    ${REG_ENCODER_VISITORS_INC}
)

set(CODEGEN_INTRINSICS_CPP ${PANDA_BINARY_ROOT}/bytecode_optimizer/generated/codegen_intrinsics.cpp)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/bytecode_optimizer/templates/codegen_intrinsics.cpp.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${CODEGEN_INTRINSICS_CPP}
)

add_custom_target(bytecode_optimizer_codegen_intrinsics_cpp DEPENDS
    plugin_options_gen
    ${CODEGEN_INTRINSICS_CPP}
)

add_dependencies(arkbytecodeopt
    bytecode_optimizer_codegen_visitors_inc
    bytecode_optimizer_reg_encoder_visitors_inc
    bytecode_optimizer_codegen_intrinsics_cpp
)
