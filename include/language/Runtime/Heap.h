//===--- Heap.h - Codira Language Heap ABI -----------------------*- C++ -*-===//
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
// Codira Heap ABI
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_HEAP_H
#define LANGUAGE_RUNTIME_HEAP_H

#include <cstddef>
#include <limits>
#include <new>
#include <utility>

#include "language/Runtime/Config.h"
#include "language/shims/Visibility.h"

namespace language {
// Allocate plain old memory. This is the generalized entry point
// Never returns nil. The returned memory is uninitialized. 
//
// An "alignment mask" is just the alignment (a power of 2) minus 1.
LANGUAGE_EXTERN_C LANGUAGE_RETURNS_NONNULL LANGUAGE_NODISCARD LANGUAGE_RUNTIME_EXPORT_ATTRIBUTE
void *language_slowAlloc(size_t bytes, size_t alignMask);

using MallocTypeId = unsigned long long;

LANGUAGE_RETURNS_NONNULL LANGUAGE_NODISCARD
void *language_slowAllocTyped(size_t bytes, size_t alignMask, MallocTypeId typeId);

// If LANGUAGE_STDLIB_HAS_MALLOC_TYPE is defined, allocate typed memory.
// Otherwise, allocate plain memory.
LANGUAGE_RUNTIME_EXPORT
void *language_coroFrameAlloc(size_t bytes, MallocTypeId typeId);

// If the caller cannot promise to zero the object during destruction,
// then call these corresponding APIs:
LANGUAGE_RUNTIME_EXPORT
void language_slowDealloc(void *ptr, size_t bytes, size_t alignMask);

LANGUAGE_RUNTIME_EXPORT
void language_clearSensitive(void *ptr, size_t bytes);

/// Allocate and construct an instance of type \c T.
///
/// \param args The arguments to pass to the constructor for \c T.
///
/// \returns A pointer to a new, fully constructed instance of \c T. This
///   function never returns \c nullptr. The caller is responsible for
///   eventually destroying the resulting object by passing it to
///   \c language_cxx_deleteObject().
///
/// This function avoids the use of the global \c operator \c new (which may be
/// overridden by other code in a process) in favor of calling
/// \c language_slowAlloc() and constructing the new object with placement new.
///
/// This function is capable of returning well-aligned memory even on platforms
/// that do not implement the C++17 "over-aligned new" feature.
template <typename T, typename... Args>
LANGUAGE_RETURNS_NONNULL LANGUAGE_NODISCARD
static inline T *language_cxx_newObject(Args &&... args) {
  auto result = reinterpret_cast<T *>(language_slowAlloc(sizeof(T),
                                                      alignof(T) - 1));
  ::new (result) T(std::forward<Args>(args)...);
  return result;
}

/// Destruct and deallocate an instance of type \c T.
///
/// \param ptr A pointer to an instance of type \c T previously created with a
///   call to \c language_cxx_newObject().
///
/// This function avoids the use of the global \c operator \c delete (which may
/// be overridden by other code in a process) in favor of directly calling the
/// destructor for \a *ptr and then freeing its memory by calling
/// \c language_slowDealloc().
///
/// The effect of passing a pointer to this function that was \em not returned
/// from \c language_cxx_newObject() is undefined.
template <typename T>
static inline void language_cxx_deleteObject(T *ptr) {
  if (ptr) {
    ptr->~T();
    language_slowDealloc(ptr, sizeof(T), alignof(T) - 1);
  }
}

/// Define a custom operator delete; this is useful when a class has a
/// virtual destructor, as in that case the compiler will emit a deleting
/// version of the destructor, which will call ::operator delete unless the
/// class (or its superclasses) define one of their own.
#define LANGUAGE_CXX_DELETE_OPERATOR(T)                    \
  void operator delete(void *ptr) {                     \
    language_slowDealloc(ptr, sizeof(T), alignof(T) - 1);  \
  }

/// A C++ Allocator that uses the above functions instead of operator new
/// and operator delete.  This lets us use STL containers without pulling
/// in global operator new and global operator delete.
template <typename T>
struct cxx_allocator {
  // Member types
  typedef T              value_type;
  typedef T             *pointer;
  typedef const T       *const_pointer;
  typedef T             &reference;
  typedef const T       &const_reference;
  typedef std::size_t    size_type;
  typedef std::ptrdiff_t difference_type;
  typedef std::true_type propagate_on_container_move_assignment;
  typedef std::true_type is_always_equal;

  template <typename U>
  struct rebind {
    typedef cxx_allocator<U> other;
  };

  cxx_allocator() noexcept {}
  cxx_allocator(const cxx_allocator &other) noexcept { (void)other; }

  template <class U>
  cxx_allocator(const cxx_allocator<U> &other) noexcept { (void)other; }

  ~cxx_allocator() {}

  pointer address(reference x) const noexcept {
    return reinterpret_cast<pointer>(&reinterpret_cast<volatile char &>(x));
  }

  const_pointer address(const_reference x) const noexcept {
    return reinterpret_cast<const_pointer>(&
      reinterpret_cast<const volatile char &>(x));
  }

  T *allocate(std::size_t n) {
    return reinterpret_cast<T *>(language_slowAlloc(sizeof(T) * n,
                                                 alignof(T) - 1));
  }
  T *allocate(std::size_t n, const void *hint) {
    (void)hint;
    return allocate(n);
  }

  void deallocate(T *p, std::size_t n) {
    language_slowDealloc(p, sizeof(T) * n, alignof(T) - 1);
  }

  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  template <class U, class... Args>
  void construct(U *p, Args&&... args) {
    ::new((void *)p) U(std::forward<Args>(args)...);
  }

  template <class U>
  void destroy(U *p) {
    p->~U();
  }
};

template <typename T, typename U>
bool operator==(const cxx_allocator<T> &lhs,
                const cxx_allocator<U> &rhs) noexcept {
  return true;
}

template <typename T, typename U>
bool operator!=(const cxx_allocator<T> &lha,
                const cxx_allocator<U> &rhs) noexcept {
  return false;
}

}

#endif // LANGUAGE_RUNTIME_HEAP_H
