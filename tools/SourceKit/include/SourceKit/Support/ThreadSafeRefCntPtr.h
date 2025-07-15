//===--- ThreadSafeRefCntPtr.h - --------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_SUPPORT_THREADSAFEREFCNTPTR_H
#define TOOLCHAIN_SOURCEKIT_SUPPORT_THREADSAFEREFCNTPTR_H

#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/Support/Mutex.h"
#include <atomic>
#include <type_traits>
#include <utility>

namespace SourceKit {

class ThreadSafeRefCntPtrImpl {
protected:
  static toolchain::sys::Mutex *getMutex(void *Ptr);
};

/// This is to be used judiciously as a global or member variable that can get
/// accessed by multiple threads. Don't use it for parameters or return
/// values, use IntrusiveRefCntPtr for that.
template <typename T>
class ThreadSafeRefCntPtr : ThreadSafeRefCntPtrImpl {
  std::atomic<T*> Obj;

public:
  typedef T element_type;

  ThreadSafeRefCntPtr() : Obj(nullptr) {}
  ThreadSafeRefCntPtr(std::nullptr_t) : Obj(nullptr) {}

  template <class X,
      class = typename std::enable_if<std::is_convertible<X*, T*>::value>::type>
  explicit ThreadSafeRefCntPtr(X* obj) : Obj(obj) {
    if (obj)
      obj->Retain();
  }

  ThreadSafeRefCntPtr(const ThreadSafeRefCntPtr& S) {
    toolchain::IntrusiveRefCntPtr<T> Ref = S;
    Obj = Ref.get();
    Ref.resetWithoutRelease();
  }

  template <class X,
      class = typename std::enable_if<std::is_convertible<X*, T*>::value>::type>
  ThreadSafeRefCntPtr(const ThreadSafeRefCntPtr<X>& S) {
    toolchain::IntrusiveRefCntPtr<T> Ref = S;
    Obj = Ref.get();
    Ref.resetWithoutRelease();
  }

  ThreadSafeRefCntPtr(ThreadSafeRefCntPtr&& S) : Obj(S.get()) {
    S.Obj = nullptr;
  }

  template <class X,
      class = typename std::enable_if<std::is_convertible<X*, T*>::value>::type>
  ThreadSafeRefCntPtr(ThreadSafeRefCntPtr<X>&& S) : Obj(S.get()) {
    S.Obj = nullptr;
  }

  template <class X,
      class = typename std::enable_if<std::is_convertible<X*, T*>::value>::type>
  ThreadSafeRefCntPtr(toolchain::IntrusiveRefCntPtr<X> S) {
    Obj = S.get();
    S.resetWithoutRelease();
  }

  ThreadSafeRefCntPtr& operator=(const ThreadSafeRefCntPtr& S) {
    toolchain::IntrusiveRefCntPtr<T> Ref = S;
    swap(Ref);
    return *this;
  }

  template <class X,
      class = typename std::enable_if<std::is_convertible<X*, T*>::value>::type>
  ThreadSafeRefCntPtr& operator=(const ThreadSafeRefCntPtr<X>& S) {
    toolchain::IntrusiveRefCntPtr<T> Ref = S;
    swap(Ref);
    return *this;
  }

  ThreadSafeRefCntPtr& operator=(ThreadSafeRefCntPtr&& S) {
    toolchain::sys::ScopedLock L(*getMutex(this));
    if (T *O = Obj.load())
      O->Release();
    Obj = S.Obj;
    S.Obj = nullptr;
    return *this;
  }

  template <class X,
      class = typename std::enable_if<std::is_convertible<X*, T*>::value>::type>
  ThreadSafeRefCntPtr& operator=(ThreadSafeRefCntPtr<X>&& S) {
    toolchain::sys::ScopedLock L(*getMutex(this));
    if (T *O = Obj.load())
      O->Release();
    Obj = S.Obj;
    S.Obj = nullptr;
    return *this;
  }

  template <class X,
      class = typename std::enable_if<std::is_convertible<X*, T*>::value>::type>
  ThreadSafeRefCntPtr& operator=(toolchain::IntrusiveRefCntPtr<X> S) {
    toolchain::IntrusiveRefCntPtr<T> Ref = std::move(S);
    swap(Ref);
    return *this;
  }

  operator toolchain::IntrusiveRefCntPtr<T>() const {
    toolchain::sys::ScopedLock L(*getMutex(
        reinterpret_cast<void *>(const_cast<ThreadSafeRefCntPtr *>(this))));
    toolchain::IntrusiveRefCntPtr<T> Ref(Obj.load());
    return Ref;
  }

  ~ThreadSafeRefCntPtr() {
    if (T *O = Obj.load())
      O->Release();
  }

  T& operator*() const { return *Obj; }

  T* operator->() const { return Obj; }

  T* get() const { return Obj; }

  explicit operator bool() const { return get(); }

  void swap(toolchain::IntrusiveRefCntPtr<T> &other) {
    toolchain::sys::ScopedLock L(*getMutex(this));
    // FIXME: If ThreadSafeRefCntPtr has private access to IntrusiveRefCntPtr
    // we can eliminate the Retain/Release pair for this->Obj.
    toolchain::IntrusiveRefCntPtr<T> Ref(Obj.load());
    Ref.swap(other);
    if (T *O = Obj.load())
      O->Release();
    Obj = Ref.get();
    Ref.resetWithoutRelease();
  }

  void reset() {
    toolchain::IntrusiveRefCntPtr<T> NullRef;
    swap(NullRef);
  }

  void resetWithoutRelease() {
    toolchain::IntrusiveRefCntPtr<T> Ref;
    swap(Ref);
    Ref.resetWithoutRelease();
  }

private:
  template <typename X>
  friend class ThreadSafeRefCntPtr;
};

template<class T, class U>
inline bool operator==(const ThreadSafeRefCntPtr<T>& A,
                       const ThreadSafeRefCntPtr<U>& B)
{
  return A.get() == B.get();
}

template<class T, class U>
inline bool operator!=(const ThreadSafeRefCntPtr<T>& A,
                       const ThreadSafeRefCntPtr<U>& B)
{
  return A.get() != B.get();
}

template<class T, class U>
inline bool operator==(const ThreadSafeRefCntPtr<T>& A,
                       U* B)
{
  return A.get() == B;
}

template<class T, class U>
inline bool operator!=(const ThreadSafeRefCntPtr<T>& A,
                       U* B)
{
  return A.get() != B;
}

template<class T, class U>
inline bool operator==(T* A,
                       const ThreadSafeRefCntPtr<U>& B)
{
  return A == B.get();
}

template<class T, class U>
inline bool operator!=(T* A,
                       const ThreadSafeRefCntPtr<U>& B)
{
  return A != B.get();
}

template<class T>
inline bool operator==(const ThreadSafeRefCntPtr<T>& A, std::nullptr_t) {
  return A.get() == nullptr;
}

template<class T>
inline bool operator!=(const ThreadSafeRefCntPtr<T>& A, std::nullptr_t) {
  return A.get() != nullptr;
}


} // namespace SourceKit

#endif
