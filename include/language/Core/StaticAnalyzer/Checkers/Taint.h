//=== Taint.h - Taint tracking and basic propagation rules. --------*- C++ -*-//
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
// Defines basic, non-domain-specific mechanisms for tracking tainted values.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_TAINT_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_TAINT_H

#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporterVisitors.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"

namespace language::Core {
namespace ento {
namespace taint {

/// The type of taint, which helps to differentiate between different types of
/// taint.
using TaintTagType = unsigned;

static constexpr TaintTagType TaintTagGeneric = 0;

/// Create a new state in which the value of the statement is marked as tainted.
[[nodiscard]] ProgramStateRef addTaint(ProgramStateRef State, const Stmt *S,
                                       const LocationContext *LCtx,
                                       TaintTagType Kind = TaintTagGeneric);

/// Create a new state in which the value is marked as tainted.
[[nodiscard]] ProgramStateRef addTaint(ProgramStateRef State, SVal V,
                                       TaintTagType Kind = TaintTagGeneric);

/// Create a new state in which the symbol is marked as tainted.
[[nodiscard]] ProgramStateRef addTaint(ProgramStateRef State, SymbolRef Sym,
                                       TaintTagType Kind = TaintTagGeneric);

/// Create a new state in which the pointer represented by the region
/// is marked as tainted.
[[nodiscard]] ProgramStateRef addTaint(ProgramStateRef State,
                                       const MemRegion *R,
                                       TaintTagType Kind = TaintTagGeneric);

[[nodiscard]] ProgramStateRef removeTaint(ProgramStateRef State, SVal V);

[[nodiscard]] ProgramStateRef removeTaint(ProgramStateRef State,
                                          const MemRegion *R);

[[nodiscard]] ProgramStateRef removeTaint(ProgramStateRef State, SymbolRef Sym);

/// Create a new state in a which a sub-region of a given symbol is tainted.
/// This might be necessary when referring to regions that can not have an
/// individual symbol, e.g. if they are represented by the default binding of
/// a LazyCompoundVal.
[[nodiscard]] ProgramStateRef
addPartialTaint(ProgramStateRef State, SymbolRef ParentSym,
                const SubRegion *SubRegion,
                TaintTagType Kind = TaintTagGeneric);

/// Check if the statement has a tainted value in the given state.
bool isTainted(ProgramStateRef State, const Stmt *S,
               const LocationContext *LCtx,
               TaintTagType Kind = TaintTagGeneric);

/// Check if the value is tainted in the given state.
bool isTainted(ProgramStateRef State, SVal V,
               TaintTagType Kind = TaintTagGeneric);

/// Check if the symbol is tainted in the given state.
bool isTainted(ProgramStateRef State, SymbolRef Sym,
               TaintTagType Kind = TaintTagGeneric);

/// Check if the pointer represented by the region is tainted in the given
/// state.
bool isTainted(ProgramStateRef State, const MemRegion *Reg,
               TaintTagType Kind = TaintTagGeneric);

/// Returns the tainted Symbols for a given Statement and state.
std::vector<SymbolRef> getTaintedSymbols(ProgramStateRef State, const Stmt *S,
                                         const LocationContext *LCtx,
                                         TaintTagType Kind = TaintTagGeneric);

/// Returns the tainted Symbols for a given SVal and state.
std::vector<SymbolRef> getTaintedSymbols(ProgramStateRef State, SVal V,
                                         TaintTagType Kind = TaintTagGeneric);

/// Returns the tainted Symbols for a SymbolRef and state.
std::vector<SymbolRef> getTaintedSymbols(ProgramStateRef State, SymbolRef Sym,
                                         TaintTagType Kind = TaintTagGeneric);

/// Returns the tainted (index, super/sub region, symbolic region) symbols
/// for a given memory region.
std::vector<SymbolRef> getTaintedSymbols(ProgramStateRef State,
                                         const MemRegion *Reg,
                                         TaintTagType Kind = TaintTagGeneric);

std::vector<SymbolRef> getTaintedSymbolsImpl(ProgramStateRef State,
                                             const Stmt *S,
                                             const LocationContext *LCtx,
                                             TaintTagType Kind,
                                             bool returnFirstOnly);

std::vector<SymbolRef> getTaintedSymbolsImpl(ProgramStateRef State, SVal V,
                                             TaintTagType Kind,
                                             bool returnFirstOnly);

std::vector<SymbolRef> getTaintedSymbolsImpl(ProgramStateRef State,
                                             SymbolRef Sym, TaintTagType Kind,
                                             bool returnFirstOnly);

std::vector<SymbolRef> getTaintedSymbolsImpl(ProgramStateRef State,
                                             const MemRegion *Reg,
                                             TaintTagType Kind,
                                             bool returnFirstOnly);

void printTaint(ProgramStateRef State, raw_ostream &Out, const char *nl = "\n",
                const char *sep = "");

LLVM_DUMP_METHOD void dumpTaint(ProgramStateRef State);
} // namespace taint
} // namespace ento
} // namespace language::Core

#endif
