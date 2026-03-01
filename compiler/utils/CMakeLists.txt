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

# copy envsetup file to output folder
if(WIN32)
    set(SCRIPT_SUFFIX bat ps1 sh)
elseif(UNIX)
    set(SCRIPT_SUFFIX sh)
endif()

set(ENV_FILE)
set(RUNTIME_ENV_FILE)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    string(TOLOWER "llvm_${CMAKE_SYSTEM_NAME}" HOST_PLATFORM)
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        string(TOLOWER "_${CMAKE_SYSTEM_PROCESSOR}" HARDWARE_ARCH)
        string(APPEND HOST_PLATFORM ${HARDWARE_ARCH})
    endif()
    foreach(suffix ${SCRIPT_SUFFIX})
        list(APPEND ENV_FILE ${CMAKE_SOURCE_DIR}/utils/envsetup/${HOST_PLATFORM}.${suffix})
        list(APPEND RUNTIME_ENV_FILE ${CMAKE_SOURCE_DIR}/utils/envsetup/runtime/${HOST_PLATFORM}.${suffix})
    endforeach()
endif()

foreach(filename ${ENV_FILE})
    if(EXISTS ${filename})
        get_filename_component(suffix ${filename} LAST_EXT)
        install(
            FILES ${filename}
            DESTINATION bin/../
            RENAME envsetup${suffix})
    endif()
endforeach()

foreach(filename ${RUNTIME_ENV_FILE})
    if(EXISTS ${filename})
        get_filename_component(suffix ${filename} LAST_EXT)
        install(
            FILES ${filename}
            DESTINATION runtime/
            RENAME envsetup${suffix})
    endif()
endforeach()

if(DARWIN)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/entitlement.plist
        DESTINATION lib
    )
endif()

if(CODIRA_ENABLE_COMPILER_TSAN)
    add_compile_options("${TSAN_FLAGS}")
    add_link_options("${TSAN_FLAGS}")
endif()

# regenerate unicode data tables on need
if(CODIRA_GENERATE_UNICODE_TABLE)
    add_subdirectory(UnicodeScripts)
endif()
