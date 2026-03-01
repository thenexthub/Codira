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

#ifndef PANDA_RUNTIME_TOOLING_PT_THREAD_INFO_H
#define PANDA_RUNTIME_TOOLING_PT_THREAD_INFO_H

#include <cstdint>

#include "runtime/include/panda_vm.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "pt_hook_type_info.h"
#include "thread_sampling_info.h"

namespace ark::tooling {
class PtThreadInfo {
public:
    PtThreadInfo() = default;

    ~PtThreadInfo()
    {
        ASSERT(managedThreadRef_ == nullptr);
    }

    PtHookTypeInfo &GetHookTypeInfo()
    {
        return hookTypeInfo_;
    }

    sampler::ThreadSamplingInfo *GetSamplingInfo()
    {
        return &samplingInfo_;
    }

    void SetThreadObjectHeader(ObjectHeader *threadObjectHeader)
    {
        ASSERT(managedThreadRef_ == nullptr);
        managedThreadRef_ = PandaVM::GetCurrent()->GetGlobalObjectStorage()->Add(threadObjectHeader,
                                                                                 mem::Reference::ObjectType::GLOBAL);
    }

    void Destroy()
    {
        if (managedThreadRef_ != nullptr) {
            PandaVM::GetCurrent()->GetGlobalObjectStorage()->Remove(managedThreadRef_);
            managedThreadRef_ = nullptr;
        }
    }

    ObjectHeader *GetThreadObjectHeader() const
    {
        ASSERT(managedThreadRef_ != nullptr);
        return PandaVM::GetCurrent()->GetGlobalObjectStorage()->Get(managedThreadRef_);
    }

private:
    PtHookTypeInfo hookTypeInfo_ {false};
    mem::Reference *managedThreadRef_ {nullptr};
    sampler::ThreadSamplingInfo samplingInfo_ {};

    NO_COPY_SEMANTIC(PtThreadInfo);
    NO_MOVE_SEMANTIC(PtThreadInfo);
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_PT_THREAD_INFO_H
