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


# Create js plugin
#
# Example usage:
#   panda_ets_interop_js_plugin(plugin_name
#     SOURCES
#       path/to/file0.cpp
#       path/to/file1.cpp
#     LIBRARIES
#       lib_name0
#       lib_name1
#     LIBRARY_OUTPUT_DIRECTORY
#   )
function(panda_ets_interop_js_plugin TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "LIBRARY_OUTPUT_DIRECTORY"
        "SOURCES;LIBRARIES;OUTPUT_SUFFIX"
        ${ARGN}
    )

    if(NOT DEFINED ARG_LIBRARY_OUTPUT_DIRECTORY)
        # Set default value
        set(ARG_LIBRARY_OUTPUT_DIRECTORY ${PANDA_BINARY_ROOT}/lib/module)
    endif()

    panda_add_library(${TARGET} SHARED ${ARG_SOURCES})
    panda_target_link_libraries(${TARGET} ${ARG_LIBRARIES})

    if(PANDA_TARGET_OHOS)
        panda_target_link_libraries(${TARGET} ace_napi.z  -Wl,--no-undefined)
    else()
        panda_target_include_directories(${TARGET} SYSTEM PRIVATE ${NAPI_HEADERS_PATH})
        set_target_properties(${TARGET}
            PROPERTIES
                # Set module name
                PREFIX ""
                OUTPUT_NAME "${TARGET}"
                SUFFIX ${ARG_OUTPUT_SUFFIX}

                # Set output direcory
                LIBRARY_OUTPUT_DIRECTORY ${ARG_LIBRARY_OUTPUT_DIRECTORY}
        )
    endif()
endfunction(panda_ets_interop_js_plugin)

set(NODE_BINARY "node")
if(NOT PANDA_TARGET_OHOS AND NOT NAPI_HEADERS_PATH)
    if (NOT PANDA_TARGET_AMD64 OR NOT PANDA_TARGET_LINUX)
        message(FATAL_ERROR "By default interop is supported on linux-x64 and OHOS. Set NAPI_HEADERS_PATH to use on other platforms")
    endif()

    if(PANDA_JS_ETS_HYBRID_MODE)
        set(ARKUI_NAPI_HEADERS_PATH ${PANDA_ROOT}/../../../foundation/arkui/napi)
        if (NOT EXISTS ${ARKUI_NAPI_HEADERS_PATH})
            message(FATAL_ERROR "Please use panda js ets hybrid mode in ohos build")
        endif()
    endif()
    # install node
    set(NODE_VERSION "v18.13.0")
    set(DISTRO "linux-x64")

    set(NODE_INSTALL_DIR ${PANDA_ROOT}/third_party/nodejs/node-${NODE_VERSION}-${DISTRO})

    if(NOT EXISTS ${NODE_INSTALL_DIR})
        execute_process(COMMAND ${PANDA_ROOT}/scripts/install-third-party --node
                WORKING_DIRECTORY ${PANDA_ROOT}
                RESULT_VARIABLE NODEJS_DOWNLOAD_OK)
        if (NOT NODEJS_DOWNLOAD_OK EQUAL 0)
            message(FATAL_ERROR "Unable to install required nodejs dependencies")
        endif()
    endif()

    set(NAPI_HEADERS_PATH ${PANDA_ROOT}/third_party/nodejs/node-${NODE_VERSION}-${DISTRO}/include/node)
    set(NODE_BINARY ${PANDA_ROOT}/third_party/nodejs/node-${NODE_VERSION}-${DISTRO}/bin/node)
endif()
