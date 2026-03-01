//===- ConfigManager.cpp --------------------------------------------------===//
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

#include "vm/core/ObjCopy/ConfigManager.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/Error.h"

using namespace vm::core;
using namespace vm::core::objcopy;

Expected<const ELFConfig &> ConfigManager::getELFConfig() const {
  if (!Common.ExtractSection.empty())
    return createStringError(toolchain::errc::invalid_argument,
                             "option is not supported for ELF");
  return ELF;
}

Expected<const COFFConfig &> ConfigManager::getCOFFConfig() const {
  if (!Common.SplitDWO.empty() || !Common.SymbolsPrefix.empty() ||
      !Common.SymbolsPrefixRemove.empty() || !Common.SymbolsToSkip.empty() ||
      !Common.AllocSectionsPrefix.empty() || !Common.KeepSection.empty() ||
      !Common.SymbolsToGlobalize.empty() || !Common.SymbolsToKeep.empty() ||
      !Common.SymbolsToLocalize.empty() || !Common.SymbolsToWeaken.empty() ||
      !Common.SymbolsToKeepGlobal.empty() || !Common.SectionsToRename.empty() ||
      !Common.SetSectionAlignment.empty() || !Common.SetSectionType.empty() ||
      Common.ExtractDWO || Common.StripDWO || Common.StripNonAlloc ||
      Common.StripSections || Common.Weaken || Common.DecompressDebugSections ||
      Common.DiscardMode == DiscardType::Locals ||
      !Common.SymbolsToAdd.empty() || Common.GapFill != 0 ||
      Common.PadTo != 0 || Common.ChangeSectionLMAValAll != 0 ||
      !Common.ChangeSectionAddress.empty() || !Common.ExtractSection.empty())
    return createStringError(toolchain::errc::invalid_argument,
                             "option is not supported for COFF");

  return COFF;
}

Expected<const MachOConfig &> ConfigManager::getMachOConfig() const {
  if (!Common.SplitDWO.empty() || !Common.SymbolsPrefix.empty() ||
      !Common.SymbolsPrefixRemove.empty() ||
      !Common.AllocSectionsPrefix.empty() || !Common.KeepSection.empty() ||
      !Common.SymbolsToKeep.empty() || !Common.SectionsToRename.empty() ||
      !Common.UnneededSymbolsToRemove.empty() ||
      !Common.SetSectionAlignment.empty() || !Common.SetSectionFlags.empty() ||
      !Common.SetSectionType.empty() || Common.ExtractDWO ||
      Common.PreserveDates || Common.StripAllGNU || Common.StripDWO ||
      Common.StripNonAlloc || Common.StripSections ||
      Common.DecompressDebugSections || Common.StripUnneeded ||
      Common.DiscardMode == DiscardType::Locals ||
      !Common.SymbolsToAdd.empty() || Common.GapFill != 0 ||
      Common.PadTo != 0 || Common.ChangeSectionLMAValAll != 0 ||
      !Common.ChangeSectionAddress.empty() || !Common.ExtractSection.empty())
    return createStringError(toolchain::errc::invalid_argument,
                             "option is not supported for MachO");

  return MachO;
}

Expected<const WasmConfig &> ConfigManager::getWasmConfig() const {
  if (!Common.AddGnuDebugLink.empty() || Common.ExtractPartition ||
      !Common.SplitDWO.empty() || !Common.SymbolsPrefix.empty() ||
      !Common.SymbolsPrefixRemove.empty() || !Common.SymbolsToSkip.empty() ||
      !Common.AllocSectionsPrefix.empty() ||
      Common.DiscardMode != DiscardType::None || !Common.SymbolsToAdd.empty() ||
      !Common.SymbolsToGlobalize.empty() || !Common.SymbolsToLocalize.empty() ||
      !Common.SymbolsToKeep.empty() || !Common.SymbolsToRemove.empty() ||
      !Common.UnneededSymbolsToRemove.empty() ||
      !Common.SymbolsToWeaken.empty() || !Common.SymbolsToKeepGlobal.empty() ||
      !Common.SectionsToRename.empty() || !Common.SetSectionAlignment.empty() ||
      !Common.SetSectionFlags.empty() || !Common.SetSectionType.empty() ||
      !Common.SymbolsToRename.empty() || Common.GapFill != 0 ||
      Common.PadTo != 0 || Common.ChangeSectionLMAValAll != 0 ||
      !Common.ChangeSectionAddress.empty() || !Common.ExtractSection.empty())
    return createStringError(toolchain::errc::invalid_argument,
                             "only flags for section dumping, removal, and "
                             "addition are supported");

  return Wasm;
}

