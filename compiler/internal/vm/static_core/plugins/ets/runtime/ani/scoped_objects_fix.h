/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_SCOPED_OBJECTS_FIX_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_SCOPED_OBJECTS_FIX_H

#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"

namespace ark::ets::ani {

class ManagedCodeAccessor {
    // We use 'long' type for the magic number to support both 32-bit and 64-bit architectures at the same time.
    static constexpr long WREF_UNDEFINED_VALUE = -2UL;  // NOLINT(google-runtime-int)

public:
    explicit ManagedCodeAccessor(PandaEnv *env) : env_(env) {}
    ~ManagedCodeAccessor() = default;

    EtsCoroutine *GetCoroutine() const
    {
        return env_->GetEtsCoroutine();
    }

    PandaEnv *GetPandaEnv()
    {
        return env_;
    }

    static bool IsUndefined(ani_ref ref)
    {
        return EtsReference::IsUndefined(reinterpret_cast<EtsReference *>(ref));
    }

    static bool IsUndefined(ani_wref ref)
    {
        return ref == ToNativePtr<__ani_wref>(WREF_UNDEFINED_VALUE);
    }

    static ani_status GetUndefinedWRef(ani_wref *result)
    {
        *result = ToNativePtr<__ani_wref>(WREF_UNDEFINED_VALUE);
        return ANI_OK;
    }

    static ani_status GetUndefinedRef(ani_ref *result)
    {
        *result = reinterpret_cast<ani_ref>(EtsReference::GetUndefined());
        return ANI_OK;
    }

    bool IsNull(ani_ref ref)
    {
        ASSERT(!IsUndefined(ref));
        return ToInternalType(ref)->GetCoreType() == GetCoroutine()->GetNullValue();
    }

    bool IsNullishValue(ani_ref ref)
    {
        return IsUndefined(ref) || IsNull(ref);
    }

    ani_status GetNullRef(ani_ref *result)
    {
        EtsObject *nullObject = EtsObject::FromCoreType(GetCoroutine()->GetNullValue());
        return AddLocalRef(nullObject, result);
    }

    EtsArray *ToInternalType(ani_fixedarray array)
    {
        ASSERT(!IsNullishValue(array));
        return reinterpret_cast<EtsArray *>(GetInternalType(array));
    }

    EtsObjectArray *ToInternalType(ani_fixedarray_ref array)
    {
        ASSERT(!IsNullishValue(array));
        return reinterpret_cast<EtsObjectArray *>(GetInternalType(array));
    }

    EtsEscompatArray *ToInternalType(ani_array array)
    {
        ASSERT(!IsNullishValue(array));
        return reinterpret_cast<EtsEscompatArray *>(GetInternalType(array));
    }

    EtsEscompatArrayBuffer *ToInternalType(ani_arraybuffer arraybuffer)
    {
        ASSERT(!IsNullishValue(arraybuffer));
        return reinterpret_cast<EtsEscompatArrayBuffer *>(GetInternalType(arraybuffer));
    }

    EtsObject *ToInternalType(ani_ref ref)
    {
        if (IsUndefined(ref)) {
            return EtsReferenceStorage::GetUndefinedEtsObject();
        }
        return GetInternalType(ref);
    }

    EtsObject *ToInternalType(ani_object obj)
    {
        ASSERT(!IsNullishValue(obj));
        return GetInternalType(obj);
    }

    EtsClass *ToInternalType(ani_class cls)
    {
        ASSERT(!IsNullishValue(cls));
        return reinterpret_cast<EtsClass *>(GetInternalType(cls));
    }

    EtsClass *ToInternalType(ani_namespace ns)
    {
        ASSERT(!IsNullishValue(ns));
        return reinterpret_cast<EtsClass *>(GetInternalType(ns));
    }

    EtsClass *ToInternalType(ani_module module)
    {
        ASSERT(!IsNullishValue(module));
        return reinterpret_cast<EtsClass *>(GetInternalType(module));
    }

    EtsString *ToInternalType(ani_string str)
    {
        ASSERT(!IsNullishValue(str));
        return reinterpret_cast<EtsString *>(GetInternalType(str));
    }

