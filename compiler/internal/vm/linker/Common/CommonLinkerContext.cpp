//===- CommonLinkerContext.cpp --------------------------------------------===//
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

#include "lld/Common/CommonLinkerContext.h"
#include "lld/Common/ErrorHandler.h"
#include "lld/Common/Memory.h"

#include "llvm/CodeGen/CommandFlags.h"

using namespace llvm;
using namespace lld;

// Reference to the current LLD instance. This is a temporary situation, until
// we pass this context everywhere by reference, or we make it a thread_local,
// as in https://reviews.llvm.org/D108850?id=370678 where each thread can be
// associated with a LLD instance. Only then will LLD be free of global
// state.
static CommonLinkerContext *lctx;

CommonLinkerContext::CommonLinkerContext() {
  lctx = this;
  // Fire off the static initializations in CGF's constructor.
  codegen::RegisterCodeGenFlags CGF;
}

CommonLinkerContext::~CommonLinkerContext() {
  assert(lctx);
  // Explicitly call the destructors since we created the objects with placement
  // new in SpecificAlloc::create().
  for (auto &it : instances)
    it.second->~SpecificAllocBase();
  lctx = nullptr;
}

CommonLinkerContext &lld::commonContext() {
  assert(lctx);
  return *lctx;
}

bool lld::hasContext() { return lctx != nullptr; }

void CommonLinkerContext::destroy() {
  if (lctx == nullptr)
    return;
  delete lctx;
}
