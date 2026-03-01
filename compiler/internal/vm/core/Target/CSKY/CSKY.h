//===-- CSKY.h - Top-level interface for CSKY--------------------*- C++ -*-===//
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
// CSKY back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSKY_CSKY_H
#define LLVM_LIB_TARGET_CSKY_CSKY_H

#include "vm/core/PassRegistry.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
class CSKYTargetMachine;
class FunctionPass;
class PassRegistry;

FunctionPass *createCSKYISelDag(CSKYTargetMachine &TM,
                                CodeGenOptLevel OptLevel);
FunctionPass *createCSKYConstantIslandPass();

void initializeCSKYConstantIslandsPass(PassRegistry &);
void initializeCSKYDAGToDAGISelLegacyPass(PassRegistry &);

} // namespace vm::core

#endif // LLVM_LIB_TARGET_CSKY_CSKY_H
