//===-- FPEnv.cpp ---- FP Environment -------------------------------------===//
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
//
/// @file
/// This file contains the implementations of entities that describe floating
/// point environment.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/FPEnv.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Intrinsics.h"
#include <optional>

using namespace vm::core;

std::optional<RoundingMode>
toolchain::convertStrToRoundingMode(StringRef RoundingArg) {
  // For dynamic rounding mode, we use round to nearest but we will set the
  // 'exact' SDNodeFlag so that the value will not be rounded.
  return StringSwitch<std::optional<RoundingMode>>(RoundingArg)
      .Case("round.dynamic", RoundingMode::Dynamic)
      .Case("round.tonearest", RoundingMode::NearestTiesToEven)
      .Case("round.tonearestaway", RoundingMode::NearestTiesToAway)
      .Case("round.downward", RoundingMode::TowardNegative)
      .Case("round.upward", RoundingMode::TowardPositive)
      .Case("round.towardzero", RoundingMode::TowardZero)
      .Default(std::nullopt);
}

std::optional<StringRef>
toolchain::convertRoundingModeToStr(RoundingMode UseRounding) {
  std::optional<StringRef> RoundingStr;
  switch (UseRounding) {
  case RoundingMode::Dynamic:
    RoundingStr = "round.dynamic";
    break;
  case RoundingMode::NearestTiesToEven:
    RoundingStr = "round.tonearest";
    break;
  case RoundingMode::NearestTiesToAway:
    RoundingStr = "round.tonearestaway";
    break;
  case RoundingMode::TowardNegative:
    RoundingStr = "round.downward";
    break;
  case RoundingMode::TowardPositive:
    RoundingStr = "round.upward";
    break;
  case RoundingMode::TowardZero:
    RoundingStr = "round.towardzero";
    break;
  default:
    break;
  }
  return RoundingStr;
}

std::optional<fp::ExceptionBehavior>
toolchain::convertStrToExceptionBehavior(StringRef ExceptionArg) {
  return StringSwitch<std::optional<fp::ExceptionBehavior>>(ExceptionArg)
      .Case("fpexcept.ignore", fp::ebIgnore)
      .Case("fpexcept.maytrap", fp::ebMayTrap)
      .Case("fpexcept.strict", fp::ebStrict)
      .Default(std::nullopt);
}

std::optional<StringRef>
toolchain::convertExceptionBehaviorToStr(fp::ExceptionBehavior UseExcept) {
  std::optional<StringRef> ExceptStr;
  switch (UseExcept) {
  case fp::ebStrict:
    ExceptStr = "fpexcept.strict";
    break;
  case fp::ebIgnore:
    ExceptStr = "fpexcept.ignore";
    break;
  case fp::ebMayTrap:
    ExceptStr = "fpexcept.maytrap";
    break;
  }
  return ExceptStr;
}

Intrinsic::ID toolchain::getConstrainedIntrinsicID(const Instruction &Instr) {
  Intrinsic::ID IID = Intrinsic::not_intrinsic;
  switch (Instr.getOpcode()) {
  case Instruction::FCmp:
    // Unlike other instructions FCmp can be mapped to one of two intrinsic
    // functions. We choose the non-signaling variant.
    IID = Intrinsic::experimental_constrained_fcmp;
    break;

    // Instructions
#define INSTRUCTION(NAME, NARG, ROUND_MODE, INTRINSIC)                         \
  case Instruction::NAME:                                                      \
    IID = Intrinsic::INTRINSIC;                                                \
    break;
#define FUNCTION(NAME, NARG, ROUND_MODE, INTRINSIC)
#define CMP_INSTRUCTION(NAME, NARG, ROUND_MODE, INTRINSIC, DAGN)
#include "vm/core/IR/ConstrainedOps.def"

  // Intrinsic calls.
  case Instruction::Call:
    if (auto *IntrinCall = dyn_cast<IntrinsicInst>(&Instr)) {
      switch (IntrinCall->getIntrinsicID()) {
#define FUNCTION(NAME, NARG, ROUND_MODE, INTRINSIC)                            \
  case Intrinsic::NAME:                                                        \
    IID = Intrinsic::INTRINSIC;                                                \
    break;
#define INSTRUCTION(NAME, NARG, ROUND_MODE, INTRINSIC)
#define CMP_INSTRUCTION(NAME, NARG, ROUND_MODE, INTRINSIC, DAGN)
#include "vm/core/IR/ConstrainedOps.def"
      default:
        break;
      }
    }
    break;
  default:
    break;
  }

  return IID;
}
