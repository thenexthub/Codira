//===----- SemaARM.h ------- ARM target-specific routines -----*- C++ -*---===//
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
/// This file declares semantic analysis functions specific to ARM.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMAARM_H
#define LANGUAGE_CORE_SEMA_SEMAARM_H

#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Sema/SemaBase.h"
#include "toolchain/ADT/StringRef.h"
#include <tuple>

namespace toolchain {
template <typename T, unsigned N> class SmallVector;
} // namespace toolchain

namespace language::Core {
class ParsedAttr;
class TargetInfo;

class SemaARM : public SemaBase {
public:
  SemaARM(Sema &S);

  enum ArmStreamingType {
    ArmNonStreaming, /// Intrinsic is only available in normal mode
    ArmStreaming,    /// Intrinsic is only available in Streaming-SVE mode.
    ArmStreamingCompatible, /// Intrinsic is available both in normal and
                            /// Streaming-SVE mode.
    VerifyRuntimeMode       /// Intrinsic is available in normal mode with
                            /// SVE flags, or in Streaming-SVE mode with SME
                            /// flags. Do Sema checks for the runtime mode.
  };

  bool CheckImmediateArg(CallExpr *TheCall, unsigned CheckTy, unsigned ArgIdx,
                         unsigned EltBitWidth, unsigned VecBitWidth);
  bool CheckARMBuiltinExclusiveCall(const TargetInfo &TI, unsigned BuiltinID,
                                    CallExpr *TheCall);
  bool CheckNeonBuiltinFunctionCall(const TargetInfo &TI, unsigned BuiltinID,
                                    CallExpr *TheCall);
  bool PerformNeonImmChecks(
      CallExpr *TheCall,
      SmallVectorImpl<std::tuple<int, int, int, int>> &ImmChecks,
      int OverloadType = -1);
  bool
  PerformSVEImmChecks(CallExpr *TheCall,
                      SmallVectorImpl<std::tuple<int, int, int>> &ImmChecks);
  bool CheckMVEBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckSVEBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckSMEBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckCDEBuiltinFunctionCall(const TargetInfo &TI, unsigned BuiltinID,
                                   CallExpr *TheCall);
  bool CheckARMCoprocessorImmediate(const TargetInfo &TI, const Expr *CoprocArg,
                                    bool WantCDE);
  bool CheckARMBuiltinFunctionCall(const TargetInfo &TI, unsigned BuiltinID,
                                   CallExpr *TheCall);

  bool CheckAArch64BuiltinFunctionCall(const TargetInfo &TI, unsigned BuiltinID,
                                       CallExpr *TheCall);
  bool BuiltinARMSpecialReg(unsigned BuiltinID, CallExpr *TheCall, int ArgNum,
                            unsigned ExpectedFieldNum, bool AllowName);
  bool BuiltinARMMemoryTaggingCall(unsigned BuiltinID, CallExpr *TheCall);

  bool MveAliasValid(unsigned BuiltinID, toolchain::StringRef AliasName);
  bool CdeAliasValid(unsigned BuiltinID, toolchain::StringRef AliasName);
  bool SveAliasValid(unsigned BuiltinID, toolchain::StringRef AliasName);
  bool SmeAliasValid(unsigned BuiltinID, toolchain::StringRef AliasName);
  void handleBuiltinAliasAttr(Decl *D, const ParsedAttr &AL);
  void handleNewAttr(Decl *D, const ParsedAttr &AL);
  void handleCmseNSEntryAttr(Decl *D, const ParsedAttr &AL);
  void handleInterruptAttr(Decl *D, const ParsedAttr &AL);
  void handleInterruptSaveFPAttr(Decl *D, const ParsedAttr &AL);

  void CheckSMEFunctionDefAttributes(const FunctionDecl *FD);

  /// Return true if the given types are an SVE builtin and a VectorType that
  /// is a fixed-length representation of the SVE builtin for a specific
  /// vector-length.
  bool areCompatibleSveTypes(QualType FirstType, QualType SecondType);

  /// Return true if the given vector types are lax-compatible SVE vector types,
  /// false otherwise.
  bool areLaxCompatibleSveTypes(QualType FirstType, QualType SecondType);

  bool checkTargetVersionAttr(const StringRef Str, const SourceLocation Loc);
  bool checkTargetClonesAttr(SmallVectorImpl<StringRef> &Params,
                             SmallVectorImpl<SourceLocation> &Locs,
                             SmallVectorImpl<SmallString<64>> &NewParams);
};

SemaARM::ArmStreamingType getArmStreamingFnType(const FunctionDecl *FD);

} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMAARM_H
