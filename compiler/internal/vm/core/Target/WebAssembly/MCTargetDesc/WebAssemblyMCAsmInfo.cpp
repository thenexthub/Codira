//===-- WebAssemblyMCAsmInfo.cpp - WebAssembly asm properties -------------===//
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
/// This file contains the declarations of the WebAssemblyMCAsmInfo
/// properties.
///
//===----------------------------------------------------------------------===//

#include "WebAssemblyMCAsmInfo.h"
#include "WebAssemblyMCTargetDesc.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

#define DEBUG_TYPE "wasm-mc-asm-info"

const MCAsmInfo::AtSpecifier atSpecifiers[] = {
    {WebAssembly::S_TYPEINDEX, "TYPEINDEX"},
    {WebAssembly::S_TBREL, "TBREL"},
    {WebAssembly::S_MBREL, "MBREL"},
    {WebAssembly::S_TLSREL, "TLSREL"},
    {WebAssembly::S_GOT, "GOT"},
    {WebAssembly::S_GOT_TLS, "GOT@TLS"},
    {WebAssembly::S_FUNCINDEX, "FUNCINDEX"},
};

WebAssemblyMCAsmInfo::~WebAssemblyMCAsmInfo() = default; // anchor.

WebAssemblyMCAsmInfo::WebAssemblyMCAsmInfo(const Triple &T,
                                           const MCTargetOptions &Options) {
  CodePointerSize = CalleeSaveStackSlotSize = T.isArch64Bit() ? 8 : 4;

  // TODO: What should MaxInstLength be?

  UseDataRegionDirectives = true;

  // Use .skip instead of .zero because .zero is confusing when used with two
  // arguments (it doesn't actually zero things out).
  ZeroDirective = "\t.skip\t";

  Data8bitsDirective = "\t.int8\t";
  Data16bitsDirective = "\t.int16\t";
  Data32bitsDirective = "\t.int32\t";
  Data64bitsDirective = "\t.int64\t";

  AlignmentIsInBytes = false;
  COMMDirectiveAlignmentIsInBytes = false;
  LCOMMDirectiveAlignmentType = LCOMM::Log2Alignment;

  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::None;

  initializeAtSpecifiers(atSpecifiers);
}
