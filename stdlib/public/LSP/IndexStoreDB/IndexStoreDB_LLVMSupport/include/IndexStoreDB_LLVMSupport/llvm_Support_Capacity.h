//===--- Capacity.h - Generic computation of ADT memory use -----*- C++ -*-===//
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
//
// This file defines the capacity function that computes the amount of
// memory used by an ADT.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_CAPACITY_H
#define LLVM_SUPPORT_CAPACITY_H

#include <IndexStoreDB_LLVMSupport/toolchain_Config_indexstoredb-prefix.h>

#include <cstddef>

namespace toolchain {

template <typename T>
static inline size_t capacity_in_bytes(const T &x) {
  // This default definition of capacity should work for things like std::vector
  // and friends.  More specialized versions will work for others.
  return x.capacity() * sizeof(typename T::value_type);
}

} // end namespace toolchain

#endif

