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

#include "common_components/tests/test_helper.h"
#include "common_components/mutator/mutator.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_interfaces/base_runtime.h"
#include "common_interfaces/thread/thread_holder-inl.h"
#include "common_interfaces/thread/thread_holder.h"
#include "common_interfaces/thread/thread_holder_manager.h"

using namespace common;

namespace ark {
class Coroutine;
}

namespace common::test {
using Coroutine = ark::Coroutine;
namespace {
    void SetCurrentThreadHolder(ThreadHolder* holder)
    {
        ThreadHolder::SetCurrent(holder);
    }
}

class ThreadHolderTest : public BaseTestWithScope {
protected:
    static void SetUpTestCase()
    {
        BaseRuntime::GetInstance()->InitFromDynamic();
    }

    static void TearDownTestCase()
    {
        BaseRuntime::GetInstance()->FiniFromDynamic();
    }

    void SetUp() override {}

    void TearDown() override {}
};

size_t callCount;
void CallBackVisitor(void *root)
{
    callCount++;
}

HWTEST_F_L0(ThreadHolderTest, VisitAllThreadsTest) {
    ThreadHolder *curHolder = ThreadHolder::CreateAndRegisterNewThreadHolder(nullptr);
    ASSERT_TRUE(curHolder != nullptr);
    SetCurrentThreadHolder(curHolder);

    curHolder->WaitSuspension();

    CommonRootVisitor visitor = nullptr;
    curHolder->VisitAllThreads(visitor);

    ThreadHolder* result = ThreadHolder::GetCurrent();
    EXPECT_EQ(result, curHolder);
}

HWTEST_F_L0(ThreadHolderTest, UnregisterThreadHolderTest) {
    ThreadHolder *curHolder = ThreadHolder::CreateAndRegisterNewThreadHolder(nullptr);
    Mutator *mutator = static_cast<Mutator *>(curHolder->GetMutator());

    auto& mutator_manager = MutatorManager::Instance();
    auto& list = mutator_manager.allMutatorList_;
    list.clear();

    ThreadHolderManager thManager;
    thManager.UnregisterThreadHolder(curHolder);
    EXPECT_EQ(mutator->PopRawObject(), nullptr);
}

HWTEST_F_L0(ThreadHolderTest, TryBindMutatorTest) {
    ThreadHolder *curHolder = ThreadHolder::CreateAndRegisterNewThreadHolder(nullptr);
    ASSERT_TRUE(curHolder != nullptr);
    SetCurrentThreadHolder(curHolder);

    ThreadLocal::SetProcessorFlag(true);
    ThreadHolder::TryBindMutatorScope scope(curHolder);
    EXPECT_EQ(ThreadLocal::GetAllocBuffer(), nullptr);
}
}  // namespace panda::test