    EtsClass *ToInternalType(ani_type type)
    {
        ASSERT(!IsNullishValue(type));
        return reinterpret_cast<EtsClass *>(GetInternalType(type));
    }

    EtsPromise *ToInternalType(ani_resolver resolver)
    {
        auto resolverRef = reinterpret_cast<ani_ref>(resolver);
        EtsObject *obj =
            IsUndefined(resolverRef) ? EtsReferenceStorage::GetUndefinedEtsObject() : GetInternalType(resolverRef);
        return reinterpret_cast<EtsPromise *>(obj);
    }

    ani_status AddGlobalRef(ani_ref ref, ani_ref *result)
    {
        if (IsUndefined(ref)) {
            return GetUndefinedRef(result);
        }
        EtsObject *obj = ToInternalType(ref);
        EtsReference *gref = GetRefStorage()->NewEtsRef(obj, EtsReference::EtsObjectType::GLOBAL);
        ANI_CHECK_RETURN_IF_EQ(gref, nullptr, ANI_OUT_OF_REF);
        *result = EtsRefToAniRef(gref);
        return ANI_OK;
    }

    ani_status DelGlobalRef(ani_ref gref)
    {
        if (IsUndefined(gref)) {
            return ANI_OK;
        }
        EtsReference *etsGRef = AniRefToEtsRef(gref);
        ANI_CHECK_RETURN_IF_EQ(etsGRef->IsGlobal(), false, ANI_INCORRECT_REF);
        GetRefStorage()->RemoveEtsRef(etsGRef);
        return ANI_OK;
    }

    ani_status AddWeakRef(ani_ref ref, ani_wref *result)
    {
        if (IsUndefined(ref)) {
            return GetUndefinedWRef(result);
        }
        EtsObject *obj = ToInternalType(ref);
        EtsReference *etsRef = GetRefStorage()->NewEtsRef(obj, EtsReference::EtsObjectType::WEAK);
        ANI_CHECK_RETURN_IF_EQ(etsRef, nullptr, ANI_OUT_OF_REF);
        *result = EtsRefToAniWRef(etsRef);
        return ANI_OK;
    }

    ani_status DelWeakRef(ani_wref wref)
    {
        if (IsUndefined(wref)) {
            return ANI_OK;
        }
        EtsReference *etsGRef = AniWRefToEtsRef(wref);
        ASSERT(etsGRef->IsWeak());
        GetRefStorage()->RemoveEtsRef(etsGRef);
        return ANI_OK;
    }

    ani_status GetLocalRef(ani_wref wref, ani_boolean *wasReleasedResult, ani_ref *result)
    {
        if (IsUndefined(wref)) {
            return GetUndefinedRef(result);
        }
        EtsReference *etsGRef = AniWRefToEtsRef(wref);
        ASSERT(etsGRef->IsWeak());
        EtsObject *obj = GetRefStorage()->GetEtsObject(etsGRef);
        if (obj == nullptr) {
            // Reference was freed.
            *wasReleasedResult = ANI_TRUE;
            *result = nullptr;
            return ANI_OK;
        }
        *wasReleasedResult = ANI_FALSE;
        return AddLocalRef(obj, result);
    }

    ani_status AddGlobalRef(EtsObject *obj, ani_ref *result)
    {
        if (EtsReferenceStorage::IsUndefinedEtsObject(obj)) {
            return GetUndefinedRef(result);
        }
        EtsReference *gref = GetRefStorage()->NewEtsRef(obj, EtsReference::EtsObjectType::GLOBAL);
        ANI_CHECK_RETURN_IF_EQ(gref, nullptr, ANI_OUT_OF_REF);
        *result = EtsRefToAniRef(gref);
        return ANI_OK;
    }

    ani_status AddLocalRef(EtsObject *obj, ani_ref *result)
    {
        if (EtsReferenceStorage::IsUndefinedEtsObject(obj)) {
            return GetUndefinedRef(result);
        }
        EtsReference *etsRef = GetRefStorage()->NewEtsRef(obj, EtsReference::EtsObjectType::LOCAL);
        ANI_CHECK_RETURN_IF_EQ(etsRef, nullptr, ANI_OUT_OF_REF);
        *result = EtsRefToAniRef(etsRef);
        return ANI_OK;
    }

