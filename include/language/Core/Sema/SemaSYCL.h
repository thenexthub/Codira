//===----- SemaSYCL.h ------- Semantic Analysis for SYCL constructs -------===//
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
/// \file
/// This file declares semantic analysis for SYCL constructs.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMASYCL_H
#define LANGUAGE_CORE_SEMA_SEMASYCL_H

#include "language/Core/AST/ASTFwd.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Sema/Ownership.h"
#include "language/Core/Sema/SemaBase.h"
#include "toolchain/ADT/DenseSet.h"

namespace language::Core {
class Decl;
class ParsedAttr;

class SemaSYCL : public SemaBase {
public:
  SemaSYCL(Sema &S);

  /// Creates a SemaDiagnosticBuilder that emits the diagnostic if the current
  /// context is "used as device code".
  ///
  /// - If CurLexicalContext is a kernel function or it is known that the
  ///   function will be emitted for the device, emits the diagnostics
  ///   immediately.
  /// - If CurLexicalContext is a function and we are compiling
  ///   for the device, but we don't know yet that this function will be
  ///   codegen'ed for the devive, creates a diagnostic which is emitted if and
  ///   when we realize that the function will be codegen'ed.
  ///
  /// Example usage:
  ///
  /// Diagnose __float128 type usage only from SYCL device code if the current
  /// target doesn't support it
  /// if (!S.Context.getTargetInfo().hasFloat128Type() &&
  ///     S.getLangOpts().SYCLIsDevice)
  ///   DiagIfDeviceCode(Loc, diag::err_type_unsupported) << "__float128";
  SemaDiagnosticBuilder DiagIfDeviceCode(SourceLocation Loc, unsigned DiagID);

  void deepTypeCheckForDevice(SourceLocation UsedAt,
                              toolchain::DenseSet<QualType> Visited,
                              ValueDecl *DeclToCheck);

  ExprResult BuildUniqueStableNameExpr(SourceLocation OpLoc,
                                       SourceLocation LParen,
                                       SourceLocation RParen,
                                       TypeSourceInfo *TSI);
  ExprResult ActOnUniqueStableNameExpr(SourceLocation OpLoc,
                                       SourceLocation LParen,
                                       SourceLocation RParen,
                                       ParsedType ParsedTy);

  void handleKernelAttr(Decl *D, const ParsedAttr &AL);
  void handleKernelEntryPointAttr(Decl *D, const ParsedAttr &AL);

  void CheckSYCLEntryPointFunctionDecl(FunctionDecl *FD);
  StmtResult BuildSYCLKernelCallStmt(FunctionDecl *FD, CompoundStmt *Body);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMASYCL_H
