//===-- NVPTXMCAsmInfo.cpp - NVPTX asm properties -------------------------===//
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
// This file contains the declarations of the NVPTXMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "NVPTXMCAsmInfo.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

NVPTXMCAsmInfo::NVPTXMCAsmInfo(const Triple &TheTriple,
                               const MCTargetOptions &Options) {
  if (TheTriple.getArch() == Triple::nvptx64) {
    CodePointerSize = CalleeSaveStackSlotSize = 8;
  }

  CommentString = "//";

  HasSingleParameterDotFile = false;

  InlineAsmStart = " begin inline asm";
  InlineAsmEnd = " end inline asm";

  SupportsDebugInformation = true;
  // PTX does not allow .align on functions.
  HasFunctionAlignment = false;
  HasDotTypeDotSizeDirective = false;
  // PTX does not allow .hidden or .protected
  HiddenDeclarationVisibilityAttr = HiddenVisibilityAttr = MCSA_Invalid;
  ProtectedVisibilityAttr = MCSA_Invalid;

  Data8bitsDirective = ".b8 ";
  Data16bitsDirective = nullptr; // not supported
  Data32bitsDirective = ".b32 ";
  Data64bitsDirective = ".b64 ";
  ZeroDirective = ".b8";
  AsciiDirective = nullptr; // not supported
  AscizDirective = nullptr; // not supported
  SupportsQuotedNames = false;
  SupportsExtendedDwarfLocDirective = false;
  SupportsSignedData = false;

  PrivateGlobalPrefix = "$L__";
  PrivateLabelPrefix = PrivateGlobalPrefix;

  // @TODO: Can we just disable this?
  WeakDirective = "\t// .weak\t";
  GlobalDirective = "\t// .globl\t";

  UseIntegratedAssembler = false;

  // ptxas does not support DWARF `.file fileno directory filename'
  // syntax as of v11.X.
  EnableDwarfFileDirectoryDefault = false;
}
