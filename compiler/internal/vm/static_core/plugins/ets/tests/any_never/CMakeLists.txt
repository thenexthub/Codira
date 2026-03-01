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

set(ANY_NEVER_TEST_LIST
    "any_general"
    "any_array_general"
    "never_general"
)

add_custom_target(ets_tests_any_never)
add_dependencies(ets_tests ets_tests_any_never)

foreach(NAME ${ANY_NEVER_TEST_LIST})
    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/any_never/${NAME}.pa
        TARGET ets_any_never_${NAME}_cpp
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        RUNTIME_OPTIONS "--interpreter-type=cpp" "--verification-mode=disabled"
    )
    add_dependencies(ets_tests_any_never ets_any_never_${NAME}_cpp)

    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/any_never/${NAME}.pa
        TARGET ets_any_never_${NAME}_irtoc
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        RUNTIME_OPTIONS "--interpreter-type=irtoc" "--verification-mode=disabled"
    )
    add_dependencies(ets_tests_any_never ets_any_never_${NAME}_irtoc)

    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/any_never/${NAME}.pa
        TARGET ets_any_never_${NAME}_cpp_opt
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        ASSEMBLER_OPTIONS "--optimize"
        RUNTIME_OPTIONS "--interpreter-type=cpp" "--verification-mode=disabled"
    )
    add_dependencies(ets_tests_any_never ets_any_never_${NAME}_cpp_opt)

    panda_add_asm_file(
        FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/any_never/${NAME}.pa
        TARGET ets_any_never_${NAME}_irtoc_opt
        DEPENDS etsstdlib
        LANGUAGE_CONTEXT ets
        ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
        ASSEMBLER_OPTIONS "--optimize"
        RUNTIME_OPTIONS "--interpreter-type=irtoc" "--verification-mode=disabled"
    )
    add_dependencies(ets_tests_any_never ets_any_never_${NAME}_irtoc_opt)

    if(PANDA_COMPILER_ENABLE AND NOT PANDA_TARGET_ARM32)

        # 26982 64-bit pointer support
        if (PANDA_32_BIT_MANAGED_POINTER OR NOT PANDA_TARGET_ARM64)
            panda_add_asm_file(
                FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/any_never/${NAME}.pa
                TARGET ets_any_never_${NAME}_enforce_jit
                DEPENDS etsstdlib
                LANGUAGE_CONTEXT ets
                ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                ASSEMBLER_OPTIONS "--optimize"
                RUNTIME_OPTIONS "--compiler-enable-jit=true" "--verification-mode=disabled" "--compiler-hotness-threshold=0" "--no-async-jit"
            )
            add_dependencies(ets_tests_any_never ets_any_never_${NAME}_enforce_jit)
        endif()

        if (PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64)
            panda_add_asm_file(
                FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/any_never/${NAME}.pa
                TARGET ets_any_never_${NAME}_aot
                DEPENDS etsstdlib
                LANGUAGE_CONTEXT ets
                AOT_MODE TRUE
                ADDITIONAL_STDLIBS $<TARGET_PROPERTY:etsstdlib,FILE>
                ASSEMBLER_OPTIONS "--optimize"
                RUNTIME_OPTIONS "--compiler-enable-jit=true" "--verification-mode=disabled"
            )
            add_dependencies(ets_tests_any_never ets_any_never_${NAME}_aot)
        endif()
    endif()
endforeach()
