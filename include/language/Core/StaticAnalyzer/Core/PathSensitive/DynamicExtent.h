//===- DynamicExtent.h - Dynamic extent related APIs ------------*- C++ -*-===//
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
//  This file defines APIs that track and query dynamic extent information.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICEXTENT_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICEXTENT_H

#include "language/Core/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SValBuilder.h"

namespace language::Core {
namespace ento {

/// \returns The stored dynamic extent for the region \p MR.
DefinedOrUnknownSVal getDynamicExtent(ProgramStateRef State,
                                      const MemRegion *MR, SValBuilder &SVB);

/// \returns The element extent of the type \p Ty.
DefinedOrUnknownSVal getElementExtent(QualType Ty, SValBuilder &SVB);

/// \returns The stored element count of the region \p MR.
DefinedOrUnknownSVal getDynamicElementCount(ProgramStateRef State,
                                            const MemRegion *MR,
                                            SValBuilder &SVB, QualType Ty);

/// Set the dynamic extent \p Extent of the region \p MR.
ProgramStateRef setDynamicExtent(ProgramStateRef State, const MemRegion *MR,
                                 DefinedOrUnknownSVal Extent);

/// Get the dynamic extent for a symbolic value that represents a buffer. If
/// there is an offsetting to the underlying buffer we consider that too.
/// Returns with an SVal that represents the extent, this is Unknown if the
/// engine cannot deduce the extent.
/// E.g.
///   char buf[3];
///   (buf); // extent is 3
///   (buf + 1); // extent is 2
///   (buf + 3); // extent is 0
///   (buf + 4); // extent is -1
///
///   char *bufptr;
///   (bufptr) // extent is unknown
SVal getDynamicExtentWithOffset(ProgramStateRef State, SVal BufV);

/// \returns The stored element count of the region represented by a symbolic
/// value \p BufV.
DefinedOrUnknownSVal getDynamicElementCountWithOffset(ProgramStateRef State,
                                                      SVal BufV, QualType Ty);

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_DYNAMICEXTENT_H
