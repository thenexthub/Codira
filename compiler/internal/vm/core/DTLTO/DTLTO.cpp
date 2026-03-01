//===- Dtlto.cpp - Distributed ThinLTO implementation --------------------===//
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
// \file
// This file implements support functions for Distributed ThinLTO, focusing on
// archive file handling.
//
//===----------------------------------------------------------------------===//

#include "vm/core/DTLTO/DTLTO.h"

#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/BinaryFormat/Magic.h"
#include "vm/core/LTO/LTO.h"
#include "vm/core/Object/Archive.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/ManagedStatic.h"
#include "vm/core/Support/MemoryBufferRef.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Support/Process.h"
#include "vm/core/Support/raw_ostream.h"

#include <iostream>
#include <string>

using namespace vm::core;

namespace {

// Writes the content of a memory buffer into a file.
toolchain::Error saveBuffer(StringRef FileBuffer, StringRef FilePath) {
  std::error_code EC;
  raw_fd_ostream OS(FilePath.str(), EC, sys::fs::OpenFlags::OF_None);
  if (EC) {
    return createStringError(inconvertibleErrorCode(),
                             "Failed to create file %s: %s", FilePath.data(),
                             EC.message().c_str());
  }
  OS.write(FileBuffer.data(), FileBuffer.size());
  if (OS.has_error()) {
    return createStringError(inconvertibleErrorCode(),
                             "Failed writing to file %s", FilePath.data());
  }
  return Error::success();
}

// Compute the file path for a thin archive member.
//
// For thin archives, an archive member name is typically a file path relative
// to the archive file's directory. This function resolves that path.
SmallString<64> computeThinArchiveMemberPath(const StringRef ArchivePath,
                                             const StringRef MemberName) {
  assert(!ArchivePath.empty() && "An archive file path must be non empty.");
  SmallString<64> MemberPath;
  if (sys::path::is_relative(MemberName)) {
    MemberPath = sys::path::parent_path(ArchivePath);
    sys::path::append(MemberPath, MemberName);
  } else
    MemberPath = MemberName;
  sys::path::remove_dots(MemberPath, /*remove_dot_dot=*/true);
  return MemberPath;
}

} // namespace

// Determines if a file at the given path is a thin archive file.
//
// This function uses a cache to avoid repeatedly reading the same file.
// It reads only the header portion (magic bytes) of the file to identify
// the archive type.
Expected<bool> lto::DTLTO::isThinArchive(const StringRef ArchivePath) {
  // Return cached result if available.
  auto Cached = ArchiveFiles.find(ArchivePath);
  if (Cached != ArchiveFiles.end())
    return Cached->second;

  uint64_t FileSize = -1;
  bool IsThin = false;
  std::error_code EC = sys::fs::file_size(ArchivePath, FileSize);
  if (EC)
    return createStringError(inconvertibleErrorCode(),
                             "Failed to get file size from archive %s: %s",
                             ArchivePath.data(), EC.message().c_str());
  if (FileSize < sizeof(object::ThinArchiveMagic))
    return createStringError(inconvertibleErrorCode(),
                             "Archive file size is too small %s",
                             ArchivePath.data());

  // Read only the first few bytes containing the magic signature.
  ErrorOr<std::unique_ptr<MemoryBuffer>> MemBufferOrError =
      MemoryBuffer::getFileSlice(ArchivePath, sizeof(object::ThinArchiveMagic),
                                 0);

  if ((EC = MemBufferOrError.getError()))
    return createStringError(inconvertibleErrorCode(),
                             "Failed to read from archive %s: %s",
                             ArchivePath.data(), EC.message().c_str());

  StringRef MemBuf = (*MemBufferOrError.get()).getBuffer();
  if (file_magic::archive != identify_magic(MemBuf))
    return createStringError(inconvertibleErrorCode(),
                             "Unknown format for archive %s",
                             ArchivePath.data());

  IsThin = MemBuf.starts_with(object::ThinArchiveMagic);

  // Cache the result
  ArchiveFiles[ArchivePath] = IsThin;
  return IsThin;
}

// Removes any temporary regular archive member files that were created during
// processing.
void lto::DTLTO::removeTempFiles() {
  for (auto &Input : InputFiles) {
    if (Input->isMemberOfArchive())
      sys::fs::remove(Input->getName(), /*IgnoreNonExisting=*/true);
  }
}

// This function performs the following tasks:
// 1. Adds the input file to the LTO object's list of input files.
// 2. For thin archive members, generates a new module ID which is a path to a
// thin archive member file.
// 3. For regular archive members, generates a new unique module ID.
// 4. Updates the bitcode module's identifier.
Expected<std::shared_ptr<lto::InputFile>>
lto::DTLTO::addInput(std::unique_ptr<lto::InputFile> InputPtr) {

  // Add the input file to the LTO object.
  InputFiles.emplace_back(InputPtr.release());
  std::shared_ptr<lto::InputFile> &Input = InputFiles.back();

  StringRef ModuleId = Input->getName();
  StringRef ArchivePath = Input->getArchivePath();

  // Only process archive members.
  if (ArchivePath.empty())
    return Input;

  SmallString<64> NewModuleId;
  BitcodeModule &BM = Input->getPrimaryBitcodeModule();

  // Check if the archive is a thin archive.
  Expected<bool> IsThin = isThinArchive(ArchivePath);
  if (!IsThin)
    return IsThin.takeError();

  if (*IsThin) {
    // For thin archives, use the path to the actual file.
    NewModuleId =
        computeThinArchiveMemberPath(ArchivePath, Input->getMemberName());
  } else {
    // For regular archives, generate a unique name.
    Input->memberOfArchive(true);

    // Create unique identifier using process ID and sequence number.
    std::string PID = utohexstr(sys::Process::getProcessId());
    std::string Seq = std::to_string(InputFiles.size());

    NewModuleId = {sys::path::filename(ModuleId), ".", Seq, ".", PID, ".o"};
  }

  // Update the module identifier and save it.
  BM.setModuleIdentifier(Saver.save(NewModuleId.str()));

  return Input;
}

// Write the archive member content to a file named after the module ID.
// If a file with that name already exists, it's likely a leftover from a
// previously terminated linker process and can be safely overwritten.
Error lto::DTLTO::saveInputArchiveMember(lto::InputFile *Input) {
  StringRef ModuleId = Input->getName();
  if (Input->isMemberOfArchive()) {
    MemoryBufferRef MemoryBufferRef = Input->getFileBuffer();
    if (Error EC = saveBuffer(MemoryBufferRef.getBuffer(), ModuleId))
      return EC;
  }
  return Error::success();
}

// Iterates through all ThinLTO-enabled input files and saves their content
// to separate files if they are regular archive members.
Error lto::DTLTO::saveInputArchiveMembers() {
  for (auto &Input : InputFiles) {
    if (!Input->isThinLTO())
      continue;
    if (Error EC = saveInputArchiveMember(Input.get()))
      return EC;
  }
  return Error::success();
}

// Entry point for DTLTO archives support.
//
// Sets up the temporary file remover and processes archive members.
// Must be called after all inputs are added but before optimization begins.
toolchain::Error lto::DTLTO::handleArchiveInputs() {

  // Process and save archive members to separate files if needed.
  if (Error EC = saveInputArchiveMembers())
    return EC;
  return Error::success();
}
