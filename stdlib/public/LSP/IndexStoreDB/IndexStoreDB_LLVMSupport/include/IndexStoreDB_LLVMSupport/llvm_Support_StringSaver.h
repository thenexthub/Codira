//===- toolchain/Support/StringSaver.h -------------------------------*- C++ -*-===//
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

#ifndef LLVM_SUPPORT_STRINGSAVER_H
#define LLVM_SUPPORT_STRINGSAVER_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_DenseSet.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringRef.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_Twine.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Allocator.h>

namespace toolchain {

/// Saves strings in the provided stable storage and returns a
/// StringRef with a stable character pointer.
class StringSaver final {
  BumpPtrAllocator &Alloc;

public:
  StringSaver(BumpPtrAllocator &Alloc) : Alloc(Alloc) {}

  // All returned strings are null-terminated: *save(S).end() == 0.
  StringRef save(const char *S) { return save(StringRef(S)); }
  StringRef save(StringRef S);
  StringRef save(const Twine &S) { return save(StringRef(S.str())); }
  StringRef save(const std::string &S) { return save(StringRef(S)); }
};

/// Saves strings in the provided stable storage and returns a StringRef with a
/// stable character pointer. Saving the same string yields the same StringRef.
///
/// Compared to StringSaver, it does more work but avoids saving the same string
/// multiple times.
///
/// Compared to StringPool, it performs fewer allocations but doesn't support
/// refcounting/deletion.
class UniqueStringSaver final {
  StringSaver Strings;
  toolchain::DenseSet<toolchain::StringRef> Unique;

public:
  UniqueStringSaver(BumpPtrAllocator &Alloc) : Strings(Alloc) {}

  // All returned strings are null-terminated: *save(S).end() == 0.
  StringRef save(const char *S) { return save(StringRef(S)); }
  StringRef save(StringRef S);
  StringRef save(const Twine &S) { return save(StringRef(S.str())); }
  StringRef save(const std::string &S) { return save(StringRef(S)); }
};

}
#endif
