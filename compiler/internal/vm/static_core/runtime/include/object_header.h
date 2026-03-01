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
// All common ObjectHeader methods can be found here:
// - Get/Set Mark or Class word
// - Get size of the object header and an object itself
// - Get/Generate an object hash
// Methods, specific for Class word:
// - Get different object fields
// - Return object type
// - Verify object
// - Is it a subclass of not
// - Get field addr
// Methods, specific for Mark word:
// - Object locked/unlocked
// - Marked for GC or not
// - Monitor functions (get monitor, notify, notify all, wait)
// - Forwarded or not
#ifndef PANDA_RUNTIME_OBJECT_HEADER_H_
#define PANDA_RUNTIME_OBJECT_HEADER_H_

#include <atomic>
#include <ctime>

#include "runtime/mem/lock_config_helper.h"
#include "runtime/include/class_helper.h"
#include "runtime/mark_word.h"

namespace ark {

namespace object_header_traits {

constexpr const uint32_t LINEAR_X = 1103515245U;
constexpr const uint32_t LINEAR_Y = 12345U;
constexpr const uint32_t LINEAR_SEED = 987654321U;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
extern std::atomic<uint32_t> g_hashSeed;

}  // namespace object_header_traits

class BaseClass;
class Class;
class Field;
class ManagedThread;

class PANDA_PUBLIC_API ObjectHeader {
public:
    // Simple getters and setters for Class and Mark words.
    // Use it only in single thread
    inline MarkWord GetMark() const
    {
        return *(const_cast<MarkWord *>(reinterpret_cast<const MarkWord *>(&markWord_)));
    }
    inline void SetMark(volatile MarkWord markWord)
    {
        markWord_ = markWord.Value();
    }

    inline MarkWord AtomicGetMark(std::memory_order memoryOrder = std::memory_order_seq_cst) const
    {
        auto *ptr = const_cast<MarkWord *>(reinterpret_cast<const MarkWord *>(&markWord_));
        auto *atomicPtr = reinterpret_cast<std::atomic<MarkWord> *>(ptr);
        // Atomic with parameterized order reason: memory order passed as argument
        return atomicPtr->load(memoryOrder);
    }

    inline void SetClass(BaseClass *klass)
    {
        static_assert(sizeof(ClassHelper::ClassWordSize) == sizeof(ObjectPointerType));
        auto *classWordPtr = reinterpret_cast<std::atomic<ClassHelper::ClassWordSize> *>(&classWord_);
        // NOTE(ipetrov): Set class with flags allocated by external CMC GC, will be removed when allocation in 32-bit
        // space will be supported
#if defined(ARK_HYBRID) && defined(PANDA_TARGET_64)
        // Atomic with acquire order reason: data race with classWord_ with dependecies on reads after the load which
        // should become visible
        auto flags = classWordPtr->load(std::memory_order_acquire) & ~GetClassMask();
#else
        constexpr ClassHelper::ClassWordSize flags = 0U;  // NOLINT(readability-identifier-naming)
#endif
        // Atomic with release order reason: data race with classWord_ with dependecies on writes before the store which
        // should become visible acquire
        classWordPtr->store(static_cast<ClassHelper::ClassWordSize>(ToObjPtrType(klass)) | flags,
                            std::memory_order_release);
        ASSERT_PRINT(ClassAddr<BaseClass>() == klass,
                     "Stored class = " << ClassAddr<BaseClass>() << ", but passed is " << klass);
    }

    template <typename T>
    inline T *ClassAddr() const
    {
        auto ptr = const_cast<ClassHelper::ClassWordSize *>(&classWord_);
        // Atomic with acquire order reason: data race with classWord_ with dependecies on reads after the load which
        // should become visible
        return reinterpret_cast<T *>(
            reinterpret_cast<std::atomic<ClassHelper::ClassWordSize> *>(ptr)->load(std::memory_order_acquire) &
            GetClassMask());
    }

    template <typename T>
    inline T *NotAtomicClassAddr() const
    {
        return reinterpret_cast<T *>(GetClassWord());
    }

    // Generate hash value for an object.
    static inline uint32_t GenerateHashCode()
    {
        uint32_t exVal;
        uint32_t nVal;
        do {
            // Atomic with relaxed order reason: data race with hash_seed with no synchronization or ordering
            // constraints imposed on other reads or writes
            exVal = object_header_traits::g_hashSeed.load(std::memory_order_relaxed);
            nVal = exVal * object_header_traits::LINEAR_X + object_header_traits::LINEAR_Y;
        } while (!object_header_traits::g_hashSeed.compare_exchange_weak(exVal, nVal, std::memory_order_relaxed) ||
                 (exVal & MarkWord::HASH_MASK) == 0);
        return exVal & MarkWord::HASH_MASK;
    }

