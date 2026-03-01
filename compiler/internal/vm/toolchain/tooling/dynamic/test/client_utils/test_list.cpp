/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "tooling/dynamic/test/client_utils/test_list.h"

#include "tooling/dynamic/test/client_utils/test_util.h"

// testcase list
#include "tooling/dynamic/test/testcases/js_accelerate_launch_test.h"
#include "tooling/dynamic/test/testcases/js_allocationtrack_loop_test.h"
#include "tooling/dynamic/test/testcases/js_allocationtrack_recursion_test.h"
#include "tooling/dynamic/test/testcases/js_allocationtrack_test.h"
#include "tooling/dynamic/test/testcases/js_breakpoint_arrow_test.h"
#include "tooling/dynamic/test/testcases/js_breakpoint_async_test.h"
#include "tooling/dynamic/test/testcases/js_breakpoint_cannot_hit_test.h"
#include "tooling/dynamic/test/testcases/js_breakpoint_in_different_branch.h"
#include "tooling/dynamic/test/testcases/js_breakpoint_loop_test.h"
#include "tooling/dynamic/test/testcases/js_breakpoint_recursion_test.h"
#include "tooling/dynamic/test/testcases/js_breakpoint_switch_test.h"
#include "tooling/dynamic/test/testcases/js_breakpoint_test.h"
#include "tooling/dynamic/test/testcases/js_closure_scope_test.h"
#include "tooling/dynamic/test/testcases/js_container_test.h"
#include "tooling/dynamic/test/testcases/js_exception_test.h"
#include "tooling/dynamic/test/testcases/js_heapdump_loop_test.h"
#include "tooling/dynamic/test/testcases/js_heapdump_test.h"
#include "tooling/dynamic/test/testcases/js_heapusage_async_test.h"
#include "tooling/dynamic/test/testcases/js_heapusage_loop_test.h"
#include "tooling/dynamic/test/testcases/js_heapusage_recursion_test.h"
#include "tooling/dynamic/test/testcases/js_heapusage_step_test.h"
#include "tooling/dynamic/test/testcases/js_heapusage_test.h"
#include "tooling/dynamic/test/testcases/js_local_variable_scope_test.h"
#include "tooling/dynamic/test/testcases/js_module_variable_test.h"
#include "tooling/dynamic/test/testcases/js_multiple_breakpoint_in_function_test.h"
#include "tooling/dynamic/test/testcases/js_multiple_common_breakpoint_test.h"
#include "tooling/dynamic/test/testcases/js_smart_stepInto_test.h"
#include "tooling/dynamic/test/testcases/js_source_test.h"
#include "tooling/dynamic/test/testcases/js_special_location_breakpoint_test.h"
#include "tooling/dynamic/test/testcases/js_stepinto_and_stepout_test.h"
#include "tooling/dynamic/test/testcases/js_stepinto_arrow_test.h"
#include "tooling/dynamic/test/testcases/js_stepinto_async_test.h"
#include "tooling/dynamic/test/testcases/js_stepinto_loop_test.h"
#include "tooling/dynamic/test/testcases/js_stepinto_recursion_test.h"
#include "tooling/dynamic/test/testcases/js_stepinto_switch_test.h"
#include "tooling/dynamic/test/testcases/js_stepinto_test.h"
#include "tooling/dynamic/test/testcases/js_stepout_arrow_test.h"
#include "tooling/dynamic/test/testcases/js_stepout_async_test.h"
#include "tooling/dynamic/test/testcases/js_stepout_before_function_test.h"
#include "tooling/dynamic/test/testcases/js_stepout_loop_test.h"
#include "tooling/dynamic/test/testcases/js_stepout_recursion_test.h"
#include "tooling/dynamic/test/testcases/js_stepout_switch_test.h"
#include "tooling/dynamic/test/testcases/js_stepout_test.h"
#include "tooling/dynamic/test/testcases/js_stepover_loop_test.h"
#include "tooling/dynamic/test/testcases/js_stepover_recursion_test.h"
#include "tooling/dynamic/test/testcases/js_stepover_switch_test.h"
#include "tooling/dynamic/test/testcases/js_stepover_test.h"
#include "tooling/dynamic/test/testcases/js_tracing_test.h"
#include "tooling/dynamic/test/testcases/js_watch_basic_type_test.h"
#include "tooling/dynamic/test/testcases/js_watch_closure_variable_test.h"
#include "tooling/dynamic/test/testcases/js_watch_other_type_test.h"
#include "tooling/dynamic/test/testcases/js_watch_set_type_test.h"
#include "tooling/dynamic/test/testcases/js_watch_test.h"
#include "tooling/dynamic/test/testcases/js_watch_module_test.h"
#include "tooling/dynamic/test/testcases/js_watch_variable_test.h"
#include "tooling/dynamic/test/testcases/js_symbolbreakpoint_test.h"
#include "tooling/dynamic/test/testcases/js_variable_property_with_range_test.h"

