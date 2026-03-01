//===-- RISCVCallingConv.h - RISC-V Custom CC Routines ----------*- C++ -*-===//
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
// This file defines the custom routines for the RISC-V Calling Convention.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/RISCVBaseInfo.h"
#include "vm/core/CodeGen/CallingConvLower.h"

namespace vm::core {

/// RISCVCCAssignFn - This target-specific function extends the default
/// CCValAssign with additional information used to lower RISC-V calling
/// conventions.
typedef bool RISCVCCAssignFn(unsigned ValNo, MVT ValVT, MVT LocVT,
                             CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State,
                             bool IsRet, Type *OrigTy);

bool CC_RISCV(unsigned ValNo, MVT ValVT, MVT LocVT,
              CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
              CCState &State, bool IsRet, Type *OrigTy);

bool CC_RISCV_FastCC(unsigned ValNo, MVT ValVT, MVT LocVT,
                     CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                     CCState &State, bool IsRet, Type *OrigTy);

bool CC_RISCV_GHC(unsigned ValNo, MVT ValVT, MVT LocVT,
                  CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                  Type *OrigTy, CCState &State);

namespace RISCV {

ArrayRef<MCPhysReg> getArgGPRs(const RISCVABI::ABI ABI);

} // end namespace RISCV

} // end namespace vm::core
