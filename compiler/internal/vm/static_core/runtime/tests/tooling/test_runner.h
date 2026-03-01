/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_RUNTIME_TESTS_TOOLING_TEST_RUNNER_H
#define PANDA_RUNTIME_TESTS_TOOLING_TEST_RUNNER_H

#include "test_util.h"

#include "runtime/include/runtime.h"

namespace ark::tooling::test {
class TestRunner : public PtHooks {
public:
    explicit TestRunner(const char *testName)
    {
        debugSession_ = Runtime::GetCurrent()->StartDebugSession();
        debugInterface_ = &debugSession_->GetDebugger();
        testName_ = testName;
        test_ = TestUtil::GetTest(testName);
        test_->debugInterface = debugInterface_;
        TestUtil::Reset();
        debugInterface_->RegisterHooks(this);
    }

    void Run()
    {
        if (test_->scenario) {
            test_->scenario();
        }
    }

    void Breakpoint(PtThread thread, Method *method, const PtLocation &location) override
    {
        if (test_->breakpoint) {
            test_->breakpoint(thread, method, location);
        }
    }

    void LoadModule(std::string_view pandaFileName) override
    {
        if (test_->loadModule) {
            test_->loadModule(pandaFileName);
        }
    }

    void Paused(PauseReason reason) override
    {
        if (test_->paused) {
            test_->paused(reason);
        }
    };

    void Exception(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                   Method *catchMethod, const PtLocation &catchLocation) override
    {
        if (test_->exception) {
            test_->exception(thread, method, location, object, catchMethod, catchLocation);
        }
    }

    void ExceptionCatch(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object) override
    {
        if (test_->exceptionCatch) {
            test_->exceptionCatch(thread, method, location, object);
        }
    }

    void PropertyAccess(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                        PtProperty property) override
    {
        if (test_->propertyAccess) {
            test_->propertyAccess(thread, method, location, object, property);
        }
    }

    void PropertyModification(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *object,
                              PtProperty property, VRegValue newValue) override
    {
        if (test_->propertyModification) {
            test_->propertyModification(thread, method, location, object, property, newValue);
        }
    }

    void FramePop(PtThread thread, Method *method, bool wasPoppedByException) override
    {
        if (test_->framePop) {
            test_->framePop(thread, method, wasPoppedByException);
        }
    }

    void GarbageCollectionStart() override
    {
        if (test_->garbageCollectionStart) {
            test_->garbageCollectionStart();
        }
    }

    void GarbageCollectionFinish() override
    {
        if (test_->garbageCollectionFinish) {
            test_->garbageCollectionFinish();
        }
    }

    void MethodEntry(PtThread thread, Method *method) override
    {
        if (test_->methodEntry) {
            test_->methodEntry(thread, method);
        }
    }

    void MethodExit(PtThread thread, Method *method, bool wasPoppedByException, VRegValue returnValue) override
    {
        if (test_->methodExit) {
            test_->methodExit(thread, method, wasPoppedByException, returnValue);
        }
    }

    void SingleStep(PtThread thread, Method *method, const PtLocation &location) override
    {
        if (test_->singleStep) {
            test_->singleStep(thread, method, location);
        }
    }

    void ThreadStart(PtThread thread) override
    {
        if (test_->threadStart) {
            test_->threadStart(thread);
        }
    }

    void ThreadEnd(PtThread thread) override
    {
        if (test_->threadEnd) {
            test_->threadEnd(thread);
        }
    }

    void VmDeath() override
    {
        if (test_->vmDeath) {
            test_->vmDeath();
        }
        TestUtil::Event(DebugEvent::VM_DEATH);
    }

    void VmInitialization(PtThread thread) override
    {
        if (test_->vmInit) {
            test_->vmInit(thread);
        }
        TestUtil::Event(DebugEvent::VM_INITIALIZATION);
    }

    void VmStart() override
    {
        if (test_->vmStart) {
            test_->vmStart();
        }
    }

    void ExceptionRevoked(ExceptionWrapper reason, ExceptionID exceptionId) override
    {
        if (test_->exceptionRevoked) {
            test_->exceptionRevoked(reason, exceptionId);
        }
    }

    void ExecutionContextCreated(ExecutionContextWrapper context) override
    {
        if (test_->executionContextCreated) {
            test_->executionContextCreated(context);
        }
    }

    void ExecutionContextDestroyed(ExecutionContextWrapper context) override
    {
        if (test_->executionContextDestroyed) {
            test_->executionContextDestroyed(context);
        }
    }

    void ExecutionContextsCleared() override
    {
        if (test_->executionContextCleared) {
            test_->executionContextCleared();
        }
    }

    void ClassLoad(PtThread thread, BaseClass *klass) override
    {
        if (test_->classLoad) {
            test_->classLoad(thread, klass);
        }
    }

    void ClassPrepare(PtThread thread, BaseClass *klass) override
    {
        if (test_->classPrepare) {
            test_->classPrepare(thread, klass);
        }
    }

    void MonitorWait(PtThread thread, ObjectHeader *object, int64_t timeout) override
    {
        if (test_->monitorWait) {
            test_->monitorWait(thread, object, timeout);
        }
    }

    void MonitorWaited(PtThread thread, ObjectHeader *object, bool timedOut) override
    {
        if (test_->monitorWaited) {
            test_->monitorWaited(thread, object, timedOut);
        }
    }

    void MonitorContendedEnter(PtThread thread, ObjectHeader *object) override
    {
        if (test_->monitorContendedEnter) {
            test_->monitorContendedEnter(thread, object);
        }
    }

    void MonitorContendedEntered(PtThread thread, ObjectHeader *object) override
    {
        if (test_->monitorContendedEntered) {
            test_->monitorContendedEntered(thread, object);
        }
    }

    void ObjectAlloc(BaseClass *klass, ObjectHeader *object, PtThread thread, size_t size) override
    {
        if (test_->objectAlloc) {
            test_->objectAlloc(klass, object, thread, size);
        }
    }

    void TerminateTest()
    {
        debugInterface_->UnregisterHooks();
        if (TestUtil::IsTestFinished()) {
            return;
        }
        LOG(FATAL, DEBUGGER) << "Test " << testName_ << " failed";
    }

private:
    Runtime::DebugSessionHandle debugSession_;
    DebugInterface *debugInterface_;
    const char *testName_;
    ApiTest *test_;
};
}  // namespace ark::tooling::test

#endif  // PANDA_RUNTIME_TESTS_TOOLING_TEST_RUNNER_H
