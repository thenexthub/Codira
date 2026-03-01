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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_WEAK_REFERENCE_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_WEAK_REFERENCE_H

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

namespace test {
class EtsFinalizableWeakRefTest;
}  // namespace test

/// @class EtsWeakReference represent std.core.WeakRef class
class EtsWeakReference : public EtsObject {
public:
    EtsWeakReference() = delete;
    NO_COPY_SEMANTIC(EtsWeakReference);
    NO_MOVE_SEMANTIC(EtsWeakReference);
    ~EtsWeakReference() = delete;

    static constexpr size_t GetReferentOffset()
    {
        return MEMBER_OFFSET(EtsWeakReference, referent_);
    }

    template <bool NEED_READ_BARRIER = true>
    ALWAYS_INLINE EtsObject *GetReferent() const
    {
        return EtsObject::FromCoreType(
            ObjectAccessor::GetObject<false, NEED_READ_BARRIER, false>(this, GetReferentOffset()));
    }

    template <bool NEED_WRITE_BARRIER = true>
    ALWAYS_INLINE void ClearReferent()
    {
        ObjectAccessor::SetObject<false, NEED_WRITE_BARRIER, false>(this, GetReferentOffset(), nullptr);
    }

    ALWAYS_INLINE void SetReferent(EtsObject *addr)
    {
        ObjectAccessor::SetObject(this, GetReferentOffset(), addr->GetCoreType());
    }

private:
    // Such field has the same layout as referent in std.core.WeakRef class
    ObjectPointer<EtsObject> referent_;

    friend class test::EtsFinalizableWeakRefTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_WEAK_REFERENCE_H
