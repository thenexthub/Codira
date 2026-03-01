/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_MEM_ETS_REFERENCE_H
#define PANDA_PLUGINS_ETS_RUNTIME_MEM_ETS_REFERENCE_H

#include "libarkbase/macros.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/mem/refstorage/reference_storage.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {
class EtsReference final {
    // We use 'long' type for the magic number to support both 32-bit and 64-bit architectures at the same time.
    static constexpr long REF_UNDEFINED_VALUE = -1UL;  // NOLINT(google-runtime-int)

public:
    DEFAULT_COPY_SEMANTIC(EtsReference);
    DEFAULT_MOVE_SEMANTIC(EtsReference);
    EtsReference() = default;
    ~EtsReference() = default;

    using EtsObjectType = mem::Reference::ObjectType;

    bool IsStack() const
    {
        return GetReference()->IsStack();
    }

    bool IsLocal() const
    {
        return GetReference()->IsLocal();
    }

    bool IsGlobal() const
    {
        return GetReference()->IsGlobal();
    }

    bool IsWeak() const
    {
        return GetReference()->IsWeak();
    }

    static mem::Reference *CastToReference(EtsReference *etsRef)
    {
        ASSERT(etsRef != nullptr);
        if (IsUndefined(etsRef)) {
            return nullptr;
        }
        return reinterpret_cast<mem::Reference *>(etsRef);
    }

    static EtsReference *CastFromReference(mem::Reference *ref)
    {
        if (ref == nullptr) {
            return GetUndefined();
        }
        return reinterpret_cast<EtsReference *>(ref);
    }

    static EtsReference *GetUndefined()
    {
        return ToNativePtr<EtsReference>(REF_UNDEFINED_VALUE);
    }

    static bool IsUndefined(EtsReference *ref)
    {
        ASSERT(ref != nullptr);
        return ref == GetUndefined();
    }

private:
    const mem::Reference *GetReference() const
    {
        return reinterpret_cast<const mem::Reference *>(this);
    }
};

class EtsReferenceStorage final : private mem::ReferenceStorage {
public:
    EtsReferenceStorage(mem::GlobalObjectStorage *globalStorage, mem::InternalAllocatorPtr allocator,
                        bool refCheckValidate)
        : mem::ReferenceStorage(globalStorage, allocator, refCheckValidate)
    {
    }
    ~EtsReferenceStorage() = default;

    NO_COPY_SEMANTIC(EtsReferenceStorage);
    NO_MOVE_SEMANTIC(EtsReferenceStorage);

    void CleanUp()
    {
        mem::ReferenceStorage::CleanUp();
    }

    void ReInitialize()
    {
        mem::ReferenceStorage::ReInitialize();
    }

    static EtsReference *NewEtsStackRef(EtsObject **obj)
    {
        mem::Reference *ref = mem::ReferenceStorage::NewStackRef(reinterpret_cast<ObjectHeader **>(obj));
        return EtsReference::CastFromReference(ref);
    }

    EtsReference *NewEtsRef(EtsObject *obj, EtsReference::EtsObjectType objType)
    {
        ASSERT(obj != nullptr);
        mem::Reference *ref = NewRef(obj->GetCoreType(), objType);
        return EtsReference::CastFromReference(ref);
    }

    void RemoveEtsRef(EtsReference *etsRef)
    {
        RemoveRef(EtsReference::CastToReference(etsRef));
    }

    [[nodiscard]] EtsObject *GetEtsObject(EtsReference *etsRef)
    {
        ASSERT(etsRef != nullptr);
        return EtsObject::FromCoreType(GetObject(EtsReference::CastToReference(etsRef)));
    }

    [[nodiscard]] bool IsValidEtsRef(EtsReference *etsRef)
    {
        return EtsReference::IsUndefined(etsRef) || IsValidRef(EtsReference::CastToReference(etsRef));
    }

    [[nodiscard]] static bool IsUndefinedEtsObject(EtsObject *obj)
    {
        return obj == GetUndefinedEtsObject();
    }

    [[nodiscard]] static EtsObject *GetUndefinedEtsObject()
    {
        return nullptr;
    }

    bool PushLocalEtsFrame(uint32_t capacity)
    {
        return PushLocalFrame(capacity);
    }

    EtsReference *PopLocalEtsFrame(EtsReference *result)
    {
        return EtsReference::CastFromReference(PopLocalFrame(EtsReference::CastToReference(result)));
    }

    bool EnsureLocalEtsCapacity(uint32_t capacity)
    {
        return EnsureLocalCapacity(capacity);
    }

    mem::ReferenceStorage *GetAsReferenceStorage()
    {
        return this;
    }
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_MEM_ETS_REFERENCE_H
