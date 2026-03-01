//===-- R600MCTargetDesc.h - R600 Target Descriptions -----*- C++ -*-===//
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
/// Provides R600 specific target descriptions.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_R600MCTARGETDESC_H
#define LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_R600MCTARGETDESC_H

#include <cstdint>

namespace vm::core {
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;

MCCodeEmitter *createR600MCCodeEmitter(const MCInstrInfo &MCII,
                                       MCContext &Ctx);
MCInstrInfo *createR600MCInstrInfo();

} // namespace vm::core

#define GET_REGINFO_ENUM
#include "R600GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_SCHED_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "R600GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "R600GenSubtargetInfo.inc"

#endif
