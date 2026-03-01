//===- CommonLinkerContext.h ------------------------------------*- C++ -*-===//
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
// Entry point for all global state in lldCommon. The objective is for LLD to be
// used "as a library" in a thread-safe manner.
//
// Instead of program-wide globals or function-local statics, we prefer
// aggregating all "global" states into a heap-based structure
// (CommonLinkerContext). This also achieves deterministic initialization &
// shutdown for all "global" states.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_COMMON_COMMONLINKINGCONTEXT_H
#define LLD_COMMON_COMMONLINKINGCONTEXT_H

#include "lld/Common/ErrorHandler.h"
#include "lld/Common/Memory.h"
#include "llvm/Support/StringSaver.h"

namespace llvm {
class raw_ostream;
} // namespace llvm

namespace lld {
struct SpecificAllocBase;
class CommonLinkerContext {
public:
  CommonLinkerContext();
  virtual ~CommonLinkerContext();

  static void destroy();

  llvm::BumpPtrAllocator bAlloc;
  llvm::StringSaver saver{bAlloc};
  llvm::UniqueStringSaver uniqueSaver{bAlloc};
  llvm::DenseMap<void *, SpecificAllocBase *> instances;

  ErrorHandler e;
};

// Retrieve the global state. Currently only one state can exist per process,
// but in the future we plan on supporting an arbitrary number of LLD instances
// in a single process.
CommonLinkerContext &commonContext();

template <typename T = CommonLinkerContext> T &context() {
  return static_cast<T &>(commonContext());
}

bool hasContext();

inline llvm::BumpPtrAllocator &bAlloc() { return context().bAlloc; }
inline llvm::StringSaver &saver() { return context().saver; }
inline llvm::UniqueStringSaver &uniqueSaver() { return context().uniqueSaver; }
} // namespace lld

#endif
