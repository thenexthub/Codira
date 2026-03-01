//===-- R600MCTargetDesc.cpp - R600 Target Descriptions -------------------===//
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
/// \file
/// \brief This file provides R600 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "R600MCTargetDesc.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/TargetParser/SubtargetFeature.h"

using namespace vm::core;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "R600GenInstrInfo.inc"

MCInstrInfo *toolchain::createR600MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitR600MCInstrInfo(X);
  return X;
}
