//===--------- SectCreate.cpp - Emulate ld64's -sectcreate option ---------===//
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

#include "vm/core/ExecutionEngine/Orc/SectCreate.h"

#define DEBUG_TYPE "orc"

using namespace vm::core::jitlink;

namespace vm::core::orc {

void SectCreateMaterializationUnit::materialize(
    std::unique_ptr<MaterializationResponsibility> R) {
  auto G = std::make_unique<LinkGraph>(
      "orc_sectcreate_" + SectName,
      ObjLinkingLayer.getExecutionSession().getSymbolStringPool(),
      ObjLinkingLayer.getExecutionSession().getTargetTriple(),
      SubtargetFeatures(), getGenericEdgeKindName);

  auto &Sect = G->createSection(SectName, MP);
  auto Content = G->allocateContent(ArrayRef<char>(Data->getBuffer()));
  auto &B = G->createContentBlock(Sect, Content, ExecutorAddr(), Alignment, 0);

  for (auto &[Name, Info] : ExtraSymbols) {
    auto L = Info.Flags.isStrong() ? Linkage::Strong : Linkage::Weak;
    auto S = Info.Flags.isExported() ? Scope::Default : Scope::Hidden;
    G->addDefinedSymbol(B, Info.Offset, *Name, 0, L, S, Info.Flags.isCallable(),
                        true);
  }

  ObjLinkingLayer.emit(std::move(R), std::move(G));
}

void SectCreateMaterializationUnit::discard(const JITDylib &JD,
                                            const SymbolStringPtr &Name) {
  ExtraSymbols.erase(Name);
}

MaterializationUnit::Interface SectCreateMaterializationUnit::getInterface(
    const ExtraSymbolsMap &ExtraSymbols) {
  SymbolFlagsMap SymbolFlags;
  for (auto &[Name, Info] : ExtraSymbols)
    SymbolFlags[Name] = Info.Flags;
  return {std::move(SymbolFlags), nullptr};
}

} // End namespace vm::core::orc.
