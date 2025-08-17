//===----- SemaAMDGPU.h --- AMDGPU target-specific routines ---*- C++ -*---===//
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
/// This file declares semantic analysis functions specific to AMDGPU.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMAAMDGPU_H
#define LANGUAGE_CORE_SEMA_SEMAAMDGPU_H

#include "language/Core/AST/ASTFwd.h"
#include "language/Core/Sema/SemaBase.h"

namespace language::Core {
class AttributeCommonInfo;
class ParsedAttr;

class SemaAMDGPU : public SemaBase {
public:
  SemaAMDGPU(Sema &S);

  bool CheckAMDGCNBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);

  bool checkMovDPPFunctionCall(CallExpr *TheCall, unsigned NumArgs,
                               unsigned NumDataArgs);

  /// Create an AMDGPUWavesPerEUAttr attribute.
  AMDGPUFlatWorkGroupSizeAttr *
  CreateAMDGPUFlatWorkGroupSizeAttr(const AttributeCommonInfo &CI, Expr *Min,
                                    Expr *Max);

  /// addAMDGPUFlatWorkGroupSizeAttr - Adds an amdgpu_flat_work_group_size
  /// attribute to a particular declaration.
  void addAMDGPUFlatWorkGroupSizeAttr(Decl *D, const AttributeCommonInfo &CI,
                                      Expr *Min, Expr *Max);

  /// Create an AMDGPUWavesPerEUAttr attribute.
  AMDGPUWavesPerEUAttr *
  CreateAMDGPUWavesPerEUAttr(const AttributeCommonInfo &CI, Expr *Min,
                             Expr *Max);

  /// addAMDGPUWavePersEUAttr - Adds an amdgpu_waves_per_eu attribute to a
  /// particular declaration.
  void addAMDGPUWavesPerEUAttr(Decl *D, const AttributeCommonInfo &CI,
                               Expr *Min, Expr *Max);

  /// Create an AMDGPUMaxNumWorkGroupsAttr attribute.
  AMDGPUMaxNumWorkGroupsAttr *
  CreateAMDGPUMaxNumWorkGroupsAttr(const AttributeCommonInfo &CI, Expr *XExpr,
                                   Expr *YExpr, Expr *ZExpr);

  /// addAMDGPUMaxNumWorkGroupsAttr - Adds an amdgpu_max_num_work_groups
  /// attribute to a particular declaration.
  void addAMDGPUMaxNumWorkGroupsAttr(Decl *D, const AttributeCommonInfo &CI,
                                     Expr *XExpr, Expr *YExpr, Expr *ZExpr);

  void handleAMDGPUWavesPerEUAttr(Decl *D, const ParsedAttr &AL);
  void handleAMDGPUNumSGPRAttr(Decl *D, const ParsedAttr &AL);
  void handleAMDGPUNumVGPRAttr(Decl *D, const ParsedAttr &AL);
  void handleAMDGPUMaxNumWorkGroupsAttr(Decl *D, const ParsedAttr &AL);
  void handleAMDGPUFlatWorkGroupSizeAttr(Decl *D, const ParsedAttr &AL);
};
} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMAAMDGPU_H
