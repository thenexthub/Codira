//===-- XCoreMCTargetDesc.h - XCore Target Descriptions ---------*- C++ -*-===//
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
// This file provides XCore specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XCORE_MCTARGETDESC_XCOREMCTARGETDESC_H
#define LLVM_LIB_TARGET_XCORE_MCTARGETDESC_XCOREMCTARGETDESC_H

// Defines symbolic names for XCore registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "XCoreGenRegisterInfo.inc"

// Defines symbolic names for the XCore instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "XCoreGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "XCoreGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_XCORE_MCTARGETDESC_XCOREMCTARGETDESC_H
