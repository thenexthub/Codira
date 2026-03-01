/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_H_

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_flags.h"

#include <node_api.h>

#if defined(PANDA_JS_ETS_HYBRID_MODE)
#include "interfaces/inner_api/napi/native_node_hybrid_api.h"
#endif

namespace ark::ets::interop::js {
class InteropCtx;
}  // namespace ark::ets::interop::js

namespace ark::ets::interop::js::ets_proxy {
// Forward declaration
class SharedReferenceStorage;
namespace testing {
class SharedReferenceStorage1GTest;
}  // namespace testing

class SharedReference {
public:
    static constexpr size_t MAX_MARK_BITS = 27U;
    static_assert(MAX_MARK_BITS <= EtsMarkWord::INTEROP_INDEX_SIZE);

    // Actual state of shared object is contained in ETS
    void InitETSObject(InteropCtx *ctx, EtsObject *etsObject, napi_ref jsRef, uint32_t refIdx);

    // Actual state of shared object is contained in JS
    void InitJSObject(InteropCtx *ctx, EtsObject *etsObject, napi_ref jsRef, uint32_t refIdx);

    // State of object is shared between ETS and JS
    void InitHybridObject(InteropCtx *ctx, EtsObject *etsObject, napi_ref jsRef, uint32_t refIdx);

    using InitFn = decltype(&SharedReference::InitHybridObject);

    EtsObject *GetEtsObject() const
    {
        ASSERT(etsRef_ != nullptr);
        return etsRef_;
    }

    napi_ref GetJsRef() const
    {
        return jsRef_;
    }

    const InteropCtx *GetCtx() const
    {
        ASSERT(ctx_ != nullptr);
        return ctx_;
    }

    bool HasETSFlag() const
    {
        return flags_.HasBit<SharedReferenceFlags::Bit::ETS>();
    }

    bool HasJSFlag() const
    {
        return flags_.HasBit<SharedReferenceFlags::Bit::JS>();
    }

    /**
     * Empty ref means the ref is not allocated or removed from storage and can't be used
     * @return true if reference is empty, false - otherwise
     */
    bool IsEmpty() const
    {
        return flags_.IsEmpty();
    }

    /**
     * Mark the reference as live. It's special method for XGC marking
     * @return true if the method marked the reference, false if the reference has already been marked
     */
    PANDA_PUBLIC_API bool MarkIfNotMarked();

    /// @return true if the refernce is marked, false - otherwise
    bool IsMarked() const
    {
        return flags_.HasBit<SharedReferenceFlags::Bit::MARK>();
    }

    /// @class provides forward iterator for SharedReference with chain of contexts and one EtsObject
    class Iterator {
    public:
        // NOLINTBEGIN(readability-identifier-naming)
        using iterator_category = std::forward_iterator_tag;
        using value_type = SharedReference *;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;
        // NOLINTEND(readability-identifier-naming)

        friend class SharedReference;

        PANDA_PUBLIC_API Iterator &operator++();
        PANDA_PUBLIC_API Iterator operator++(int);  // NOLINT(cert-dcl21-cpp)

        bool operator==(const Iterator &other) const
        {
            return ref_ == other.ref_;
        }

        bool operator!=(const Iterator &other) const
        {
            return ref_ != other.ref_;
        }

        SharedReference *operator*()
        {
            return const_cast<SharedReference *>(ref_);
        }

        const SharedReference *operator*() const
        {
            return ref_;
        }

        SharedReference *operator->()
        {
            return const_cast<SharedReference *>(ref_);
        }

        const SharedReference *operator->() const
        {
            return ref_;
        }

        Iterator() = default;
        explicit Iterator(const SharedReference *ref) : ref_(ref) {}
        DEFAULT_COPY_SEMANTIC(Iterator);
        DEFAULT_NOEXCEPT_MOVE_SEMANTIC(Iterator);
        ~Iterator() = default;

    private:
        const SharedReference *ref_ {nullptr};
    };

    Iterator GetIterator() const
    {
        return Iterator(this);
    }

    friend std::ostream &operator<<(std::ostream &out, const SharedReference *ref)
    {
        out << static_cast<const void *>(ref);
        if (LIKELY(ref != nullptr)) {
            out << ": | ETSObject:" << ref->etsRef_ << " | napi_ref:" << ref->jsRef_ << " | ctx:" << ref->ctx_ << " | "
                << ref->flags_ << " |";
        }
        return out;
    }

private:
    friend class SharedReferenceSanity;
    friend class SharedReferenceStorage;
    friend class testing::SharedReferenceStorage1GTest;

    void InitRef(InteropCtx *ctx, EtsObject *etsObject, napi_ref jsRef, uint32_t refIdx);

    void SetETSObject(EtsObject *obj)
    {
        ASSERT(obj != nullptr);
        etsRef_ = obj;
    }

    static bool HasReference(EtsObject *etsObject)
    {
        return etsObject->HasInteropIndex();
    }

    mem::GCRoot GetGCRoot()
    {
        ASSERT(etsRef_ != nullptr);
        return mem::GCRoot {mem::RootType::ROOT_VM, reinterpret_cast<ObjectHeader **>(&etsRef_)};
    }

    static uint32_t ExtractMaybeIndex(EtsObject *etsObject)
    {
        ASSERT(HasReference(etsObject));
        return etsObject->GetInteropIndex();
    }

    /// Unset mark flag from the reference
    void Unmark()
    {
        flags_.Unmark();
    }

    /* Possible values:
     *                 ets_proxy:  {instance,       proxy}
     *      extensible ets_proxy:  {proxy-base,     extender-proxy}
     *                  js_proxy:  {proxy,          instance}
     *      extensible  js_proxy:  {extender-proxy, proxy-base}
     */
    EtsObject *etsRef_ {};
    napi_ref jsRef_ {};
    const InteropCtx *ctx_ {};

    SharedReferenceFlags flags_ {};
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_H_
