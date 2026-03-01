/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/heap_verifier.h"
#include "runtime/mem/object_helpers-inl.h"
#include "libarkbase/utils/utf.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_HEAP_VERIFIER LOG(ERROR, GC) << "HEAP_VERIFIER: "

template <LangTypeT LANG_TYPE>
static Field *GetFieldFromOffsetForObject([[maybe_unused]] const ObjectHeader *object, Class *cls,
                                          const uint32_t offset)
{
    if constexpr (LANG_TYPE == LangTypeT::LANG_TYPE_DYNAMIC) {
        UNREACHABLE();  // Dynamic classes do not provide info
    }
    ASSERT(object != nullptr);
    ASSERT(cls != nullptr);
    ASSERT(!cls->IsDynamicClass());

    while (cls != nullptr) {
        size_t refNum = cls->GetRefFieldsNum<false>();
        int32_t localI = (static_cast<int32_t>(offset) - static_cast<int32_t>(cls->GetRefFieldsOffset<false>())) /
                         ClassHelper::OBJECT_POINTER_SIZE;
        if (localI < 0 || localI >= static_cast<int32_t>(refNum)) {
            cls = cls->GetBase();
            continue;
        }
        auto fields = cls->GetFields();
        ASSERT(fields.size() == refNum);
        return &fields[localI];
    }
    return nullptr;
}

template <LangTypeT LANG_TYPE>
static uint32_t GetIndexFromOffsetForArray(const coretypes::Array *array, [[maybe_unused]] Class *cls,
                                           [[maybe_unused]] const uint32_t offset)
{
    if constexpr (LANG_TYPE == LangTypeT::LANG_TYPE_DYNAMIC) {
        UNREACHABLE();  // Dynamic classes do not provide info
    }
    ASSERT(array != nullptr);
    ASSERT(cls != nullptr);
    ASSERT(!cls->IsDynamicClass());

    uintptr_t arrayStart = ToUintPtr(array->GetBase<ObjectPointerType *>());
    ASSERT(offset > arrayStart);
    return static_cast<uint32_t>((offset - arrayStart) / coretypes::Array::GetElementSize<ObjectHeader *, false>());
}

template <LangTypeT LANG_TYPE>
static Field *GetFieldFromOffsetForClass(const Class *cls, const uint32_t offset)
{
    if constexpr (LANG_TYPE == LangTypeT::LANG_TYPE_DYNAMIC) {
        UNREACHABLE();  // Dynamic classes do not provide info
    }
    ASSERT(cls != nullptr);
    ASSERT(!cls->IsDynamicClass());
    ObjectHeader *object = cls->GetManagedObject();
    ASSERT(object != nullptr);
    size_t localI = (offset - ToUintPtr(cls) + cls->GetRefFieldsOffset<true>() - ToUintPtr(object)) /
                    ClassHelper::OBJECT_POINTER_SIZE;
    auto fields = cls->GetFields();
    ASSERT(localI < fields.size());
    return &fields[localI];
}

template <LangTypeT LANG_TYPE>
static std::variant<Field *, uint32_t> GetFieldFromOffset([[maybe_unused]] const ObjectHeader *objectHeader,
                                                          [[maybe_unused]] const uint32_t offset)
{
    if constexpr (LANG_TYPE == LangTypeT::LANG_TYPE_DYNAMIC) {
        return nullptr;  // Dynamic classes do not provide info
    }
    auto *cls = objectHeader->ClassAddr<Class>();
    ASSERT(cls != nullptr);

    if (cls->IsObjectArrayClass()) {
        return GetIndexFromOffsetForArray<LANG_TYPE>(static_cast<const coretypes::Array *>(objectHeader), cls, offset);
    }
    if (cls->IsClassClass()) {
        auto objectCls = ark::Class::FromClassObject(objectHeader);
        if (objectCls->IsInitializing() || objectCls->IsInitialized()) {
            return GetFieldFromOffsetForClass<LANG_TYPE>(objectCls, offset);
        }
        return nullptr;
    }
    return GetFieldFromOffsetForObject<LANG_TYPE>(objectHeader, cls, offset);
}

template <LangTypeT LANG_TYPE>
static PandaString GetFieldInfo(const ObjectHeader *obj, const uint32_t offset)
{
    ASSERT(obj != nullptr);
    std::variant<Field *, uint32_t> res = GetFieldFromOffset<LANG_TYPE>(obj, offset);
    if (std::holds_alternative<uint32_t>(res)) {
        return "index " + PandaString(std::to_string(std::get<uint32_t>(res)));
    }
    Field *field = std::get<Field *>(res);
    if (field == nullptr) {
        return "unresolved field";
    }
    auto fieldName = PandaString(utf::Mutf8AsCString(field->GetName().data));
    return "field: " + fieldName;
}

