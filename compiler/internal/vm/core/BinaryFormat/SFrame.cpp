//===-- SFrame.cpp -----------------------------------------------*- C++-*-===//
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

#include "vm/core/BinaryFormat/SFrame.h"
#include "vm/core/Support/ScopedPrinter.h"

using namespace vm::core;

ArrayRef<EnumEntry<sframe::Version>> sframe::getVersions() {
  static constexpr EnumEntry<Version> Versions[] = {
#define HANDLE_SFRAME_VERSION(CODE, NAME) {#NAME, sframe::Version::NAME},
#include "vm/core/BinaryFormat/SFrameConstants.def"
  };

  return ArrayRef(Versions);
}

ArrayRef<EnumEntry<sframe::Flags>> sframe::getFlags() {
  static constexpr EnumEntry<sframe::Flags> Flags[] = {
#define HANDLE_SFRAME_FLAG(CODE, NAME) {#NAME, sframe::Flags::NAME},
#include "vm/core/BinaryFormat/SFrameConstants.def"
  };
  return ArrayRef(Flags);
}

ArrayRef<EnumEntry<sframe::ABI>> sframe::getABIs() {
  static constexpr EnumEntry<sframe::ABI> ABIs[] = {
#define HANDLE_SFRAME_ABI(CODE, NAME) {#NAME, sframe::ABI::NAME},
#include "vm/core/BinaryFormat/SFrameConstants.def"
  };
  return ArrayRef(ABIs);
}

ArrayRef<EnumEntry<sframe::FREType>> sframe::getFRETypes() {
  static constexpr EnumEntry<sframe::FREType> FRETypes[] = {
#define HANDLE_SFRAME_FRE_TYPE(CODE, NAME) {#NAME, sframe::FREType::NAME},
#include "vm/core/BinaryFormat/SFrameConstants.def"
  };
  return ArrayRef(FRETypes);
}

ArrayRef<EnumEntry<sframe::FDEType>> sframe::getFDETypes() {
  static constexpr EnumEntry<sframe::FDEType> FDETypes[] = {
#define HANDLE_SFRAME_FDE_TYPE(CODE, NAME) {#NAME, sframe::FDEType::NAME},
#include "vm/core/BinaryFormat/SFrameConstants.def"
  };
  return ArrayRef(FDETypes);
}

ArrayRef<EnumEntry<sframe::AArch64PAuthKey>> sframe::getAArch64PAuthKeys() {
  static constexpr EnumEntry<sframe::AArch64PAuthKey> AArch64PAuthKeys[] = {
#define HANDLE_SFRAME_AARCH64_PAUTH_KEY(CODE, NAME)                            \
  {#NAME, sframe::AArch64PAuthKey::NAME},
#include "vm/core/BinaryFormat/SFrameConstants.def"
  };
  return ArrayRef(AArch64PAuthKeys);
}

ArrayRef<EnumEntry<sframe::FREOffset>> sframe::getFREOffsets() {
  static constexpr EnumEntry<sframe::FREOffset> FREOffsets[] = {
#define HANDLE_SFRAME_FRE_OFFSET(CODE, NAME) {#NAME, sframe::FREOffset::NAME},
#include "vm/core/BinaryFormat/SFrameConstants.def"
  };
  return ArrayRef(FREOffsets);
}

ArrayRef<EnumEntry<sframe::BaseReg>> sframe::getBaseRegisters() {
  static constexpr EnumEntry<sframe::BaseReg> BaseRegs[] = {
      {"FP", sframe::BaseReg::FP},
      {"SP", sframe::BaseReg::SP},
  };
  return ArrayRef(BaseRegs);
}
