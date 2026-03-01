//===- ARC.h - Top-level interface for ARC representation -------*- C++ -*-===//
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
// This file contains the entry points for global functions defined in the LLVM
// ARC back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARC_ARC_H
#define LLVM_LIB_TARGET_ARC_ARC_H

#include "MCTargetDesc/ARCMCTargetDesc.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {

class ARCTargetMachine;
class FunctionPass;
class PassRegistry;

FunctionPass *createARCISelDag(ARCTargetMachine &TM, CodeGenOptLevel OptLevel);
FunctionPass *createARCExpandPseudosPass();
FunctionPass *createARCOptAddrMode();
FunctionPass *createARCBranchFinalizePass();
void initializeARCAsmPrinterPass(PassRegistry &);
void initializeARCDAGToDAGISelLegacyPass(PassRegistry &);

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARC_ARC_H
