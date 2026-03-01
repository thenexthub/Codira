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

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VERBOSE_MAKEFILEON ON)
set(EXTRA_WARNING_SETTINGS "-Wsign-compare")
set(WARNINGS_SETTINGS "-Wall ${EXTRA_WARNING_SETTINGS} -Werror -Wdate-time")
set(C_OTHER_FLAGS "-fsigned-char")
set(CXX_OTHER_FLAGS "-Weffc++")

set(OTHER_FLAGS "-fno-omit-frame-pointer -pipe -fno-common -fno-strict-aliasing -fstack-protector-all")
set(GCOV_FLAGS "-fno-inline -O0 -fprofile-arcs -ftest-coverage")
set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
set(TSAN_FLAGS "-fsanitize=thread")

set(LINK_FLAGS "-Wl,-z,relro,-z,now,-z,noexecstack")
set(STRIP_FLAG "-s")
set(SAFE_EXE_LINK_FLAG "-pie")

set(LINK_FLAGS_BUILD_ID "-Wl,--build-id=none")

set(C_FLAGS "${WARNINGS_SETTINGS} ${C_OTHER_FLAGS} ${OTHER_FLAGS}")
set(CPP_FLAGS "${WARNINGS_SETTINGS} ${CXX_OTHER_FLAGS} ${OTHER_FLAGS}")

set(CMAKE_C_FLAGS "${C_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_C_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os")
set(CMAKE_CXX_FLAGS "${CPP_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fstandalone-debug")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS} ${LINK_FLAGS_BUILD_ID} ${STRIP_FLAG}")
    set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS} ${LINK_FLAGS_BUILD_ID} ${STRIP_FLAG}")
else()
    set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS} ${LINK_FLAGS_BUILD_ID}")
endif()

set(LINKER_OPTION_PREFIX "-Wl,")
set(MAKE_SO_STACK_PROTECTOR_OPTION)
set(LLVM_BUILD_C_COMPILER ${CMAKE_C_COMPILER})
set(LLVM_BUILD_CXX_COMPILER ${CMAKE_CXX_COMPILER})

if(CODIRA_TARGET_SYSROOT)
    set(CMAKE_SYSROOT ${CODIRA_TARGET_SYSROOT})
endif()
