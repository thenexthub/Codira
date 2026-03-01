//=== ARMCallingConv.h - ARM Custom Calling Convention Routines -*- C++ -*-===//
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
// This file declares the entry points for ARM calling convention analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_ARMCALLINGCONV_H
#define LLVM_LIB_TARGET_ARM_ARMCALLINGCONV_H

#include "vm/core/CodeGen/CallingConvLower.h"

namespace vm::core {

bool CC_ARM_AAPCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                  CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                  Type *OrigTy, CCState &State);
bool CC_ARM_AAPCS_VFP(unsigned ValNo, MVT ValVT, MVT LocVT,
                      CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                      Type *OrigTy, CCState &State);
bool CC_ARM_APCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                 CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                 Type *OrigTy, CCState &State);
bool CC_ARM_APCS_GHC(unsigned ValNo, MVT ValVT, MVT LocVT,
                     CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                     Type *OrigTy, CCState &State);
bool FastCC_ARM_APCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                     CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                     Type *OrigTy, CCState &State);
bool CC_ARM_Win32_CFGuard_Check(unsigned ValNo, MVT ValVT, MVT LocVT,
                                CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                                CCState &State);
bool RetCC_ARM_AAPCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                     CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                     Type *OrigTy, CCState &State);
bool RetCC_ARM_AAPCS_VFP(unsigned ValNo, MVT ValVT, MVT LocVT,
                         CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                         Type *OrigTy, CCState &State);
bool RetCC_ARM_APCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                    CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                    Type *OrigTy, CCState &State);
bool RetFastCC_ARM_APCS(unsigned ValNo, MVT ValVT, MVT LocVT,
                        CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                        Type *OrigTy, CCState &State);

} // namespace vm::core

#endif
