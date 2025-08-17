//===--- Utils.h - Misc utilities for the front-end -------------*- C++ -*-===//
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
//  This header contains miscellaneous utilities for various front-end actions
//  which were split from Frontend to minimise Frontend's dependencies.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_FRONTENDTOOL_UTILS_H
#define LANGUAGE_CORE_FRONTENDTOOL_UTILS_H

#include <memory>

namespace language::Core {

class CompilerInstance;
class FrontendAction;

/// Construct the FrontendAction of a compiler invocation based on the
/// options specified for the compiler invocation.
///
/// \return - The created FrontendAction object
std::unique_ptr<FrontendAction> CreateFrontendAction(CompilerInstance &CI);

/// ExecuteCompilerInvocation - Execute the given actions described by the
/// compiler invocation object in the given compiler instance.
///
/// \return - True on success.
bool ExecuteCompilerInvocation(CompilerInstance *Clang);

}  // end namespace language::Core

#endif
