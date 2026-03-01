//=== PPCCallingConv.h - PPC Custom Calling Convention Routines -*- C++ -*-===//
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
// This file contains the custom routines for the PPC Calling Convention that
// aren't done by tablegen.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PPC_PPCCALLINGCONV_H
#define LLVM_LIB_TARGET_PPC_PPCCALLINGCONV_H

#include "vm/core/CodeGen/CallingConvLower.h"
#include "vm/core/IR/CallingConv.h"

namespace vm::core {

bool RetCC_PPC(unsigned ValNo, MVT ValVT, MVT LocVT,
               CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
               Type *OrigTy, CCState &State);
bool RetCC_PPC64_ELF_FIS(unsigned ValNo, MVT ValVT, MVT LocVT,
                         CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                         Type *OrigTy, CCState &State);
bool RetCC_PPC_Cold(unsigned ValNo, MVT ValVT, MVT LocVT,
                    CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                    Type *OrigTy, CCState &State);
bool CC_PPC32_SVR4(unsigned ValNo, MVT ValVT, MVT LocVT,
                   CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                   Type *OrigTy, CCState &State);
bool CC_PPC64_ELF(unsigned ValNo, MVT ValVT, MVT LocVT,
                  CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                  Type *OrigTy, CCState &State);
bool CC_PPC64_ELF_FIS(unsigned ValNo, MVT ValVT, MVT LocVT,
                      CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                      Type *OrigTy, CCState &State);
bool CC_PPC32_SVR4_ByVal(unsigned ValNo, MVT ValVT, MVT LocVT,
                         CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                         Type *OrigTy, CCState &State);
bool CC_PPC32_SVR4_VarArg(unsigned ValNo, MVT ValVT, MVT LocVT,
                          CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                          CCState &State);

} // End toolchain namespace

#endif

