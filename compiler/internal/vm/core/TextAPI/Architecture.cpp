//===- Architecture.cpp ---------------------------------------------------===//
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
// Implements the architecture helper functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/TextAPI/Architecture.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/BinaryFormat/MachO.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {
namespace MachO {

Architecture getArchitectureFromCpuType(uint32_t CPUType, uint32_t CPUSubType) {
#define ARCHINFO(Arch, Name, Type, Subtype, NumBits)                           \
  if (CPUType == (Type) &&                                                     \
      (CPUSubType & ~MachO::CPU_SUBTYPE_MASK) == (Subtype))                    \
    return AK_##Arch;
#include "vm/core/TextAPI/Architecture.def"
#undef ARCHINFO

  return AK_unknown;
}

Architecture getArchitectureFromName(StringRef Name) {
  return StringSwitch<Architecture>(Name)
#define ARCHINFO(Arch, Name, Type, Subtype, NumBits) .Case(#Name, AK_##Arch)
#include "vm/core/TextAPI/Architecture.def"
#undef ARCHINFO
      .Default(AK_unknown);
}

StringRef getArchitectureName(Architecture Arch) {
  switch (Arch) {
#define ARCHINFO(Arch, Name, Type, Subtype, NumBits)                           \
  case AK_##Arch:                                                              \
    return #Name;
#include "vm/core/TextAPI/Architecture.def"
#undef ARCHINFO
  case AK_unknown:
    return "unknown";
  }

  // Appease some compilers that cannot figure out that this is a fully covered
  // switch statement.
  return "unknown";
}

std::pair<uint32_t, uint32_t> getCPUTypeFromArchitecture(Architecture Arch) {
  switch (Arch) {
#define ARCHINFO(Arch, Name, Type, Subtype, NumBits)                           \
  case AK_##Arch:                                                              \
    return std::make_pair(Type, Subtype);
#include "vm/core/TextAPI/Architecture.def"
#undef ARCHINFO
  case AK_unknown:
    return std::make_pair(0, 0);
  }

  // Appease some compilers that cannot figure out that this is a fully covered
  // switch statement.
  return std::make_pair(0, 0);
}

Architecture mapToArchitecture(const Triple &Target) {
  return getArchitectureFromName(Target.getArchName());
}

bool is64Bit(Architecture Arch) {
  switch (Arch) {
#define ARCHINFO(Arch, Name, Type, Subtype, NumBits)                           \
  case AK_##Arch:                                                              \
    return NumBits == 64;
#include "vm/core/TextAPI/Architecture.def"
#undef ARCHINFO
  case AK_unknown:
    return false;
  }

  llvm_unreachable("Fully handled switch case above.");
}

raw_ostream &operator<<(raw_ostream &OS, Architecture Arch) {
  OS << getArchitectureName(Arch);
  return OS;
}

} // end namespace MachO.
} // end namespace vm::core.
