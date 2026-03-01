//===- GCStrategy.cpp - Garbage Collector Description ---------------------===//
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
// This file implements the policy object GCStrategy which describes the
// behavior of a given garbage collector.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/GCStrategy.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/IR/BuiltinGCs.h"

using namespace vm::core;

LLVM_INSTANTIATE_REGISTRY(GCRegistry)

GCStrategy::GCStrategy() = default;

std::unique_ptr<GCStrategy> toolchain::getGCStrategy(const StringRef Name) {
  for (auto &S : GCRegistry::entries())
    if (S.getName() == Name)
      return S.instantiate();

  // We need to link all the builtin GCs when LLVM is used as a static library.
  // The linker will quite happily remove the static constructors that register
  // the builtin GCs if we don't use a function from that object. This function
  // does nothing but we need to make sure it is (or at least could be, even
  // with all optimisations enabled) called *somewhere*, and this is a good
  // place to do that: if the GC strategies are being used then this function
  // obviously can't be removed by the linker, and here it won't affect
  // performance, since there's about to be a fatal error anyway.
  toolchain::linkAllBuiltinGCs();

  if (GCRegistry::begin() == GCRegistry::end()) {
    // In normal operation, the registry should not be empty.  There should
    // be the builtin GCs if nothing else.  The most likely scenario here is
    // that we got here without running the initializers used by the Registry
    // itself and it's registration mechanism.
    report_fatal_error(
        "unsupported GC: " + Name +
        " (did you remember to link and initialize the library?)");
  } else
    report_fatal_error(Twine("unsupported GC: ") + Name);
}
