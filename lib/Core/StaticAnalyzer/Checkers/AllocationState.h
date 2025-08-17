//===--- AllocationState.h ------------------------------------- *- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_ALLOCATIONSTATE_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_ALLOCATIONSTATE_H

#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporterVisitors.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"

namespace language::Core {
namespace ento {

namespace allocation_state {

ProgramStateRef markReleased(ProgramStateRef State, SymbolRef Sym,
                             const Expr *Origin);

/// This function provides an additional visitor that augments the bug report
/// with information relevant to memory errors caused by the misuse of
/// AF_InnerBuffer symbols.
std::unique_ptr<BugReporterVisitor> getInnerPointerBRVisitor(SymbolRef Sym);

/// 'Sym' represents a pointer to the inner buffer of a container object.
/// This function looks up the memory region of that object in
/// DanglingInternalBufferChecker's program state map.
const MemRegion *getContainerObjRegion(ProgramStateRef State, SymbolRef Sym);

} // end namespace allocation_state

} // end namespace ento
} // end namespace language::Core

#endif
