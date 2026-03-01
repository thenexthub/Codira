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

#ifndef PANDA_TOOLING_PT_SCOPED_MANAGED_CODE_H
#define PANDA_TOOLING_PT_SCOPED_MANAGED_CODE_H

#include "runtime/include/mtmanaged_thread.h"

namespace ark::tooling {
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class PtScopedManagedCode {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit PtScopedManagedCode() : managedThread_(MTManagedThread::GetCurrent())
    {
        ASSERT(managedThread_ != nullptr);
        managedThread_->ManagedCodeBegin();
    }

    ~PtScopedManagedCode()
    {
        managedThread_->ManagedCodeEnd();
    }

    NO_COPY_SEMANTIC(PtScopedManagedCode);
    NO_MOVE_SEMANTIC(PtScopedManagedCode);

private:
    MTManagedThread *managedThread_;
};
}  // namespace ark::tooling

#endif  // PANDA_TOOLING_PT_SCOPED_MANAGED_CODE_H
