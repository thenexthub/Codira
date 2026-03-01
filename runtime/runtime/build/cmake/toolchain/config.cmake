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

# compile flags for debug version only
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-4")

if("${BUILDING_STAGE}" STREQUAL "asan")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize-recover=address")
elseif("${BUILDING_STAGE}" STREQUAL "publish")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")
elseif("${BUILDING_STAGE}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wframe-larger-than=2048")
endif()

# compile flags for release version only
set(CMAKE_C_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_CXX_FLAGS_RELEASE "")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -D_FORTIFY_SOURCE=2 -O2")

# assemble flags
set(CMAKE_ASM_FLAGS
    "${CMAKE_C_FLAGS}"
)
 
# Attention we need to clear CMAKE_ASM_FLAGS_DEBUG and CMAKE_ASM_FLAGS_RELEASE
# otherwise cmake will add some default compile option which we may not want
 
# assemble flags for debug version only
set(CMAKE_ASM_FLAGS_DEBUG "")
 
# assemble flags for release version only
set(CMAKE_ASM_FLAGS_RELEASE "")