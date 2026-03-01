//===----------------------------------------------------------------------===//
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
/// \file This file implements the common abstractions for CAS database file.
///
//===----------------------------------------------------------------------===//

#include "DatabaseFile.h"

using namespace vm::core;
using namespace vm::core::cas;
using namespace vm::core::cas::ondisk;

Error ondisk::createTableConfigError(std::errc ErrC, StringRef Path,
                                     StringRef TableName, const Twine &Msg) {
  return createStringError(make_error_code(ErrC),
                           Path + "[" + TableName + "]: " + Msg);
}

Error ondisk::checkTable(StringRef Label, size_t Expected, size_t Observed,
                         StringRef Path, StringRef TrieName) {
  if (Expected == Observed)
    return Error::success();
  return createTableConfigError(std::errc::invalid_argument, Path, TrieName,
                                "mismatched " + Label +
                                    " (expected: " + Twine(Expected) +
                                    ", observed: " + Twine(Observed) + ")");
}

Expected<DatabaseFile>
DatabaseFile::create(const Twine &Path, uint64_t Capacity,
                     function_ref<Error(DatabaseFile &)> NewDBConstructor) {
  // Constructor for if the file doesn't exist.
  auto NewFileConstructor = [&](MappedFileRegionArena &Alloc) -> Error {
    if (Alloc.capacity() <
        sizeof(Header) + sizeof(MappedFileRegionArena::Header))
      return createTableConfigError(std::errc::argument_out_of_domain,
                                    Path.str(), "datafile",
                                    "Allocator too small for header");
    (void)new (Alloc.data()) Header{getMagic(), getVersion(), {0}};
    DatabaseFile DB(Alloc);
    return NewDBConstructor(DB);
  };

  // Get or create the file.
  MappedFileRegionArena Alloc;
  if (Error E = MappedFileRegionArena::create(Path, Capacity, sizeof(Header),
                                              NewFileConstructor)
                    .moveInto(Alloc))
    return std::move(E);

  return DatabaseFile::get(
      std::make_unique<MappedFileRegionArena>(std::move(Alloc)));
}

Error DatabaseFile::addTable(TableHandle Table) {
  assert(Table);
  assert(&Table.getRegion() == &getRegion());
  int64_t ExistingRootOffset = 0;
  const int64_t NewOffset =
      reinterpret_cast<const char *>(&Table.getHeader()) - getRegion().data();
  if (H->RootTableOffset.compare_exchange_strong(ExistingRootOffset, NewOffset))
    return Error::success();

  // Silently ignore attempts to set the root to itself.
  if (ExistingRootOffset == NewOffset)
    return Error::success();

  // Return an proper error message.
  TableHandle Root(getRegion(), ExistingRootOffset);
  if (Root.getName() == Table.getName())
    return createStringError(
        make_error_code(std::errc::not_supported),
        "collision with existing table of the same name '" + Table.getName() +
            "'");

  return createStringError(make_error_code(std::errc::not_supported),
                           "cannot add new table '" + Table.getName() +
                               "'"
                               " to existing root '" +
                               Root.getName() + "'");
}

std::optional<TableHandle> DatabaseFile::findTable(StringRef Name) {
  int64_t RootTableOffset = H->RootTableOffset.load();
  if (!RootTableOffset)
    return std::nullopt;

  TableHandle Root(getRegion(), RootTableOffset);
  if (Root.getName() == Name)
    return Root;

  return std::nullopt;
}

Error DatabaseFile::validate(MappedFileRegion &Region) {
  if (Region.size() < sizeof(Header))
    return createStringError(std::errc::invalid_argument,
                             "database: missing header");

  // Check the magic and version.
  auto *H = reinterpret_cast<Header *>(Region.data());
  if (H->Magic != getMagic())
    return createStringError(std::errc::invalid_argument,
                             "database: bad magic");
  if (H->Version != getVersion())
    return createStringError(std::errc::invalid_argument,
                             "database: wrong version");

  auto *MFH = reinterpret_cast<MappedFileRegionArena::Header *>(Region.data() +
                                                                sizeof(Header));
  // Check the bump-ptr, which should point past the header.
  if (MFH->BumpPtr.load() < (int64_t)sizeof(Header))
    return createStringError(std::errc::invalid_argument,
                             "database: corrupt bump-ptr");

  return Error::success();
}
