/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGINS_ETS_RUNTIME_INTEROP_JS_STS_VM_INTERFACE_IMPL_H
#define PLUGINS_ETS_RUNTIME_INTEROP_JS_STS_VM_INTERFACE_IMPL_H

#include <atomic>
#include <functional>
// platform
#include "hybrid/sts_vm_interface.h"

#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"

namespace ark::ets::interop::js {

namespace testing {
class STSVMInterfaceImplTest;
}  // namespace testing

class STSVMInterfaceImpl final : public platform::STSVMInterface {
public:
    NO_COPY_SEMANTIC(STSVMInterfaceImpl);
    NO_MOVE_SEMANTIC(STSVMInterfaceImpl);
    PANDA_PUBLIC_API STSVMInterfaceImpl();
    PANDA_PUBLIC_API ~STSVMInterfaceImpl() override = default;

    PANDA_PUBLIC_API void MarkFromObject(void *obj) override;

    PANDA_PUBLIC_API void OnVMAttach() override;
    PANDA_PUBLIC_API void OnVMDetach() override;

    PANDA_PUBLIC_API bool StartXGCBarrier(const NoWorkPred &func) override;
    PANDA_PUBLIC_API bool WaitForConcurrentMark(const NoWorkPred &func) override;
    PANDA_PUBLIC_API void RemarkStartBarrier() override;
    PANDA_PUBLIC_API bool WaitForRemark(const NoWorkPred &func) override;
    PANDA_PUBLIC_API void FinishXGCBarrier() override;

    PANDA_PUBLIC_API void NotifyWaiters();

private:
    enum class XGCSyncState { NONE, CONCURRENT_PHASE, CONCURRENT_FINISHED, REMARK_PHASE };

    class VMBarrier {
    public:
        NO_COPY_SEMANTIC(VMBarrier);
        NO_MOVE_SEMANTIC(VMBarrier);

        PANDA_PUBLIC_API explicit VMBarrier(size_t vmsCount);
        PANDA_PUBLIC_API ~VMBarrier() = default;

        PANDA_PUBLIC_API void Increment();
        PANDA_PUBLIC_API void Decrement();

        PANDA_PUBLIC_API bool InitialWait(const NoWorkPred &noWorkPred = nullptr);
        PANDA_PUBLIC_API bool Wait(const NoWorkPred &noWorkPred = nullptr);
        PANDA_PUBLIC_API void Signal();

    private:
        bool Wait(const NoWorkPred &noWorkPred, bool isInitial);

        bool WaitNextEpochOrSignal(const NoWorkPred &noWorkPred) REQUIRES(barrierMutex_);
        size_t IncrementCurrentWaitersCount() REQUIRES(barrierMutex_);
        void Wake() REQUIRES(barrierMutex_);

        bool CheckNoWorkPred(const NoWorkPred &noWorkPred)
        {
            return noWorkPred && !noWorkPred();
        }

        os::memory::Mutex barrierMutex_;
        os::memory::ConditionVariable condVar_ GUARDED_BY(barrierMutex_);

        size_t currentVMsCount_ GUARDED_BY(barrierMutex_);
        size_t vmsCount_ GUARDED_BY(barrierMutex_);
        size_t epochCount_ GUARDED_BY(barrierMutex_);
        size_t currentWaitersCount_ GUARDED_BY(barrierMutex_);
        size_t weakCount_ GUARDED_BY(barrierMutex_);
    };

    VMBarrier xgcBarrier_;
    // xgcSyncState_ is used only for debug
    thread_local static XGCSyncState xgcSyncState_;

    // friend test class to access internal structures
    friend class testing::STSVMInterfaceImplTest;
};

}  // namespace ark::ets::interop::js

#endif  // PLUGINS_ETS_RUNTIME_INTEROP_JS_STS_VM_INTERFACE_IMPL_H
