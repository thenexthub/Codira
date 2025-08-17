//===- StringSet.h - The LLVM Compiler Driver -------------------*- C++ -*-===//
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
//  StringSet - A set-like wrapper for the StringMap.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ADT_STRINGSET_H
#define LLVM_ADT_STRINGSET_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringMap.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringRef.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Allocator.h>
#include <cassert>
#include <initializer_list>
#include <utility>

namespace toolchain {

  /// StringSet - A wrapper for StringMap that provides set-like functionality.
  template <class AllocatorTy = MallocAllocator>
  class StringSet : public StringMap<char, AllocatorTy> {
    using base = StringMap<char, AllocatorTy>;

  public:
    StringSet() = default;
    StringSet(std::initializer_list<StringRef> S) {
      for (StringRef X : S)
        insert(X);
    }
    explicit StringSet(AllocatorTy A) : base(A) {}

    std::pair<typename base::iterator, bool> insert(StringRef Key) {
      assert(!Key.empty());
      return base::insert(std::make_pair(Key, '\0'));
    }

    template <typename InputIt>
    void insert(const InputIt &Begin, const InputIt &End) {
      for (auto It = Begin; It != End; ++It)
        base::insert(std::make_pair(*It, '\0'));
    }
  };

} // end namespace toolchain

#endif // LLVM_ADT_STRINGSET_H