    // Get Hash value for an object.
    template <MTModeT MT_MODE>
    uint32_t GetHashCode();
    uint32_t GetHashCodeFromMonitor(Monitor *monitorP);

    // Size of object header
    static constexpr size_t ObjectHeaderSize()
    {
        return sizeof(ObjectHeader);
    }

    static constexpr size_t GetClassOffset()
    {
        return MEMBER_OFFSET(ObjectHeader, classWord_);
    }

    // Mask to get class word from register:
    // * 64-bit system, 64-bit class word: 0xFFFFFFFFFFFFFFFF
    // * 64-bit system, 48-bit class word: 0x0000FFFFFFFFFFFF (external CMC GC)
    // * 64-bit system, 32-bit class word: 0x00000000FFFFFFFF
    // * 32-bit system, 32-bit class word: 0xFFFFFFFF
    // * 32-bit system, 16-bit class word: 0x0000FFFF
    static constexpr size_t GetClassMask()
    {
#if defined(ARK_HYBRID) && defined(PANDA_TARGET_64)
        return static_cast<size_t>((1ULL << 48ULL) - 1ULL);
#else
        return std::numeric_limits<size_t>::max() >> (ClassHelper::POINTER_SIZE - OBJECT_POINTER_SIZE) * BITS_PER_BYTE;
#endif
    }

    static constexpr size_t GetMarkWordOffset()
    {
        return MEMBER_OFFSET(ObjectHeader, markWord_);
    }

    // Garbage collection method
    template <bool ATOMIC_FLAG = true>
    inline bool IsMarkedForGC() const
    {
        if constexpr (!ATOMIC_FLAG) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            return GetMark().IsMarkedForGC();
        }
        return AtomicGetMark().IsMarkedForGC();
    }
    template <bool ATOMIC_FLAG = true>
    inline void SetMarkedForGC()
    {
        if constexpr (!ATOMIC_FLAG) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            SetMark(GetMark().SetMarkedForGC());
            return;
        }
        bool res;
        MarkWord word = AtomicGetMark();
        do {
            res = AtomicSetMark<false>(word, word.SetMarkedForGC());
        } while (!res);
    }
    template <bool ATOMIC_FLAG = true>
    inline void SetUnMarkedForGC()
    {
        if constexpr (!ATOMIC_FLAG) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            SetMark(GetMark().SetUnMarkedForGC());
            return;
        }
        bool res;
        MarkWord word = AtomicGetMark();
        do {
            res = AtomicSetMark<false>(word, word.SetUnMarkedForGC());
        } while (!res);
    }
    inline bool IsForwarded() const
    {
        return AtomicGetMark().GetState() == MarkWord::ObjectState::STATE_GC;
    }

    // Type test methods
    inline bool IsInstance() const;

    // Get field address in Class
    inline void *FieldAddr(int offset) const;

    template <bool STRONG = true>
    bool AtomicSetMark(MarkWord &oldMarkWord, MarkWord newMarkWord,
                       std::memory_order memoryOrder = std::memory_order_seq_cst)
    {
        // This is the way to operate with casting MarkWordSize <-> MarkWord and atomics
        auto ptr = reinterpret_cast<MarkWord *>(&markWord_);
        auto atomicPtr = reinterpret_cast<std::atomic<MarkWord> *>(ptr);
        // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
        if constexpr (STRONG) {  // NOLINT(bugprone-suspicious-semicolon)
            // Atomic with parameterized order reason: memory order passed as argument
            return atomicPtr->compare_exchange_strong(oldMarkWord, newMarkWord, memoryOrder);
        }
        // CAS weak may return false results, but is more efficient, use it only in loops
        // Atomic with parameterized order reason: memory order passed as argument
        return atomicPtr->compare_exchange_weak(oldMarkWord, newMarkWord, memoryOrder);
    }

    // Accessors to typical Class types

    template <class T, bool IS_VOLATILE = false>
    T GetFieldPrimitive(size_t offset) const;

    template <class T, bool IS_VOLATILE = false>
    void SetFieldPrimitive(size_t offset, T value);

    template <bool IS_VOLATILE = false, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetFieldObject(int offset) const;

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(size_t offset, ObjectHeader *value);

    template <class T>
    T GetFieldPrimitive(const Field &field) const;

    template <class T>
    void SetFieldPrimitive(const Field &field, T value);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetFieldObject(const Field &field) const;

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(const Field &field, ObjectHeader *value);

    // Pass thread parameter to speed up interpreter
    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetFieldObject(const ManagedThread *thread, const Field &field);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(const ManagedThread *thread, const Field &field, ObjectHeader *value);

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(const ManagedThread *thread, size_t offset, ObjectHeader *value);

    template <class T>
    T GetFieldPrimitive(size_t offset, std::memory_order memoryOrder) const;

    template <class T>
    void SetFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetFieldObject(size_t offset, std::memory_order memoryOrder) const;

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder);

    template <typename T>
    bool CompareAndSetFieldPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder, bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    bool CompareAndSetFieldObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                  std::memory_order memoryOrder, bool strong);

    template <typename T>
    T CompareAndExchangeFieldPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder,
                                       bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *CompareAndExchangeFieldObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                                std::memory_order memoryOrder, bool strong);

    template <typename T>
    T GetAndSetFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetAndSetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndAddFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseOrFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseAndFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseXorFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    /*
     * Is the object is an instance of specified class.
     * Object of type O is instance of type T if O is the same as T or is subtype of T. For arrays T should be a root
     * type in type hierarchy or T is such array that O array elements are the same or subtype of T array elements.
     */
    inline bool IsInstanceOf(const Class *klass) const;

    // Verification methods
    static void Verify(ObjectHeader *objectHeader);

    static ObjectHeader *Create(BaseClass *klass);
    static ObjectHeader *Create(ManagedThread *thread, BaseClass *klass);

    static ObjectHeader *CreateNonMovable(BaseClass *klass);

    static ObjectHeader *Clone(ObjectHeader *src);

    static ObjectHeader *ShallowCopy(ObjectHeader *src);

    size_t ObjectSize() const;

    template <LangTypeT LANG>
    size_t ObjectSize(BaseClass *baseKlass) const
    {
        if constexpr (LANG == LangTypeT::LANG_TYPE_DYNAMIC) {
            return ObjectSizeDyn(baseKlass);
        } else {
            static_assert(LANG == LangTypeT::LANG_TYPE_STATIC);
            return ObjectSizeStatic(baseKlass);
        }
    }

