//===--- CFGStmtMap.h - Map from Stmt* to CFGBlock* -----------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//  This file defines the CFGStmtMap class, which defines a mapping from
//  Stmt* to CFGBlock*
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_CFGSTMTMAP_H
#define LANGUAGE_CORE_ANALYSIS_CFGSTMTMAP_H

#include "language/Core/Analysis/CFG.h"

namespace language::Core {

class ParentMap;
class Stmt;

class CFGStmtMap {
  ParentMap *PM;
  void *M;

  CFGStmtMap(ParentMap *pm, void *m) : PM(pm), M(m) {}
  CFGStmtMap(const CFGStmtMap &) = delete;
  CFGStmtMap &operator=(const CFGStmtMap &) = delete;

public:
  ~CFGStmtMap();

  /// Returns a new CFGMap for the given CFG.  It is the caller's
  /// responsibility to 'delete' this object when done using it.
  static CFGStmtMap *Build(CFG* C, ParentMap *PM);

  /// Returns the CFGBlock the specified Stmt* appears in.  For Stmt* that
  /// are terminators, the CFGBlock is the block they appear as a terminator,
  /// and not the block they appear as a block-level expression (e.g, '&&').
  /// CaseStmts and LabelStmts map to the CFGBlock they label.
  CFGBlock *getBlock(Stmt * S);

  const CFGBlock *getBlock(const Stmt * S) const {
    return const_cast<CFGStmtMap*>(this)->getBlock(const_cast<Stmt*>(S));
  }
};

} // end clang namespace
#endif
