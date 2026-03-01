//===- lib/TextAPI/SymbolSet.cpp - TAPI Symbol Set ------------*- C++-*----===//
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

#include "vm/core/TextAPI/SymbolSet.h"

using namespace vm::core;
using namespace vm::core::MachO;

SymbolSet::~SymbolSet() {
  for (auto &[Key, Sym] : Symbols)
    Sym->~Symbol();
}

Symbol *SymbolSet::addGlobalImpl(EncodeKind Kind, StringRef Name,
                                 SymbolFlags Flags) {
  Name = copyString(Name);
  auto Result = Symbols.try_emplace(SymbolsMapKey{Kind, Name}, nullptr);
  if (Result.second)
    Result.first->second =
        new (Allocator) Symbol{Kind, Name, TargetList(), Flags};
  return Result.first->second;
}

Symbol *SymbolSet::addGlobal(EncodeKind Kind, StringRef Name, SymbolFlags Flags,
                             const Target &Targ) {
  auto *Sym = addGlobalImpl(Kind, Name, Flags);
  Sym->addTarget(Targ);
  return Sym;
}

const Symbol *SymbolSet::findSymbol(EncodeKind Kind, StringRef Name,
                                    ObjCIFSymbolKind ObjCIF) const {
  if (auto result = Symbols.lookup({Kind, Name}))
    return result;
  if ((ObjCIF == ObjCIFSymbolKind::None) || (ObjCIF > ObjCIFSymbolKind::EHType))
    return nullptr;
  assert(ObjCIF <= ObjCIFSymbolKind::EHType &&
         "expected single ObjCIFSymbolKind enum value");
  // Non-complete ObjC Interfaces are represented as global symbols.
  if (ObjCIF == ObjCIFSymbolKind::Class)
    return Symbols.lookup(
        {EncodeKind::GlobalSymbol, (ObjC2ClassNamePrefix + Name).str()});
  if (ObjCIF == ObjCIFSymbolKind::MetaClass)
    return Symbols.lookup(
        {EncodeKind::GlobalSymbol, (ObjC2MetaClassNamePrefix + Name).str()});
  return Symbols.lookup(
      {EncodeKind::GlobalSymbol, (ObjC2EHTypePrefix + Name).str()});
}
