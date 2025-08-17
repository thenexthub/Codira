//===- DiagnosticBuilderWrappers.cpp ----------------------------*- C++-*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "DiagnosticBuilderWrappers.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/TextAPI/Platform.h"

using language::Core::DiagnosticBuilder;

namespace toolchain {
namespace MachO {
const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                    const Architecture &Arch) {
  DB.AddString(getArchitectureName(Arch));
  return DB;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                    const ArchitectureSet &ArchSet) {
  DB.AddString(std::string(ArchSet));
  return DB;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                    const PlatformType &Platform) {
  DB.AddString(getPlatformName(Platform));
  return DB;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                    const PlatformVersionSet &Platforms) {
  std::string PlatformAsString;
  raw_string_ostream Stream(PlatformAsString);

  Stream << "[ ";
  toolchain::interleaveComma(
      Platforms, Stream,
      [&Stream](const std::pair<PlatformType, VersionTuple> &PV) {
        Stream << getPlatformName(PV.first);
        if (!PV.second.empty())
          Stream << PV.second.getAsString();
      });
  Stream << " ]";
  DB.AddString(PlatformAsString);
  return DB;
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                    const FileType &Type) {
  switch (Type) {
  case FileType::MachO_Bundle:
    DB.AddString("mach-o bundle");
    return DB;
  case FileType::MachO_DynamicLibrary:
    DB.AddString("mach-o dynamic library");
    return DB;
  case FileType::MachO_DynamicLibrary_Stub:
    DB.AddString("mach-o dynamic library stub");
    return DB;
  case FileType::TBD_V1:
    DB.AddString("tbd-v1");
    return DB;
  case FileType::TBD_V2:
    DB.AddString("tbd-v2");
    return DB;
  case FileType::TBD_V3:
    DB.AddString("tbd-v3");
    return DB;
  case FileType::TBD_V4:
    DB.AddString("tbd-v4");
    return DB;
  case FileType::TBD_V5:
    DB.AddString("tbd-v5");
    return DB;
  case FileType::Invalid:
  case FileType::All:
    break;
  }
  toolchain_unreachable("Unexpected file type for diagnostics.");
}

const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                    const PackedVersion &Version) {
  std::string VersionString;
  raw_string_ostream OS(VersionString);
  OS << Version;
  DB.AddString(VersionString);
  return DB;
}

const language::Core::DiagnosticBuilder &
operator<<(const language::Core::DiagnosticBuilder &DB,
           const language::Core::installapi::LibAttrs::Entry &LibAttr) {
  std::string Entry;
  raw_string_ostream OS(Entry);

  OS << LibAttr.first << " [ " << LibAttr.second << " ]";
  DB.AddString(Entry);
  return DB;
}

} // namespace MachO
} // namespace toolchain
