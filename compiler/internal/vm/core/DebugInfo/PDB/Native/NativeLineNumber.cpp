//===- NativeLineNumber.cpp - Native line number implementation -*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/NativeLineNumber.h"
#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"

using namespace vm::core;
using namespace vm::core::pdb;

NativeLineNumber::NativeLineNumber(const NativeSession &Session,
                                   const codeview::LineInfo Line,
                                   uint32_t ColumnNumber, uint32_t Section,
                                   uint32_t Offset, uint32_t Length,
                                   uint32_t SrcFileId, uint32_t CompilandId)
    : Session(Session), Line(Line), ColumnNumber(ColumnNumber),
      Section(Section), Offset(Offset), Length(Length), SrcFileId(SrcFileId),
      CompilandId(CompilandId) {}

uint32_t NativeLineNumber::getLineNumber() const { return Line.getStartLine(); }

uint32_t NativeLineNumber::getLineNumberEnd() const {
  return Line.getEndLine();
}

uint32_t NativeLineNumber::getColumnNumber() const { return ColumnNumber; }

uint32_t NativeLineNumber::getColumnNumberEnd() const { return 0; }

uint32_t NativeLineNumber::getAddressSection() const { return Section; }

uint32_t NativeLineNumber::getAddressOffset() const { return Offset; }

uint32_t NativeLineNumber::getRelativeVirtualAddress() const {
  return Session.getRVAFromSectOffset(Section, Offset);
}

uint64_t NativeLineNumber::getVirtualAddress() const {
  return Session.getVAFromSectOffset(Section, Offset);
}

uint32_t NativeLineNumber::getLength() const { return Length; }

uint32_t NativeLineNumber::getSourceFileId() const { return SrcFileId; }

uint32_t NativeLineNumber::getCompilandId() const { return CompilandId; }

bool NativeLineNumber::isStatement() const { return Line.isStatement(); }
