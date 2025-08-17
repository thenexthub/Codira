//===----- SemaRISCV.h ---- RISC-V target-specific routines ---*- C++ -*---===//
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
/// This file declares semantic analysis functions specific to RISC-V.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMARISCV_H
#define LANGUAGE_CORE_SEMA_SEMARISCV_H

#include "language/Core/AST/ASTFwd.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Sema/SemaBase.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include <memory>

namespace language::Core {
namespace sema {
class RISCVIntrinsicManager;
} // namespace sema

class ParsedAttr;
class TargetInfo;

class SemaRISCV : public SemaBase {
public:
  SemaRISCV(Sema &S);

  bool CheckLMUL(CallExpr *TheCall, unsigned ArgNum);
  bool CheckBuiltinFunctionCall(const TargetInfo &TI, unsigned BuiltinID,
                                CallExpr *TheCall);
  void checkRVVTypeSupport(QualType Ty, SourceLocation Loc, Decl *D,
                           const toolchain::StringMap<bool> &FeatureMap);

  bool isValidRVVBitcast(QualType srcType, QualType destType);

  void handleInterruptAttr(Decl *D, const ParsedAttr &AL);
  bool isAliasValid(unsigned BuiltinID, toolchain::StringRef AliasName);
  bool isValidFMVExtension(StringRef Ext);

  /// Indicate RISC-V vector builtin functions enabled or not.
  bool DeclareRVVBuiltins = false;

  /// Indicate RISC-V SiFive vector builtin functions enabled or not.
  bool DeclareSiFiveVectorBuiltins = false;

  /// Indicate RISC-V Andes vector builtin functions enabled or not.
  bool DeclareAndesVectorBuiltins = false;

  std::unique_ptr<sema::RISCVIntrinsicManager> IntrinsicManager;

  bool checkTargetVersionAttr(const StringRef Param, const SourceLocation Loc);
  bool checkTargetClonesAttr(SmallVectorImpl<StringRef> &Params,
                             SmallVectorImpl<SourceLocation> &Locs,
                             SmallVectorImpl<SmallString<64>> &NewParams);
};

std::unique_ptr<sema::RISCVIntrinsicManager>
CreateRISCVIntrinsicManager(Sema &S);
} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMARISCV_H
