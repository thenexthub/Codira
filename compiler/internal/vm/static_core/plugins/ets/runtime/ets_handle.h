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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_HANDLE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_HANDLE_H_

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"

namespace ark::ets {

template <typename T>
class EtsHandle : public VMHandle<T> {
public:
    inline explicit EtsHandle() : VMHandle<T>() {}
    explicit EtsHandle(EtsCoroutine *coroutine, T *etsObj)
        : VMHandle<T>(ManagedThread::CastFromThread(coroutine), GetObjectHeader(etsObj))
    {
    }

    template <typename P>
    inline explicit EtsHandle(const VMHandle<P> &other) : VMHandle<T>(other)
    {
    }

    ~EtsHandle() = default;
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(EtsHandle);
    DEFAULT_COPY_SEMANTIC(EtsHandle);

private:
    static constexpr ObjectHeader *GetObjectHeader(T *etsObj)
    {
        EtsObject *object = nullptr;
        if constexpr (std::is_same_v<T, EtsObject>) {
            object = etsObj;
        } else {
            object = etsObj != nullptr ? etsObj->AsObject() : nullptr;
        }
        return object != nullptr ? object->GetCoreType() : nullptr;
    }
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_HANDLE_H_
