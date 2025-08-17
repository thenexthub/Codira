//===- StmtSYCL.h - Classes for SYCL kernel calls ---------------*- C++ -*-===//
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
/// This file defines SYCL AST classes used to represent calls to SYCL kernels.
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_STMTSYCL_H
#define LANGUAGE_CORE_AST_STMTSYCL_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/Basic/SourceLocation.h"

namespace language::Core {

//===----------------------------------------------------------------------===//
// AST classes for SYCL kernel calls.
//===----------------------------------------------------------------------===//

/// SYCLKernelCallStmt represents the transformation that is applied to the body
/// of a function declared with the sycl_kernel_entry_point attribute. The body
/// of such a function specifies the statements to be executed on a SYCL device
/// to invoke a SYCL kernel with a particular set of kernel arguments. The
/// SYCLKernelCallStmt associates an original statement (the compound statement
/// that is the function body) with an OutlinedFunctionDecl that holds the
/// kernel parameters and the transformed body. During code generation, the
/// OutlinedFunctionDecl is used to emit an offload kernel entry point suitable
/// for invocation from a SYCL library implementation. If executed, the
/// SYCLKernelCallStmt behaves as a no-op; no code generation is performed for
/// it.
class SYCLKernelCallStmt : public Stmt {
  friend class ASTStmtReader;
  friend class ASTStmtWriter;

private:
  Stmt *OriginalStmt = nullptr;
  OutlinedFunctionDecl *OFDecl = nullptr;

public:
  /// Construct a SYCL kernel call statement.
  SYCLKernelCallStmt(CompoundStmt *CS, OutlinedFunctionDecl *OFD)
      : Stmt(SYCLKernelCallStmtClass), OriginalStmt(CS), OFDecl(OFD) {}

  /// Construct an empty SYCL kernel call statement.
  SYCLKernelCallStmt(EmptyShell Empty) : Stmt(SYCLKernelCallStmtClass, Empty) {}

  /// Retrieve the model statement.
  CompoundStmt *getOriginalStmt() { return cast<CompoundStmt>(OriginalStmt); }
  const CompoundStmt *getOriginalStmt() const {
    return cast<CompoundStmt>(OriginalStmt);
  }
  void setOriginalStmt(CompoundStmt *CS) { OriginalStmt = CS; }

  /// Retrieve the outlined function declaration.
  OutlinedFunctionDecl *getOutlinedFunctionDecl() { return OFDecl; }
  const OutlinedFunctionDecl *getOutlinedFunctionDecl() const { return OFDecl; }

  /// Set the outlined function declaration.
  void setOutlinedFunctionDecl(OutlinedFunctionDecl *OFD) { OFDecl = OFD; }

  SourceLocation getBeginLoc() const LLVM_READONLY {
    return getOriginalStmt()->getBeginLoc();
  }

  SourceLocation getEndLoc() const LLVM_READONLY {
    return getOriginalStmt()->getEndLoc();
  }

  SourceRange getSourceRange() const LLVM_READONLY {
    return getOriginalStmt()->getSourceRange();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == SYCLKernelCallStmtClass;
  }

  child_range children() {
    return child_range(&OriginalStmt, &OriginalStmt + 1);
  }

  const_child_range children() const {
    return const_child_range(&OriginalStmt, &OriginalStmt + 1);
  }
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_AST_STMTSYCL_H
