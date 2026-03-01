//===- toolchain/Object/BuildID.cpp - Build ID ---------------------------------===//
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
/// This file defines a library for handling Build IDs and using them to find
/// debug info.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Object/BuildID.h"

#include "vm/core/Object/ELFObjectFile.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/Path.h"

using namespace vm::core;
using namespace vm::core::object;

namespace {

template <typename ELFT> BuildIDRef getBuildID(const ELFFile<ELFT> &Obj) {
  auto findBuildID = [&Obj](const auto &ShdrOrPhdr,
                            uint64_t Alignment) -> std::optional<BuildIDRef> {
    Error Err = Error::success();
    for (auto N : Obj.notes(ShdrOrPhdr, Err))
      if (N.getType() == ELF::NT_GNU_BUILD_ID &&
          N.getName() == ELF::ELF_NOTE_GNU)
        return N.getDesc(Alignment);
    consumeError(std::move(Err));
    return std::nullopt;
  };

  auto Sections = cantFail(Obj.sections());
  for (const auto &S : Sections) {
    if (S.sh_type != ELF::SHT_NOTE)
      continue;
    if (std::optional<BuildIDRef> ShdrRes = findBuildID(S, S.sh_addralign))
      return ShdrRes.value();
  }
  auto PhdrsOrErr = Obj.program_headers();
  if (!PhdrsOrErr) {
    consumeError(PhdrsOrErr.takeError());
    return {};
  }
  for (const auto &P : *PhdrsOrErr) {
    if (P.p_type != ELF::PT_NOTE)
      continue;
    if (std::optional<BuildIDRef> PhdrRes = findBuildID(P, P.p_align))
      return PhdrRes.value();
  }
  return {};
}

} // namespace

BuildID toolchain::object::parseBuildID(StringRef Str) {
  std::string Bytes;
  if (!tryGetFromHex(Str, Bytes))
    return {};
  ArrayRef<uint8_t> BuildID(reinterpret_cast<const uint8_t *>(Bytes.data()),
                            Bytes.size());
  return SmallVector<uint8_t>(BuildID);
}

BuildIDRef toolchain::object::getBuildID(const ObjectFile *Obj) {
  if (auto *O = dyn_cast<ELFObjectFile<ELF32LE>>(Obj))
    return ::getBuildID(O->getELFFile());
  if (auto *O = dyn_cast<ELFObjectFile<ELF32BE>>(Obj))
    return ::getBuildID(O->getELFFile());
  if (auto *O = dyn_cast<ELFObjectFile<ELF64LE>>(Obj))
    return ::getBuildID(O->getELFFile());
  if (auto *O = dyn_cast<ELFObjectFile<ELF64BE>>(Obj))
    return ::getBuildID(O->getELFFile());
  return {};
}

std::optional<std::string> BuildIDFetcher::fetch(BuildIDRef BuildID) const {
  auto GetDebugPath = [&](StringRef Directory) {
    SmallString<128> Path{Directory};
    sys::path::append(Path, ".build-id",
                      toolchain::toHex(BuildID[0], /*LowerCase=*/true),
                      toolchain::toHex(BuildID.slice(1), /*LowerCase=*/true));
    Path += ".debug";
    return Path;
  };
  if (DebugFileDirectories.empty()) {
    SmallString<128> Path = GetDebugPath(
#if defined(__NetBSD__)
        // Try /usr/libdata/debug/.build-id/../...
        "/usr/libdata/debug"
#else
        // Try /usr/lib/debug/.build-id/../...
        "/usr/lib/debug"
#endif
    );
    if (toolchain::sys::fs::exists(Path))
      return std::string(Path);
  } else {
    for (const auto &Directory : DebugFileDirectories) {
      // Try <debug-file-directory>/.build-id/../...
      SmallString<128> Path = GetDebugPath(Directory);
      if (toolchain::sys::fs::exists(Path))
        return std::string(Path);
    }
  }
  return std::nullopt;
}
