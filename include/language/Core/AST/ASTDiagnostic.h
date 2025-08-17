//===--- ASTDiagnostic.h - Diagnostics for the AST library ------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_ASTDIAGNOSTIC_H
#define LANGUAGE_CORE_AST_ASTDIAGNOSTIC_H

#include "language/Core/AST/Type.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/DiagnosticAST.h"

namespace language::Core {
  /// DiagnosticsEngine argument formatting function for diagnostics that
  /// involve AST nodes.
  ///
  /// This function formats diagnostic arguments for various AST nodes,
  /// including types, declaration names, nested name specifiers, and
  /// declaration contexts, into strings that can be printed as part of
  /// diagnostics. It is meant to be used as the argument to
  /// \c DiagnosticsEngine::SetArgToStringFn(), where the cookie is an \c
  /// ASTContext pointer.
  void FormatASTNodeDiagnosticArgument(
      DiagnosticsEngine::ArgumentKind Kind,
      intptr_t Val,
      StringRef Modifier,
      StringRef Argument,
      ArrayRef<DiagnosticsEngine::ArgumentValue> PrevArgs,
      SmallVectorImpl<char> &Output,
      void *Cookie,
      ArrayRef<intptr_t> QualTypeVals);

  /// Returns a desugared version of the QualType, and marks ShouldAKA as true
  /// whenever we remove significant sugar from the type. Make sure ShouldAKA
  /// is initialized before passing it in.
  QualType desugarForDiagnostic(ASTContext &Context, QualType QT,
                                bool &ShouldAKA);

  std::string FormatUTFCodeUnitAsCodepoint(unsigned Value, QualType T);

}  // end namespace language::Core

#endif
