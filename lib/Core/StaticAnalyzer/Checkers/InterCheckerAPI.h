//==--- InterCheckerAPI.h ---------------------------------------*- C++ -*-==//
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
// This file allows introduction of checker dependencies. It contains APIs for
// inter-checker communications.
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_INTERCHECKERAPI_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_INTERCHECKERAPI_H

// FIXME: This file goes against how a checker should be implemented either in
// a single file, or be exposed in a header file. Let's try to get rid of it!

namespace language::Core {
namespace ento {

class CheckerManager;

/// Register the part of MallocChecker connected to InnerPointerChecker.
void registerInnerPointerCheckerAux(CheckerManager &Mgr);

} // namespace ento
} // namespace language::Core

#endif /* INTERCHECKERAPI_H_ */
