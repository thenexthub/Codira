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

#include "common_components/heap/heap.h"
#include "common_interfaces/base_runtime.h"
#include "common_components/tests/test_helper.h"
#include <thread>

namespace common::test {

class BaseRuntimeTest : public BaseTestWithScope {
protected:
    void SetUp() override {}
    void TearDown() override {}
    void ResetRuntime()
    {
        if (auto* instance = BaseRuntime::GetInstance()) {
            BaseRuntime::DestroyInstance();
        }
    }
};

HWTEST_F_L0(BaseRuntimeTest, RequestGC_Test1)
{
    BaseRuntime* runtime = BaseRuntime::GetInstance();
    ASSERT_TRUE(runtime != nullptr);
    runtime->RequestGC(static_cast<GCReason>(-1), false, static_cast<GCType>(-1));

    BaseObject obj;
    RefField<false> field(reinterpret_cast<HeapAddress>(&obj));
    auto result = runtime->ReadBarrier(reinterpret_cast<void*>(&field));
    EXPECT_TRUE(result != nullptr);
}

HWTEST_F_L0(BaseRuntimeTest, GetInstance_NotNull) {
    auto* instance = BaseRuntime::GetInstance();
    ASSERT_NE(instance, nullptr);
}

HWTEST_F_L0(BaseRuntimeTest, DestroyInstance_SafeIfUninitialized) {
    auto* instance = BaseRuntime::GetInstance();
    ASSERT_NE(instance, nullptr);

    EXPECT_NO_FATAL_FAILURE(BaseRuntime::DestroyInstance());

    instance = BaseRuntime::GetInstance();
    ASSERT_NE(instance, nullptr);
}
HWTEST_F_L0(BaseRuntimeTest, GetInstance_ReturnsValidInstance) {
    ResetRuntime();
    BaseRuntime* instance1 = BaseRuntime::GetInstance();
    BaseRuntime* instance2 = BaseRuntime::GetInstance();
    
    ASSERT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

HWTEST_F_L0(BaseRuntimeTest, ThreadSafe_GetInstance) {
    ResetRuntime();
    constexpr int kThreads = 4;
    std::vector<BaseRuntime*> instances(kThreads);
    std::vector<std::thread> threads;
    
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&instances, i]() {
            instances[i] = BaseRuntime::GetInstance();
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    for (int i = 1; i < kThreads; ++i) {
        EXPECT_EQ(instances[0], instances[i]);
    }
}

HWTEST_F_L0(BaseRuntimeTest, RequestGC_Sync_CallsHeapManager) {
    auto* runtime = BaseRuntime::GetInstance();
    ASSERT_NE(runtime, nullptr);
    runtime->InitFromDynamic();
    struct TestCase {
        GCReason reason;
        bool async;
        GCType gcType;
    };
    
    const std::vector<TestCase> testCases = {
        {GC_REASON_USER, false, GC_TYPE_FULL},
        {GC_REASON_USER, true, GC_TYPE_FULL},
        {GC_REASON_BACKUP, false, GC_TYPE_FULL},
        {GC_REASON_APPSPAWN, false, GC_TYPE_FULL}
    };

    for (TestCase tc : testCases) {
        testing::internal::CaptureStderr();
        EXPECT_NO_FATAL_FAILURE(runtime->RequestGC(tc.reason, tc.async, tc.gcType));
        std::string output = testing::internal::GetCapturedStderr();

        EXPECT_TRUE(output.empty()) << "GC reason " << static_cast<int>(tc.reason)
                                    << " produced unexpected stderr output.";
    }
    
    runtime->Fini();
    BaseRuntime::DestroyInstance();
}

} // namespace common::test