    ani_status DelLocalRef(ani_ref ref)
    {
        if (IsUndefined(ref)) {
            return ANI_OK;
        }
        EtsReference *etsRef = AniRefToEtsRef(ref);
        ANI_CHECK_RETURN_IF_EQ(etsRef->IsLocal(), false, ANI_INCORRECT_REF);
        GetRefStorage()->RemoveEtsRef(etsRef);
        return ANI_OK;
    }

    ani_status EnsureLocalEnoughRefs(ani_size nrRefs)
    {
        ANI_CHECK_RETURN_IF_EQ(nrRefs, 0, ANI_INVALID_ARGS);
        ANI_CHECK_RETURN_IF_GT(nrRefs, std::numeric_limits<uint32_t>::max(), ANI_OUT_OF_MEMORY);

        auto etsNrRefs = static_cast<uint32_t>(nrRefs);
        bool ret = GetRefStorage()->EnsureLocalEtsCapacity(etsNrRefs);
        ANI_CHECK_RETURN_IF_EQ(ret, false, ANI_OUT_OF_MEMORY);

        return ANI_OK;
    }

    ani_status CreateLocalScope(ani_size nrRefs)
    {
        ANI_CHECK_RETURN_IF_EQ(nrRefs, 0, ANI_INVALID_ARGS);
        ANI_CHECK_RETURN_IF_GT(nrRefs, std::numeric_limits<uint32_t>::max(), ANI_OUT_OF_MEMORY);

        auto nrRefsU32 = static_cast<uint32_t>(nrRefs);
        bool ret = GetRefStorage()->PushLocalEtsFrame(nrRefsU32);
        ANI_CHECK_RETURN_IF_EQ(ret, false, ANI_OUT_OF_MEMORY);
        return ANI_OK;
    }

    ani_status DestroyLocalScope()
    {
        GetRefStorage()->PopLocalEtsFrame(EtsReference::GetUndefined());
        return ANI_OK;
    }

    ani_status CreateEscapeLocalScope(ani_size nrRefs)
    {
        ANI_CHECK_RETURN_IF_EQ(nrRefs, 0, ANI_INVALID_ARGS);
        ANI_CHECK_RETURN_IF_GT(nrRefs, std::numeric_limits<uint32_t>::max(), ANI_OUT_OF_MEMORY);

        auto nrRefsU32 = static_cast<uint32_t>(nrRefs);
        bool ret = GetRefStorage()->PushLocalEtsFrame(nrRefsU32);
        ANI_CHECK_RETURN_IF_EQ(ret, false, ANI_OUT_OF_MEMORY);
        return ANI_OK;
    }

    ani_status DestroyEscapeLocalScope(ani_ref ref, ani_ref *result)
    {
        if (IsUndefined(ref)) {
            GetRefStorage()->PopLocalEtsFrame(EtsReference::GetUndefined());
            return GetUndefinedRef(result);
        }
        EtsReference *resultEtsRef = AniRefToEtsRef(ref);
        EtsReference *etsRef = GetRefStorage()->PopLocalEtsFrame(resultEtsRef);
        ANI_CHECK_RETURN_IF_EQ(etsRef, nullptr, ANI_OUT_OF_REF);
        *result = EtsRefToAniRef(etsRef);
        return ANI_OK;
    }

    NO_COPY_SEMANTIC(ManagedCodeAccessor);
    NO_MOVE_SEMANTIC(ManagedCodeAccessor);

private:
    EtsReferenceStorage *GetRefStorage()
    {
        return env_->GetEtsReferenceStorage();
    }

    EtsObject *GetInternalType(ani_ref ref)
    {
        ASSERT(!IsUndefined(ref));
        EtsReference *etsRef = AniRefToEtsRef(ref);
        return GetRefStorage()->GetEtsObject(etsRef);
    }

