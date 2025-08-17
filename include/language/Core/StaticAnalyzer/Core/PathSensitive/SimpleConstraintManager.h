//== SimpleConstraintManager.h ----------------------------------*- C++ -*--==//
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
//  Simplified constraint manager backend.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SIMPLECONSTRAINTMANAGER_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SIMPLECONSTRAINTMANAGER_H

#include "language/Core/StaticAnalyzer/Core/PathSensitive/ConstraintManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"

namespace language::Core {

namespace ento {

class SimpleConstraintManager : public ConstraintManager {
  ExprEngine *EE;
  SValBuilder &SVB;

public:
  SimpleConstraintManager(ExprEngine *exprengine, SValBuilder &SB)
      : EE(exprengine), SVB(SB) {}

  ~SimpleConstraintManager() override;

  //===------------------------------------------------------------------===//
  // Implementation for interface from ConstraintManager.
  //===------------------------------------------------------------------===//

protected:
  //===------------------------------------------------------------------===//
  // Interface that subclasses must implement.
  //===------------------------------------------------------------------===//

  /// Given a symbolic expression that can be reasoned about, assume that it is
  /// true/false and generate the new program state.
  virtual ProgramStateRef assumeSym(ProgramStateRef State, SymbolRef Sym,
                                    bool Assumption) = 0;

  /// Given a symbolic expression within the range [From, To], assume that it is
  /// true/false and generate the new program state.
  /// This function is used to handle case ranges produced by a language
  /// extension for switch case statements.
  virtual ProgramStateRef assumeSymInclusiveRange(ProgramStateRef State,
                                                  SymbolRef Sym,
                                                  const toolchain::APSInt &From,
                                                  const toolchain::APSInt &To,
                                                  bool InRange) = 0;

  /// Given a symbolic expression that cannot be reasoned about, assume that
  /// it is zero/nonzero and add it directly to the solver state.
  virtual ProgramStateRef assumeSymUnsupported(ProgramStateRef State,
                                               SymbolRef Sym,
                                               bool Assumption) = 0;

  //===------------------------------------------------------------------===//
  // Internal implementation.
  //===------------------------------------------------------------------===//

  /// Ensures that the DefinedSVal conditional is expressed as a NonLoc by
  /// creating boolean casts to handle Loc's.
  ProgramStateRef assumeInternal(ProgramStateRef State, DefinedSVal Cond,
                                 bool Assumption) override;

  ProgramStateRef assumeInclusiveRangeInternal(ProgramStateRef State,
                                               NonLoc Value,
                                               const toolchain::APSInt &From,
                                               const toolchain::APSInt &To,
                                               bool InRange) override;

  SValBuilder &getSValBuilder() const { return SVB; }
  BasicValueFactory &getBasicVals() const { return SVB.getBasicValueFactory(); }
  SymbolManager &getSymbolManager() const { return SVB.getSymbolManager(); }

private:
  ProgramStateRef assume(ProgramStateRef State, NonLoc Cond, bool Assumption);

  ProgramStateRef assumeAux(ProgramStateRef State, NonLoc Cond,
                            bool Assumption);
};

} // end namespace ento

} // end namespace language::Core

#endif
