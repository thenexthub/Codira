//===- toolchain/ADT/SmallPtrSet.h - 'Normally small' pointer set ----*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This file defines the SmallPtrSet class.  See the doxygen comment for
// SmallPtrSetImplBase for more details on the algorithm used.
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_ADT_SMALLPTRSET_H
#define TOOLCHAIN_ADT_SMALLPTRSET_H

#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/type_traits.h"
#include "toolchain/Support/PointerLikeTypeTraits.h"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <utility>

inline namespace __language { inline namespace __runtime {
namespace toolchain {

/// SmallPtrSetImplBase - This is the common code shared among all the
/// SmallPtrSet<>'s, which is almost everything.  SmallPtrSet has two modes, one
/// for small and one for large sets.
///
/// Small sets use an array of pointers allocated in the SmallPtrSet object,
/// which is treated as a simple array of pointers.  When a pointer is added to
/// the set, the array is scanned to see if the element already exists, if not
/// the element is 'pushed back' onto the array.  If we run out of space in the
/// array, we grow into the 'large set' case.  SmallSet should be used when the
/// sets are often small.  In this case, no memory allocation is used, and only
/// light-weight and cache-efficient scanning is used.
///
/// Large sets use a classic exponentially-probed hash table.  Empty buckets are
/// represented with an illegal pointer value (-1) to allow null pointers to be
/// inserted.  Tombstones are represented with another illegal pointer value
/// (-2), to allow deletion.  The hash table is resized when the table is 3/4 or
/// more.  When this happens, the table is doubled in size.
///
class SmallPtrSetImplBase {
  friend class SmallPtrSetIteratorImpl;

protected:
  /// SmallArray - Points to a fixed size set of buckets, used in 'small mode'.
  const void **SmallArray;
  /// CurArray - This is the current set of buckets.  If equal to SmallArray,
  /// then the set is in 'small mode'.
  const void **CurArray;
  /// CurArraySize - The allocated size of CurArray, always a power of two.
  unsigned CurArraySize;

  /// Number of elements in CurArray that contain a value or are a tombstone.
  /// If small, all these elements are at the beginning of CurArray and the rest
  /// is uninitialized.
  unsigned NumNonEmpty;
  /// Number of tombstones in CurArray.
  unsigned NumTombstones;

  // Helpers to copy and move construct a SmallPtrSet.
  SmallPtrSetImplBase(const void **SmallStorage,
                      const SmallPtrSetImplBase &that);
  SmallPtrSetImplBase(const void **SmallStorage, unsigned SmallSize,
                      SmallPtrSetImplBase &&that);

  explicit SmallPtrSetImplBase(const void **SmallStorage, unsigned SmallSize)
      : SmallArray(SmallStorage), CurArray(SmallStorage),
        CurArraySize(SmallSize), NumNonEmpty(0), NumTombstones(0) {
    assert(SmallSize && (SmallSize & (SmallSize-1)) == 0 &&
           "Initial size must be a power of two!");
  }

  ~SmallPtrSetImplBase() {
    if (!isSmall())
      free(CurArray);
  }

public:
  using size_type = unsigned;

  SmallPtrSetImplBase &operator=(const SmallPtrSetImplBase &) = delete;

  [[nodiscard]] bool empty() const { return size() == 0; }
  size_type size() const { return NumNonEmpty - NumTombstones; }

  void clear() {
    // If the capacity of the array is huge, and the # elements used is small,
    // shrink the array.
    if (!isSmall()) {
      if (size() * 4 < CurArraySize && CurArraySize > 32)
        return shrink_and_clear();
      // Fill the array with empty markers.
      memset(CurArray, -1, CurArraySize * sizeof(void *));
    }

    NumNonEmpty = 0;
    NumTombstones = 0;
  }

protected:
  static void *getTombstoneMarker() { return reinterpret_cast<void*>(-2); }

  static void *getEmptyMarker() {
    // Note that -1 is chosen to make clear() efficiently implementable with
    // memset and because it's not a valid pointer value.
    return reinterpret_cast<void*>(-1);
  }

  const void **EndPointer() const {
    return isSmall() ? CurArray + NumNonEmpty : CurArray + CurArraySize;
  }

  /// insert_imp - This returns true if the pointer was new to the set, false if
  /// it was already in the set.  This is hidden from the client so that the
  /// derived class can check that the right type of pointer is passed in.
  std::pair<const void *const *, bool> insert_imp(const void *Ptr) {
    if (isSmall()) {
      // Check to see if it is already in the set.
      const void **LastTombstone = nullptr;
      for (const void **APtr = SmallArray, **E = SmallArray + NumNonEmpty;
           APtr != E; ++APtr) {
        const void *Value = *APtr;
        if (Value == Ptr)
          return std::make_pair(APtr, false);
        if (Value == getTombstoneMarker())
          LastTombstone = APtr;
      }

      // Did we find any tombstone marker?
      if (LastTombstone != nullptr) {
        *LastTombstone = Ptr;
        --NumTombstones;
        return std::make_pair(LastTombstone, true);
      }

      // Nope, there isn't.  If we stay small, just 'pushback' now.
      if (NumNonEmpty < CurArraySize) {
        SmallArray[NumNonEmpty++] = Ptr;
        return std::make_pair(SmallArray + (NumNonEmpty - 1), true);
      }
      // Otherwise, hit the big set case, which will call grow.
    }
    return insert_imp_big(Ptr);
  }

  /// erase_imp - If the set contains the specified pointer, remove it and
  /// return true, otherwise return false.  This is hidden from the client so
  /// that the derived class can check that the right type of pointer is passed
  /// in.
  bool erase_imp(const void * Ptr) {
    const void *const *P = find_imp(Ptr);
    if (P == EndPointer())
      return false;

    const void **Loc = const_cast<const void **>(P);
    assert(*Loc == Ptr && "broken find!");
    *Loc = getTombstoneMarker();
    NumTombstones++;
    return true;
  }

  /// Returns the raw pointer needed to construct an iterator.  If element not
  /// found, this will be EndPointer.  Otherwise, it will be a pointer to the
  /// slot which stores Ptr;
  const void *const * find_imp(const void * Ptr) const {
    if (isSmall()) {
      // Linear search for the item.
      for (const void *const *APtr = SmallArray,
                      *const *E = SmallArray + NumNonEmpty; APtr != E; ++APtr)
        if (*APtr == Ptr)
          return APtr;
      return EndPointer();
    }

    // Big set case.
    auto *Bucket = FindBucketFor(Ptr);
    if (*Bucket == Ptr)
      return Bucket;
    return EndPointer();
  }

private:
  bool isSmall() const { return CurArray == SmallArray; }

  std::pair<const void *const *, bool> insert_imp_big(const void *Ptr);

  const void * const *FindBucketFor(const void *Ptr) const;
  void shrink_and_clear();

  /// Grow - Allocate a larger backing store for the buckets and move it over.
  void Grow(unsigned NewSize);

protected:
  /// swap - Swaps the elements of two sets.
  /// Note: This method assumes that both sets have the same small size.
  void swap(SmallPtrSetImplBase &RHS);

  void CopyFrom(const SmallPtrSetImplBase &RHS);
  void MoveFrom(unsigned SmallSize, SmallPtrSetImplBase &&RHS);

private:
  /// Code shared by MoveFrom() and move constructor.
  void MoveHelper(unsigned SmallSize, SmallPtrSetImplBase &&RHS);
  /// Code shared by CopyFrom() and copy constructor.
  void CopyHelper(const SmallPtrSetImplBase &RHS);
};

/// SmallPtrSetIteratorImpl - This is the common base class shared between all
/// instances of SmallPtrSetIterator.
class SmallPtrSetIteratorImpl {
protected:
  const void *const *Bucket;
  const void *const *End;

public:
  explicit SmallPtrSetIteratorImpl(const void *const *BP, const void*const *E)
    : Bucket(BP), End(E) {
    AdvanceIfNotValid();
  }

  bool operator==(const SmallPtrSetIteratorImpl &RHS) const {
    return Bucket == RHS.Bucket;
  }
  bool operator!=(const SmallPtrSetIteratorImpl &RHS) const {
    return Bucket != RHS.Bucket;
  }

protected:
  /// AdvanceIfNotValid - If the current bucket isn't valid, advance to a bucket
  /// that is.   This is guaranteed to stop because the end() bucket is marked
  /// valid.
  void AdvanceIfNotValid() {
    assert(Bucket <= End);
    while (Bucket != End &&
           (*Bucket == SmallPtrSetImplBase::getEmptyMarker() ||
            *Bucket == SmallPtrSetImplBase::getTombstoneMarker()))
      ++Bucket;
  }
  void RetreatIfNotValid() {
    assert(Bucket >= End);
    while (Bucket != End &&
           (Bucket[-1] == SmallPtrSetImplBase::getEmptyMarker() ||
            Bucket[-1] == SmallPtrSetImplBase::getTombstoneMarker())) {
      --Bucket;
    }
  }
};

/// SmallPtrSetIterator - This implements a const_iterator for SmallPtrSet.
template <typename PtrTy>
class SmallPtrSetIterator : public SmallPtrSetIteratorImpl {
  using PtrTraits = PointerLikeTypeTraits<PtrTy>;

public:
  using value_type = PtrTy;
  using reference = PtrTy;
  using pointer = PtrTy;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  explicit SmallPtrSetIterator(const void *const *BP, const void *const *E)
      : SmallPtrSetIteratorImpl(BP, E) {}

  // Most methods are provided by the base class.

  const PtrTy operator*() const {
    assert(Bucket < End);
    return PtrTraits::getFromVoidPointer(const_cast<void*>(*Bucket));
  }

  inline SmallPtrSetIterator& operator++() {          // Preincrement
    ++Bucket;
    AdvanceIfNotValid();
    return *this;
  }

  SmallPtrSetIterator operator++(int) {        // Postincrement
    SmallPtrSetIterator tmp = *this;
    ++*this;
    return tmp;
  }
};

/// RoundUpToPowerOfTwo - This is a helper template that rounds N up to the next
/// power of two (which means N itself if N is already a power of two).
template<unsigned N>
struct RoundUpToPowerOfTwo;

/// RoundUpToPowerOfTwoH - If N is not a power of two, increase it.  This is a
/// helper template used to implement RoundUpToPowerOfTwo.
template<unsigned N, bool isPowerTwo>
struct RoundUpToPowerOfTwoH {
  enum { Val = N };
};
template<unsigned N>
struct RoundUpToPowerOfTwoH<N, false> {
  enum {
    // We could just use NextVal = N+1, but this converges faster.  N|(N-1) sets
    // the right-most zero bits to one all at once, e.g. 0b0011000 -> 0b0011111.
    Val = RoundUpToPowerOfTwo<(N|(N-1)) + 1>::Val
  };
};

template<unsigned N>
struct RoundUpToPowerOfTwo {
  enum { Val = RoundUpToPowerOfTwoH<N, (N&(N-1)) == 0>::Val };
};

/// A templated base class for \c SmallPtrSet which provides the
/// typesafe interface that is common across all small sizes.
///
/// This is particularly useful for passing around between interface boundaries
/// to avoid encoding a particular small size in the interface boundary.
template <typename PtrType>
class SmallPtrSetImpl : public SmallPtrSetImplBase {
  using ConstPtrType = typename add_const_past_pointer<PtrType>::type;
  using PtrTraits = PointerLikeTypeTraits<PtrType>;
  using ConstPtrTraits = PointerLikeTypeTraits<ConstPtrType>;

protected:
  // Forward constructors to the base.
  using SmallPtrSetImplBase::SmallPtrSetImplBase;

public:
  using iterator = SmallPtrSetIterator<PtrType>;
  using const_iterator = SmallPtrSetIterator<PtrType>;
  using key_type = ConstPtrType;
  using value_type = PtrType;

  SmallPtrSetImpl(const SmallPtrSetImpl &) = delete;

  /// Inserts Ptr if and only if there is no element in the container equal to
  /// Ptr. The bool component of the returned pair is true if and only if the
  /// insertion takes place, and the iterator component of the pair points to
  /// the element equal to Ptr.
  std::pair<iterator, bool> insert(PtrType Ptr) {
    auto p = insert_imp(PtrTraits::getAsVoidPointer(Ptr));
    return std::make_pair(makeIterator(p.first), p.second);
  }

  /// Insert the given pointer with an iterator hint that is ignored. This is
  /// identical to calling insert(Ptr), but allows SmallPtrSet to be used by
  /// std::insert_iterator and std::inserter().
  iterator insert(iterator, PtrType Ptr) {
    return insert(Ptr).first;
  }

  /// erase - If the set contains the specified pointer, remove it and return
  /// true, otherwise return false.
  bool erase(PtrType Ptr) {
    return erase_imp(PtrTraits::getAsVoidPointer(Ptr));
  }
  /// count - Return 1 if the specified pointer is in the set, 0 otherwise.
  size_type count(ConstPtrType Ptr) const {
    return find_imp(ConstPtrTraits::getAsVoidPointer(Ptr)) != EndPointer();
  }
  iterator find(ConstPtrType Ptr) const {
    return makeIterator(find_imp(ConstPtrTraits::getAsVoidPointer(Ptr)));
  }
  bool contains(ConstPtrType Ptr) const {
    return find_imp(ConstPtrTraits::getAsVoidPointer(Ptr)) != EndPointer();
  }

  template <typename IterT>
  void insert(IterT I, IterT E) {
    for (; I != E; ++I)
      insert(*I);
  }

  void insert(std::initializer_list<PtrType> IL) {
    insert(IL.begin(), IL.end());
  }

  iterator begin() const {
    return makeIterator(CurArray);
  }
  iterator end() const { return makeIterator(EndPointer()); }

private:
  /// Create an iterator that dereferences to same place as the given pointer.
  iterator makeIterator(const void *const *P) const {
    return iterator(P, EndPointer(), *this);
  }
};

/// Equality comparison for SmallPtrSet.
///
/// Iterates over elements of LHS confirming that each value from LHS is also in
/// RHS, and that no additional values are in RHS.
template <typename PtrType>
bool operator==(const SmallPtrSetImpl<PtrType> &LHS,
                const SmallPtrSetImpl<PtrType> &RHS) {
  if (LHS.size() != RHS.size())
    return false;

  for (const auto *KV : LHS)
    if (!RHS.count(KV))
      return false;

  return true;
}

/// Inequality comparison for SmallPtrSet.
///
/// Equivalent to !(LHS == RHS).
template <typename PtrType>
bool operator!=(const SmallPtrSetImpl<PtrType> &LHS,
                const SmallPtrSetImpl<PtrType> &RHS) {
  return !(LHS == RHS);
}

/// SmallPtrSet - This class implements a set which is optimized for holding
/// SmallSize or less elements.  This internally rounds up SmallSize to the next
/// power of two if it is not already a power of two.  See the comments above
/// SmallPtrSetImplBase for details of the algorithm.
template<class PtrType, unsigned SmallSize>
class SmallPtrSet : public SmallPtrSetImpl<PtrType> {
  // In small mode SmallPtrSet uses linear search for the elements, so it is
  // not a good idea to choose this value too high. You may consider using a
  // DenseSet<> instead if you expect many elements in the set.
  static_assert(SmallSize <= 32, "SmallSize should be small");

  using BaseT = SmallPtrSetImpl<PtrType>;

  // Make sure that SmallSize is a power of two, round up if not.
  enum { SmallSizePowTwo = RoundUpToPowerOfTwo<SmallSize>::Val };
  /// SmallStorage - Fixed size storage used in 'small mode'.
  const void *SmallStorage[SmallSizePowTwo];

public:
  SmallPtrSet() : BaseT(SmallStorage, SmallSizePowTwo) {}
  SmallPtrSet(const SmallPtrSet &that) : BaseT(SmallStorage, that) {}
  SmallPtrSet(SmallPtrSet &&that)
      : BaseT(SmallStorage, SmallSizePowTwo, std::move(that)) {}

  template<typename It>
  SmallPtrSet(It I, It E) : BaseT(SmallStorage, SmallSizePowTwo) {
    this->insert(I, E);
  }

  SmallPtrSet(std::initializer_list<PtrType> IL)
      : BaseT(SmallStorage, SmallSizePowTwo) {
    this->insert(IL.begin(), IL.end());
  }

  SmallPtrSet<PtrType, SmallSize> &
  operator=(const SmallPtrSet<PtrType, SmallSize> &RHS) {
    if (&RHS != this)
      this->CopyFrom(RHS);
    return *this;
  }

  SmallPtrSet<PtrType, SmallSize> &
  operator=(SmallPtrSet<PtrType, SmallSize> &&RHS) {
    if (&RHS != this)
      this->MoveFrom(SmallSizePowTwo, std::move(RHS));
    return *this;
  }

  SmallPtrSet<PtrType, SmallSize> &
  operator=(std::initializer_list<PtrType> IL) {
    this->clear();
    this->insert(IL.begin(), IL.end());
    return *this;
  }

  /// swap - Swaps the elements of two sets.
  void swap(SmallPtrSet<PtrType, SmallSize> &RHS) {
    SmallPtrSetImplBase::swap(RHS);
  }
};

} // end namespace toolchain
}} // namespace language::runtime

namespace std {

  /// Implement std::swap in terms of SmallPtrSet swap.
  template<class T, unsigned N>
  inline void swap(toolchain::SmallPtrSet<T, N> &LHS, toolchain::SmallPtrSet<T, N> &RHS) {
    LHS.swap(RHS);
  }

} // end namespace std

#endif // TOOLCHAIN_ADT_SMALLPTRSET_H