// Should be called only with MutatorLock held
template <class LanguageConfig>
size_t HeapVerifier<LanguageConfig>::VerifyAllPaused() const
{
    Rendezvous *rendezvous = Thread::GetCurrent()->GetVM()->GetRendezvous();
    rendezvous->SafepointBegin();
    size_t failCount = VerifyAll();
    rendezvous->SafepointEnd();
    return failCount;
}

template <LangTypeT LANG_TYPE>
void HeapObjectVerifier<LANG_TYPE>::operator()(ObjectHeader *obj)
{
    HeapReferenceVerifier<LANG_TYPE> refVerifier(heap_, failCount_);
    ObjectHelpers<LANG_TYPE>::template TraverseAllObjectsWithInfo<false>(obj, refVerifier);
}

template <LangTypeT LANG_TYPE>
bool HeapReferenceVerifier<LANG_TYPE>::operator()(ObjectHeader *objectHeader, ObjectHeader *referent,
                                                  [[maybe_unused]] uint32_t offset, [[maybe_unused]] bool isVolatile)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (LANG_TYPE == LANG_TYPE_DYNAMIC) {
        // Weak reference can be passed here, need to resolve the referent
        coretypes::TaggedValue value(referent);
        if (value.IsWeak()) {
            referent = value.GetWeakReferent();
        }
    }
    if (!heap_->IsLiveObject(referent)) {
        if constexpr (LANG_TYPE == LANG_TYPE_DYNAMIC) {
            LOG_HEAP_VERIFIER << "Heap corruption found! Heap object " << std::hex << objectHeader
                              << " references a dead object at " << referent;
        } else {
            auto *cls = objectHeader->template ClassAddr<Class>();
            LOG_HEAP_VERIFIER << "Heap corruption found! Heap object at " << std::hex << objectHeader
                              << " (class: " << cls->GetName() << ") references a dead object at " << referent
                              << " with " << GetFieldInfo<LANG_TYPE>(objectHeader, offset);
        }
        ++(*failCount_);
    } else if (referent->IsForwarded()) {
        if constexpr (LANG_TYPE == LANG_TYPE_DYNAMIC) {
            LOG_HEAP_VERIFIER << "Heap corruption found! Heap object " << std::hex << objectHeader
                              << " references a forwarded object at " << referent;
        } else {
            auto *cls = objectHeader->template ClassAddr<Class>();
            LOG_HEAP_VERIFIER << "Heap corruption found! Heap object at " << std::hex << objectHeader
                              << " (class: " << cls->GetName() << ") references a forwarded object at " << referent
                              << " with " << GetFieldInfo<LANG_TYPE>(objectHeader, offset);
        }
        ++(*failCount_);
    }
    return true;
}

template <LangTypeT LANG_TYPE>
void HeapReferenceVerifier<LANG_TYPE>::operator()(const GCRoot &root)
{
    auto referent = root.GetObjectHeader();
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (LANG_TYPE == LANG_TYPE_DYNAMIC) {
        // Weak reference can be passed here, need to resolve the referent
        coretypes::TaggedValue value(referent);
        if (value.IsWeak()) {
            referent = value.GetWeakReferent();
        }
    }
    if (!heap_->IsLiveObject(referent)) {
        LOG_HEAP_VERIFIER << "Heap corruption found! Root references a dead object at " << std::hex << referent;
        ++(*failCount_);
    } else if (referent->IsForwarded()) {
        LOG_HEAP_VERIFIER << "Heap corruption found! Root references a forwarded object at " << std::hex << referent;
        ++(*failCount_);
    }
}

template <class LanguageConfig>
bool HeapVerifier<LanguageConfig>::IsValidObjectAddress(void *addr) const
{
    return IsAligned<DEFAULT_ALIGNMENT_IN_BYTES>(ToUintPtr(addr)) && IsHeapAddress(addr);
}

template <class LanguageConfig>
bool HeapVerifier<LanguageConfig>::IsHeapAddress(void *addr) const
{
    return heap_->ContainObject(reinterpret_cast<ObjectHeader *>(addr));
}

template <class LanguageConfig>
size_t HeapVerifier<LanguageConfig>::VerifyHeap() const
{
    return heap_->VerifyHeapReferences();
}

