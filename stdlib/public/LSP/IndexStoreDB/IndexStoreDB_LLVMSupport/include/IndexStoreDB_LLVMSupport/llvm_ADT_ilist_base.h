//===- toolchain/ADT/ilist_base.h - Intrusive List Base --------------*- C++ -*-===//
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

#ifndef LLVM_ADT_ILIST_BASE_H
#define LLVM_ADT_ILIST_BASE_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_ilist_node_base.h>
#include <cassert>

namespace toolchain {

/// Implementations of list algorithms using ilist_node_base.
template <bool EnableSentinelTracking> class ilist_base {
public:
  using node_base_type = ilist_node_base<EnableSentinelTracking>;

  static void insertBeforeImpl(node_base_type &Next, node_base_type &N) {
    node_base_type &Prev = *Next.getPrev();
    N.setNext(&Next);
    N.setPrev(&Prev);
    Prev.setNext(&N);
    Next.setPrev(&N);
  }

  static void removeImpl(node_base_type &N) {
    node_base_type *Prev = N.getPrev();
    node_base_type *Next = N.getNext();
    Next->setPrev(Prev);
    Prev->setNext(Next);

    // Not strictly necessary, but helps catch a class of bugs.
    N.setPrev(nullptr);
    N.setNext(nullptr);
  }

  static void removeRangeImpl(node_base_type &First, node_base_type &Last) {
    node_base_type *Prev = First.getPrev();
    node_base_type *Final = Last.getPrev();
    Last.setPrev(Prev);
    Prev->setNext(&Last);

    // Not strictly necessary, but helps catch a class of bugs.
    First.setPrev(nullptr);
    Final->setNext(nullptr);
  }

  static void transferBeforeImpl(node_base_type &Next, node_base_type &First,
                                 node_base_type &Last) {
    if (&Next == &Last || &First == &Last)
      return;

    // Position cannot be contained in the range to be transferred.
    assert(&Next != &First &&
           // Check for the most common mistake.
           "Insertion point can't be one of the transferred nodes");

    node_base_type &Final = *Last.getPrev();

    // Detach from old list/position.
    First.getPrev()->setNext(&Last);
    Last.setPrev(First.getPrev());

    // Splice [First, Final] into its new list/position.
    node_base_type &Prev = *Next.getPrev();
    Final.setNext(&Next);
    First.setPrev(&Prev);
    Prev.setNext(&First);
    Next.setPrev(&Final);
  }

  template <class T> static void insertBefore(T &Next, T &N) {
    insertBeforeImpl(Next, N);
  }

  template <class T> static void remove(T &N) { removeImpl(N); }
  template <class T> static void removeRange(T &First, T &Last) {
    removeRangeImpl(First, Last);
  }

  template <class T> static void transferBefore(T &Next, T &First, T &Last) {
    transferBeforeImpl(Next, First, Last);
  }
};

} // end namespace toolchain

#endif // LLVM_ADT_ILIST_BASE_H
