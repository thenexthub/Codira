//===- toolchain/ADT/ilist_node_base.h - Intrusive List Node Base -----*- C++ -*-==//
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

#ifndef LLVM_ADT_ILIST_NODE_BASE_H
#define LLVM_ADT_ILIST_NODE_BASE_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_PointerIntPair.h>

namespace toolchain {

/// Base class for ilist nodes.
///
/// Optionally tracks whether this node is the sentinel.
template <bool EnableSentinelTracking> class ilist_node_base;

template <> class ilist_node_base<false> {
  ilist_node_base *Prev = nullptr;
  ilist_node_base *Next = nullptr;

public:
  void setPrev(ilist_node_base *Prev) { this->Prev = Prev; }
  void setNext(ilist_node_base *Next) { this->Next = Next; }
  ilist_node_base *getPrev() const { return Prev; }
  ilist_node_base *getNext() const { return Next; }

  bool isKnownSentinel() const { return false; }
  void initializeSentinel() {}
};

template <> class ilist_node_base<true> {
  PointerIntPair<ilist_node_base *, 1> PrevAndSentinel;
  ilist_node_base *Next = nullptr;

public:
  void setPrev(ilist_node_base *Prev) { PrevAndSentinel.setPointer(Prev); }
  void setNext(ilist_node_base *Next) { this->Next = Next; }
  ilist_node_base *getPrev() const { return PrevAndSentinel.getPointer(); }
  ilist_node_base *getNext() const { return Next; }

  bool isSentinel() const { return PrevAndSentinel.getInt(); }
  bool isKnownSentinel() const { return isSentinel(); }
  void initializeSentinel() { PrevAndSentinel.setInt(true); }
};

} // end namespace toolchain

#endif // LLVM_ADT_ILIST_NODE_BASE_H
