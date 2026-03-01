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

# Generate codira code for libast using flatc with codira backend.
set(FLATC_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/flatc")

set(FLATBUFFERS_SOURCE
    ModuleFormat.fbs
    CachedASTFormat.fbs
    NodeFormat.fbs)

list(APPEND FLATBUFFERS_SOURCE BCHIRFormat.fbs PackageFormat.fbs)
list(APPEND FLATBUFFERS_SOURCE MacroMsgFormat.fbs)

set(FLATC_OUTPUTS)
foreach(FILE ${FLATBUFFERS_SOURCE})
    get_filename_component(FILENAME ${FILE} NAME_WE)
    set(FLATC_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/flatbuffers/${FILENAME}_generated.h")

    add_custom_command(
        OUTPUT ${FLATC_OUTPUT}
        COMMAND ${FLATC_EXECUTABLE} ARGS --no-warnings -c -o "${CMAKE_CURRENT_BINARY_DIR}/flatbuffers" ${FILE}
        DEPENDS ${FILE} flatbuffers # flatc defined in flatbuffers project: add_executable(flatc..)
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    list(APPEND FLATC_OUTPUTS ${FLATC_OUTPUT})
endforeach()

set(FLATBUFFERS_INCLUDE_DIR
    ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/include
    PARENT_SCOPE)
add_custom_target(CodiraFlatbuffersHeaders DEPENDS ${FLATC_OUTPUTS} ${FLATC_LIBAST_OUTPUTS})
