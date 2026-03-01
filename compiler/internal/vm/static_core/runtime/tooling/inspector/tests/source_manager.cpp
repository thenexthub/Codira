/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "runtime.h"
#include "runtime_options.h"
#include "source_manager.h"

// NOLINTBEGIN

namespace ark::tooling::inspector::test {
class SourceManagerTest : public testing::Test {
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

    SourceManager sm_;
};

void RunManagedThread(std::atomic<bool> *sync_flag, [[maybe_unused]] PtThread *thread)
{
    auto *m_thr = MTManagedThread::Create(Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
    auto pt_thr = PtThread(m_thr);
    *thread = pt_thr;

    *sync_flag = true;

    while (*sync_flag) {
    }
    m_thr->Destroy();
}

TEST_F(SourceManagerTest, General)
{
    std::atomic<bool> sync_flag1 = false;
    PtThread pt_thread1 = PtThread::NONE;
    std::thread mthread1(RunManagedThread, &sync_flag1, &pt_thread1);

    while (!sync_flag1) {
        ;
    }

    auto test_id0 = sm_.GetScriptId("test.pa");
    ASSERT_EQ(test_id0, ScriptId(0));

    ASSERT_EQ(sm_.GetSourceFileName(test_id0), "test.pa");
    ASSERT_EQ(sm_.GetSourceFileName(ScriptId(1)), "");

    test_id0 = sm_.GetScriptId("test.pa");
    ASSERT_EQ(test_id0, ScriptId(0));

    auto test_id1 = sm_.GetScriptId("test1.pa");
    ASSERT_EQ(test_id1, ScriptId(1));

    auto test_id2 = sm_.GetScriptId("test2.pa");
    auto test_id3 = sm_.GetScriptId("test3.pa");
    ASSERT_EQ(sm_.GetSourceFileName(test_id2), "test2.pa");
    ASSERT_EQ(sm_.GetSourceFileName(test_id3), "test3.pa");

    ASSERT_EQ(sm_.GetSourceFileName(ScriptId(5U)), "");

    ASSERT_EQ(sm_.GetSourceFileName(test_id2), "test2.pa");

    test_id0 = sm_.GetScriptId("test.pa");
    ASSERT_EQ(test_id0, ScriptId(0));

    sync_flag1 = false;

    mthread1.join();
}

}  // namespace ark::tooling::inspector::test

// NOLINTEND
