//===- DynamicType.h - Dynamic type related APIs ----------------*- C++ -*-===//
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
//  This file defines APIs that track and query dynamic type information. This
//  information can be used to devirtualize calls during the symbolic execution
//  or do type checking.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICTYPE_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICTYPE_H

#include "language/Core/AST/Type.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/DynamicCastInfo.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/DynamicTypeInfo.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"

namespace language::Core {
namespace ento {

/// Get dynamic type information for the region \p MR.
DynamicTypeInfo getDynamicTypeInfo(ProgramStateRef State, const MemRegion *MR);

/// Get raw dynamic type information for the region \p MR.
/// It might return null.
const DynamicTypeInfo *getRawDynamicTypeInfo(ProgramStateRef State,
                                             const MemRegion *MR);

/// Get dynamic type information stored in a class object represented by \p Sym.
DynamicTypeInfo getClassObjectDynamicTypeInfo(ProgramStateRef State,
                                              SymbolRef Sym);

/// Get dynamic cast information from \p CastFromTy to \p CastToTy of \p MR.
const DynamicCastInfo *getDynamicCastInfo(ProgramStateRef State,
                                          const MemRegion *MR,
                                          QualType CastFromTy,
                                          QualType CastToTy);

/// Set dynamic type information of the region; return the new state.
ProgramStateRef setDynamicTypeInfo(ProgramStateRef State, const MemRegion *MR,
                                   DynamicTypeInfo NewTy);

/// Set dynamic type information of the region; return the new state.
ProgramStateRef setDynamicTypeInfo(ProgramStateRef State, const MemRegion *MR,
                                   QualType NewTy, bool CanBeSubClassed = true);

/// Set constraint on a type contained in a class object; return the new state.
ProgramStateRef setClassObjectDynamicTypeInfo(ProgramStateRef State,
                                              SymbolRef Sym,
                                              DynamicTypeInfo NewTy);

/// Set constraint on a type contained in a class object; return the new state.
ProgramStateRef setClassObjectDynamicTypeInfo(ProgramStateRef State,
                                              SymbolRef Sym, QualType NewTy,
                                              bool CanBeSubClassed = true);

/// Set dynamic type and cast information of the region; return the new state.
ProgramStateRef setDynamicTypeAndCastInfo(ProgramStateRef State,
                                          const MemRegion *MR,
                                          QualType CastFromTy,
                                          QualType CastToTy,
                                          bool IsCastSucceeds);

/// Removes the dead type informations from \p State.
ProgramStateRef removeDeadTypes(ProgramStateRef State, SymbolReaper &SR);

/// Removes the dead cast informations from \p State.
ProgramStateRef removeDeadCasts(ProgramStateRef State, SymbolReaper &SR);

/// Removes the dead Class object type informations from \p State.
ProgramStateRef removeDeadClassObjectTypes(ProgramStateRef State,
                                           SymbolReaper &SR);

void printDynamicTypeInfoJson(raw_ostream &Out, ProgramStateRef State,
                              const char *NL = "\n", unsigned int Space = 0,
                              bool IsDot = false);

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICTYPE_H
