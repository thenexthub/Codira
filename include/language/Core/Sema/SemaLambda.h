//===--- SemaLambda.h - Lambda Helper Functions --------------*- C++ -*-===//
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
///
/// \file
/// This file provides some common utility functions for processing
/// Lambdas.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMALAMBDA_H
#define LANGUAGE_CORE_SEMA_SEMALAMBDA_H

#include "language/Core/AST/ASTLambda.h"
#include <optional>

namespace language::Core {
namespace sema {
class FunctionScopeInfo;
}
class Sema;

/// Examines the FunctionScopeInfo stack to determine the nearest
/// enclosing lambda (to the current lambda) that is 'capture-capable' for
/// the variable referenced in the current lambda (i.e. \p VarToCapture).
/// If successful, returns the index into Sema's FunctionScopeInfo stack
/// of the capture-capable lambda's LambdaScopeInfo.
/// See Implementation for more detailed comments.

UnsignedOrNone getStackIndexOfNearestEnclosingCaptureCapableLambda(
    ArrayRef<const sema::FunctionScopeInfo *> FunctionScopes,
    ValueDecl *VarToCapture, Sema &S);

} // clang

#endif
