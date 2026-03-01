//===------- LoadLinkableFile.cpp -- Load relocatables and archives -------===//
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

#include "vm/core/ExecutionEngine/Orc/LoadLinkableFile.h"

#include "vm/core/ADT/ScopeExit.h"
#include "vm/core/BinaryFormat/Magic.h"
#include "vm/core/ExecutionEngine/Orc/MachO.h"
#include "vm/core/Support/FileSystem.h"

#define DEBUG_TYPE "orc"

namespace vm::core {
namespace orc {

static Expected<std::unique_ptr<MemoryBuffer>>
checkCOFFRelocatableObject(std::unique_ptr<MemoryBuffer> Obj,
                           const Triple &TT) {
  // TODO: Actually check the architecture of the file.
  return std::move(Obj);
}

static Expected<std::unique_ptr<MemoryBuffer>>
checkXCOFFRelocatableObject(std::unique_ptr<MemoryBuffer> Obj,
                            const Triple &TT) {
  // TODO: Actually check the architecture of the file.
  return std::move(Obj);
}

static Expected<std::unique_ptr<MemoryBuffer>>
checkELFRelocatableObject(std::unique_ptr<MemoryBuffer> Obj, const Triple &TT) {
  // TODO: Actually check the architecture of the file.
  return std::move(Obj);
}

Expected<std::pair<std::unique_ptr<MemoryBuffer>, LinkableFileKind>>
loadLinkableFile(StringRef Path, const Triple &TT, LoadArchives LA,
                 std::optional<StringRef> IdentifierOverride) {
  if (!IdentifierOverride)
    IdentifierOverride = Path;

  Expected<sys::fs::file_t> FDOrErr =
      sys::fs::openNativeFileForRead(Path, sys::fs::OF_None);
  if (!FDOrErr)
    return createFileError(Path, FDOrErr.takeError());
  sys::fs::file_t FD = *FDOrErr;
  toolchain::scope_exit CloseFile([&]() { sys::fs::closeFile(FD); });

  auto Buf =
      MemoryBuffer::getOpenFile(FD, *IdentifierOverride, /*FileSize=*/-1);
  if (!Buf)
    return make_error<StringError>(
        StringRef("Could not load object at path ") + Path, Buf.getError());

  std::optional<Triple::ObjectFormatType> RequireFormat;
  if (TT.getObjectFormat() != Triple::UnknownObjectFormat)
    RequireFormat = TT.getObjectFormat();

  switch (identify_magic((*Buf)->getBuffer())) {
  case file_magic::archive:
    if (LA != LoadArchives::Never)
      return std::make_pair(std::move(*Buf), LinkableFileKind::Archive);
    return make_error<StringError>(
        Path + " does not contain a relocatable object file",
        inconvertibleErrorCode());
  case file_magic::coff_object:
    if (LA == LoadArchives::Required)
      return make_error<StringError>(Path + " does not contain an archive",
                                     inconvertibleErrorCode());

    if (!RequireFormat || *RequireFormat == Triple::COFF) {
      auto CheckedBuf = checkCOFFRelocatableObject(std::move(*Buf), TT);
      if (!CheckedBuf)
        return CheckedBuf.takeError();
      return std::make_pair(std::move(*CheckedBuf),
                            LinkableFileKind::RelocatableObject);
    }
    break;
  case file_magic::elf_relocatable:
    if (LA == LoadArchives::Required)
      return make_error<StringError>(Path + " does not contain an archive",
                                     inconvertibleErrorCode());

    if (!RequireFormat || *RequireFormat == Triple::ELF) {
      auto CheckedBuf = checkELFRelocatableObject(std::move(*Buf), TT);
      if (!CheckedBuf)
        return CheckedBuf.takeError();
      return std::make_pair(std::move(*CheckedBuf),
                            LinkableFileKind::RelocatableObject);
    }
    break;
  case file_magic::macho_object:
    if (LA == LoadArchives::Required)
      return make_error<StringError>(Path + " does not contain an archive",
                                     inconvertibleErrorCode());

    if (!RequireFormat || *RequireFormat == Triple::MachO) {
      auto CheckedBuf = checkMachORelocatableObject(std::move(*Buf), TT, false);
      if (!CheckedBuf)
        return CheckedBuf.takeError();
      return std::make_pair(std::move(*CheckedBuf),
                            LinkableFileKind::RelocatableObject);
    }
    break;
  case file_magic::macho_universal_binary:
    if (!RequireFormat || *RequireFormat == Triple::MachO)
      return loadLinkableSliceFromMachOUniversalBinary(
          FD, std::move(*Buf), TT, LA, Path, *IdentifierOverride);
    break;
  case file_magic::xcoff_object_64:
    if (!RequireFormat || *RequireFormat == Triple::XCOFF) {
      auto CheckedBuf = checkXCOFFRelocatableObject(std::move(*Buf), TT);
      if (!CheckedBuf)
        return CheckedBuf.takeError();
      return std::make_pair(std::move(*CheckedBuf),
                            LinkableFileKind::RelocatableObject);
    }
    break;
  default:
    break;
  }

  return make_error<StringError>(
      Path +
          " does not contain a relocatable object file or archive compatible "
          "with " +
          TT.str(),
      inconvertibleErrorCode());
}

} // End namespace orc.
} // End namespace vm::core.
