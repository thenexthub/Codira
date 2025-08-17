//===--- InterpreterUtils.h - Incremental Utils --------*- C++ -*-===//
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
// This file implements some common utils used in the incremental library.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INTERPRETER_UTILS_H
#define LANGUAGE_CORE_INTERPRETER_UTILS_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Mangle.h"
#include "language/Core/AST/TypeVisitor.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/CodeGen/ModuleBuilder.h"
#include "language/Core/Driver/Compilation.h"
#include "language/Core/Driver/Driver.h"
#include "language/Core/Driver/Job.h"
#include "language/Core/Driver/Options.h"
#include "language/Core/Driver/Tool.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/TextDiagnosticBuffer.h"
#include "language/Core/Lex/PreprocessorOptions.h"

#include "language/Core/Sema/Lookup.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Support/Errc.h"
#include "toolchain/TargetParser/Host.h"

namespace language::Core {
IntegerLiteral *IntegerLiteralExpr(ASTContext &C, uint64_t Val);

Expr *CStyleCastPtrExpr(Sema &S, QualType Ty, Expr *E);

Expr *CStyleCastPtrExpr(Sema &S, QualType Ty, uintptr_t Ptr);

Sema::DeclGroupPtrTy CreateDGPtrFrom(Sema &S, Decl *D);

NamespaceDecl *LookupNamespace(Sema &S, toolchain::StringRef Name,
                               const DeclContext *Within = nullptr);

NamedDecl *LookupNamed(Sema &S, toolchain::StringRef Name,
                       const DeclContext *Within = nullptr);

std::string GetFullTypeName(ASTContext &Ctx, QualType QT);
} // namespace language::Core

#endif
