# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

add_subdirectory(Format)
add_executable(codefmt main.cpp)
target_link_libraries(codefmt
        cangjie
        CodiraFormat
        ${labList}
        )
if (CMAKE_BUILD_TYPE MATCHES Release AND MINGW)
    target_link_options(codefmt PRIVATE -static-libgcc -static-libstdc++ LINKER:-Bstatic,--whole-archive -lwinpthread LINKER:-Bdynamic,--no-whole-archive)
endif ()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(codefmt PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Wl,-rpath,@loader_path/../lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(codefmt PROPERTIES LINK_FLAGS "-Wl,-rpath,\\$ORIGIN/../lib")
else()
endif()
