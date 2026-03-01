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

project(isa)

set(ISA_FILE "${CMAKE_BINARY_DIR}/isa/isa.yaml")
set(ISA_ASSERTS "${PROJECT_SOURCE_DIR}/asserts.rb")
set(ISAPI "${PROJECT_SOURCE_DIR}/isapi.rb")

add_custom_target(isa_combine)
set_target_properties(isa_combine PROPERTIES ISA_PLUGIN_FILES "")

function(add_isa_plugin YAML_FILE_PATH)
    get_target_property(YAML_FILES isa_combine ISA_PLUGIN_FILES)
    list(APPEND YAML_FILES ${YAML_FILE_PATH})
    set_target_properties(isa_combine PROPERTIES ISA_PLUGIN_FILES "${YAML_FILES}")
endfunction()

# Check ISA_FILE content

set(ASSERT_MARKER "${PROJECT_BINARY_DIR}/assert.ok")
set(ISA_GEN_RB "${PROJECT_SOURCE_DIR}/gen.rb")
add_custom_command(OUTPUT ${ASSERT_MARKER}
    COMMENT "Check ISA content"
    COMMAND ${ISA_GEN_RB} -d ${ISA_FILE} -q ${ISAPI} -a ${ISA_ASSERTS}
    COMMAND ${CMAKE_COMMAND} -E touch ${ASSERT_MARKER}
    DEPENDS ${ISA_FILE} ${ISA_GEN_RB} ${ISA_ASSERTS} ${ISAPI}
)
add_custom_target(isa_assert DEPENDS ${ASSERT_MARKER})
add_dependencies(isa_assert isa_combine)

# Template generation
set(TEMPLATES
    isa.md.erb
)
panda_isa_gen(TEMPLATES ${TEMPLATES})
