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

#include <thread>
#include "ani_gtest.h"

namespace ark::ets::ani::testing {
using ThreadTestFunc = std::function<void(void)>;
const int NUM_THREADS = 3;
/*
  when test in sub thread, we use EXPECT_EQ to replace ASSERT_EQ,
  otherwise the main thread will not wait for the sub thread to finish.
*/
class AttachThreadTest : public AniTest {};

static void DoTest(AttachThreadTest *test, const ThreadTestFunc &func)
{
    ASSERT_NE(test, nullptr);
    ASSERT_NE(func, nullptr);
    std::array<std::thread, NUM_THREADS> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i] = std::thread(func);
    }
    // wait for all thread stop
    for (auto &t : threads) {
        t.join();
    }
}

TEST_F(AttachThreadTest, attach_interopDisabled)
{
    ThreadTestFunc fun = [this]() {
        ani_env *etsEnv {nullptr};
        ani_option interopEnabled {"--interop=disable", nullptr};
        ani_options aniArgs {1, &interopEnabled};
        EXPECT_NE(vm_, nullptr) << "vm_ is nullptr";
        EXPECT_EQ(vm_->c_api->AttachCurrentThread(nullptr, &aniArgs, ANI_VERSION_1, &etsEnv), ANI_INVALID_ARGS);
        const int32_t versionNew = 2;
        const int32_t versionOld = 0;
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, versionNew, &etsEnv), ANI_INVALID_VERSION);
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, versionOld, &etsEnv), ANI_INVALID_VERSION);
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, nullptr), ANI_INVALID_ARGS);
        EXPECT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &etsEnv), ANI_OK);
        EXPECT_NE(etsEnv, nullptr);
        ani_class kclass {};
        EXPECT_EQ(etsEnv->FindClass("attach_thread_test.A", &kclass), ANI_OK);
        EXPECT_NE(kclass, nullptr);
        EXPECT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    };
    DoTest(this, fun);
}

TEST_F(AttachThreadTest, attach_three_times_disable)
{
    ThreadTestFunc fun = [this]() {
        ani_env *etsEnv {nullptr};
        ani_option interopEnabled {"--interop=disable", nullptr};
        ani_options aniArgs {1, &interopEnabled};
        EXPECT_NE(vm_, nullptr) << "vm_ is nullptr";
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv), ANI_OK);
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv), ANI_ERROR);
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv), ANI_ERROR);
        EXPECT_NE(etsEnv, nullptr);
        ani_class kclass {};
        EXPECT_EQ(etsEnv->FindClass("attach_thread_test.A", &kclass), ANI_OK);
        EXPECT_NE(kclass, nullptr);
        EXPECT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    };
    DoTest(this, fun);
}

TEST_F(AttachThreadTest, attach_many_times_disable)
{
    ThreadTestFunc fun = [this]() {
        ani_env *etsEnv {nullptr};
        ani_option interopEnabled {"--interop=disable", nullptr};
        ani_options aniArgs {1, &interopEnabled};
        EXPECT_NE(vm_, nullptr) << "vm_ is nullptr";
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv), ANI_OK);
        EXPECT_NE(etsEnv, nullptr);
        ani_class kclass {};
        EXPECT_EQ(etsEnv->FindClass("attach_thread_test.A", &kclass), ANI_OK);
        EXPECT_NE(kclass, nullptr);
        EXPECT_EQ(vm_->DetachCurrentThread(), ANI_OK);
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv), ANI_OK);
        EXPECT_NE(etsEnv, nullptr);
        EXPECT_EQ(etsEnv->FindClass("attach_thread_test.A", &kclass), ANI_OK);
        EXPECT_NE(kclass, nullptr);
        EXPECT_EQ(vm_->DetachCurrentThread(), ANI_OK);
        EXPECT_EQ(vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv), ANI_OK);
        EXPECT_NE(etsEnv, nullptr);
        EXPECT_EQ(etsEnv->FindClass("attach_thread_test.A", &kclass), ANI_OK);
        EXPECT_NE(kclass, nullptr);
        EXPECT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    };
    DoTest(this, fun);
}
}  // namespace ark::ets::ani::testing
