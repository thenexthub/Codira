//===-- NoopAnalysis.h ------------------------------------------*- C++ -*-===//
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
//  This file defines a NoopAnalysis class that just uses the builtin transfer.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_NOOPANALYSIS_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_NOOPANALYSIS_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/Analysis/CFG.h"
#include "language/Core/Analysis/FlowSensitive/DataflowAnalysis.h"
#include "language/Core/Analysis/FlowSensitive/DataflowEnvironment.h"
#include "language/Core/Analysis/FlowSensitive/NoopLattice.h"

namespace language::Core {
namespace dataflow {

class NoopAnalysis : public DataflowAnalysis<NoopAnalysis, NoopLattice> {
public:
  NoopAnalysis(ASTContext &Context)
      : DataflowAnalysis<NoopAnalysis, NoopLattice>(Context) {}

  NoopAnalysis(ASTContext &Context, DataflowAnalysisOptions Options)
      : DataflowAnalysis<NoopAnalysis, NoopLattice>(Context, Options) {}

  static NoopLattice initialElement() { return {}; }

  void transfer(const CFGElement &E, NoopLattice &L, Environment &Env) {}
};

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_NOOPANALYSIS_H