Expected<const XCOFFConfig &> ConfigManager::getXCOFFConfig() const {
  if (!Common.AddGnuDebugLink.empty() || Common.ExtractPartition ||
      !Common.SplitDWO.empty() || !Common.SymbolsPrefix.empty() ||
      !Common.SymbolsPrefixRemove.empty() || !Common.SymbolsToSkip.empty() ||
      !Common.AllocSectionsPrefix.empty() ||
      Common.DiscardMode != DiscardType::None || !Common.AddSection.empty() ||
      !Common.DumpSection.empty() || !Common.SymbolsToAdd.empty() ||
      !Common.KeepSection.empty() || !Common.OnlySection.empty() ||
      !Common.ToRemove.empty() || !Common.SymbolsToGlobalize.empty() ||
      !Common.SymbolsToKeep.empty() || !Common.SymbolsToLocalize.empty() ||
      !Common.SymbolsToRemove.empty() ||
      !Common.UnneededSymbolsToRemove.empty() ||
      !Common.SymbolsToWeaken.empty() || !Common.SymbolsToKeepGlobal.empty() ||
      !Common.SectionsToRename.empty() || !Common.SetSectionAlignment.empty() ||
      !Common.SetSectionFlags.empty() || !Common.SetSectionType.empty() ||
      !Common.SymbolsToRename.empty() || Common.ExtractDWO ||
      Common.ExtractMainPartition || Common.OnlyKeepDebug ||
      Common.PreserveDates || Common.StripAllGNU || Common.StripDWO ||
      Common.StripDebug || Common.StripNonAlloc || Common.StripSections ||
      Common.Weaken || Common.StripUnneeded || Common.DecompressDebugSections ||
      Common.GapFill != 0 || Common.PadTo != 0 ||
      Common.ChangeSectionLMAValAll != 0 ||
      !Common.ChangeSectionAddress.empty() || !Common.ExtractSection.empty()) {
    return createStringError(
        toolchain::errc::invalid_argument,
        "no flags are supported yet, only basic copying is allowed");
  }

  return XCOFF;
}

Expected<const DXContainerConfig &>
ConfigManager::getDXContainerConfig() const {
  // All other flags are either supported or not applicable for DXContainer
  // object files and will be silently ignored.
  if (!Common.AddGnuDebugLink.empty() || !Common.SplitDWO.empty() ||
      !Common.AllocSectionsPrefix.empty() ||
      Common.DiscardMode != DiscardType::None || !Common.AddSection.empty() ||
      !Common.KeepSection.empty() || !Common.SectionsToRename.empty() ||
      !Common.SetSectionAlignment.empty() || !Common.SetSectionFlags.empty() ||
      !Common.SetSectionType.empty() || Common.ExtractDWO ||
      Common.OnlyKeepDebug || Common.StripAllGNU || Common.StripDWO ||
      Common.StripDebug || Common.StripNonAlloc || Common.StripSections ||
      Common.StripUnneeded || Common.DecompressDebugSections ||
      Common.GapFill != 0 || Common.PadTo != 0 ||
      Common.ChangeSectionLMAValAll != 0 ||
      !Common.ChangeSectionAddress.empty()) {
    return createStringError(toolchain::errc::invalid_argument,
                             "option is not supported for DXContainer");
  }
  return DXContainer;
}
