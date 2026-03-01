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

set(GEN_INCLUDE_DIR "${PANDA_BINARY_ROOT}/runtime/include")

# Add icu dependency and initialization
add_custom_command(OUTPUT ${GEN_INCLUDE_DIR}/init_icu_gen.cpp
    COMMAND ${PANDA_ROOT}/runtime/templates/substitute_icu_path.rb
            ${PANDA_ROOT}/runtime/templates/init_icu_gen.cpp.erb
            ${GEN_INCLUDE_DIR}/init_icu_gen.cpp ${PANDA_THIRD_PARTY_SOURCES_DIR}
    DEPENDS ${PANDA_ROOT}/runtime/templates/init_icu_gen.cpp.erb)

add_custom_target(init_icu_gen_cpp
    DEPENDS ${GEN_INCLUDE_DIR}/init_icu_gen.cpp)

add_dependencies(arkruntime_obj init_icu_gen_cpp)
add_dependencies(panda_gen_files init_icu_gen_cpp)

# Runtime uses unicode and i18n.
set(ICU_INCLUDE_DIR
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/icu/icu4c/source/common
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/icu/icu4c/source/i18n
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/icu/icu4c/source
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/icu
)

set(ICU_LIB_TYPE SHARED)
if (PANDA_TARGET_MOBILE OR PANDA_TARGET_OHOS)
    set(ICU_LIB_TYPE STATIC)
endif()

panda_add_library(init_icu ${ICU_LIB_TYPE} ${PANDA_ROOT}/runtime/init_icu.cpp)
panda_target_include_directories(init_icu
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${PANDA_ROOT}/runtime/
)

panda_target_include_directories(init_icu SYSTEM PUBLIC ${ICU_INCLUDE_DIR})

add_dependencies(init_icu init_icu_gen_cpp)

panda_target_include_directories(arkruntime_interpreter_impl
    SYSTEM PRIVATE ${ICU_INCLUDE_DIR}
)

panda_target_include_directories(csa_tests_arkruntime_interpreter_impl
    SYSTEM PRIVATE ${ICU_INCLUDE_DIR}
)

if (TARGET arkruntime_test_interpreter_impl)
    panda_target_include_directories(arkruntime_test_interpreter_impl
        SYSTEM PRIVATE ${ICU_INCLUDE_DIR}
    )
endif()

panda_target_include_directories(asm_defines
    SYSTEM PRIVATE ${ICU_INCLUDE_DIR}
)

panda_target_link_libraries(arkruntime_obj init_icu hmicuuc.z hmicui18n.z)
