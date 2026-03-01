/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "session_manager.h"

#include <string>

#include "gtest/gtest.h"

#include "runtime.h"
#include "runtime_options.h"

// NOLINTBEGIN

namespace ark::tooling::inspector::test {
class SessionManagerTest : public testing::Test {
protected:
    void SetUp()
    {
        RuntimeOptions options;
        options.SetShouldInitializeIntrinsics(false);
        options.SetShouldLoadBootPandaFiles(false);
        Runtime::Create(options);
    }
    void TearDown()
    {
        Runtime::Destroy();
    }

    SessionManager sm_;
};

class SessionManagerTestDeath : public SessionManagerTest {};

void RunMThread(std::atomic<bool> *sync_flag, [[maybe_unused]] PtThread *thread)
{
    auto *m_thr = MTManagedThread::Create(Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
    auto pt_thr = PtThread(m_thr);
    *thread = pt_thr;

    *sync_flag = true;

    while (*sync_flag) {
    }
    m_thr->Destroy();
}

TEST_F(SessionManagerTest, Test)
{
    std::atomic<bool> sync_flag0 = false;
    std::atomic<bool> sync_flag1 = false;
    std::atomic<bool> sync_flag2 = false;

    std::vector<PtThread> pt_threads = {PtThread::NONE, PtThread::NONE, PtThread::NONE,
                                        PtThread(ManagedThread::GetCurrent())};

    std::thread thread0(RunMThread, &sync_flag0, &pt_threads[0U]);
    std::thread thread1(RunMThread, &sync_flag1, &pt_threads[1U]);
    std::thread thread2(RunMThread, &sync_flag2, &pt_threads[2U]);

    while (!sync_flag0 || !sync_flag1 || !sync_flag2) {
        ;
    }

    for (auto thread : pt_threads) {
        ASSERT_NE(thread, PtThread::NONE);
        auto id = sm_.AddSession(thread);
        ASSERT_EQ(sm_.GetSessionIdByThread(thread), id);

        auto test = sm_.GetThreadBySessionId(id);
        ASSERT_EQ(test, thread);
    }

    size_t sessions = 0;
    sm_.EnumerateSessions([&sessions, &pt_threads](auto, auto thread) {
        sessions++;
        ASSERT_NE(std::find(pt_threads.begin(), pt_threads.end(), thread), pt_threads.end());
    });
    ASSERT_EQ(sessions, 4UL);

    sm_.RemoveSession(sm_.GetSessionIdByThread(pt_threads[0]));
    sm_.EnumerateSessions([&sessions, &pt_threads](auto, auto thread) {
        sessions++;
        ASSERT_NE(std::find(pt_threads.begin(), pt_threads.end(), thread), pt_threads.end());
    });
    ASSERT_EQ(sessions, 7UL);

    sync_flag0 = false;
    sync_flag1 = false;
    sync_flag2 = false;

    thread0.join();
    thread1.join();
    thread2.join();
}

}  // namespace ark::tooling::inspector::test

// NOLINTEND