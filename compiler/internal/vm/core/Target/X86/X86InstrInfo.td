//===-- X86InstrInfo.td - Main X86 Instruction Properties --*- tablegen -*-===//
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
// This file describes the X86 properties of the instructions which are needed
// for code generation, machine code emission, and analysis.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// X86 Pattern fragments.
//
include "X86InstrFragments.td"
include "X86InstrFragmentsSIMD.td"

//===----------------------------------------------------------------------===//
// X86 Predicate Definitions.
//
include "X86InstrPredicates.td"

//===----------------------------------------------------------------------===//
// X86 Operand Definitions.
//
include "X86InstrOperands.td"

//===----------------------------------------------------------------------===//
// X86 Instruction Format Definitions.
//
include "X86InstrFormats.td"

//===----------------------------------------------------------------------===//
// X86 Instruction utilities.
//
include "X86InstrUtils.td"

//===----------------------------------------------------------------------===//
// Global ISel
//
include "X86InstrGISel.td"

//===----------------------------------------------------------------------===//
// Subsystems.
//===----------------------------------------------------------------------===//

include "X86InstrMisc.td"
include "X86InstrTBM.td"
include "X86InstrArithmetic.td"
include "X86InstrCMovSetCC.td"
include "X86InstrConditionalCompare.td"
include "X86InstrExtension.td"
include "X86InstrControl.td"
include "X86InstrShiftRotate.td"

// X87 Floating Point Stack.
include "X86InstrFPStack.td"


// FMA - Fused Multiply-Add support (requires FMA)
include "X86InstrFMA.td"

// XOP
include "X86InstrXOP.td"

// SSE, MMX and 3DNow! vector support.
include "X86InstrSSE.td"
include "X86InstrAVX512.td"
include "X86InstrAVX10.td"
include "X86InstrMMX.td"
include "X86Instr3DNow.td"

include "X86InstrVMX.td"
include "X86InstrSVM.td"
include "X86InstrSNP.td"
include "X86InstrTSX.td"
include "X86InstrSGX.td"
include "X86InstrTDX.td"

// Key Locker instructions
include "X86InstrKL.td"

// AMX instructions
include "X86InstrAMX.td"

// RAO-INT instructions
include "X86InstrRAOINT.td"

// System instructions.
include "X86InstrSystem.td"

// Compiler Pseudo Instructions and Pat Patterns
include "X86InstrCompiler.td"
include "X86InstrVecCompiler.td"

// Assembler mnemonic/instruction aliases
include "X86InstrAsmAlias.td"
