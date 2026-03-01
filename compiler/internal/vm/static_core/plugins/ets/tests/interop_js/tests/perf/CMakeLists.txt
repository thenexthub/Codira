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

add_custom_target(ets_interop_js_perf_tests COMMENT "Running all js interop perf tests")

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

set(WARMUP 1000)
set(ITERS 10000)

set(HEAVY_RUN OFF)
if (HEAVY_RUN)
    set(WARMUP 100000)
    set(ITERS 1000000)
endif()

list(APPEND PERF_TESTS
    "voidCall"
    "voidCall10str"
    "voidCall10dbl"
    "strCall"
    "anystrCall"
    "dblCall"
    "anydblCall"
    "anyobjCall"
)

add_custom_target(ets_interop_js_frontend_tests)

foreach(TEST ${PERF_TESTS})
    set(ETS_VERIFICATOR_ERRORS "NodeHasParent,EveryChildHasValidParent,VariableHasScope,NodeHasType,IdentifierHasVariable,ReferenceTypeAnnotationIsNull,ArithmeticOperationValid,SequenceExpressionHasLastType,ForLoopCorrectlyInitialized,VariableHasEnclosingScope,ModifierAccessValid")
    # NOTE(dkofanov): `ImportExportAccessValid` need to be fixed
    # set(ETS_VERIFICATOR_ERRORS "${ETS_VERIFICATOR_ERRORS}:ImportExportAccessValid")
    panda_ets_interop_js_test(ets_interop_js_perf_test_frontend_${TEST}
        ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/test_frontend.ets
        JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_test.js
        JS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/code.js
        LAUNCHER_ARGS ${TEST} ${WARMUP} ${ITERS}
        ETS_CONFIG ${ETS_CONFIG}
        ETS_VERIFICATOR_ERRORS ${ETS_VERIFICATOR_ERRORS}
    )
    add_dependencies(ets_interop_js_frontend_tests ets_interop_js_perf_test_frontend_${TEST})
endforeach()

add_subdirectory(benchmarks)
