//=== AArch64CallingConvention.h - AArch64 CC entry points ------*- C++ -*-===//
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
// This file declares the entry points for AArch64 calling convention analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64CALLINGCONVENTION_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64CALLINGCONVENTION_H

#include "vm/core/CodeGen/CallingConvLower.h"

namespace vm::core {
bool CC_AArch64_AAPCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                      CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                      Type *OrigTy, CCState &State);
bool CC_AArch64_Arm64EC_VarArg(unsigned ValNo, MVT ValVT, MVT LocVT,
                               CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                               CCState &State);
bool CC_AArch64_Arm64EC_Thunk(unsigned ValNo, MVT ValVT, MVT LocVT,
                              CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                              CCState &State);
bool CC_AArch64_Arm64EC_Thunk_Native(unsigned ValNo, MVT ValVT, MVT LocVT,
                                     CCValAssign::LocInfo LocInfo,
                                     ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                                     CCState &State);
bool CC_AArch64_DarwinPCS_VarArg(unsigned ValNo, MVT ValVT, MVT LocVT,
                                 CCValAssign::LocInfo LocInfo,
                                 ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                                 CCState &State);
bool CC_AArch64_DarwinPCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                          CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                          CCState &State);
bool CC_AArch64_DarwinPCS_ILP32_VarArg(unsigned ValNo, MVT ValVT, MVT LocVT,
                                       CCValAssign::LocInfo LocInfo,
                                       ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                                       CCState &State);
bool CC_AArch64_Win64PCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                         CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                         Type *OrigTy, CCState &State);
bool CC_AArch64_Win64_VarArg(unsigned ValNo, MVT ValVT, MVT LocVT,
                             CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                             CCState &State);
bool CC_AArch64_Win64_CFGuard_Check(unsigned ValNo, MVT ValVT, MVT LocVT,
                                    CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                                    CCState &State);
bool CC_AArch64_Arm64EC_CFGuard_Check(unsigned ValNo, MVT ValVT, MVT LocVT,
                                      CCValAssign::LocInfo LocInfo,
                                      ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                                      CCState &State);
bool CC_AArch64_GHC(unsigned ValNo, MVT ValVT, MVT LocVT,
                    CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                    Type *OrigTy, CCState &State);
bool CC_AArch64_Preserve_None(unsigned ValNo, MVT ValVT, MVT LocVT,
                              CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                              CCState &State);
bool RetCC_AArch64_AAPCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                         CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                         Type *OrigTy, CCState &State);
bool RetCC_AArch64_Arm64EC_Thunk(unsigned ValNo, MVT ValVT, MVT LocVT,
                                 CCValAssign::LocInfo LocInfo,
                                 ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                                 CCState &State);
bool RetCC_AArch64_Arm64EC_CFGuard_Check(unsigned ValNo, MVT ValVT, MVT LocVT,
                                         CCValAssign::LocInfo LocInfo,
                                         ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                                         CCState &State);
} // namespace vm::core

#endif