    static EtsReference *AniRefToEtsRef(ani_ref ref)
    {
        auto etsRef = reinterpret_cast<EtsReference *>(ref);
        ASSERT(etsRef != nullptr);
        ASSERT(!etsRef->IsWeak());
        return etsRef;
    }

    static ani_ref EtsRefToAniRef(EtsReference *etsRef)
    {
        ASSERT(etsRef != nullptr);
        ASSERT(!etsRef->IsWeak());
        return reinterpret_cast<ani_ref>(etsRef);
    }

    static EtsReference *AniWRefToEtsRef(ani_wref wref)
    {
        auto etsRef = reinterpret_cast<EtsReference *>(wref);
        ASSERT(etsRef != nullptr);
        ASSERT(etsRef->IsWeak());
        return etsRef;
    }

    static ani_wref EtsRefToAniWRef(EtsReference *etsRef)
    {
        ASSERT(etsRef != nullptr);
        ASSERT(etsRef->IsWeak());
        return reinterpret_cast<ani_wref>(etsRef);
    }

    PandaEnv *env_;
};

class ScopedManagedCodeFix : public ManagedCodeAccessor {
    class ExceptionData {
    public:
        ExceptionData(const char *name, const char *message) : className_(name)
        {
            if (message != nullptr) {
                message_ = message;
            }
        }

        const char *GetClassName() const
        {
            return className_.c_str();
        }

        const char *GetMessage() const
        {
            return message_ ? message_.value().c_str() : nullptr;
        }

    private:
        PandaString className_;
        std::optional<PandaString> message_;
    };

public:
    explicit ScopedManagedCodeFix(PandaEnv *env)
        : ManagedCodeAccessor(env), alreadyInManaged_(ManagedThread::IsManagedScope())
    {
        ASSERT(env == PandaEnv::GetCurrent());
        ASSERT(alreadyInManaged_ ? !StackWalker::Create(GetCoroutine()).GetMethod()->IsCriticalNative() : true);

        if (alreadyInManaged_ && IsAccessFromManagedAllowed()) {
            return;
        }

        GetCoroutine()->ManagedCodeBegin();
    }
    explicit ScopedManagedCodeFix(ani_env *env) : ScopedManagedCodeFix(PandaEnv::FromAniEnv(env)) {}

    ~ScopedManagedCodeFix()
    {
        if (exceptionData_) {
            // NOTE: Throw exception
        }

        if (!alreadyInManaged_) {
            GetCoroutine()->ManagedCodeEnd();
        }
    }

    bool HasPendingException()
    {
        return exceptionData_ || GetCoroutine()->HasPendingException();
    }

    NO_COPY_SEMANTIC(ScopedManagedCodeFix);
    NO_MOVE_SEMANTIC(ScopedManagedCodeFix);

private:
    bool IsAccessFromManagedAllowed()
    {
#ifndef NDEBUG
        auto stack = StackWalker::Create(GetCoroutine());
        auto method = EtsMethod::FromRuntimeMethod(stack.GetMethod());
        ASSERT(method != nullptr);
        return method->IsFastNative();
#else
        return true;
#endif  // NDEBUG
    }

    PandaUniquePtr<ExceptionData> exceptionData_;
    bool alreadyInManaged_;
};

template <typename Type>
class LocalRef {
public:
    explicit LocalRef(ani_env *env) : env_(PandaEnv::FromAniEnv(env)), ref_(nullptr) {}
    ~LocalRef()
    {
        Reset();
    }

    Type &GetRef()
    {
        return ref_;
    }

    void Reset()
    {
        if (ref_ != nullptr) {
            ASSERT(ScopedManagedCodeFix(env_).DelLocalRef(static_cast<ani_ref>(ref_)) == ANI_OK);
        }
        ref_ = NULL;
    }

    Type Release()
    {
        Type tmpRef = ref_;
        ref_ = nullptr;
        return tmpRef;
    }

private:
    ani_env *env_;
    Type ref_;

    NO_COPY_SEMANTIC(LocalRef);
    NO_MOVE_SEMANTIC(LocalRef);
};
}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_SCOPED_OBJECTS_FIX_H
