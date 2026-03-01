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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_SINGLE_STEP_TEST_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_SINGLE_STEP_TEST_H

#include "test/utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsSingleStepTest : public TestEvents {
public:
    JsSingleStepTest()
    {
        vmDeath = [this]() {
            ASSERT_NE(stepCounter_, 0);  // 0: step counter
            ASSERT_EQ(breakpointCounter_, 2);  // 2: break point counter
            return true;
        };

        loadModule = [this](std::string_view moduleName) {
            runtime_->Enable();
            // 19: line number
            locationStart_ = TestUtil::GetLocation(sourceFile_.c_str(), 19, 0, pandaFile_.c_str());
            // 22: line number
            locationEnd_ = TestUtil::GetLocation(sourceFile_.c_str(), 22, 0, pandaFile_.c_str());
            TestUtil::SuspendUntilContinue(DebugEvent::LOAD_MODULE);
            ASSERT_EQ(moduleName, pandaFile_);
            auto condFuncRef = FunctionRef::Undefined(vm_);
            auto ret = debugInterface_->SetBreakpoint(locationEnd_, condFuncRef);
            ASSERT_TRUE(ret);
            return true;
        };

        breakpoint = [this](const JSPtLocation &location) {
            ASSERT_TRUE(location.GetMethodId().IsValid());
            ASSERT_LOCATION_EQ(location, locationEnd_);
            // Check what step signalled before breakpoint
            ASSERT_LOCATION_EQ(location, locationStep_);
            ASSERT_TRUE(collectSteps_);
            breakpointCounter_++;
            // Disable collect steps
            collectSteps_ = false;
            return true;
        };

        singleStep = [this](const JSPtLocation &location) {
            ASSERT_TRUE(location.GetMethodId().IsValid());
            if (!collectSteps_) {
                if (locationStart_ == location) {
                    collectSteps_ = true;
                }
                return false;
            }

            ASSERT_NE(bytecodeOffset_, location.GetBytecodeOffset());
            locationStep_ = location;
            stepCounter_++;
            bytecodeOffset_ = location.GetBytecodeOffset();
            return false;
        };

        scenario = []() {
            TestUtil::WaitForLoadModule();
            TestUtil::Continue();
            return true;
        };
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "sample.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "sample.js";
    std::string entryPoint_ = "sample";
    JSPtLocation locationStart_ {nullptr, JSPtLocation::EntityId(0), 0};
    JSPtLocation locationEnd_ {nullptr, JSPtLocation::EntityId(0), 0};
    JSPtLocation locationStep_ {nullptr, JSPtLocation::EntityId(0), 0};
    int32_t stepCounter_ = 0;
    int32_t breakpointCounter_ = 0;
    bool collectSteps_ = false;
    uint32_t bytecodeOffset_ = std::numeric_limits<uint32_t>::max();
};

std::unique_ptr<TestEvents> GetJsSingleStepTest()
{
    return std::make_unique<JsSingleStepTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_SINGLE_STEP_TEST_H
