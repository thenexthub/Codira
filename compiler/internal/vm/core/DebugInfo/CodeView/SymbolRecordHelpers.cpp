//===- SymbolRecordHelpers.cpp ----------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/CodeView/SymbolRecordHelpers.h"

#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/DebugInfo/CodeView/SymbolDeserializer.h"

using namespace vm::core;
using namespace vm::core::codeview;

template <typename RecordT> static RecordT createRecord(const CVSymbol &sym) {
  RecordT record(static_cast<SymbolRecordKind>(sym.kind()));
  cantFail(SymbolDeserializer::deserializeAs<RecordT>(sym, record));
  return record;
}

uint32_t toolchain::codeview::getScopeEndOffset(const CVSymbol &Sym) {
  assert(symbolOpensScope(Sym.kind()));
  switch (Sym.kind()) {
  case SymbolKind::S_GPROC32:
  case SymbolKind::S_LPROC32:
  case SymbolKind::S_GPROC32_ID:
  case SymbolKind::S_LPROC32_ID:
  case SymbolKind::S_LPROC32_DPC:
  case SymbolKind::S_LPROC32_DPC_ID: {
    ProcSym Proc = createRecord<ProcSym>(Sym);
    return Proc.End;
  }
  case SymbolKind::S_BLOCK32: {
    BlockSym Block = createRecord<BlockSym>(Sym);
    return Block.End;
  }
  case SymbolKind::S_THUNK32: {
    Thunk32Sym Thunk = createRecord<Thunk32Sym>(Sym);
    return Thunk.End;
  }
  case SymbolKind::S_INLINESITE: {
    InlineSiteSym Site = createRecord<InlineSiteSym>(Sym);
    return Site.End;
  }
  default:
    assert(false && "Unknown record type");
    return 0;
  }
}

uint32_t
toolchain::codeview::getScopeParentOffset(const toolchain::codeview::CVSymbol &Sym) {
  assert(symbolOpensScope(Sym.kind()));
  switch (Sym.kind()) {
  case SymbolKind::S_GPROC32:
  case SymbolKind::S_LPROC32:
  case SymbolKind::S_GPROC32_ID:
  case SymbolKind::S_LPROC32_ID:
  case SymbolKind::S_LPROC32_DPC:
  case SymbolKind::S_LPROC32_DPC_ID: {
    ProcSym Proc = createRecord<ProcSym>(Sym);
    return Proc.Parent;
  }
  case SymbolKind::S_BLOCK32: {
    BlockSym Block = createRecord<BlockSym>(Sym);
    return Block.Parent;
  }
  case SymbolKind::S_THUNK32: {
    Thunk32Sym Thunk = createRecord<Thunk32Sym>(Sym);
    return Thunk.Parent;
  }
  case SymbolKind::S_INLINESITE: {
    InlineSiteSym Site = createRecord<InlineSiteSym>(Sym);
    return Site.Parent;
  }
  default:
    assert(false && "Unknown record type");
    return 0;
  }
}

CVSymbolArray
toolchain::codeview::limitSymbolArrayToScope(const CVSymbolArray &Symbols,
                                        uint32_t ScopeBegin) {
  CVSymbol Opener = *Symbols.at(ScopeBegin);
  assert(symbolOpensScope(Opener.kind()));
  uint32_t EndOffset = getScopeEndOffset(Opener);
  CVSymbol Closer = *Symbols.at(EndOffset);
  EndOffset += Closer.RecordData.size();
  return Symbols.substream(ScopeBegin, EndOffset);
}
