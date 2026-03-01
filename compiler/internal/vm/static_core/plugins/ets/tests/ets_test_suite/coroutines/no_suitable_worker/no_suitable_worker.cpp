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

#include <gtest/gtest.h>

#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"

namespace ark::ets::test {

class EtsNoSuitableWorkerTest : public ani::testing::AniTest {};

TEST_F(EtsNoSuitableWorkerTest, NoSuitableWorkerForNativeCoro)
{
    auto coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] ScopedManagedCodeThread sj(coro);
    [[maybe_unused]] EtsHandleScope scope(coro);
    auto etsVm = coro->GetPandaVM();
    auto *coroManager = static_cast<StackfulCoroutineManager *>(etsVm->GetCoroutineManager());
    auto groupId = CoroutineWorkerGroup::FromDomain(coroManager, CoroutineWorkerDomain::EXACT_ID, {99});
    auto cb = []([[maybe_unused]] void *data) {};
    auto launchRes = coroManager->LaunchNative(cb, reinterpret_cast<void *>(1U), "NoSuitableWorkerForNativeCoro",
                                               groupId, CoroutinePriority::DEFAULT_PRIORITY, false, false);
    ASSERT_EQ(launchRes, LaunchResult::NO_SUITABLE_WORKER);
    ASSERT_TRUE(coro->HasPendingException());
}

}  // namespace ark::ets::test
