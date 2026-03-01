//===-- DirectXSubtarget.cpp - DirectX Subtarget Information --------------===//
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
///
/// \file
/// This file implements the DirectX-specific subclass of TargetSubtarget.
///
//===----------------------------------------------------------------------===//

#include "DirectXSubtarget.h"
#include "DirectXTargetLowering.h"

using namespace vm::core;

#define DEBUG_TYPE "directx-subtarget"

#define GET_SUBTARGETINFO_CTOR
#define GET_SUBTARGETINFO_TARGET_DESC
#include "DirectXGenSubtargetInfo.inc"

DirectXSubtarget::DirectXSubtarget(const Triple &TT, StringRef CPU,
                                   StringRef FS, const DirectXTargetMachine &TM)
    : DirectXGenSubtargetInfo(TT, CPU, CPU, FS), InstrInfo(*this), FL(*this),
      TL(TM, *this) {}

void DirectXSubtarget::anchor() {}
