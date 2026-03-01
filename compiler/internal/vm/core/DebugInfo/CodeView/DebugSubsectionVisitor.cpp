//===- DebugSubsectionVisitor.cpp -------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/CodeView/DebugSubsectionVisitor.h"

#include "vm/core/DebugInfo/CodeView/CodeView.h"
#include "vm/core/DebugInfo/CodeView/DebugChecksumsSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugCrossExSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugCrossImpSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugFrameDataSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugInlineeLinesSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugLinesSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugStringTableSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugSubsectionRecord.h"
#include "vm/core/DebugInfo/CodeView/DebugSymbolRVASubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugSymbolsSubsection.h"
#include "vm/core/DebugInfo/CodeView/DebugUnknownSubsection.h"
#include "vm/core/Support/BinaryStreamReader.h"

using namespace vm::core;
using namespace vm::core::codeview;

Error toolchain::codeview::visitDebugSubsection(
    const DebugSubsectionRecord &R, DebugSubsectionVisitor &V,
    const StringsAndChecksumsRef &State) {
  BinaryStreamReader Reader(R.getRecordData());
  switch (R.kind()) {
  case DebugSubsectionKind::Lines: {
    DebugLinesSubsectionRef Fragment;
    if (auto EC = Fragment.initialize(Reader))
      return EC;

    return V.visitLines(Fragment, State);
  }
  case DebugSubsectionKind::FileChecksums: {
    DebugChecksumsSubsectionRef Fragment;
    if (auto EC = Fragment.initialize(Reader))
      return EC;

    return V.visitFileChecksums(Fragment, State);
  }
  case DebugSubsectionKind::InlineeLines: {
    DebugInlineeLinesSubsectionRef Fragment;
    if (auto EC = Fragment.initialize(Reader))
      return EC;
    return V.visitInlineeLines(Fragment, State);
  }
  case DebugSubsectionKind::CrossScopeExports: {
    DebugCrossModuleExportsSubsectionRef Section;
    if (auto EC = Section.initialize(Reader))
      return EC;
    return V.visitCrossModuleExports(Section, State);
  }
  case DebugSubsectionKind::CrossScopeImports: {
    DebugCrossModuleImportsSubsectionRef Section;
    if (auto EC = Section.initialize(Reader))
      return EC;
    return V.visitCrossModuleImports(Section, State);
  }
  case DebugSubsectionKind::Symbols: {
    DebugSymbolsSubsectionRef Section;
    if (auto EC = Section.initialize(Reader))
      return EC;
    return V.visitSymbols(Section, State);
  }
  case DebugSubsectionKind::StringTable: {
    DebugStringTableSubsectionRef Section;
    if (auto EC = Section.initialize(Reader))
      return EC;
    return V.visitStringTable(Section, State);
  }
  case DebugSubsectionKind::FrameData: {
    DebugFrameDataSubsectionRef Section;
    if (auto EC = Section.initialize(Reader))
      return EC;
    return V.visitFrameData(Section, State);
  }
  case DebugSubsectionKind::CoffSymbolRVA: {
    DebugSymbolRVASubsectionRef Section;
    if (auto EC = Section.initialize(Reader))
      return EC;
    return V.visitCOFFSymbolRVAs(Section, State);
  }
  default: {
    DebugUnknownSubsectionRef Fragment(R.kind(), R.getRecordData());
    return V.visitUnknown(Fragment);
  }
  }
}
