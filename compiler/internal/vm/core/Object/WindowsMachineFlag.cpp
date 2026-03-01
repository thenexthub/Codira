//===- WindowsMachineFlag.cpp ---------------------------------------------===//
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
// Functions for implementing the /machine: flag.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Object/WindowsMachineFlag.h"

#include "vm/core/ADT/StringRef.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/BinaryFormat/COFF.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

// Returns /machine's value.
COFF::MachineTypes toolchain::getMachineType(StringRef S) {
  // Flags must be a superset of Microsoft lib.exe /machine flags.
  return StringSwitch<COFF::MachineTypes>(S.lower())
      .Cases({"x64", "amd64"}, COFF::IMAGE_FILE_MACHINE_AMD64)
      .Cases({"x86", "i386"}, COFF::IMAGE_FILE_MACHINE_I386)
      .Case("arm", COFF::IMAGE_FILE_MACHINE_ARMNT)
      .Case("arm64", COFF::IMAGE_FILE_MACHINE_ARM64)
      .Case("arm64ec", COFF::IMAGE_FILE_MACHINE_ARM64EC)
      .Case("arm64x", COFF::IMAGE_FILE_MACHINE_ARM64X)
      .Case("mips", COFF::IMAGE_FILE_MACHINE_R4000)
      .Default(COFF::IMAGE_FILE_MACHINE_UNKNOWN);
}

StringRef toolchain::machineToStr(COFF::MachineTypes MT) {
  switch (MT) {
  case COFF::IMAGE_FILE_MACHINE_ARMNT:
    return "arm";
  case COFF::IMAGE_FILE_MACHINE_ARM64:
    return "arm64";
  case COFF::IMAGE_FILE_MACHINE_ARM64EC:
    return "arm64ec";
  case COFF::IMAGE_FILE_MACHINE_ARM64X:
    return "arm64x";
  case COFF::IMAGE_FILE_MACHINE_AMD64:
    return "x64";
  case COFF::IMAGE_FILE_MACHINE_I386:
    return "x86";
  default:
    llvm_unreachable("unknown machine type");
  }
}
