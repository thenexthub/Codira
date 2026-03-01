//===-- CFCReleaser.h -------------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCRELEASER_H
#define LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCRELEASER_H

#include <CoreFoundation/CoreFoundation.h>

#include <cassert>

// Templatized CF helper class that can own any CF pointer and will
// call CFRelease() on any valid pointer it owns unless that pointer is
// explicitly released using the release() member function. This class
// is designed to mimic the std::auto_ptr<T> class and has all of the
// same functions. The one thing to watch out for is the
// CFCReleaser<T>::release() function won't actually CFRelease any owned
// pointer, it is designed to relinquish ownership of the pointer just
// like std:auto_ptr<T>::release() does.
template <class T> class CFCReleaser {
public:
  // Constructor that takes a pointer to a CF object that is
  // to be released when this object goes out of scope
  CFCReleaser(T ptr = NULL) : _ptr(ptr) {}

  // Copy constructor
  //
  // Note that copying a CFCReleaser will not transfer
  // ownership of the contained pointer, but it will bump its
  // reference count. This is where this class differs from
  // std::auto_ptr.
  CFCReleaser(const CFCReleaser &rhs) : _ptr(rhs.get()) {
    if (get())
      ::CFRetain(get());
  }

  // The destructor will release the pointer that it contains
  // if it has a valid pointer.
  virtual ~CFCReleaser() { reset(); }

  // Assignment operator.
  //
  // Note that assigning one CFCReleaser to another will
  // not transfer ownership of the contained pointer, but it
  // will bump its reference count. This is where this class
  // differs from std::auto_ptr.
  CFCReleaser &operator=(const CFCReleaser<T> &rhs) {
    if (this != &rhs) {
      // Replace our owned pointer with the new one
      reset(rhs.get());
      // Retain the current pointer that we own
      if (get())
        ::CFRetain(get());
    }
    return *this;
  }

  // Get the address of the contained type in case it needs
  // to be passed to a function that will fill in a pointer
  // value. The function currently will assert if _ptr is not
  // NULL because the only time this method should be used is
  // if another function will modify the contents, and we
  // could leak a pointer if this is not NULL. If the
  // assertion fires, check the offending code, or call
  // reset() prior to using the "ptr_address()" member to make
  // sure any owned objects has CFRelease called on it.
  // I had to add the "enforce_null" bool here because some
  // API's require the pointer address even though they don't change it.
  T *ptr_address(bool enforce_null = true) {
    if (enforce_null)
      assert(_ptr == NULL);
    return &_ptr;
  }

  // Access the pointer itself
  T get() { return _ptr; }

  const T get() const { return _ptr; }

  // Set a new value for the pointer and CFRelease our old
  // value if we had a valid one.
  void reset(T ptr = NULL) {
    if ((_ptr != NULL) && (ptr != _ptr))
      ::CFRelease(_ptr);
    _ptr = ptr;
  }

  // Release ownership without calling CFRelease. This class
  // is designed to mimic std::auto_ptr<T>, so the release
  // method releases ownership of the contained pointer
  // and does NOT call CFRelease.
  T release() {
    T tmp = _ptr;
    _ptr = NULL;
    return tmp;
  }

private:
  T _ptr;
};

#endif // LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCRELEASER_H
