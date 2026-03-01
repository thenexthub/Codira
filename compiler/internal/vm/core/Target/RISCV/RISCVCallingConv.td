//===-- RISCVCallingConv.td - Calling Conventions RISC-V ---*- tablegen -*-===//
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
// This describes the calling conventions for the RISC-V architecture.
//
//===----------------------------------------------------------------------===//

// The RISC-V calling convention is handled with custom code in
// RISCVISelLowering.cpp (CC_RISCV).

def CSR_ILP32E_LP64E : CalleeSavedRegs<(add X1, X8, X9)>;

def CSR_ILP32_LP64
    : CalleeSavedRegs<(add CSR_ILP32E_LP64E, (sequence "X%u", 18, 27))>;

def CSR_ILP32F_LP64F
    : CalleeSavedRegs<(add CSR_ILP32_LP64,
                       F8_F, F9_F, (sequence "F%u_F", 18, 27))>;

def CSR_ILP32D_LP64D
    : CalleeSavedRegs<(add CSR_ILP32_LP64,
                       F8_D, F9_D, (sequence "F%u_D", 18, 27))>;

defvar CSR_V = (add (sequence "V%u", 1, 7), (sequence "V%u", 24, 31),
                     V2M2, V4M2, V6M2, V24M2, V26M2, V28M2, V30M2,
                     V4M4, V24M4, V28M4, V24M8);

def CSR_ILP32_LP64_V
    : CalleeSavedRegs<(add CSR_ILP32_LP64, CSR_V)>;

def CSR_ILP32F_LP64F_V
    : CalleeSavedRegs<(add CSR_ILP32F_LP64F, CSR_V)>;

def CSR_ILP32D_LP64D_V
    : CalleeSavedRegs<(add CSR_ILP32D_LP64D, CSR_V)>;

// Needed for implementation of RISCVRegisterInfo::getNoPreservedMask()
def CSR_NoRegs : CalleeSavedRegs<(add)>;

def CSR_IPRA : CalleeSavedRegs<(add X1)>;

// Interrupt handler needs to save/restore all registers that are used,
// both Caller and Callee saved registers.
def CSR_Interrupt : CalleeSavedRegs<(add X1, (sequence "X%u", 5, 31))>;

// Same as CSR_Interrupt, but including all 32-bit FP registers.
def CSR_XLEN_F32_Interrupt: CalleeSavedRegs<(add CSR_Interrupt,
                                             (sequence "F%u_F", 0, 31))>;

// Same as CSR_Interrupt, but including all 64-bit FP registers.
def CSR_XLEN_F64_Interrupt: CalleeSavedRegs<(add CSR_Interrupt,
                                             (sequence "F%u_D", 0, 31))>;

defvar VREGS = (add (sequence "V%u", 0, 31),
                    (sequence "V%uM2", 0, 31, 2),
                    (sequence "V%uM4", 0, 31, 4),
                    (sequence "V%uM8", 0, 31, 8));

// Same as CSR_Interrupt, but including all vector registers.
def CSR_XLEN_V_Interrupt: CalleeSavedRegs<(add CSR_Interrupt, VREGS)>;

// Same as CSR_Interrupt, but including all 32-bit FP registers and all vector
// registers.
def CSR_XLEN_F32_V_Interrupt: CalleeSavedRegs<(add CSR_XLEN_F32_Interrupt, VREGS)>;

// Same as CSR_Interrupt, but including all 64-bit FP registers and all vector
// registers.
def CSR_XLEN_F64_V_Interrupt: CalleeSavedRegs<(add CSR_XLEN_F64_Interrupt, VREGS)>;

// Same as CSR_Interrupt, but excluding X16-X31.
def CSR_Interrupt_RVE : CalleeSavedRegs<(sub CSR_Interrupt,
                                        (sequence "X%u", 16, 31))>;

// Same as CSR_XLEN_F32_Interrupt, but excluding X16-X31.
def CSR_XLEN_F32_Interrupt_RVE: CalleeSavedRegs<(sub CSR_XLEN_F32_Interrupt,
                                                 (sequence "X%u", 16, 31))>;

// Same as CSR_XLEN_F64_Interrupt, but excluding X16-X31.
def CSR_XLEN_F64_Interrupt_RVE: CalleeSavedRegs<(sub CSR_XLEN_F64_Interrupt,
                                                 (sequence "X%u", 16, 31))>;

// Same as CSR_XLEN_V_Interrupt, but excluding X16-X31.
def CSR_XLEN_V_Interrupt_RVE: CalleeSavedRegs<(sub CSR_XLEN_V_Interrupt,
                                               (sequence "X%u", 16, 31))>;

// Same as CSR_XLEN_F32_V_Interrupt, but excluding X16-X31.
def CSR_XLEN_F32_V_Interrupt_RVE: CalleeSavedRegs<(sub CSR_XLEN_F32_V_Interrupt,
                                                   (sequence "X%u", 16, 31))>;

// Same as CSR_XLEN_F64_V_Interrupt, but excluding X16-X31.
def CSR_XLEN_F64_V_Interrupt_RVE: CalleeSavedRegs<(sub CSR_XLEN_F64_V_Interrupt,
                                                   (sequence "X%u", 16, 31))>;

def CSR_RT_MostRegs : CalleeSavedRegs<(sub CSR_Interrupt, X6, X7, X28)>;
def CSR_RT_MostRegs_RVE : CalleeSavedRegs<(sub CSR_RT_MostRegs,
                                               (sequence "X%u", 16, 31))>;
