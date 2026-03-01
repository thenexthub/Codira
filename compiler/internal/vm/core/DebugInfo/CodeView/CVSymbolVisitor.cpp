//===- CVSymbolVisitor.cpp --------------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/CodeView/CVSymbolVisitor.h"

#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/CodeView/SymbolRecordHelpers.h"
#include "vm/core/DebugInfo/CodeView/SymbolVisitorCallbacks.h"
#include "vm/core/Support/BinaryStreamArray.h"

using namespace vm::core;
using namespace vm::core::codeview;

CVSymbolVisitor::CVSymbolVisitor(SymbolVisitorCallbacks &Callbacks)
    : Callbacks(Callbacks) {}

template <typename T>
static Error visitKnownRecord(CVSymbol &Record,
                              SymbolVisitorCallbacks &Callbacks) {
  SymbolRecordKind RK = static_cast<SymbolRecordKind>(Record.kind());
  T KnownRecord(RK);
  if (auto EC = Callbacks.visitKnownRecord(Record, KnownRecord))
    return EC;
  return Error::success();
}

static Error finishVisitation(CVSymbol &Record,
                              SymbolVisitorCallbacks &Callbacks) {
  switch (Record.kind()) {
  default:
    if (auto EC = Callbacks.visitUnknownSymbol(Record))
      return EC;
    break;
#define SYMBOL_RECORD(EnumName, EnumVal, Name)                                 \
  case EnumName: {                                                             \
    if (auto EC = visitKnownRecord<Name>(Record, Callbacks))                   \
      return EC;                                                               \
    break;                                                                     \
  }
#define SYMBOL_RECORD_ALIAS(EnumName, EnumVal, Name, AliasName)                \
  SYMBOL_RECORD(EnumVal, EnumVal, AliasName)
#include "vm/core/DebugInfo/CodeView/CodeViewSymbols.def"
  }

  if (auto EC = Callbacks.visitSymbolEnd(Record))
    return EC;

  return Error::success();
}

Error CVSymbolVisitor::visitSymbolRecord(CVSymbol &Record) {
  if (auto EC = Callbacks.visitSymbolBegin(Record))
    return EC;
  return finishVisitation(Record, Callbacks);
}

Error CVSymbolVisitor::visitSymbolRecord(CVSymbol &Record, uint32_t Offset) {
  if (auto EC = Callbacks.visitSymbolBegin(Record, Offset))
    return EC;
  return finishVisitation(Record, Callbacks);
}

Error CVSymbolVisitor::visitSymbolStream(const CVSymbolArray &Symbols) {
  for (auto I : Symbols) {
    if (auto EC = visitSymbolRecord(I))
      return EC;
  }
  return Error::success();
}

Error CVSymbolVisitor::visitSymbolStream(const CVSymbolArray &Symbols,
                                         uint32_t InitialOffset) {
  for (auto I : Symbols) {
    if (auto EC = visitSymbolRecord(I, InitialOffset + Symbols.skew()))
      return EC;
    InitialOffset += I.length();
  }
  return Error::success();
}

Error CVSymbolVisitor::visitSymbolStreamFiltered(const CVSymbolArray &Symbols,
                                                 const FilterOptions &Filter) {
  if (!Filter.SymbolOffset)
    return visitSymbolStream(Symbols);
  uint32_t SymbolOffset = *Filter.SymbolOffset;
  uint32_t ParentRecurseDepth = Filter.ParentRecursiveDepth.value_or(0);
  uint32_t ChildrenRecurseDepth = Filter.ChildRecursiveDepth.value_or(0);
  if (!Symbols.isOffsetValid(SymbolOffset))
    return createStringError(inconvertibleErrorCode(), "Invalid symbol offset");
  CVSymbol Sym = *Symbols.at(SymbolOffset);
  uint32_t SymEndOffset =
      symbolOpensScope(Sym.kind()) ? getScopeEndOffset(Sym) : 0;

  std::vector<uint32_t> ParentOffsets;
  std::vector<uint32_t> ParentEndOffsets;
  uint32_t ChildrenDepth = 0;
  for (auto Begin = Symbols.begin(), End = Symbols.end(); Begin != End;
       ++Begin) {
    uint32_t BeginOffset = Begin.offset();
    CVSymbol BeginSym = *Begin;
    if (BeginOffset < SymbolOffset) {
      if (symbolOpensScope(Begin->kind())) {
        uint32_t EndOffset = getScopeEndOffset(BeginSym);
        if (SymbolOffset < EndOffset) {
          ParentOffsets.push_back(BeginOffset);
          ParentEndOffsets.push_back(EndOffset);
        }
      }
    } else if (BeginOffset == SymbolOffset) {
      // Found symbol at offset. Visit its parent up to ParentRecurseDepth.
      if (ParentRecurseDepth >= ParentOffsets.size())
        ParentRecurseDepth = ParentOffsets.size();
      uint32_t StartIndex = ParentOffsets.size() - ParentRecurseDepth;
      while (StartIndex < ParentOffsets.size()) {
        if (!Symbols.isOffsetValid(ParentOffsets[StartIndex]))
          break;
        CVSymbol Parent = *Symbols.at(ParentOffsets[StartIndex]);
        if (auto EC = visitSymbolRecord(Parent, ParentOffsets[StartIndex]))
          return EC;
        ++StartIndex;
      }
      if (auto EC = visitSymbolRecord(Sym, SymbolOffset))
        return EC;
    } else if (BeginOffset <= SymEndOffset) {
      if (ChildrenRecurseDepth) {
        // Visit children.
        if (symbolEndsScope(Begin->kind()))
          --ChildrenDepth;
        if (ChildrenDepth < ChildrenRecurseDepth ||
            BeginOffset == SymEndOffset) {
          if (auto EC = visitSymbolRecord(BeginSym, BeginOffset))
            return EC;
        }
        if (symbolOpensScope(Begin->kind()))
          ++ChildrenDepth;
      }
    } else {
      // Visit parents' ends.
      if (ParentRecurseDepth && BeginOffset == ParentEndOffsets.back()) {
        if (auto EC = visitSymbolRecord(BeginSym, BeginOffset))
          return EC;
        ParentEndOffsets.pop_back();
        --ParentRecurseDepth;
      }
    }
  }
  return Error::success();
}
