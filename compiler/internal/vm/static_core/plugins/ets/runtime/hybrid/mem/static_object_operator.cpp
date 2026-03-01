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

#include "runtime/mem/object_helpers.h"
#include "runtime/mem/object-references-iterator-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"
#include "plugins/ets/runtime/hybrid/mem/static_object_operator.h"

#if defined(ARK_HYBRID)
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#endif

namespace ark::mem {

StaticObjectOperator StaticObjectOperator::instance_;

#ifdef PANDA_JS_ETS_HYBRID_MODE
class SkipReferentHandler {
public:
    explicit SkipReferentHandler(const common::RefFieldVisitor &visitor, ObjectPointerType *weakReferentPointer)
        : visitor_(visitor), weakReferentPointer_(weakReferentPointer)
    {
    }

    ~SkipReferentHandler() = default;

    bool ProcessObjectPointer([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *p)
    {
        if (p == weakReferentPointer_) {
            // skip referent
            return true;
        }
        if (*p != 0) {
            visitor_(reinterpret_cast<common::RefField<> &>(*reinterpret_cast<common::BaseObject **>(p)));
        }
        return true;
    }

private:
    const common::RefFieldVisitor &visitor_;
    const ObjectPointerType *weakReferentPointer_ {nullptr};
};
#endif

class Handler {
public:
    explicit Handler(const common::RefFieldVisitor &visitor) : visitor_(visitor) {}

    ~Handler() = default;

    bool ProcessObjectPointer([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *p)
    {
        if (*p != 0) {
            visitor_(reinterpret_cast<common::RefField<> &>(*reinterpret_cast<common::BaseObject **>(p)));
        }
        return true;
    }

private:
    const common::RefFieldVisitor &visitor_;
};

void StaticObjectOperator::Initialize(ark::ets::PandaEtsVM *vm)
{
#if defined(ARK_HYBRID)
    instance_.vm_ = vm;
    common::BaseObject::RegisterStatic(&instance_);
#endif
}

void StaticObjectOperator::ForEachRefField(const common::BaseObject *object,
                                           const common::RefFieldVisitor &visitor) const
{
    const auto *objHeader = reinterpret_cast<const ObjectHeader *>(object);
    const auto *cls = objHeader->template ClassAddr<const Class>();
    auto *etsClass = ark::ets::EtsClass::FromRuntimeClass(cls);
    if (UNLIKELY(etsClass->IsReference())) {
        auto *refProcessor = static_cast<ark::mem::ets::EtsReferenceProcessor *>(vm_->GetReferenceProcessor());
        refProcessor->EnqueueReference(objHeader);
        ObjectPointerType *referentPointer = reinterpret_cast<ObjectPointerType *>(
            ToUintPtr(objHeader) + ark::ets::EtsWeakReference::GetReferentOffset());
        SkipReferentHandler handler(visitor, referentPointer);
        ObjectIterator<LangTypeT::LANG_TYPE_STATIC>::template Iterate<false>(cls, const_cast<ObjectHeader *>(objHeader),
                                                                             &handler);
    } else {
        Handler handler(visitor);
        ObjectIterator<LangTypeT::LANG_TYPE_STATIC>::template Iterate<false>(cls, const_cast<ObjectHeader *>(objHeader),
                                                                             &handler);
    }
}

size_t StaticObjectOperator::ForEachRefFieldAndGetSize(const common::BaseObject *object,
                                                       const common::RefFieldVisitor &visitor) const
{
    size_t size = GetSize(object);
    ForEachRefField(object, visitor);
    return size;
}

common::BaseObject *StaticObjectOperator::GetForwardingPointer(const common::BaseObject *object) const
{
    // Overwrite class by forwarding address. Read barrier must be called before reading class.
    uint64_t fwdAddr = *reinterpret_cast<const uint64_t *>(object);
    return reinterpret_cast<common::BaseObject *>(fwdAddr & ObjectHeader::GetClassMask());
}

void StaticObjectOperator::SetForwardingPointerAfterExclusive(common::BaseObject *object, common::BaseObject *fwdPtr)
{
    auto &word = *reinterpret_cast<uint64_t *>(object);
    uint64_t flags = word & (~ObjectHeader::GetClassMask());
    word = flags | (reinterpret_cast<uint64_t>(fwdPtr) & ObjectHeader::GetClassMask());
}

}  // namespace ark::mem