template <class LanguageConfig>
size_t HeapVerifier<LanguageConfig>::VerifyRoot() const
{
    RootManager<LanguageConfig> rootManager(heap_->GetPandaVM());
    size_t failCount = 0;
    rootManager.VisitNonHeapRoots([this, &failCount](const GCRoot &root) {
        if (root.GetType() == RootType::ROOT_FRAME || root.GetType() == RootType::ROOT_THREAD) {
            auto *baseCls = root.GetObjectHeader()->ClassAddr<BaseClass>();
            if (baseCls == nullptr) {
                LOG_HEAP_VERIFIER << "Heap corruption found! Class address for root " << std::hex
                                  << root.GetObjectHeader() << " is null";
                ++failCount;
            } else if (!(!baseCls->IsDynamicClass() && static_cast<Class *>(baseCls)->IsClassClass())) {
                HeapReferenceVerifier<LanguageConfig::LANG_TYPE>(heap_, &failCount)(root);
            }
        }
    });

    return failCount;
}

template <class LanguageConfig>
static PandaString GetClassName(const ObjectHeader *obj)
{
    if constexpr (LanguageConfig::LANG_TYPE == LANG_TYPE_STATIC) {
        auto *cls = obj->template ClassAddr<Class>();
        if (IsAddressInObjectsHeap(cls)) {
            return cls->GetName().c_str();  // NOLINT(readability-redundant-string-cstr)
        }
    }
    return "unknown class";
}

template <class LanguageConfig>
size_t FastHeapVerifier<LanguageConfig>::CheckHeap(const PandaUnorderedSet<const ObjectHeader *> &heapObjects,
                                                   const PandaVector<ObjectCache> &referentObjects) const
{
    size_t failsCount = 0;
    for (auto objectCache : referentObjects) {
        if (heapObjects.find(objectCache.referent) == heapObjects.end()) {
            LOG_HEAP_VERIFIER << "Heap object " << std::hex << objectCache.heapObject << " ("
                              << GetClassName<LanguageConfig>(objectCache.heapObject)
                              << ") references a dead object at " << objectCache.referent << " ("
                              << GetClassName<LanguageConfig>(objectCache.referent) << ") with "
                              << GetFieldInfo<LanguageConfig::LANG_TYPE>(objectCache.heapObject, objectCache.offset);
            ++failsCount;
        }
    }
    return failsCount;
}

template <class LanguageConfig>
// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
size_t FastHeapVerifier<LanguageConfig>::VerifyAll() const
{
    PandaUnorderedSet<const ObjectHeader *> heapObjects;
    PandaVector<ObjectCache> referentObjects;
    size_t failsCount = 0;

    auto lazyVerify = [&heapObjects, &referentObjects, &failsCount](const ObjectHeader *objectHeader,
                                                                    const ObjectHeader *referent, size_t offset,
                                                                    [[maybe_unused]] bool isVolatile) {
        // Lazy verify during heap objects collection
        if (heapObjects.find(referent) == heapObjects.end()) {
            referentObjects.push_back(ObjectCache({objectHeader, referent, offset}));
        }
        if (referent->IsForwarded()) {
            LOG_HEAP_VERIFIER << "Heap object " << std::hex << objectHeader << " ("
                              << GetClassName<LanguageConfig>(objectHeader) << ") references a forwarded object at "
                              << referent << " (" << GetClassName<LanguageConfig>(referent) << ") with "
                              << GetFieldInfo<LanguageConfig::LANG_TYPE>(objectHeader, offset);
            ++failsCount;
        }
        auto *classAddr = objectHeader->ClassAddr<BaseClass>();
        if (!IsAddressInObjectsHeap(classAddr)) {
            LOG_HEAP_VERIFIER << "Heap object " << std::hex << objectHeader
                              << " has non-heap class address: " << classAddr;
            ++failsCount;
        }
        return true;
    };
    const std::function<bool(ObjectHeader *, ObjectHeader *, size_t, bool)> lazyVerifyFunctor(lazyVerify);
    auto collectObjects = [&heapObjects, &lazyVerifyFunctor](ObjectHeader *object) {
        heapObjects.insert(object);
        ObjectHelpers<LanguageConfig::LANG_TYPE>::template TraverseAllObjectsWithInfo<false>(object, lazyVerifyFunctor);
    };

    // Heap objects verifier

    // Add strings from string table because these objects are like a phoenix.
    // A string object may exist but there are no live references to it (no bit set in the live bitmap).
    // But later code may reuse it by calling StringTable::GetOrInternString so this string
    // get alive. That is why we mark all strings as alive by visiting the string table.
    Thread::GetCurrent()->GetVM()->VisitStrings(collectObjects);
    heap_->IterateOverObjects(collectObjects);
    failsCount += this->CheckHeap(heapObjects, referentObjects);
    // Stack verifier
    RootManager<LanguageConfig> rootManager(heap_->GetPandaVM());
    auto rootVerifier = [&heapObjects, &failsCount](const GCRoot &root) {
        const auto *rootObjHeader = root.GetObjectHeader();
        auto *baseCls = rootObjHeader->ClassAddr<BaseClass>();
        if (!IsAddressInObjectsHeap(ToUintPtr(baseCls))) {
            LOG_HEAP_VERIFIER << "Class address for root " << std::hex << rootObjHeader
                              << " is not in objects heap: " << baseCls;
            ++failsCount;
        } else if (baseCls->IsDynamicClass() || !static_cast<Class *>(baseCls)->IsClassClass()) {
            if (heapObjects.find(rootObjHeader) == heapObjects.end()) {
                LOG_HEAP_VERIFIER << "Root references a dead object at " << std::hex << rootObjHeader;
                ++failsCount;
            }
        }
    };
    rootManager.VisitLocalRoots(rootVerifier);

    return failsCount;
}

