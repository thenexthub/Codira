//=== DWARFLinkerBase.cpp -------------------------------------------------===//
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

#include "vm/core/DWARFLinker/DWARFLinkerBase.h"
#include "vm/core/ADT/StringSwitch.h"

using namespace vm::core;
using namespace vm::core::dwarf_linker;

std::optional<DebugSectionKind>
toolchain::dwarf_linker::parseDebugTableName(toolchain::StringRef SecName) {
  return toolchain::StringSwitch<std::optional<DebugSectionKind>>(
             SecName.substr(SecName.find_first_not_of("._")))
      .Case(getSectionName(DebugSectionKind::DebugInfo),
            DebugSectionKind::DebugInfo)
      .Case(getSectionName(DebugSectionKind::DebugLine),
            DebugSectionKind::DebugLine)
      .Case(getSectionName(DebugSectionKind::DebugFrame),
            DebugSectionKind::DebugFrame)
      .Case(getSectionName(DebugSectionKind::DebugRange),
            DebugSectionKind::DebugRange)
      .Case(getSectionName(DebugSectionKind::DebugRngLists),
            DebugSectionKind::DebugRngLists)
      .Case(getSectionName(DebugSectionKind::DebugLoc),
            DebugSectionKind::DebugLoc)
      .Case(getSectionName(DebugSectionKind::DebugLocLists),
            DebugSectionKind::DebugLocLists)
      .Case(getSectionName(DebugSectionKind::DebugARanges),
            DebugSectionKind::DebugARanges)
      .Case(getSectionName(DebugSectionKind::DebugAbbrev),
            DebugSectionKind::DebugAbbrev)
      .Case(getSectionName(DebugSectionKind::DebugMacinfo),
            DebugSectionKind::DebugMacinfo)
      .Case(getSectionName(DebugSectionKind::DebugMacro),
            DebugSectionKind::DebugMacro)
      .Case(getSectionName(DebugSectionKind::DebugAddr),
            DebugSectionKind::DebugAddr)
      .Case(getSectionName(DebugSectionKind::DebugStr),
            DebugSectionKind::DebugStr)
      .Case(getSectionName(DebugSectionKind::DebugLineStr),
            DebugSectionKind::DebugLineStr)
      .Case(getSectionName(DebugSectionKind::DebugStrOffsets),
            DebugSectionKind::DebugStrOffsets)
      .Case(getSectionName(DebugSectionKind::DebugPubNames),
            DebugSectionKind::DebugPubNames)
      .Case(getSectionName(DebugSectionKind::DebugPubTypes),
            DebugSectionKind::DebugPubTypes)
      .Case(getSectionName(DebugSectionKind::DebugNames),
            DebugSectionKind::DebugNames)
      .Case(getSectionName(DebugSectionKind::AppleNames),
            DebugSectionKind::AppleNames)
      .Case(getSectionName(DebugSectionKind::AppleNamespaces),
            DebugSectionKind::AppleNamespaces)
      .Case(getSectionName(DebugSectionKind::AppleObjC),
            DebugSectionKind::AppleObjC)
      .Case(getSectionName(DebugSectionKind::AppleTypes),
            DebugSectionKind::AppleTypes)
      .Default(std::nullopt);
}
