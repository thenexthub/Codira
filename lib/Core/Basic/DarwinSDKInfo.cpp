//===--- DarwinSDKInfo.cpp - SDK Information parser for darwin - ----------===//
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

#include "language/Core/Basic/DarwinSDKInfo.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/ErrorOr.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include <optional>

using namespace language::Core;

std::optional<VersionTuple> DarwinSDKInfo::RelatedTargetVersionMapping::map(
    const VersionTuple &Key, const VersionTuple &MinimumValue,
    std::optional<VersionTuple> MaximumValue) const {
  if (Key < MinimumKeyVersion)
    return MinimumValue;
  if (Key > MaximumKeyVersion)
    return MaximumValue;
  auto KV = Mapping.find(Key.normalize());
  if (KV != Mapping.end())
    return KV->getSecond();
  // If no exact entry found, try just the major key version. Only do so when
  // a minor version number is present, to avoid recursing indefinitely into
  // the major-only check.
  if (Key.getMinor())
    return map(VersionTuple(Key.getMajor()), MinimumValue, MaximumValue);
  // If this a major only key, return std::nullopt for a missing entry.
  return std::nullopt;
}

std::optional<DarwinSDKInfo::RelatedTargetVersionMapping>
DarwinSDKInfo::RelatedTargetVersionMapping::parseJSON(
    const toolchain::json::Object &Obj, VersionTuple MaximumDeploymentTarget) {
  VersionTuple Min = VersionTuple(std::numeric_limits<unsigned>::max());
  VersionTuple Max = VersionTuple(0);
  VersionTuple MinValue = Min;
  toolchain::DenseMap<VersionTuple, VersionTuple> Mapping;
  for (const auto &KV : Obj) {
    if (auto Val = KV.getSecond().getAsString()) {
      toolchain::VersionTuple KeyVersion;
      toolchain::VersionTuple ValueVersion;
      if (KeyVersion.tryParse(KV.getFirst()) || ValueVersion.tryParse(*Val))
        return std::nullopt;
      Mapping[KeyVersion.normalize()] = ValueVersion;
      if (KeyVersion < Min)
        Min = KeyVersion;
      if (KeyVersion > Max)
        Max = KeyVersion;
      if (ValueVersion < MinValue)
        MinValue = ValueVersion;
    }
  }
  if (Mapping.empty())
    return std::nullopt;
  return RelatedTargetVersionMapping(
      Min, Max, MinValue, MaximumDeploymentTarget, std::move(Mapping));
}

static toolchain::Triple::OSType parseOS(const toolchain::json::Object &Obj) {
  // The CanonicalName is the Xcode platform followed by a version, e.g.
  // macosx16.0.
  auto CanonicalName = Obj.getString("CanonicalName");
  if (!CanonicalName)
    return toolchain::Triple::UnknownOS;
  size_t VersionStart = CanonicalName->find_first_of("0123456789");
  StringRef XcodePlatform = CanonicalName->slice(0, VersionStart);
  return toolchain::StringSwitch<toolchain::Triple::OSType>(XcodePlatform)
      .Case("macosx", toolchain::Triple::MacOSX)
      .Case("iphoneos", toolchain::Triple::IOS)
      .Case("iphonesimulator", toolchain::Triple::IOS)
      .Case("appletvos", toolchain::Triple::TvOS)
      .Case("appletvsimulator", toolchain::Triple::TvOS)
      .Case("watchos", toolchain::Triple::WatchOS)
      .Case("watchsimulator", toolchain::Triple::WatchOS)
      .Case("xros", toolchain::Triple::XROS)
      .Case("xrsimulator", toolchain::Triple::XROS)
      .Case("driverkit", toolchain::Triple::DriverKit)
      .Default(toolchain::Triple::UnknownOS);
}

static std::optional<VersionTuple> getVersionKey(const toolchain::json::Object &Obj,
                                                 StringRef Key) {
  auto Value = Obj.getString(Key);
  if (!Value)
    return std::nullopt;
  VersionTuple Version;
  if (Version.tryParse(*Value))
    return std::nullopt;
  return Version;
}

std::optional<DarwinSDKInfo>
DarwinSDKInfo::parseDarwinSDKSettingsJSON(const toolchain::json::Object *Obj) {
  auto Version = getVersionKey(*Obj, "Version");
  if (!Version)
    return std::nullopt;
  auto MaximumDeploymentVersion =
      getVersionKey(*Obj, "MaximumDeploymentTarget");
  if (!MaximumDeploymentVersion)
    return std::nullopt;
  toolchain::Triple::OSType OS = parseOS(*Obj);
  toolchain::DenseMap<OSEnvPair::StorageType,
                 std::optional<RelatedTargetVersionMapping>>
      VersionMappings;
  if (const auto *VM = Obj->getObject("VersionMap")) {
    // FIXME: Generalize this out beyond iOS-deriving targets.
    // Look for ios_<targetos> version mapping for targets that derive from ios.
    for (const auto &KV : *VM) {
      auto Pair = StringRef(KV.getFirst()).split("_");
      if (Pair.first.compare_insensitive("ios") == 0) {
        toolchain::Triple TT(toolchain::Twine("--") + Pair.second.lower());
        if (TT.getOS() != toolchain::Triple::UnknownOS) {
          auto Mapping = RelatedTargetVersionMapping::parseJSON(
              *KV.getSecond().getAsObject(), *MaximumDeploymentVersion);
          if (Mapping)
            VersionMappings[OSEnvPair(toolchain::Triple::IOS,
                                      toolchain::Triple::UnknownEnvironment,
                                      TT.getOS(),
                                      toolchain::Triple::UnknownEnvironment)
                                .Value] = std::move(Mapping);
        }
      }
    }

    if (const auto *Mapping = VM->getObject("macOS_iOSMac")) {
      auto VersionMap = RelatedTargetVersionMapping::parseJSON(
          *Mapping, *MaximumDeploymentVersion);
      if (!VersionMap)
        return std::nullopt;
      VersionMappings[OSEnvPair::macOStoMacCatalystPair().Value] =
          std::move(VersionMap);
    }
    if (const auto *Mapping = VM->getObject("iOSMac_macOS")) {
      auto VersionMap = RelatedTargetVersionMapping::parseJSON(
          *Mapping, *MaximumDeploymentVersion);
      if (!VersionMap)
        return std::nullopt;
      VersionMappings[OSEnvPair::macCatalystToMacOSPair().Value] =
          std::move(VersionMap);
    }
  }

  return DarwinSDKInfo(std::move(*Version),
                       std::move(*MaximumDeploymentVersion), OS,
                       std::move(VersionMappings));
}

Expected<std::optional<DarwinSDKInfo>>
language::Core::parseDarwinSDKInfo(toolchain::vfs::FileSystem &VFS, StringRef SDKRootPath) {
  toolchain::SmallString<256> Filepath = SDKRootPath;
  toolchain::sys::path::append(Filepath, "SDKSettings.json");
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> File =
      VFS.getBufferForFile(Filepath);
  if (!File) {
    // If the file couldn't be read, assume it just doesn't exist.
    return std::nullopt;
  }
  Expected<toolchain::json::Value> Result =
      toolchain::json::parse(File.get()->getBuffer());
  if (!Result)
    return Result.takeError();

  if (const auto *Obj = Result->getAsObject()) {
    if (auto SDKInfo = DarwinSDKInfo::parseDarwinSDKSettingsJSON(Obj))
      return std::move(SDKInfo);
  }
  return toolchain::make_error<toolchain::StringError>("invalid SDKSettings.json",
                                             toolchain::inconvertibleErrorCode());
}
