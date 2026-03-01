//===-- PPCPredicates.cpp - PPC Branch Predicate Information --------------===//
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
// This file implements the PowerPC branch predicates.
//
//===----------------------------------------------------------------------===//

#include "PPCPredicates.h"
#include "vm/core/Support/ErrorHandling.h"
using namespace vm::core;

PPC::Predicate PPC::InvertPredicate(PPC::Predicate Opcode) {
  switch (Opcode) {
  case PPC::PRED_EQ: return PPC::PRED_NE;
  case PPC::PRED_NE: return PPC::PRED_EQ;
  case PPC::PRED_LT: return PPC::PRED_GE;
  case PPC::PRED_GE: return PPC::PRED_LT;
  case PPC::PRED_GT: return PPC::PRED_LE;
  case PPC::PRED_LE: return PPC::PRED_GT;
  case PPC::PRED_NU: return PPC::PRED_UN;
  case PPC::PRED_UN: return PPC::PRED_NU;
  case PPC::PRED_EQ_MINUS: return PPC::PRED_NE_PLUS;
  case PPC::PRED_NE_MINUS: return PPC::PRED_EQ_PLUS;
  case PPC::PRED_LT_MINUS: return PPC::PRED_GE_PLUS;
  case PPC::PRED_GE_MINUS: return PPC::PRED_LT_PLUS;
  case PPC::PRED_GT_MINUS: return PPC::PRED_LE_PLUS;
  case PPC::PRED_LE_MINUS: return PPC::PRED_GT_PLUS;
  case PPC::PRED_NU_MINUS: return PPC::PRED_UN_PLUS;
  case PPC::PRED_UN_MINUS: return PPC::PRED_NU_PLUS;
  case PPC::PRED_EQ_PLUS: return PPC::PRED_NE_MINUS;
  case PPC::PRED_NE_PLUS: return PPC::PRED_EQ_MINUS;
  case PPC::PRED_LT_PLUS: return PPC::PRED_GE_MINUS;
  case PPC::PRED_GE_PLUS: return PPC::PRED_LT_MINUS;
  case PPC::PRED_GT_PLUS: return PPC::PRED_LE_MINUS;
  case PPC::PRED_LE_PLUS: return PPC::PRED_GT_MINUS;
  case PPC::PRED_NU_PLUS: return PPC::PRED_UN_MINUS;
  case PPC::PRED_UN_PLUS: return PPC::PRED_NU_MINUS;

  // Simple predicates for single condition-register bits.
  case PPC::PRED_BIT_SET:   return PPC::PRED_BIT_UNSET;
  case PPC::PRED_BIT_UNSET: return PPC::PRED_BIT_SET;
  }
  llvm_unreachable("Unknown PPC branch opcode!");
}

PPC::Predicate PPC::getSwappedPredicate(PPC::Predicate Opcode) {
  switch (Opcode) {
  case PPC::PRED_EQ: return PPC::PRED_EQ;
  case PPC::PRED_NE: return PPC::PRED_NE;
  case PPC::PRED_LT: return PPC::PRED_GT;
  case PPC::PRED_GE: return PPC::PRED_LE;
  case PPC::PRED_GT: return PPC::PRED_LT;
  case PPC::PRED_LE: return PPC::PRED_GE;
  case PPC::PRED_NU: return PPC::PRED_NU;
  case PPC::PRED_UN: return PPC::PRED_UN;
  case PPC::PRED_EQ_MINUS: return PPC::PRED_EQ_MINUS;
  case PPC::PRED_NE_MINUS: return PPC::PRED_NE_MINUS;
  case PPC::PRED_LT_MINUS: return PPC::PRED_GT_MINUS;
  case PPC::PRED_GE_MINUS: return PPC::PRED_LE_MINUS;
  case PPC::PRED_GT_MINUS: return PPC::PRED_LT_MINUS;
  case PPC::PRED_LE_MINUS: return PPC::PRED_GE_MINUS;
  case PPC::PRED_NU_MINUS: return PPC::PRED_NU_MINUS;
  case PPC::PRED_UN_MINUS: return PPC::PRED_UN_MINUS;
  case PPC::PRED_EQ_PLUS: return PPC::PRED_EQ_PLUS;
  case PPC::PRED_NE_PLUS: return PPC::PRED_NE_PLUS;
  case PPC::PRED_LT_PLUS: return PPC::PRED_GT_PLUS;
  case PPC::PRED_GE_PLUS: return PPC::PRED_LE_PLUS;
  case PPC::PRED_GT_PLUS: return PPC::PRED_LT_PLUS;
  case PPC::PRED_LE_PLUS: return PPC::PRED_GE_PLUS;
  case PPC::PRED_NU_PLUS: return PPC::PRED_NU_PLUS;
  case PPC::PRED_UN_PLUS: return PPC::PRED_UN_PLUS;

  case PPC::PRED_BIT_SET:
  case PPC::PRED_BIT_UNSET:
    llvm_unreachable("Invalid use of bit predicate code");
  }
  llvm_unreachable("Unknown PPC branch opcode!");
}

