//===-- M68kLegalizerInfo.cpp -----------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
/// This file implements the targeting of the Machinelegalizer class for M68k.
//===----------------------------------------------------------------------===//

#include "M68kLegalizerInfo.h"
#include "vm/core/CodeGen/GlobalISel/LegalizerHelper.h"
#include "vm/core/CodeGen/GlobalISel/LegalizerInfo.h"
#include "vm/core/CodeGen/TargetOpcodes.h"
#include "vm/core/CodeGen/ValueTypes.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Type.h"

using namespace vm::core;

M68kLegalizerInfo::M68kLegalizerInfo(const M68kSubtarget &ST) {
  using namespace TargetOpcode;
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT p0 = LLT::pointer(0, 32);

  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL, G_UDIV, G_AND})
      .legalFor({s8, s16, s32})
      .clampScalar(0, s8, s32)
      .widenScalarToNextPow2(0, 8);

  getActionDefinitionsBuilder(G_CONSTANT)
      .legalFor({s32, p0})
      .clampScalar(0, s32, s32);

  getActionDefinitionsBuilder({G_FRAME_INDEX, G_GLOBAL_VALUE}).legalFor({p0});

  getActionDefinitionsBuilder({G_STORE, G_LOAD})
      .legalForTypesWithMemDesc({{s32, p0, s32, 4},
                                 {s32, p0, s16, 4},
                                 {s32, p0, s8, 4},
                                 {s16, p0, s16, 2},
                                 {s8, p0, s8, 1},
                                 {p0, p0, s32, 4}})
      .clampScalar(0, s8, s32);

  getActionDefinitionsBuilder(G_PTR_ADD).legalFor({{p0, s32}});

  getLegacyLegalizerInfo().computeTables();
}
