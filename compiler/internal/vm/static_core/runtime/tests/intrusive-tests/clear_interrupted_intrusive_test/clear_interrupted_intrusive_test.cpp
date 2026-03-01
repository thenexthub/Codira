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

#include "gtest/gtest.h"

#include "runtime/tests/intrusive-tests/intrusive_test_suite.h"
#include "runtime/include/runtime.h"

namespace ark::test {

class IntrusiveClearInterruptedThreadTest : public testing::Test {
public:
    IntrusiveClearInterruptedThreadTest()
    {
        options_.SetShouldLoadBootPandaFiles(false);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetIntrusiveTest(CLEAR_INTERRUPTED_INTRUSIVE_TEST);
        Runtime::Create(options_);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~IntrusiveClearInterruptedThreadTest()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(IntrusiveClearInterruptedThreadTest);
    NO_MOVE_SEMANTIC(IntrusiveClearInterruptedThreadTest);

protected:
    ark::MTManagedThread *thread_;
    RuntimeOptions options_;
};

void SleepTestThread(MTManagedThread *main_thread)
{
    /* @sync 1
     * @description Before main thread has been interrupted.
     * */
    MTManagedThread::Interrupt(main_thread);
}

TEST_F(IntrusiveClearInterruptedThreadTest, ClearInterruptedTest)
{
    auto main_thread = MTManagedThread::GetCurrent();
    std::thread interrupting_thread(SleepTestThread, main_thread);
    main_thread->ClearInterrupted();
    auto is_interrupted = main_thread->IsInterrupted();
    interrupting_thread.join();
    LOG(DEBUG, RUNTIME) << "Sleep was interrupted = " << is_interrupted;
}

}  // namespace ark::test