private:
    uint32_t GetHashCodeMTSingle();
    uint32_t GetHashCodeMTMulti();
    size_t ObjectSizeDyn(BaseClass *baseKlass) const;
    size_t ObjectSizeStatic(BaseClass *baseKlass) const;

    // NOTE(ipetrov): ClassWord mask usage is temporary solution, in the future all classes will be allocated in 32-bit
    // address space
    ALWAYS_INLINE ClassHelper::ClassWordSize GetClassWord() const
    {
        return classWord_ & GetClassMask();
    }

#if defined(ARK_HYBRID) && defined(PANDA_TARGET_32)
    MarkWord::MarkWordSize markWord_;
    ClassHelper::ClassWordSize classWord_;
#else
    ClassHelper::ClassWordSize classWord_;
    MarkWord::MarkWordSize markWord_;
#endif
    /**
     * Allocates memory for the Object. No ctor is called.
     * @param klass - class of Object
     * @param non_movable - if true, object will be allocated in non-movable space
     * @return pointer to the created Object
     */
    static ObjectHeader *CreateObject(BaseClass *klass, bool nonMovable);
    static ObjectHeader *CreateObject(ManagedThread *thread, BaseClass *klass, bool nonMovable);
};

#if defined(ARK_HYBRID) && defined(PANDA_TARGET_32)
constexpr uint32_t OBJECT_HEADER_CLASS_OFFSET = 0U;
static_assert(OBJECT_HEADER_CLASS_OFFSET == ark::ObjectHeader::GetMarkWordOffset());
#else
constexpr uint32_t OBJECT_HEADER_CLASS_OFFSET = 0U;
static_assert(OBJECT_HEADER_CLASS_OFFSET == ark::ObjectHeader::GetClassOffset());
#endif

// NOLINTBEGIN(readability-identifier-naming)
template <class T>
using is_object = std::bool_constant<std::is_pointer_v<T> && std::is_base_of_v<ObjectHeader, std::remove_pointer_t<T>>>;

template <class T>
constexpr bool is_object_v = is_object<T>::value;
// NOLINTEND(readability-identifier-naming)

}  // namespace ark

#endif  // PANDA_RUNTIME_OBJECT_HEADER_H_
