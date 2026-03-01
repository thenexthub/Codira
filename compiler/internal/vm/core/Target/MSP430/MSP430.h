//==-- MSP430.h - Top-level interface for MSP430 representation --*- C++ -*-==//
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
// This file contains the entry points for global functions defined in
// the LLVM MSP430 backend.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MSP430_MSP430_H
#define LLVM_LIB_TARGET_MSP430_MSP430_H

#include "MCTargetDesc/MSP430MCTargetDesc.h"
#include "vm/core/Target/TargetMachine.h"

namespace MSP430CC {
  // MSP430 specific condition code.
  enum CondCodes {
    COND_E  = 0,  // aka COND_Z
    COND_NE = 1,  // aka COND_NZ
    COND_HS = 2,  // aka COND_C
    COND_LO = 3,  // aka COND_NC
    COND_GE = 4,
    COND_L  = 5,
    COND_N  = 6,  // jump if negative
    COND_NONE,    // unconditional

    COND_INVALID = -1
  };
}

namespace vm::core {
class FunctionPass;
class MSP430TargetMachine;
class PassRegistry;

FunctionPass *createMSP430ISelDag(MSP430TargetMachine &TM,
                                  CodeGenOptLevel OptLevel);

FunctionPass *createMSP430BranchSelectionPass();

void initializeMSP430AsmPrinterPass(PassRegistry &);
void initializeMSP430DAGToDAGISelLegacyPass(PassRegistry &);

} // namespace vm::core

#endif
