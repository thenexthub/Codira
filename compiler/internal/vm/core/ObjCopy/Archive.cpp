//===- Archive.cpp --------------------------------------------------------===//
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

#include "Archive.h"
#include "vm/core/ObjCopy/CommonConfig.h"
#include "vm/core/ObjCopy/MultiFormatConfig.h"
#include "vm/core/ObjCopy/ObjCopy.h"
#include "vm/core/Object/Error.h"
#include "vm/core/Support/FileOutputBuffer.h"
#include "vm/core/Support/SmallVectorMemoryBuffer.h"

using namespace vm::core;
using namespace vm::core::objcopy;
using namespace vm::core::object;

Expected<std::vector<NewArchiveMember>>
objcopy::createNewArchiveMembers(const MultiFormatConfig &Config,
                                 const Archive &Ar) {
  std::vector<NewArchiveMember> NewArchiveMembers;
  Error Err = Error::success();
  for (const Archive::Child &Child : Ar.children(Err)) {
    Expected<StringRef> ChildNameOrErr = Child.getName();
    if (!ChildNameOrErr)
      return createFileError(Ar.getFileName(), ChildNameOrErr.takeError());

    Expected<std::unique_ptr<Binary>> ChildOrErr = Child.getAsBinary();
    if (!ChildOrErr)
      return createFileError(Ar.getFileName() + "(" + *ChildNameOrErr + ")",
                             ChildOrErr.takeError());

    SmallVector<char, 0> Buffer;
    raw_svector_ostream MemStream(Buffer);

    if (Error E = executeObjcopyOnBinary(Config, *ChildOrErr->get(), MemStream))
      return std::move(E);

    Expected<NewArchiveMember> Member = NewArchiveMember::getOldMember(
        Child, Config.getCommonConfig().DeterministicArchives);
    if (!Member)
      return createFileError(Ar.getFileName(), Member.takeError());

    Member->Buf = std::make_unique<SmallVectorMemoryBuffer>(
        std::move(Buffer), ChildNameOrErr.get());
    Member->MemberName = Member->Buf->getBufferIdentifier();
    NewArchiveMembers.push_back(std::move(*Member));
  }
  if (Err)
    return createFileError(Config.getCommonConfig().InputFilename,
                           std::move(Err));
  return std::move(NewArchiveMembers);
}

// For regular archives this function simply calls toolchain::writeArchive,
// For thin archives it writes the archive file itself as well as its members.
static Error deepWriteArchive(StringRef ArcName,
                              ArrayRef<NewArchiveMember> NewMembers,
                              SymtabWritingMode WriteSymtab,
                              object::Archive::Kind Kind, bool Deterministic,
                              bool Thin) {
  if (Kind == object::Archive::K_BSD && !NewMembers.empty() &&
      NewMembers.front().detectKindFromObject() == object::Archive::K_DARWIN)
    Kind = object::Archive::K_DARWIN;

  if (Error E = writeArchive(ArcName, NewMembers, WriteSymtab, Kind,
                             Deterministic, Thin))
    return createFileError(ArcName, std::move(E));

  if (!Thin)
    return Error::success();

  for (const NewArchiveMember &Member : NewMembers) {
    // For regular files (as is the case for deepWriteArchive),
    // FileOutputBuffer::create will return OnDiskBuffer.
    // OnDiskBuffer uses a temporary file and then renames it. So in reality
    // there is no inefficiency / duplicated in-memory buffers in this case. For
    // now in-memory buffers can not be completely avoided since
    // NewArchiveMember still requires them even though writeArchive does not
    // write them on disk.
    Expected<std::unique_ptr<FileOutputBuffer>> FB =
        FileOutputBuffer::create(Member.MemberName, Member.Buf->getBufferSize(),
                                 FileOutputBuffer::F_executable);
    if (!FB)
      return FB.takeError();
    std::copy(Member.Buf->getBufferStart(), Member.Buf->getBufferEnd(),
              (*FB)->getBufferStart());
    if (Error E = (*FB)->commit())
      return E;
  }
  return Error::success();
}

Error objcopy::executeObjcopyOnArchive(const MultiFormatConfig &Config,
                                       const object::Archive &Ar) {
  Expected<std::vector<NewArchiveMember>> NewArchiveMembersOrErr =
      createNewArchiveMembers(Config, Ar);
  if (!NewArchiveMembersOrErr)
    return NewArchiveMembersOrErr.takeError();
  const CommonConfig &CommonConfig = Config.getCommonConfig();
  return deepWriteArchive(CommonConfig.OutputFilename, *NewArchiveMembersOrErr,
                          Ar.hasSymbolTable() ? SymtabWritingMode::NormalSymtab
                                              : SymtabWritingMode::NoSymtab,
                          Ar.kind(), CommonConfig.DeterministicArchives,
                          Ar.isThin());
}
