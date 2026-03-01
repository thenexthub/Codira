//===-- NVPTXBaseInfo.h - Top-level definitions for NVPTX -------*- C++ -*-===//
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
// This file contains small standalone helper functions and enum definitions for
// the NVPTX target useful for the compiler back-end and the MC libraries.
// As such, it deliberately does not include references to LLVM core
// code gen types, passes, etc..
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NVPTX_MCTARGETDESC_NVPTXBASEINFO_H
#define LLVM_LIB_TARGET_NVPTX_MCTARGETDESC_NVPTXBASEINFO_H

#include "vm/core/Support/NVPTXAddrSpace.h"
namespace vm::core {

using namespace NVPTXAS;

namespace NVPTXII {
enum {
  // These must be kept in sync with TSFlags in NVPTXInstrFormats.td
  // clang-format off
  IsTexFlag            =  0x40,
  IsSuldMask           = 0x180,
  IsSuldShift          =   0x7,
  IsSustFlag           = 0x200,
  IsSurfTexQueryFlag   = 0x400,
  IsTexModeUnifiedFlag = 0x800,
  // clang-format on
};
} // namespace NVPTXII

} // namespace vm::core
#endif
