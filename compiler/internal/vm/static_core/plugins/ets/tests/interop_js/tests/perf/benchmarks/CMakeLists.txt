# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(WARMUP 50000)
set(ITERS 100000)

set(HEAVY_RUN OFF)
if (HEAVY_RUN)
    set(WARMUP 500000)
    set(ITERS 1000000)
endif()

list(APPEND BENCHMARKS
    "voidPasser"
    "numPasser"
    "strPasser"
    "arrPasser"
    "arrGetter"
    "strGetter"
    "objGetter"
    "numGetter"
)

list(APPEND EXTRA_BENCHMARKS_STS_JS
    "arrAnyGetter"
    "strAnyGetter"
    "numAnyGetter"
)

list(APPEND SITES
    "JS"
    "STS"
)

add_custom_target(ets_interop_js_perf_benchmarks)

macro(add_ets_interop_js_benchmark BENCHMARK CALLER CALLEE)
    # NOTE: must be refactored, because not all targets has dependency from caller.js and callee.js
    panda_ets_interop_js_test(ets_interop_js_perf_benchmarks_${CALLER}_${CALLEE}_${BENCHMARK}
        ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/caller.ets ${CMAKE_CURRENT_LIST_DIR}/callee.ets
        JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/launcher.js
        JS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/callee.js ${CMAKE_CURRENT_LIST_DIR}/caller.js
        LAUNCHER_ARGS ${BENCHMARK} ${CALLER} ${CALLEE} ${WARMUP} ${ITERS}
        COMPILATION_JS_WITH_CODES_ON
        PACKAGE_NAME "caller"
    )
    add_dependencies(ets_interop_js_perf_benchmarks_${CALLER}_${CALLEE} ets_interop_js_perf_benchmarks_${CALLER}_${CALLEE}_${BENCHMARK})
endmacro()

foreach(CALLER ${SITES})
    foreach(CALLEE ${SITES})
        add_custom_target(ets_interop_js_perf_benchmarks_${CALLER}_${CALLEE})
        foreach(BENCHMARK ${BENCHMARKS})
            add_ets_interop_js_benchmark(${BENCHMARK} ${CALLER} ${CALLEE})
        endforeach()
        foreach(BENCHMARK ${EXTRA_BENCHMARKS_${CALLER}_${CALLEE}})
            add_ets_interop_js_benchmark(${BENCHMARK} ${CALLER} ${CALLEE})
        endforeach()
        add_dependencies(ets_interop_js_perf_benchmarks ets_interop_js_perf_benchmarks_${CALLER}_${CALLEE})
    endforeach()
endforeach()

add_dependencies(ets_interop_js_perf_tests ets_interop_js_perf_benchmarks)
