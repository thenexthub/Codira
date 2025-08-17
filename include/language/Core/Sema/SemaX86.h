//===----- SemaX86.h ------- X86 target-specific routines -----*- C++ -*---===//
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
/// This file declares semantic analysis functions specific to X86.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMAX86_H
#define LANGUAGE_CORE_SEMA_SEMAX86_H

#include "language/Core/AST/ASTFwd.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Sema/SemaBase.h"

namespace language::Core {
class ParsedAttr;
class TargetInfo;

class SemaX86 : public SemaBase {
public:
  SemaX86(Sema &S);

  bool CheckBuiltinRoundingOrSAE(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckBuiltinGatherScatterScale(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckBuiltinTileArguments(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckBuiltinTileArgumentsRange(CallExpr *TheCall, ArrayRef<int> ArgNums);
  bool CheckBuiltinTileDuplicate(CallExpr *TheCall, ArrayRef<int> ArgNums);
  bool CheckBuiltinTileRangeAndDuplicate(CallExpr *TheCall,
                                         ArrayRef<int> ArgNums);
  bool CheckBuiltinFunctionCall(const TargetInfo &TI, unsigned BuiltinID,
                                CallExpr *TheCall);

  void handleAnyInterruptAttr(Decl *D, const ParsedAttr &AL);
  void handleForceAlignArgPointerAttr(Decl *D, const ParsedAttr &AL);

  bool checkTargetClonesAttr(SmallVectorImpl<StringRef> &Params,
                             SmallVectorImpl<SourceLocation> &Locs,
                             SmallVectorImpl<SmallString<64>> &NewParams);
};
} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMAX86_H
