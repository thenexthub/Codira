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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_STEP_OVER_TEST_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_STEP_OVER_TEST_H

#include "test/utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsStepOverTest : public TestEvents {
public:
    JsStepOverTest()
    {
        vmDeath = [this]() {
            ASSERT_EQ(breakpointCounter_, pointerLocations_.size());  // size: break point counter
            ASSERT_EQ(stepCompleteCounter_, stepLocations_.size());  // size: step complete counter
            return true;
        };

        loadModule = [this](std::string_view moduleName) {
            runtime_->Enable();
            // 24、27: line number for breakpoint array
            size_t breakpoint[8][2] = {{24, 0}, {27, 0}, {36, 0}, {50, 0}, {60, 0}, {90, 0}, {96, 0}, {54, 0}};
            // 25、28: line number for stepinto array
            size_t stepOver[7][2] = {{25, 5}, {28, 0}, {37, 0}, {51, 0}, {61, 0}, {91, 5}, {97, 15}};
            SetJSPtLocation(breakpoint[0], POINTER_SIZE, pointerLocations_);
            SetJSPtLocation(stepOver[0], STEP_SIZE, stepLocations_);
            TestUtil::SuspendUntilContinue(DebugEvent::LOAD_MODULE);
            ASSERT_EQ(moduleName, pandaFile_);
            debugger_->NotifyScriptParsed(0, moduleName.data());
            auto condFuncRef = FunctionRef::Undefined(vm_);
            for (auto &iter : pointerLocations_) {
                auto ret = debugInterface_->SetBreakpoint(iter, condFuncRef);
                ASSERT_TRUE(ret);
            }
            return true;
        };

        breakpoint = [this](const JSPtLocation &location) {
            ASSERT_TRUE(location.GetMethodId().IsValid());
            ASSERT_LOCATION_EQ(location, pointerLocations_.at(breakpointCounter_));
            ++breakpointCounter_;
            TestUtil::SuspendUntilContinue(DebugEvent::BREAKPOINT, location);
            debugger_->SetDebuggerState(DebuggerState::PAUSED);
            if (stepCompleteCounter_ < STEP_SIZE) {
                debugger_->StepOver(StepOverParams());
            } else {
                debugger_->StepOut();
            }
            return true;
        };

        singleStep = [this](const JSPtLocation &location) {
            if (debugger_->NotifySingleStep(location)) {
                ASSERT_TRUE(location.GetMethodId().IsValid());
                ASSERT_LOCATION_EQ(location, stepLocations_.at(stepCompleteCounter_));
                stepCompleteCounter_++;
                TestUtil::SuspendUntilContinue(DebugEvent::STEP_COMPLETE, location);
                return true;
            }
            return false;
        };

        scenario = [this]() {
            TestUtil::WaitForLoadModule();
            TestUtil::Continue();
            size_t index = 0;
            while (index < pointerLocations_.size()) {
                TestUtil::WaitForBreakpoint(pointerLocations_.at(index));
                TestUtil::Continue();
                if (index < STEP_SIZE) {
                    TestUtil::WaitForStepComplete(stepLocations_.at(index));
                    TestUtil::Continue();
                }
                ++index;
            }
            ASSERT_EXITED();
            return true;
        };
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }

private:
    static constexpr size_t LINE_COLUMN = 2;
    static constexpr size_t POINTER_SIZE = 8;
    static constexpr size_t STEP_SIZE = 7;

    std::string pandaFile_ = DEBUGGER_ABC_DIR "step.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "step.js";
    std::string entryPoint_ = "step";
    JSPtLocation location1_ {nullptr, JSPtLocation::EntityId(0), 0};
    JSPtLocation location2_ {nullptr, JSPtLocation::EntityId(0), 0};
    size_t breakpointCounter_ = 0;
    size_t stepCompleteCounter_ = 0;
    std::vector<JSPtLocation> pointerLocations_;
    std::vector<JSPtLocation> stepLocations_;

    void SetJSPtLocation(size_t *arr, size_t number, std::vector<JSPtLocation> &locations)
    {
        for (size_t i = 0; i < number; i++) {
            JSPtLocation location_ = TestUtil::GetLocation(sourceFile_.c_str(), arr[i * LINE_COLUMN],
                                                           arr[i * LINE_COLUMN + 1], pandaFile_.c_str());
            locations.push_back(location_);
        }
    };
};

std::unique_ptr<TestEvents> GetJsStepOverTest()
{
    return std::make_unique<JsStepOverTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_STEP_OVER_TEST_H
