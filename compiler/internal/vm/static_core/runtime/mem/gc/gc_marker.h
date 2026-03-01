/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_GC_MARKER_H
#define PANDA_RUNTIME_MEM_GC_GC_MARKER_H

#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/bitmap.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/include/object_header.h"
#include "runtime/include/language_config.h"

namespace ark::coretypes {
class DynClass;
}  // namespace ark::coretypes

namespace ark::mem {

class GCMarkerBase {
public:
    explicit GCMarkerBase(GC *gc) : gc_(gc) {}

    GC *GetGC() const
    {
        return gc_;
    }

private:
    GC *gc_;
};

template <typename Marker, LangTypeT LANG_TYPE>
class GCMarker;

template <typename Marker>
class GCMarker<Marker, LANG_TYPE_STATIC> : public GCMarkerBase {
public:
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

    explicit GCMarker(GC *gc) : GCMarkerBase(gc) {}

    void MarkInstance(GCMarkingStackType *objectsStack, const ObjectHeader *object, const BaseClass *cls,
                      const ReferenceCheckPredicateT &refPred);

    void MarkInstance(GCMarkingStackType *objectsStack, const ObjectHeader *object, const BaseClass *cls);

private:
    Marker *AsMarker()
    {
        return static_cast<Marker *>(this);
    }

    /**
     * Iterate over all fields with references of object and add all not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param object
     * @param base_cls - class of object(used for perf in case if class for the object already was obtained)
     */
    void HandleObject(GCMarkingStackType *objectsStack, const ObjectHeader *object, const Class *cls);

    /**
     * Iterate over class data and add all found not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param cls - class
     */
    void HandleClass(GCMarkingStackType *objectsStack, const Class *cls);

    /**
     * For arrays of objects add all not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param array_object - array object
     * @param cls - class of array object(used for perf)
     */
    void HandleArrayClass(GCMarkingStackType *objectsStack, const coretypes::Array *arrayObject, const Class *cls);
};

template <typename Marker>
class GCMarker<Marker, LANG_TYPE_DYNAMIC> : public GCMarkerBase {
public:
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

    explicit GCMarker(GC *gc) : GCMarkerBase(gc) {}

    void MarkInstance(GCMarkingStackType *objectsStack, const ObjectHeader *object, const BaseClass *cls,
                      const ReferenceCheckPredicateT &refPred);

    void MarkInstance(GCMarkingStackType *objectsStack, const ObjectHeader *object, const BaseClass *cls);

private:
    Marker *AsMarker()
    {
        return static_cast<Marker *>(this);
    }

    /**
     * Iterate over all fields with references of object and add all not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param object
     * @param base_cls - class of object(used for perf in case if class for the object already was obtained)
     */
    void HandleObject(GCMarkingStackType *objectsStack, const ObjectHeader *object, const BaseClass *cls);

    /**
     * Iterate over class data and add all found not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param cls - class
     */
    void HandleClass(GCMarkingStackType *objectsStack, const coretypes::DynClass *cls);

    /**
     * For arrays of objects add all not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param array_object - array object
     * @param cls - class of array object(used for perf)
     */
    void HandleArrayClass(GCMarkingStackType *objectsStack, const coretypes::Array *arrayObject, const BaseClass *cls);
};

template <typename Marker, class LanguageConfig>
class DefaultGCMarker : public GCMarker<Marker, LanguageConfig::LANG_TYPE> {
    using Base = GCMarker<Marker, LanguageConfig::LANG_TYPE>;

public:
    explicit DefaultGCMarker(GC *gc) : GCMarker<Marker, LanguageConfig::LANG_TYPE>(gc) {}

    template <bool REVERSED_MARK = false>
    bool MarkIfNotMarked(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = GetMarkBitMap(object);
        if (bitmap != nullptr) {
            if (atomicMarkFlag_) {
                return !bitmap->AtomicTestAndSet(object);
            }
            if (bitmap->Test(object)) {
                return false;
            }
            bitmap->Set(object);
            return true;
        }
        if (atomicMarkFlag_) {
            if (IsObjectHeaderMarked<REVERSED_MARK, true>(object)) {
                return false;
            }
            MarkObjectHeader<REVERSED_MARK, true>(object);
        } else {
            if (IsObjectHeaderMarked<REVERSED_MARK, false>(object)) {
                return false;
            }
            MarkObjectHeader<REVERSED_MARK, false>(object);
        }
        return true;
    }

    template <bool REVERSED_MARK = false>
    void Mark(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = GetMarkBitMap(object);
        if (bitmap != nullptr) {
            if (atomicMarkFlag_) {
                bitmap->AtomicTestAndSet(object);
            } else {
                bitmap->Set(object);
            }
            return;
        }
        if constexpr (REVERSED_MARK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            if (atomicMarkFlag_) {
                object->SetUnMarkedForGC<true>();
            } else {
                object->SetUnMarkedForGC<false>();
            }
            return;
        }
        if (atomicMarkFlag_) {
            object->SetMarkedForGC<true>();
        } else {
            object->SetMarkedForGC<false>();
        }
    }

    template <bool REVERSED_MARK = false>
    void UnMark(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = GetMarkBitMap(object);
        if (bitmap != nullptr) {
            return;  // no need for bitmap
        }
        if constexpr (REVERSED_MARK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            if (atomicMarkFlag_) {
                object->SetMarkedForGC<true>();
            } else {
                object->SetMarkedForGC<false>();
            }
            return;
        }
        if (atomicMarkFlag_) {
            object->SetUnMarkedForGC<true>();
        } else {
            object->SetUnMarkedForGC<false>();
        }
    }

