//=== SmartPtr.h - Tracking smart pointer state. -------------------*- C++ -*-//
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
// Defines inter-checker API for the smart pointer modeling. It allows
// dependent checkers to figure out if an smart pointer is null or not.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_SMARTPTR_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_SMARTPTR_H

#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"

namespace language::Core {
namespace ento {
namespace smartptr {

/// Returns true if the event call is on smart pointer.
bool isStdSmartPtrCall(const CallEvent &Call);
bool isStdSmartPtr(const CXXRecordDecl *RD);
bool isStdSmartPtr(const Expr *E);

/// Returns whether the smart pointer is null or not.
bool isNullSmartPtr(const ProgramStateRef State, const MemRegion *ThisRegion);

const BugType *getNullDereferenceBugType();

} // namespace smartptr
} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_SMARTPTR_H
