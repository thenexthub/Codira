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

function(panda_ets_interop_js_test_timer)
    list(GET ARGV 0 TEST_NAME)
    list(GET ARGV 1 ENTRY_FUNC)

    panda_ets_interop_js_test(${TEST_NAME}_internal_timer
        ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
        JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
        LAUNCHER_ARGS ${ENTRY_FUNC} "false"
    )

    panda_ets_interop_js_test(${TEST_NAME}_external_timer
        ETS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/timer_tests.ets
        JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_timer_test.js
        LAUNCHER_ARGS ${ENTRY_FUNC} "true"
    )

endfunction()

panda_ets_interop_js_test_timer(ets_interop_js_test_settimeout "testSetTimeout")

panda_ets_interop_js_test_timer(ets_interop_js_test_cleartimeout "testClearTimeout")

panda_ets_interop_js_test_timer(ets_interop_js_test_setinterval "testSetInterval")

panda_ets_interop_js_test_timer(ets_interop_js_test_settimeout_execute_order "testSetTimeoutExecuteOrder")

panda_ets_interop_js_test_timer(ets_interop_js_test_settimeout_invoke_callback_with_specified_parameter "testSetTimeoutInvokeCallbackWithSpecifiedParameter")

panda_ets_interop_js_test_timer(ets_interop_js_test_settimeout_invoke_callback_with_4_specified_parameters "testSetTimeoutInvokeCallbackWith4SpecifiedParameters")

panda_ets_interop_js_test_timer(ets_interop_js_test_setinterval_invoke_callback_with_specified_parameter "testSetIntervalInvokeCallbacWithSpecifiedParameter")

panda_ets_interop_js_test_timer(ets_interop_js_test_setinterval_invoke_callback_with_4_specified_parameters "testSetIntervalInvokeCallbacWith4SpecifiedParameters")

panda_ets_interop_js_test_timer(ets_interop_js_test_settimeout_with_negitive_delay "testSetTimeoutWithNegitiveDelay")

panda_ets_interop_js_test_timer(ets_interop_js_test_setinterval_with_negitive_delay "testSetIntervalWithNegitiveDelay")

panda_ets_interop_js_test_timer(ets_interop_js_test_settimeout_with_string "testSetTimeoutWithString")

panda_ets_interop_js_test_timer(ets_interop_js_test_setinterval_with_string "testSetIntervalWithString")

panda_ets_interop_js_test_timer(ets_interop_js_test_clear_timer_cross_worker "testClearTimerCrossWorker")
