//===-- Iterable.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_ITERABLE_H
#define LLDB_UTILITY_ITERABLE_H

#include <utility>

#include <llvm/ADT/iterator.h>

namespace lldb_private {

template <typename WrappedIteratorT,
          typename T = typename std::iterator_traits<
              WrappedIteratorT>::value_type::second_type>
struct ValueMapIterator
    : llvm::iterator_adaptor_base<
          ValueMapIterator<WrappedIteratorT, T>, WrappedIteratorT,
          typename std::iterator_traits<WrappedIteratorT>::iterator_category,
          T> {
  ValueMapIterator() = default;
  explicit ValueMapIterator(WrappedIteratorT u)
      : ValueMapIterator::iterator_adaptor_base(std::move(u)) {}

  const T &operator*() { return (*this->I).second; }
  const T &operator*() const { return (*this->I).second; }
};

template <typename MutexType, typename C,
          typename IteratorT = typename C::const_iterator>
class LockingAdaptedIterable : public llvm::iterator_range<IteratorT> {
public:
  LockingAdaptedIterable(const C &container, MutexType &mutex)
      : llvm::iterator_range<IteratorT>(container), m_mutex(&mutex) {
    m_mutex->lock();
  }

  LockingAdaptedIterable(LockingAdaptedIterable &&rhs)
      : llvm::iterator_range<IteratorT>(rhs), m_mutex(rhs.m_mutex) {
    rhs.m_mutex = nullptr;
  }

  ~LockingAdaptedIterable() {
    if (m_mutex)
      m_mutex->unlock();
  }

private:
  MutexType *m_mutex = nullptr;

  LockingAdaptedIterable(const LockingAdaptedIterable &) = delete;
  LockingAdaptedIterable &operator=(const LockingAdaptedIterable &) = delete;
};

} // namespace lldb_private

#endif // LLDB_UTILITY_ITERABLE_H
