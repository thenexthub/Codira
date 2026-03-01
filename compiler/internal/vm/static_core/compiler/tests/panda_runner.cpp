/*
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

#include "panda_runner.h"

namespace ark::test {

int PandaRunnerHookAArch64()
{
    ASSERT(PandaRunner::callback_);
    ManagedThread::GetCurrent()->SetCurrentFrameIsCompiled(true);
    auto fp = reinterpret_cast<uintptr_t>(ManagedThread::GetCurrent()->GetCurrentFrame());
    auto lr = ManagedThread::GetCurrent()->GetNativePc();
    return PandaRunner::callback_(lr, fp);
}

int PandaRunnerHook(uintptr_t lr, uintptr_t fp)
{
    ASSERT(PandaRunner::callback_);
    ManagedThread::GetCurrent()->SetCurrentFrameIsCompiled(true);
    ManagedThread::GetCurrent()->SetCurrentFrame(reinterpret_cast<Frame *>(fp));
    ManagedThread::GetCurrent()->SetNativePc(lr);
    return PandaRunner::callback_(lr, fp);
}
}  // namespace ark::test
