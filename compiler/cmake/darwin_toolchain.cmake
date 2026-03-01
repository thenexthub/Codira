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

set(CMAKE_SYSTEM_NAME Darwin)
set(DARWIN ON)

set(WARNINGS_SETTINGS "-Wall -Werror -Wdate-time ${CUSTOM_WARNING_SETTINGS}")
set(C_OTHER_FLAGS "-fsigned-char")
set(CXX_OTHER_FLAGS "-Weffc++")
set(OTHER_FLAGS "-pipe -fno-common -fno-strict-aliasing -fPIC -fstack-protector-all")

set(STRIP_FLAG "-s")

set(C_FLAGS "${WARNINGS_SETTINGS} ${C_OTHER_FLAGS} ${OTHER_FLAGS}")
set(CPP_FLAGS "${WARNINGS_SETTINGS} ${CXX_OTHER_FLAGS} ${OTHER_FLAGS}")

set(CMAKE_C_FLAGS "${C_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-D_FORTIFY_SOURCE=2 -O2 -g")
set(CMAKE_C_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS "${CPP_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-D_FORTIFY_SOURCE=2 -O2 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fstandalone-debug")
if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS} ${STRIP_FLAG} ")
else()
    set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}")
endif()

find_program(CMAKE_AR llvm-ar REQUIRED)
find_program(CMAKE_RANLIB llvm-ranlib REQUIRED)
set(MAKE_SO_STACK_PROTECTOR_OPTION)
set(LLVM_BUILD_C_COMPILER ${CMAKE_C_COMPILER})
set(LLVM_BUILD_CXX_COMPILER ${CMAKE_CXX_COMPILER})

if(CODIRA_TARGET_SYSROOT)
    set(CMAKE_SYSROOT ${CODIRA_TARGET_SYSROOT})
endif()
