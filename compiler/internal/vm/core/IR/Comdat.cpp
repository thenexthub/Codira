//===- Comdat.cpp - Implement Metadata classes ----------------------------===//
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
// This file implements the Comdat class (including the C bindings).
//
//===----------------------------------------------------------------------===//

#include "vm/core-c/Comdat.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/StringMapEntry.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/IR/Comdat.h"
#include "vm/core/IR/GlobalObject.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Value.h"

using namespace vm::core;

Comdat::Comdat(Comdat &&C) : Name(C.Name), SK(C.SK) {}

Comdat::Comdat() = default;

StringRef Comdat::getName() const { return Name->first(); }

void Comdat::addUser(GlobalObject *GO) { Users.insert(GO); }

void Comdat::removeUser(GlobalObject *GO) { Users.erase(GO); }

LLVMComdatRef LLVMGetOrInsertComdat(LLVMModuleRef M, const char *Name) {
  return wrap(unwrap(M)->getOrInsertComdat(Name));
}

LLVMComdatRef LLVMGetComdat(LLVMValueRef V) {
  GlobalObject *G = unwrap<GlobalObject>(V);
  return wrap(G->getComdat());
}

void LLVMSetComdat(LLVMValueRef V, LLVMComdatRef C) {
  GlobalObject *G = unwrap<GlobalObject>(V);
  G->setComdat(unwrap(C));
}

LLVMComdatSelectionKind LLVMGetComdatSelectionKind(LLVMComdatRef C) {
  switch (unwrap(C)->getSelectionKind()) {
  case Comdat::Any:
    return LLVMAnyComdatSelectionKind;
  case Comdat::ExactMatch:
    return LLVMExactMatchComdatSelectionKind;
  case Comdat::Largest:
    return LLVMLargestComdatSelectionKind;
  case Comdat::NoDeduplicate:
    return LLVMNoDeduplicateComdatSelectionKind;
  case Comdat::SameSize:
    return LLVMSameSizeComdatSelectionKind;
  }
  llvm_unreachable("Invalid Comdat SelectionKind!");
}

void LLVMSetComdatSelectionKind(LLVMComdatRef C, LLVMComdatSelectionKind kind) {
  Comdat *Cd = unwrap(C);
  switch (kind) {
  case LLVMAnyComdatSelectionKind:
    Cd->setSelectionKind(Comdat::Any);
    break;
  case LLVMExactMatchComdatSelectionKind:
    Cd->setSelectionKind(Comdat::ExactMatch);
    break;
  case LLVMLargestComdatSelectionKind:
    Cd->setSelectionKind(Comdat::Largest);
    break;
  case LLVMNoDeduplicateComdatSelectionKind:
    Cd->setSelectionKind(Comdat::NoDeduplicate);
    break;
  case LLVMSameSizeComdatSelectionKind:
    Cd->setSelectionKind(Comdat::SameSize);
    break;
  }
}
