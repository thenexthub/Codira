//===- Target.cpp -----------------------------------------------*- C++ -*-===//
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

#include "vm/core/TextAPI/Target.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/raw_ostream.h"

namespace vm::core {
namespace MachO {

Expected<Target> Target::create(StringRef TargetValue) {
  auto Result = TargetValue.split('-');
  auto ArchitectureStr = Result.first;
  auto Architecture = getArchitectureFromName(ArchitectureStr);
  auto PlatformStr = Result.second;
  PlatformType Platform;
  Platform = StringSwitch<PlatformType>(PlatformStr)
#define PLATFORM(platform, id, name, build_name, target, tapi_target,          \
                 marketing)                                                    \
  .Case(#tapi_target, PLATFORM_##platform)
#include "vm/core/BinaryFormat/MachO.def"
                 .Default(PLATFORM_UNKNOWN);

  if (Platform == PLATFORM_UNKNOWN) {
    if (PlatformStr.starts_with("<") && PlatformStr.ends_with(">")) {
      PlatformStr = PlatformStr.drop_front().drop_back();
      unsigned long long RawValue;
      if (!PlatformStr.getAsInteger(10, RawValue))
        Platform = (PlatformType)RawValue;
    }
  }

  return Target{Architecture, Platform};
}

Target::operator std::string() const {
  auto Version = MinDeployment.empty() ? "" : MinDeployment.getAsString();

  return (getArchitectureName(Arch) + " (" + getPlatformName(Platform) +
          Version + ")")
      .str();
}

raw_ostream &operator<<(raw_ostream &OS, const Target &Target) {
  OS << std::string(Target);
  return OS;
}

PlatformVersionSet mapToPlatformVersionSet(ArrayRef<Target> Targets) {
  PlatformVersionSet Result;
  for (const auto &Target : Targets)
    Result.insert({Target.Platform, Target.MinDeployment});
  return Result;
}

PlatformSet mapToPlatformSet(ArrayRef<Target> Targets) {
  PlatformSet Result;
  for (const auto &Target : Targets)
    Result.insert(Target.Platform);
  return Result;
}

ArchitectureSet mapToArchitectureSet(ArrayRef<Target> Targets) {
  ArchitectureSet Result;
  for (const auto &Target : Targets)
    Result.set(Target.Arch);
  return Result;
}

std::string getTargetTripleName(const Target &Targ) {
  auto Version =
      Targ.MinDeployment.empty() ? "" : Targ.MinDeployment.getAsString();

  return (getArchitectureName(Targ.Arch) + "-apple-" +
          getOSAndEnvironmentName(Targ.Platform, Version))
      .str();
}

} // end namespace MachO.
} // end namespace vm::core.