    template <bool REVERSED_MARK = false>
    bool IsMarked(const ObjectHeader *object) const
    {
        MarkBitmap *bitmap = GetMarkBitMap(object);
        if (bitmap != nullptr) {
            if (atomicMarkFlag_) {
                return bitmap->AtomicTest(object);
            }
            return bitmap->Test(object);
        }
        bool isMarked = atomicMarkFlag_ ? object->IsMarkedForGC<true>() : object->IsMarkedForGC<false>();
        if constexpr (REVERSED_MARK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            return !isMarked;
        }
        return isMarked;
    }

    template <bool REVERSED_MARK = false>
    ObjectStatus MarkChecker(const ObjectHeader *object) const
    {
        if constexpr (!REVERSED_MARK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            // If ClassAddr is not set - it means object header initialization is in progress now
            if (object->ClassAddr<BaseClass>() == nullptr) {
                return ObjectStatus::ALIVE_OBJECT;
            }
        }
        ObjectStatus objectStatus =
            IsMarked<REVERSED_MARK>(object) ? ObjectStatus::ALIVE_OBJECT : ObjectStatus::DEAD_OBJECT;
        LOG(DEBUG, GC) << " Mark check for " << std::hex << object << std::dec
                       << " object is alive: " << static_cast<bool>(objectStatus);
        return objectStatus;
    }

    MarkBitmap *GetMarkBitMap(const void *object) const
    {
        for (auto bitmap : markBitmaps_) {
            if (bitmap->IsAddrInRange(object)) {
                return bitmap;
            }
        }
        return nullptr;
    }

    void ClearMarkBitMaps()
    {
        markBitmaps_.clear();
    }

    void AddMarkBitMap(MarkBitmap *bitmap)
    {
        markBitmaps_.push_back(bitmap);
    }

    template <typename It>
    void AddMarkBitMaps(It start, It end)
    {
        markBitmaps_.insert(markBitmaps_.end(), start, end);
    }

    bool HasBitMap(MarkBitmap *bitmap)
    {
        return std::find(markBitmaps_.begin(), markBitmaps_.end(), bitmap) != markBitmaps_.end();
    }

    void SetAtomicMark(bool flag)
    {
        atomicMarkFlag_ = flag;
    }

    bool GetAtomicMark() const
    {
        return atomicMarkFlag_;
    }

    void BindBitmaps(bool clearPygoteSpaceBitmaps)
    {
        // Set marking bitmaps
        ClearMarkBitMaps();
        auto pygoteSpaceAllocator = Base::GetGC()->GetObjectAllocator()->GetPygoteSpaceAllocator();
        if (pygoteSpaceAllocator != nullptr) {
            // clear live bitmaps if we decide to rebuild it in full gc,
            // it will be used as marked bitmaps and updated at end of gc
            if (clearPygoteSpaceBitmaps) {
                pygoteSpaceAllocator->ClearLiveBitmaps();
            }
            auto &bitmaps = pygoteSpaceAllocator->GetLiveBitmaps();
            AddMarkBitMaps(bitmaps.begin(), bitmaps.end());
        }
    }

private:
    template <bool REVERSED_MARK = false, bool ATOMIC_MARK = true>
    bool IsObjectHeaderMarked(const ObjectHeader *object) const
    {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (REVERSED_MARK) {  // NOLINT(bugprone-suspicious-semicolon)
            return !object->IsMarkedForGC<ATOMIC_MARK>();
        }
        return object->IsMarkedForGC<ATOMIC_MARK>();
    }

    template <bool REVERSED_MARK = false, bool ATOMIC_MARK = true>
    void MarkObjectHeader(ObjectHeader *object) const
    {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (REVERSED_MARK) {  // NOLINT(bugprone-suspicious-semicolon)
            object->SetUnMarkedForGC<ATOMIC_MARK>();
            return;
        }
        object->SetMarkedForGC<ATOMIC_MARK>();
    }

private:
    // Bitmaps for mark object
    PandaVector<MarkBitmap *> markBitmaps_;
    bool atomicMarkFlag_ = true;
};

template <class Marker>
class NoAtomicGCMarkerScope {
public:
    explicit NoAtomicGCMarkerScope(Marker *marker)
    {
        ASSERT(marker != nullptr);
        gcMarker_ = marker;
        oldState_ = gcMarker_->GetAtomicMark();
        if (oldState_) {
            gcMarker_->SetAtomicMark(false);
        }
    }

    NO_COPY_SEMANTIC(NoAtomicGCMarkerScope);
    NO_MOVE_SEMANTIC(NoAtomicGCMarkerScope);

    ~NoAtomicGCMarkerScope()
    {
        if (oldState_) {
            gcMarker_->SetAtomicMark(oldState_);
        }
    }

private:
    Marker *gcMarker_;
    bool oldState_ = false;
};

template <class LanguageConfig>
class DefaultGCMarkerImpl : public DefaultGCMarker<DefaultGCMarkerImpl<LanguageConfig>, LanguageConfig> {
    using Base = DefaultGCMarker<DefaultGCMarkerImpl<LanguageConfig>, LanguageConfig>;

public:
    explicit DefaultGCMarkerImpl(GC *gc) : Base(gc) {}
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_MARKER_H
