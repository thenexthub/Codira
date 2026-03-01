//===- MetadataImpl.h - Helpers for implementing metadata -----------------===//
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
//
// This file has private helpers for implementing metadata types.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_METADATAIMPL_H
#define LLVM_IR_METADATAIMPL_H

#include "vm/core/ADT/DenseSet.h"
#include "vm/core/IR/Metadata.h"

namespace vm::core {

template <class T, class InfoT>
static T *getUniqued(DenseSet<T *, InfoT> &Store,
                     const typename InfoT::KeyTy &Key) {
  auto I = Store.find_as(Key);
  return I == Store.end() ? nullptr : *I;
}

template <class T> T *MDNode::storeImpl(T *N, StorageType Storage) {
  switch (Storage) {
  case Uniqued:
    llvm_unreachable("Cannot unique without a uniquing-store");
  case Distinct:
    N->storeDistinctInContext();
    break;
  case Temporary:
    break;
  }
  return N;
}

template <class T, class StoreT>
T *MDNode::storeImpl(T *N, StorageType Storage, StoreT &Store) {
  switch (Storage) {
  case Uniqued:
    Store.insert(N);
    break;
  case Distinct:
    N->storeDistinctInContext();
    break;
  case Temporary:
    break;
  }
  return N;
}

} // end namespace vm::core

#endif
