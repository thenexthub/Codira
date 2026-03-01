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
#ifndef PANDA_MEM_HEAP_VERIFIER_H
#define PANDA_MEM_HEAP_VERIFIER_H

#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/logger.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/rendezvous.h"

namespace ark::mem {

class HeapManager;
class GCRoot;

/// HeapReferenceVerifier checks reference checks if the referent is with the heap and it is live.
template <LangTypeT LANG_TYPE = LANG_TYPE_STATIC>
class HeapReferenceVerifier {
public:
    explicit HeapReferenceVerifier(HeapManager *heap, size_t *count) : heap_(heap), failCount_(count) {}

    bool operator()(ObjectHeader *objectHeader, ObjectHeader *referent, uint32_t offset,
                    [[maybe_unused]] bool isVolatile);

    void operator()(const GCRoot &root);

private:
    HeapManager *const heap_ {nullptr};
    size_t *const failCount_ {nullptr};
};

/**
 * HeapObjectVerifier iterates over HeapManager's allocated objects. If an object contains reference, it checks if the
 * referent is with the heap and it is live.
 */
template <LangTypeT LANG_TYPE = LANG_TYPE_STATIC>
class HeapObjectVerifier {
public:
    HeapObjectVerifier() = delete;

    HeapObjectVerifier(HeapManager *heap, size_t *count) : heap_(heap), failCount_(count) {}

    void operator()(ObjectHeader *obj);

    size_t GetFailCount() const
    {
        return *failCount_;
    }

private:
    HeapManager *const heap_ {nullptr};
    size_t *const failCount_ {nullptr};
};

class HeapVerifierBase {
public:
    explicit HeapVerifierBase(HeapManager *heap) : heap_(heap) {}
    DEFAULT_COPY_SEMANTIC(HeapVerifierBase);
    DEFAULT_MOVE_SEMANTIC(HeapVerifierBase);
    ~HeapVerifierBase() = default;

protected:
    HeapManager *heap_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

/// A class to query address validity.
template <class LanguageConfig>
class HeapVerifier : public HeapVerifierBase {
public:
    explicit HeapVerifier(HeapManager *heap) : HeapVerifierBase(heap) {}
    DEFAULT_COPY_SEMANTIC(HeapVerifier);
    DEFAULT_MOVE_SEMANTIC(HeapVerifier);
    ~HeapVerifier() = default;

    bool IsValidObjectAddress(void *addr) const;

    bool IsHeapAddress(void *addr) const;

    size_t VerifyRoot() const;

    size_t VerifyHeap() const;

    size_t VerifyAll() const
    {
        return VerifyRoot() + VerifyHeap();
    }

    size_t VerifyAllPaused() const;
};

/// Fast heap verifier for check heap and stack
template <class LanguageConfig>
class FastHeapVerifier : public HeapVerifierBase {
public:
    explicit FastHeapVerifier(HeapManager *heap) : HeapVerifierBase(heap) {}
    DEFAULT_COPY_SEMANTIC(FastHeapVerifier);
    DEFAULT_MOVE_SEMANTIC(FastHeapVerifier);
    ~FastHeapVerifier() = default;

    size_t VerifyAll() const;

private:
    struct ObjectCache {
        const ObjectHeader *heapObject;
        const ObjectHeader *referent;
        const size_t offset;  // NOLINT(readability-identifier-naming)
    };

    size_t CheckHeap(const PandaUnorderedSet<const ObjectHeader *> &heapObjects,
                     const PandaVector<ObjectCache> &referentObjects) const;
};

class ObjectVerificationInfo {
public:
    explicit ObjectVerificationInfo(ObjectHeader *referent);
    DEFAULT_COPY_SEMANTIC(ObjectVerificationInfo);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(ObjectVerificationInfo);
    ~ObjectVerificationInfo() = default;

    /**
     * Check that updated reference \p updated_ref from \p object has same state as before
     * collecting and refs updating
     *
     * @param object the object which contains verifying reference
     * @param updated_ref new referent position after moving
     * @param in_alive_space whether the object places in space which was not moved (region promotion, for example)
     * @return true if reference is correct and false otherwise
     */
    bool VerifyUpdatedRef(ObjectHeader *object, ObjectHeader *updatedRef, bool inAliveSpace) const;

private:
    void *classAddress_ = nullptr;
    ObjectHeader *oldAddress_ = nullptr;
};

template <class LanguageConfig>
class HeapVerifierIntoGC : public HeapVerifierBase {
public:
    explicit HeapVerifierIntoGC(HeapManager *heap) : HeapVerifierBase(heap) {}
    DEFAULT_COPY_SEMANTIC(HeapVerifierIntoGC);
    DEFAULT_MOVE_SEMANTIC(HeapVerifierIntoGC);
    ~HeapVerifierIntoGC() = default;

    void CollectVerificationInfo(PandaVector<MemRange> &&collectableMemRanges);

    size_t VerifyAll(PandaVector<MemRange> &&aliveMemRanges = PandaVector<MemRange>());

private:
    using VerifyingRefs = PandaUnorderedMap<size_t, ObjectVerificationInfo>;
    using RefsVerificationInfo = PandaUnorderedMap<ObjectHeader *, VerifyingRefs>;

    bool InCollectableSpace(const ObjectHeader *object) const;
    bool InAliveSpace(const ObjectHeader *object) const;
    void AddToVerificationInfo(RefsVerificationInfo &verificationInfo, size_t refNumber, ObjectHeader *objectHeader,
                               ObjectHeader *referent);

    RefsVerificationInfo collectableVerificationInfo_;
    RefsVerificationInfo permanentVerificationInfo_;
    PandaVector<MemRange> collectableMemRanges_;
    PandaVector<MemRange> aliveMemRanges_;
};
}  // namespace ark::mem

#endif  // PANDA_MEM_HEAP_VERIFIER_H
