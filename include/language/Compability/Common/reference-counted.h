//===-- language/Compability/Common/reference-counted.h ----------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LANGUAGE_COMPABILITY_COMMON_REFERENCE_COUNTED_H_
#define LANGUAGE_COMPABILITY_COMMON_REFERENCE_COUNTED_H_

// A class template of smart pointers to objects with their own
// reference counting object lifetimes that's lighter weight
// than std::shared_ptr<>.  Not thread-safe.

namespace language::Compability::common {

// A base class for reference-counted objects.  Must be public.
template <typename A> class ReferenceCounted {
public:
  ReferenceCounted() {}
  int references() const { return references_; }
  void TakeReference() { ++references_; }
  void DropReference() {
    if (--references_ == 0) {
      delete static_cast<A *>(this);
    }
  }

private:
  int references_{0};
};

// A reference to a reference-counted object.
template <typename A> class CountedReference {
public:
  using type = A;
  CountedReference() {}
  CountedReference(type *m) : p_{m} { Take(); }
  CountedReference(const CountedReference &c) : p_{c.p_} { Take(); }
  CountedReference(CountedReference &&c) : p_{c.p_} { c.p_ = nullptr; }
  CountedReference &operator=(const CountedReference &c) {
    c.Take();
    Drop();
    p_ = c.p_;
    return *this;
  }
  CountedReference &operator=(CountedReference &&c) {
    A *p{c.p_};
    c.p_ = nullptr;
    Drop();
    p_ = p;
    return *this;
  }
  ~CountedReference() { Drop(); }
  operator bool() const { return p_ != nullptr; }
  type *get() const { return p_; }
  type &operator*() const { return *p_; }
  type *operator->() const { return p_; }

private:
  void Take() const {
    if (p_) {
      p_->TakeReference();
    }
  }
  void Drop() {
    if (p_) {
      p_->DropReference();
      p_ = nullptr;
    }
  }

  type *p_{nullptr};
};
} // namespace language::Compability::common
#endif // FORTRAN_COMMON_REFERENCE_COUNTED_H_
