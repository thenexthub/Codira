//===----- SemaPPC.h ------- PPC target-specific routines -----*- C++ -*---===//
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
/// This file declares semantic analysis functions specific to PowerPC.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMAPPC_H
#define LANGUAGE_CORE_SEMA_SEMAPPC_H

#include "language/Core/AST/ASTFwd.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Sema/SemaBase.h"

namespace language::Core {
class TargetInfo;

class SemaPPC : public SemaBase {
public:
  SemaPPC(Sema &S);

  bool CheckPPCBuiltinFunctionCall(const TargetInfo &TI, unsigned BuiltinID,
                                   CallExpr *TheCall);
  // 16 byte ByVal alignment not due to a vector member is not honoured by XL
  // on AIX. Emit a warning here that users are generating binary incompatible
  // code to be safe.
  // Here we try to get information about the alignment of the struct member
  // from the struct passed to the caller function. We only warn when the struct
  // is passed byval, hence the series of checks and early returns if we are a
  // not passing a struct byval.
  void checkAIXMemberAlignment(SourceLocation Loc, const Expr *Arg);

  /// BuiltinPPCMMACall - Check the call to a PPC MMA builtin for validity.
  /// Emit an error and return true on failure; return false on success.
  /// TypeStr is a string containing the type descriptor of the value returned
  /// by the builtin and the descriptors of the expected type of the arguments.
  bool BuiltinPPCMMACall(CallExpr *TheCall, unsigned BuiltinID,
                         const char *TypeDesc);

  bool CheckPPCMMAType(QualType Type, SourceLocation TypeLoc);

  // Customized Sema Checking for VSX builtins that have the following
  // signature: vector [...] builtinName(vector [...], vector [...], const int);
  // Which takes the same type of vectors (any legal vector type) for the first
  // two arguments and takes compile time constant for the third argument.
  // Example builtins are :
  // vector double vec_xxpermdi(vector double, vector double, int);
  // vector short vec_xxsldwi(vector short, vector short, int);
  bool BuiltinVSX(CallExpr *TheCall);
};
} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMAPPC_H