namespace panda::ecmascript::tooling::test {
static std::string g_currentTestName = "";

static void RegisterTests()
{
    // Register testcases
    TestUtil::RegisterTest("JsBreakpointTest", GetJsBreakpointTest());
    TestUtil::RegisterTest("JsBreakpointArrowTest", GetJsBreakpointArrowTest());
    TestUtil::RegisterTest("JsBreakpointAsyncTest", GetJsBreakpointAsyncTest());
    TestUtil::RegisterTest("JsClosureScopeTest", GetJsClosureScopeTest());
    TestUtil::RegisterTest("JsLocalVariableScopeTest", GetJsLocalVariableScopeTest());
    TestUtil::RegisterTest("JsExceptionTest", GetJsExceptionTest());
    TestUtil::RegisterTest("JsContainerTest", GetJsContainerTest());
    TestUtil::RegisterTest("JsModuleVariableTest", GetJsModuleVariableTest());
    TestUtil::RegisterTest("JsWatchModuleTest", GetJsWatchModuleTest());
    TestUtil::RegisterTest("JsSourceTest", GetJsSourceTest());
    TestUtil::RegisterTest("JsTracingTest", GetJsTracingTest());
    TestUtil::RegisterTest("JsWatchTest", GetJsWatchTest());
    if (!g_isEnableCMCGC) {
        // Need support heapdump
        TestUtil::RegisterTest("JsHeapdumpTest", GetJsHeapdumpTest());
        TestUtil::RegisterTest("JsHeapdumpLoopTest", GetJsHeapdumpLoopTest());
    }
    TestUtil::RegisterTest("JsStepintoTest", GetJsStepintoTest());
    TestUtil::RegisterTest("JsStepoutTest", GetJsStepoutTest());
    TestUtil::RegisterTest("JsStepoverTest", GetJsStepoverTest());
    TestUtil::RegisterTest("JsStepintoAndStepoutTest", GetJsStepintoAndStepoutTest());
    TestUtil::RegisterTest("JsStepoutBeforeFunctiontTest", GetJsStepoutBeforeFunctionTest());
    TestUtil::RegisterTest("JsSpecialLocationBreakpointTest", GetJsSpecialLocationBreakpointTest());
    TestUtil::RegisterTest("JsMultipleCommonBreakpointTest", GetJsMultipleCommonBreakpointTest());
    TestUtil::RegisterTest("JsMultipleBreakpointInFunctionTest", GetJsMultipleBreakpointInFunctionTest());
    TestUtil::RegisterTest("JsBreakpointCannotHitTest", GetJsBreakpointCannotHitTest());
    TestUtil::RegisterTest("JsBreakpointInDifferentBranchTest", GetJsBreakpointInDifferentBranchTest());
    TestUtil::RegisterTest("JsWatchVariableTest", GetJsWatchVariableTest());
    TestUtil::RegisterTest("JsStepintoArrowTest", GetJsStepintoArrowTest());
    TestUtil::RegisterTest("JsStepintoAsyncTest", GetJsStepintoAsyncTest());
    TestUtil::RegisterTest("JsStepoutArrowTest", GetJsStepoutArrowTest());
    TestUtil::RegisterTest("JsStepoutAsyncTest", GetJsStepoutAsyncTest());
    TestUtil::RegisterTest("JsHeapusageTest", GetJsHeapusageTest());
    TestUtil::RegisterTest("JsHeapusageAsyncTest", GetJsHeapusageAsyncTest());
    TestUtil::RegisterTest("JsHeapusageStepTest", GetJsHeapusageStepTest());
    if (!g_isEnableCMCGC) {
        // Need support allocation tracker
        TestUtil::RegisterTest("JsAllocationtrackTest", GetJsAllocationtrackTest());
        TestUtil::RegisterTest("JsAllocationTrackLoopTest", GetJsAllocationTrackLoopTest());
        TestUtil::RegisterTest("JsAllocationTrackRecursionTest", GetJsAllocationTrackRecursionTest());
    }
    TestUtil::RegisterTest("JsJsWatchBasicTypeTest", GetJsWatchBasicTypeTest());
    TestUtil::RegisterTest("JsJsWatchSetTypeTest", GetJsWatchSetTypeTest());
    TestUtil::RegisterTest("JsJsWatchOtherTypeTest", GetJsWatchOtherTypeTest());
    TestUtil::RegisterTest("JsWatchClosureVariableTest", GetJsWatchClosureVariableTest());
    TestUtil::RegisterTest("JsStepintoLoopTest", GetJsStepintoLoopTest());
    TestUtil::RegisterTest("JsStepintoRecursionTest", GetJsStepintoRecursionTest());
    TestUtil::RegisterTest("JsStepintoSwitchTest", GetJsStepintoSwitchTest());
    TestUtil::RegisterTest("JsStepoverLoopTest", GetJsStepoverLoopTest());
    TestUtil::RegisterTest("JsStepoverRecursionTest", GetJsStepoverRecursionTest());
    TestUtil::RegisterTest("JsStepoverSwitchTest", GetJsStepoverSwitchTest());
    TestUtil::RegisterTest("JsStepoutLoopTest", GetJsStepoutLoopTest());
    TestUtil::RegisterTest("JsStepoutRecursionTest", GetJsStepoutRecursionTest());
    TestUtil::RegisterTest("JsStepoutSwitchTest", GetJsStepoutSwitchTest());
    TestUtil::RegisterTest("JsBreakpointLoopTest", GetJsBreakpointLoopTest());
    TestUtil::RegisterTest("JsBreakpointRecursionTest", GetJsBreakpointRecursionTest());
    TestUtil::RegisterTest("JsBreakpointSwitchTest", GetJsBreakpointSwitchTest());
    TestUtil::RegisterTest("JsHeapusageLoopTest", GetJsHeapusageLoopTest());
    TestUtil::RegisterTest("JsHeapusageRecursionTest", GetJsHeapusageRecursionTest());
    TestUtil::RegisterTest("JsSmartStepoutTest", GetJsSmartStepoutTest());
    TestUtil::RegisterTest("JsAccelerateLaunchTest", GetJsAccelerateLaunchTest());
    TestUtil::RegisterTest("JsSymbolicBreakpointTest", GetJsSymbolicBreakpointTest());
    TestUtil::RegisterTest("JsVariablePropertyWithRangeTest", GetJsVariablePropertyWithRangeTest());
}

std::vector<const char *> GetTestList()
{
    RegisterTests();
    std::vector<const char *> res;

    auto &tests = TestUtil::GetTests();
    for (const auto &entry : tests) {
        res.push_back(entry.first.c_str());
    }
    return res;
}

void SetCurrentTestName(const std::string &testName)
{
    g_currentTestName = testName;
}

std::string GetCurrentTestName()
{
    return g_currentTestName;
}

std::pair<std::string, std::string> GetTestEntryPoint(const std::string &testName)
{
    return TestUtil::GetTest(testName)->GetEntryPoint();
}
}  // namespace panda::ecmascript::tooling::test
