//===----------- Mangling.cpp -- Name Mangling Utilities for ORC ----------===//
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

#include "vm/core/ExecutionEngine/Orc/Mangling.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Mangler.h"

#define DEBUG_TYPE "orc"

namespace vm::core {
namespace orc {

MangleAndInterner::MangleAndInterner(ExecutionSession &ES, const DataLayout &DL)
    : ES(ES), DL(DL) {}

SymbolStringPtr MangleAndInterner::operator()(StringRef Name) {
  std::string MangledName;
  {
    raw_string_ostream MangledNameStream(MangledName);
    Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
  }
  return ES.intern(MangledName);
}

void IRSymbolMapper::add(ExecutionSession &ES, const ManglingOptions &MO,
                         ArrayRef<GlobalValue *> GVs,
                         SymbolFlagsMap &SymbolFlags,
                         SymbolNameToDefinitionMap *SymbolToDefinition) {
  if (GVs.empty())
    return;

  MangleAndInterner Mangle(ES, GVs[0]->getDataLayout());
  for (auto *G : GVs) {
    assert(G && "GVs cannot contain null elements");
    if (!G->hasName() || G->isDeclaration() || G->hasLocalLinkage() ||
        G->hasAvailableExternallyLinkage() || G->hasAppendingLinkage())
      continue;

    if (G->isThreadLocal() && MO.EmulatedTLS) {
      auto *GV = cast<GlobalVariable>(G);

      auto Flags = JITSymbolFlags::fromGlobalValue(*GV);

      auto EmuTLSV = Mangle(("__emutls_v." + GV->getName()).str());
      SymbolFlags[EmuTLSV] = Flags;
      if (SymbolToDefinition)
        (*SymbolToDefinition)[EmuTLSV] = GV;

      // If this GV has a non-zero initializer we'll need to emit an
      // __emutls.t symbol too.
      if (GV->hasInitializer()) {
        const auto *InitVal = GV->getInitializer();

        // Skip zero-initializers.
        if (isa<ConstantAggregateZero>(InitVal))
          continue;
        const auto *InitIntValue = dyn_cast<ConstantInt>(InitVal);
        if (InitIntValue && InitIntValue->isZero())
          continue;

        auto EmuTLST = Mangle(("__emutls_t." + GV->getName()).str());
        SymbolFlags[EmuTLST] = Flags;
        if (SymbolToDefinition)
          (*SymbolToDefinition)[EmuTLST] = GV;
      }
      continue;
    }

    // Otherwise we just need a normal linker mangling.
    auto MangledName = Mangle(G->getName());
    SymbolFlags[MangledName] = JITSymbolFlags::fromGlobalValue(*G);
    if (SymbolToDefinition)
      (*SymbolToDefinition)[MangledName] = G;
  }
}

} // End namespace orc.
} // End namespace vm::core.
