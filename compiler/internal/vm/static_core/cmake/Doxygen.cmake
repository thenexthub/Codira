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
# Convenience functions for autogenerate documentation.
option(ENABLE_DOXYGEN "Enable documentation generation" false)

add_custom_target(doc_build COMMENT "Building doxygen documentation")

# Example usage:
#
#   add_doxygen(
#     NAME      "project_name"
#     PATH      "${ROOT_PROJECT_PATH}"
#   )
#
# Notes:
#    * This function is a no-op if Doxygen is not found.
#

function(add_doxygen)
    set(prefix ARG)
    set(noValues)
    set(singleValues NAME)

    find_package(Doxygen)

    if ((NOT ENABLE_DOXYGEN) OR (NOT DOXYGEN_FOUND))
        return()
    endif()

    set(DOXYGEN_CONFIG_TEMPLATE "${PANDA_ROOT}/docs/doxygen.config")

    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          "${multiValues}"
                          ${ARGN})

    set(DOXYGEN_CONF_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Configuration - replace @CMAKE_MACRO@ definition in config template
    CONFIGURE_FILE(${DOXYGEN_CONFIG_TEMPLATE} ${DOXYGEN_CONF_OUT})

    add_custom_target( doc_${ARG_NAME}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONF_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )

    add_dependencies(doc_build doc_${ARG_NAME})
endfunction()
