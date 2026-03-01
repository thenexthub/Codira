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

set(ISA_FILE "${CMAKE_BINARY_DIR}/isa/isa.yaml")
set(ISA_CORE_FILE "${PROJECT_SOURCE_DIR}/isa/isa.yaml")
set(ISA_COMBINE "${PROJECT_SOURCE_DIR}/isa/combine.rb")
set(ISA_PLUGIN_FILES "")

get_target_property(ISA_PLUGIN_FILES isa_combine ISA_PLUGIN_FILES)

string(REPLACE ";" "," ISA_PLUGIN_FILES_STR "${ISA_PLUGIN_FILES}")

# ISA combine
add_custom_command(OUTPUT ${ISA_FILE}
    COMMENT "Combine ISA files"
    COMMAND ${ISA_COMBINE} -d "${ISA_CORE_FILE},${ISA_PLUGIN_FILES_STR}" -o ${ISA_FILE}
    DEPENDS ${ISA_COMBINE} ${ISA_CORE_FILE} ${ISA_PLUGIN_FILES}
)
add_custom_target(isa_plugin_gen DEPENDS ${ISA_FILE})
add_dependencies(isa_combine isa_plugin_gen)
