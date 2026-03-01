/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_ETS_RUNTIME_HYBRID_MEM_STATIC_OBJECT_OPERATOR_H
#define PLUGINS_ETS_RUNTIME_HYBRID_MEM_STATIC_OBJECT_OPERATOR_H

#if defined(ARK_HYBRID)
#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/base_object_operator.h"
#include "runtime/include/object_header.h"
#endif

namespace ark::ets {
class PandaEtsVM;
}  // namespace ark::ets

#if defined(ARK_HYBRID)

namespace ark::mem {

class StaticObjectOperator : public common::BaseObjectOperatorInterfaces {
public:
    static void Initialize(ark::ets::PandaEtsVM *vm);

    bool IsValidObject([[maybe_unused]] const common::BaseObject *object) const override
    {
        return true;
    }

    void ForEachRefField(const common::BaseObject *object, const common::RefFieldVisitor &visitor) const override;

    size_t ForEachRefFieldAndGetSize(const common::BaseObject *object,
                                     const common::RefFieldVisitor &visitor) const override;

    size_t GetSize(const common::BaseObject *object) const override
    {
        // ObjectHeader must be binary compatible with BaseObject
        auto *hdr = reinterpret_cast<const ObjectHeader *>(object);
        return hdr->ObjectSize();
    }

    common::BaseObject *GetForwardingPointer(const common::BaseObject *object) const override;

    void SetForwardingPointerAfterExclusive(common::BaseObject *object, common::BaseObject *fwdPtr) override;

private:
    static StaticObjectOperator instance_;
    ark::ets::PandaEtsVM *vm_ {nullptr};
};

}  // namespace ark::mem

#else

namespace ark::mem {
class StaticObjectOperator {
public:
    static void Initialize([[maybe_unused]] ark::ets::PandaEtsVM *vm) {}
};
}  // namespace ark::mem

#endif  // ARK_HYBRID
#endif  // PLUGINS_ETS_RUNTIME_HYBRID_MEM_STATIC_OBJECT_OPERATOR_H
