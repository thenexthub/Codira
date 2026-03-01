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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_SYNTAX_EXCEPTION_TEST_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_SYNTAX_EXCEPTION_TEST_H

#include "test/utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsSyntaxExceptionTest : public TestEvents {
public:
    JsSyntaxExceptionTest()
    {
        breakpoint = [this](const JSPtLocation &location) {
            ASSERT_TRUE(location.GetMethodId().IsValid());
            ASSERT_LOCATION_EQ(location, location_);
            ++breakpointCounter_;
            std::vector<std::unique_ptr<CallFrame>> callFrames;
            ASSERT_TRUE(debugger_->GenerateCallFrames(&callFrames, true));
            ASSERT_TRUE(callFrames.size() > 0);
            auto jsLocation = callFrames[0]->GetLocation();
            ASSERT_TRUE(jsLocation != nullptr);
            ASSERT_EQ(jsLocation->GetLine(), 29); // 29: breakpoint line
            ASSERT_EQ(jsLocation->GetColumn(), 4); // 4: breakpoint column
            TestUtil::SuspendUntilContinue(DebugEvent::BREAKPOINT, location);
            return true;
        };

        exception = [this](const JSPtLocation &location) {
            auto sourceLocation = TestUtil::GetSourceLocation(location, pandaFile_.c_str());
            ASSERT_EQ(sourceLocation.line, 18); // 18: exception line
            ASSERT_EQ(sourceLocation.column, 8); // 8: exception column
            ++exceptionCounter_;
            std::vector<std::unique_ptr<CallFrame>> callFrames;
            ASSERT_TRUE(debugger_->GenerateCallFrames(&callFrames, true));
            ASSERT_TRUE(callFrames.size() > 0);
            auto jsLocation = callFrames[0]->GetLocation();
            ASSERT_TRUE(jsLocation != nullptr);
            ASSERT_EQ(jsLocation->GetLine(), 18); // 18: exception line
            ASSERT_EQ(jsLocation->GetColumn(), 8); // 8: exception column
            TestUtil::SuspendUntilContinue(DebugEvent::EXCEPTION, location);
            return true;
        };

        loadModule = [this](std::string_view moduleName) {
            runtime_->Enable();
            // 29: breakpointer line, 4: breakpointer column
            location_ = TestUtil::GetLocation(sourceFile_.c_str(), 29, 4, pandaFile_.c_str());
            ASSERT_TRUE(location_.GetMethodId().IsValid());
            TestUtil::SuspendUntilContinue(DebugEvent::LOAD_MODULE);
            ASSERT_EQ(moduleName, pandaFile_);
            ASSERT_TRUE(debugger_->NotifyScriptParsed(0, pandaFile_));
            auto condFuncRef = FunctionRef::Undefined(vm_);
            auto ret = debugInterface_->SetBreakpoint(location_, condFuncRef);
            ASSERT_TRUE(ret);
            return true;
        };

        scenario = [this]() {
            TestUtil::WaitForLoadModule();
            TestUtil::Continue();
            TestUtil::WaitForBreakpoint(location_);
            TestUtil::Continue();
            TestUtil::WaitForException();
            TestUtil::Continue();
            auto ret = debugInterface_->RemoveBreakpoint(location_);
            ASSERT_TRUE(ret);
            ASSERT_EXITED();
            return true;
        };

        vmDeath = [this]() {
            ASSERT_EQ(breakpointCounter_, 1U);  // 1: break point counter
            ASSERT_EQ(exceptionCounter_, 1U);  // 1: exception counter
            return true;
        };
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsSyntaxExceptionTest() = default;

private:
    const std::string testFile = "syntax_exception";
    std::string pandaFile_ = DEBUGGER_ABC_DIR + testFile + ".abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR + testFile + ".js";
    std::string entryPoint_ = testFile;
    JSPtLocation location_ {nullptr, JSPtLocation::EntityId(0), 0};
    size_t breakpointCounter_ = 0;
    size_t exceptionCounter_ = 0;
};

std::unique_ptr<TestEvents> GetJsSyntaxExceptionTest()
{
    return std::make_unique<JsSyntaxExceptionTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_SYNTAX_EXCEPTION_TEST_H
