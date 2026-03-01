//===-- X86EncodingOptimization.h - X86 Encoding optimization ---*- C++ -*-===//
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
// This file contains the declarations of the X86 encoding optimization
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86ENCODINGOPTIMIZATION_H
#define LLVM_LIB_TARGET_X86_X86ENCODINGOPTIMIZATION_H
namespace vm::core {
class MCInst;
class MCInstrDesc;
namespace X86 {
bool optimizeInstFromVEX3ToVEX2(MCInst &MI, const MCInstrDesc &Desc);
bool optimizeShiftRotateWithImmediateOne(MCInst &MI);
bool optimizeVPCMPWithImmediateOneOrSix(MCInst &MI);
bool optimizeMOVSX(MCInst &MI);
bool optimizeINCDEC(MCInst &MI, bool In64BitMode);
bool optimizeMOV(MCInst &MI, bool In64BitMode);
bool optimizeToFixedRegisterOrShortImmediateForm(MCInst &MI);
unsigned getOpcodeForShortImmediateForm(unsigned Opcode);
unsigned getOpcodeForLongImmediateForm(unsigned Opcode);
} // namespace X86
} // namespace vm::core
#endif