ObjectVerificationInfo::ObjectVerificationInfo(ObjectHeader *referent)
    : classAddress_(referent->ClassAddr<void *>()), oldAddress_(referent)
{
}

bool ObjectVerificationInfo::VerifyUpdatedRef(ObjectHeader *objectHeader, ObjectHeader *updatedRef,
                                              bool inAliveSpace) const
{
    ObjectHeader *correctAddress = oldAddress_;
    if (!inAliveSpace) {
        if (!oldAddress_->IsForwarded()) {
            LOG_HEAP_VERIFIER << "Object " << std::hex << objectHeader << " had reference " << oldAddress_
                              << ", which is not forwarded, new reference address: " << updatedRef;
            return false;
        }
        correctAddress = GetForwardAddress(oldAddress_);
    }
    if (correctAddress != updatedRef) {
        LOG_HEAP_VERIFIER << "Object " << std::hex << objectHeader << " has incorrect updated reference " << updatedRef
                          << ", correct address: " << correctAddress;
        return false;
    }
    void *newClassAddr = updatedRef->ClassAddr<void *>();
    if (newClassAddr != classAddress_) {
        LOG_HEAP_VERIFIER << "Object " << std::hex << objectHeader << " has incorrect class address (" << newClassAddr
                          << ") in updated reference " << updatedRef
                          << ", class address before collection: " << classAddress_;
        return false;
    }

    return true;
}

template <class LanguageConfig>
bool HeapVerifierIntoGC<LanguageConfig>::InCollectableSpace(const ObjectHeader *object) const
{
    for (const auto &memRange : this->collectableMemRanges_) {
        if (memRange.Contains(ToUintPtr(object))) {
            return true;
        }
    }
    return false;
}

template <class LanguageConfig>
bool HeapVerifierIntoGC<LanguageConfig>::InAliveSpace(const ObjectHeader *object) const
{
    for (const auto &memRange : this->aliveMemRanges_) {
        if (memRange.Contains(ToUintPtr(object))) {
            return true;
        }
    }
    return false;
}

template <class LanguageConfig>
void HeapVerifierIntoGC<LanguageConfig>::AddToVerificationInfo(RefsVerificationInfo &verificationInfo, size_t refNumber,
                                                               ObjectHeader *objectHeader, ObjectHeader *referent)
{
    if (this->InCollectableSpace(referent)) {
        ObjectVerificationInfo objInfo(referent);
        auto it = verificationInfo.find(objectHeader);
        if (it != verificationInfo.end()) {
            it->second.insert({refNumber, objInfo});
        } else {
            verificationInfo.insert({objectHeader, VerifyingRefs({{refNumber, objInfo}})});
        }
    }
}

