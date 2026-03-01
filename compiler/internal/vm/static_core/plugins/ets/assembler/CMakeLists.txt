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

set(EXTENSION_DIR ${PANDA_ETS_PLUGIN_SOURCE}/assembler/extension)

panda_target_sources(arkassembler_obj PRIVATE ${EXTENSION_DIR}/ets_meta.cpp)
panda_target_include_directories(arkassembler_obj PUBLIC ${EXTENSION_DIR})

set(ETS_META_GEN_H ${PANDA_BINARY_ROOT}/assembler/ets_meta_gen.h)
panda_gen_file(
    DATA ${PANDA_ETS_PLUGIN_SOURCE}/assembler/extension/metadata.yaml
    TEMPLATE ${PANDA_ROOT}/assembler/templates/meta_gen.cpp.erb
    OUTPUTFILE ${ETS_META_GEN_H}
    API ${PANDA_ROOT}/assembler/asm_metadata.rb
)
add_custom_target(ets_meta_gen_h DEPENDS ${ETS_META_GEN_H})
add_dependencies(arkassembler_obj ets_meta_gen_h)
add_dependencies(panda_gen_files ets_meta_gen_h)

if (PANDA_ENABLE_AFL)
    panda_target_sources(arkassembler_fuzz PRIVATE ${EXTENSION_DIR}/ets_meta.cpp)
    panda_target_include_directories(arkassembler_fuzz PUBLIC ${EXTENSION_DIR})
    add_dependencies(arkassembler_fuzz ets_meta_gen_h)
endif()
