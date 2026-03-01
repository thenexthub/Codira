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

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(irtoc-interpreter-tag-tests)

#Source files:
set(TAG_SOURCE_GENERATOR "${PROJECT_SOURCE_DIR}/generator")
set(TAG_YAML "${CMAKE_BINARY_DIR}/isa/isa.yaml")
set(TAG_TEMPLATE "${PROJECT_SOURCE_DIR}/templates/template.erb")
set(TAG_GENERATOR "${PROJECT_SOURCE_DIR}/generate-tag.rb")
set(ISAPI_FILE "${CMAKE_SOURCE_DIR}/isa/isapi.rb")
set(ISA_FILE "${CMAKE_SOURCE_DIR}/irtoc/lang/isa.rb")

file(
        COPY "${TAG_TEMPLATE}"
        DESTINATION "${PROJECT_BINARY_DIR}"
    )

# Generated files:
set(TAG_GENERATED_TESTS "${PROJECT_BINARY_DIR}/tests-pa/")
set(TAG_COMPILED_TESTS "${PROJECT_BINARY_DIR}/tests/")

file(MAKE_DIRECTORY ${TAG_GENERATED_TESTS})
file(MAKE_DIRECTORY ${TAG_COMPILED_TESTS})

add_custom_target(irtoc-interpreter-tests-tag-generator
        COMMENT "Generate TAG tests files"
        COMMAND ruby
                -I "${TAG_SOURCE_GENERATOR}"
                ${TAG_GENERATOR}
                -t "${TAG_TEMPLATE}"
                -y "${TAG_YAML}"
                -i "${ISAPI_FILE}"
                -o "${TAG_GENERATED_TESTS}"
                -s "${ISA_FILE}"
        DEPENDS "$<TARGET_FILE:ark>"
)

add_custom_target(irtoc-interpreter-tests-tag
        COMMENT "Run generated TAG tests"
        COMMAND ruby
                ${CMAKE_SOURCE_DIR}/tests/cts-generator/test-runner.rb
                -t "${TAG_GENERATED_TESTS}"
                -o "--interpreter-type=irtoc --gc-type=g1-gc"
                -x "${TAG_COMPILED_TESTS}"
                -p "${CMAKE_BINARY_DIR}"
)
add_dependencies(irtoc-interpreter-tests-tag irtoc-interpreter-tests-tag-generator)