template <class LanguageConfig>
void HeapVerifierIntoGC<LanguageConfig>::CollectVerificationInfo(PandaVector<MemRange> &&collectableMemRanges)
{
    size_t refNumber = 0;
    collectableMemRanges_ = std::move(collectableMemRanges);
    const std::function<void(ObjectHeader *, ObjectHeader *)> refsCollector =
        [this, &refNumber](ObjectHeader *objectHeader, ObjectHeader *referent) {
            if (this->InCollectableSpace(objectHeader)) {
                this->AddToVerificationInfo(this->collectableVerificationInfo_, refNumber, objectHeader, referent);
            } else {
                this->AddToVerificationInfo(this->permanentVerificationInfo_, refNumber, objectHeader, referent);
            }
            ++refNumber;
        };

    auto collectFunctor = [&refNumber, &refsCollector](ObjectHeader *object) {
        if (object->IsMarkedForGC<false>()) {
            refNumber = 0;
            ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjects(object, refsCollector);
        }
    };
    heap_->IterateOverObjects(collectFunctor);
}

template <class LanguageConfig>
// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
size_t HeapVerifierIntoGC<LanguageConfig>::VerifyAll(PandaVector<MemRange> &&aliveMemRanges)
{
    size_t failsCount = 0U;
    size_t refNumber = 0U;
    aliveMemRanges_ = std::move(aliveMemRanges);
    auto it = permanentVerificationInfo_.begin();
    for (auto &info : collectableVerificationInfo_) {
        ObjectHeader *obj = info.first;
        if (obj->IsForwarded()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            permanentVerificationInfo_[GetForwardAddress(obj)] = std::move(info.second);
        } else if (this->InAliveSpace(obj)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            permanentVerificationInfo_[obj] = std::move(info.second);
        }
    }
    collectableVerificationInfo_.clear();
    const std::function<bool(ObjectHeader *, ObjectHeader *, uint32_t, bool)> nonYoungChecker =
        [this, &failsCount](const ObjectHeader *objectHeader, const ObjectHeader *referent,
                            [[maybe_unused]] const uint32_t offset, [[maybe_unused]] bool isVolatile) {
            if (this->InCollectableSpace(referent)) {
                if constexpr (LanguageConfig::LANG_TYPE == LANG_TYPE_DYNAMIC) {
                    LOG_HEAP_VERIFIER << "Object " << std::hex << objectHeader << " references a dead object "
                                      << referent << " after collection";
                } else {
                    auto *cls = objectHeader->template ClassAddr<Class>();
                    ASSERT(cls != nullptr);
                    LOG_HEAP_VERIFIER << "Object " << std::hex << objectHeader << "(class: " << cls->GetName()
                                      << ") references a dead object " << referent << " after collection (with "
                                      << GetFieldInfo<LanguageConfig::LANG_TYPE>(objectHeader, offset) << ")";
                }
                ++failsCount;
            }
            return true;
        };
    const std::function<bool(ObjectHeader *, ObjectHeader *, uint32_t, bool)> sameObjChecker =
        [this, &nonYoungChecker, &refNumber, &failsCount, &it](ObjectHeader *objectHeader, ObjectHeader *referent,
                                                               [[maybe_unused]] const uint32_t offset,
                                                               [[maybe_unused]] bool isVolatile) {
            auto refIt = it->second.find(refNumber);
            if (refIt != it->second.end()) {
                if (!refIt->second.VerifyUpdatedRef(objectHeader, referent, this->InAliveSpace(referent))) {
                    ++failsCount;
                }
            } else {
                nonYoungChecker(objectHeader, referent, offset, isVolatile);
            }
            ++refNumber;
            return true;
        };
    // Check references in alive objects
    ObjectVisitor traverseAliveObj = [&nonYoungChecker, &sameObjChecker, &refNumber, this, &it](ObjectHeader *object) {
        if (!object->IsMarkedForGC<false>() || (this->InCollectableSpace(object) && !this->InAliveSpace(object))) {
            return;
        }
        it = this->permanentVerificationInfo_.find(object);
        if (it == this->permanentVerificationInfo_.end()) {
            ObjectHelpers<LanguageConfig::LANG_TYPE>::template TraverseAllObjectsWithInfo<false>(object,
                                                                                                 nonYoungChecker);
        } else {
            refNumber = 0U;
            ObjectHelpers<LanguageConfig::LANG_TYPE>::template TraverseAllObjectsWithInfo<false>(object,
                                                                                                 sameObjChecker);
        }
    };
    heap_->IterateOverObjects(traverseAliveObj);
    return failsCount;
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(HeapVerifier);
TEMPLATE_CLASS_LANGUAGE_CONFIG(FastHeapVerifier);
TEMPLATE_CLASS_LANGUAGE_CONFIG(HeapVerifierIntoGC);
}  // namespace ark::mem
