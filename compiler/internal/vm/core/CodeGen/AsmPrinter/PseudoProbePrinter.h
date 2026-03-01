//===- PseudoProbePrinter.h - Pseudo probe encoding support -----*- C++ -*-===//
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
// This file contains support for writing pseudo probe info into asm files.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_PSEUDOPROBEPRINTER_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_PSEUDOPROBEPRINTER_H

#include "vm/core/ADT/DenseMap.h"

#ifndef NDEBUG
#include "vm/core/ADT/DenseSet.h"
#endif

namespace vm::core {

class AsmPrinter;
class DILocation;

class PseudoProbeHandler {
  // Target of pseudo probe emission.
  AsmPrinter *Asm;
  // Name to GUID map, used as caching/memoization for speed.
  DenseMap<StringRef, uint64_t> NameGuidMap;

#ifndef NDEBUG
  // All GUID in toolchain.pseudo_probe_desc.
  DenseSet<uint64_t> DescGuidSet;

  void verifyGuidExistenceInDesc(uint64_t Guid, StringRef FuncName);
#endif

public:
  PseudoProbeHandler(AsmPrinter *A) : Asm(A) {};

  void emitPseudoProbe(uint64_t Guid, uint64_t Index, uint64_t Type,
                       uint64_t Attr, const DILocation *DebugLoc);
};

} // namespace vm::core
#endif // LLVM_LIB_CODEGEN_ASMPRINTER_PSEUDOPROBEPRINTER_H
