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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TEST_HOOKS_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TEST_HOOKS_H

#include "agent/debugger_impl.h"
#include "backend/js_pt_hooks.h"
#include "test/utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class TestHooks : public PtHooks {
public:
    TestHooks(const std::string &testName, const EcmaVM *vm) : vm_(vm)
    {
        testName_ = testName;
        test_ = TestUtil::GetTest(testName);
        runtime_ = std::make_unique<RuntimeImpl>(vm, test_->channel_);
        debugger_ = std::make_unique<DebuggerImpl>(vm, test_->channel_, runtime_.get());
        test_->vm_ = vm;
        test_->debugger_ = debugger_.get();
        test_->runtime_ = runtime_.get();
        test_->debugInterface_ = debugger_->jsDebugger_;
        debugInterface_ = debugger_->jsDebugger_;
        TestUtil::Reset();
        debugInterface_->RegisterHooks(this);
    }

    void Run()
    {
        if (test_->scenario) {
            test_->scenario();
        }
    }

    void DebuggerStmt(const JSPtLocation &location) override
    {
        if (test_->debugger_) {
            test_->debuggerStmt(location);
        }
    }

    void Breakpoint(const JSPtLocation &location) override
    {
        if (test_->breakpoint) {
            test_->breakpoint(location);
        }
    }

    void LoadModule(std::string_view panda_file_name, [[maybe_unused]] std::string_view entryPoint) override
    {
        if (test_->loadModule) {
            test_->loadModule(panda_file_name);
        }
    }

    void Exception(const JSPtLocation &location) override
    {
        if (test_->exception) {
            Local<JSValueRef> exception = DebuggerApi::GetAndClearException(vm_);

            test_->exception(location);

            if (!exception->IsHole()) {
                DebuggerApi::SetException(vm_, exception);
            }
        }
    }

    bool SingleStep(const JSPtLocation &location) override
    {
        if (test_->singleStep) {
            return test_->singleStep(location);
        }
        return false;
    }

    bool NativeOut() override
    {
        if (test_->nativeOut) {
            return test_->nativeOut();
        }

        return false;
    }

    void VmDeath() override
    {
        if (test_->vmDeath) {
            test_->vmDeath();
        }
        TestUtil::Event(DebugEvent::VM_DEATH);
    }

    void VmStart() override
    {
        if (test_->vmStart) {
            test_->vmStart();
        }
        TestUtil::Event(DebugEvent::VM_START);
    }

    void NativeCalling([[maybe_unused]] const void *nativeAddress) override {}

    void NativeReturn([[maybe_unused]] const void *nativeAddress) override {}

    void SendableMethodEntry([[maybe_unused]] JSHandle<Method> method) override {}

    void DisableFirstTimeFlag() override {}

    void GenerateAsyncFrames([[maybe_unused]] std::shared_ptr<AsyncStack> asyncStack,
        [[maybe_unused]] bool skipTopFrame) override {}

    void HitSymbolicBreakpoint() override {}

    const std::unordered_set<std::string> &GetAllRecordNames() const override
    {
        static const std::unordered_set<std::string> recordName;
        return recordName;
    }

    void TerminateTest()
    {
        debugInterface_->UnregisterHooks();
        if (TestUtil::IsTestFinished()) {
            return;
        }
        LOG_DEBUGGER(FATAL) << "Test " << testName_ << " failed";
    }

    void SetDebuggerAccessor(const JSHandle<GlobalEnv> &globalEnv) override {}

    ~TestHooks() = default;

private:
    const EcmaVM *vm_ {nullptr};
    std::unique_ptr<RuntimeImpl> runtime_ {nullptr};
    std::unique_ptr<DebuggerImpl> debugger_ {nullptr};
    JSDebugger *debugInterface_;
    std::string testName_;
    TestEvents *test_;
};
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TEST_HOOKS_H
