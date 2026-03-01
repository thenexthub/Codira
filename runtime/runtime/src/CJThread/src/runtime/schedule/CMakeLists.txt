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

set(SRC_LIST)
aux_source_directory(src SRC_LIST)

add_library(schedule OBJECT ${SRC_LIST})

target_link_libraries(schedule PUBLIC queue mid list netpoll basetime external)

target_include_directories(schedule PUBLIC include)
target_include_directories(schedule PUBLIC include/inner)
target_include_directories(schedule PUBLIC include/inner/gas)
target_include_directories(schedule PUBLIC include/inner/gas/${CPU_FAMILY}/${CPU_CORE})

file(GLOB ADDITIONAL_SRC_FILES
        src/gas/${CPU_FAMILY}/${CPU_CORE}/*.s
        src/gas/${CPU_FAMILY}/${CPU_CORE}/*.S
        )
target_sources(schedule PRIVATE ${ADDITIONAL_SRC_FILES})

file(COPY include/schedule.h DESTINATION ${PROJECT_SOURCE_DIR}/../../output/temp/include/)
file(COPY include/inner/gas/${CPU_FAMILY}/${CPU_CORE}/codethread_context.h DESTINATION ${PROJECT_SOURCE_DIR}/../../output/temp/include/)
