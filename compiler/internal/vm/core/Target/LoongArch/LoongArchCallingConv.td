//=- LoongArchCallingConv.td - Calling Conventions LoongArch -*- tablegen -*-=//
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
// This describes the calling conventions for the LoongArch architecture.
//
//===----------------------------------------------------------------------===//

def CSR_ILP32S_LP64S
    : CalleeSavedRegs<(add R1, (sequence "R%u", 22, 31))>;

def CSR_ILP32F_LP64F
    : CalleeSavedRegs<(add CSR_ILP32S_LP64S, (sequence "F%u", 24, 31))>;

def CSR_ILP32D_LP64D
    : CalleeSavedRegs<(add CSR_ILP32S_LP64S, (sequence "F%u_64", 24, 31))>;

// Needed for implementation of LoongArchRegisterInfo::getNoPreservedMask()
def CSR_NoRegs : CalleeSavedRegs<(add)>;

def CSR_MostRegs : CalleeSavedRegs<(add CSR_ILP32S_LP64S,
                                        (sequence "R%u", 4, 11),
                                        (sequence "R%u", 16, 19))>;
