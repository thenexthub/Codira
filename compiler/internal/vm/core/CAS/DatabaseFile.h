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
//
/// \file
/// This file declares the common interface for a DatabaseFile that is used to
/// implement OnDiskCAS.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CAS_DATABASEFILE_H
#define LLVM_LIB_CAS_DATABASEFILE_H

#include "vm/core/ADT/StringRef.h"
#include "vm/core/CAS/MappedFileRegionArena.h"
#include "vm/core/Support/Error.h"

namespace vm::core::cas::ondisk {

using MappedFileRegion = MappedFileRegionArena::RegionT;

/// Generic handle for a table.
///
/// Generic table header layout:
/// - 2-bytes: TableKind
/// - 2-bytes: TableNameSize
/// - 4-bytes: TableNameRelOffset (relative to header)
class TableHandle {
public:
  enum class TableKind : uint16_t {
    TrieRawHashMap = 1,
    DataAllocator = 2,
  };
  struct Header {
    TableKind Kind;
    uint16_t NameSize;
    int32_t NameRelOffset; ///< Relative to Header.
  };

  explicit operator bool() const { return H; }
  const Header &getHeader() const { return *H; }
  MappedFileRegion &getRegion() const { return *Region; }

  template <class T> static void check() {
    static_assert(
        std::is_same<decltype(T::Header::GenericHeader), Header>::value,
        "T::GenericHeader should be of type TableHandle::Header");
    static_assert(offsetof(typename T::Header, GenericHeader) == 0,
                  "T::GenericHeader must be the head of T::Header");
  }
  template <class T> bool is() const { return T::Kind == H->Kind; }
  template <class T> T dyn_cast() const {
    check<T>();
    if (is<T>())
      return T(*Region, *reinterpret_cast<typename T::Header *>(H));
    return T();
  }
  template <class T> T cast() const {
    assert(is<T>());
    return dyn_cast<T>();
  }

  StringRef getName() const {
    auto *Begin = reinterpret_cast<const char *>(H) + H->NameRelOffset;
    return StringRef(Begin, H->NameSize);
  }

  TableHandle() = default;
  TableHandle(MappedFileRegion &Region, Header &H) : Region(&Region), H(&H) {}
  TableHandle(MappedFileRegion &Region, intptr_t HeaderOffset)
      : TableHandle(Region,
                    *reinterpret_cast<Header *>(Region.data() + HeaderOffset)) {
  }

private:
  MappedFileRegion *Region = nullptr;
  Header *H = nullptr;
};

/// Encapsulate a database file, which:
/// - Sets/checks magic.
/// - Sets/checks version.
/// - Points at an arbitrary root table.
/// - Sets up a MappedFileRegionArena for allocation.
///
/// Top-level layout:
/// - 4-bytes: Magic
/// - 4-bytes: Version
/// - 8-bytes: RootTableOffset (16-bits: Kind; 48-bits: Offset)
/// - 8-bytes: BumpPtr from MappedFileRegionArena
class DatabaseFile {
public:
  static constexpr uint32_t getMagic() { return 0xDA7ABA53UL; }
  static constexpr uint32_t getVersion() { return 1UL; }
  struct Header {
    uint32_t Magic;
    uint32_t Version;
    std::atomic<int64_t> RootTableOffset;
  };

  const Header &getHeader() { return *H; }
  MappedFileRegionArena &getAlloc() { return Alloc; }
  MappedFileRegion &getRegion() { return Alloc.getRegion(); }

  /// Add a table. This is currently not thread safe and should be called inside
  /// NewDBConstructor.
  Error addTable(TableHandle Table);

  /// Find a table. May return null.
  std::optional<TableHandle> findTable(StringRef Name);

  /// Create the DatabaseFile at Path with Capacity.
  static Expected<DatabaseFile>
  create(const Twine &Path, uint64_t Capacity,
         function_ref<Error(DatabaseFile &)> NewDBConstructor);

  size_t size() const { return Alloc.size(); }

private:
  static Expected<DatabaseFile>
  get(std::unique_ptr<MappedFileRegionArena> Alloc) {
    if (Error E = validate(Alloc->getRegion()))
      return std::move(E);
    return DatabaseFile(std::move(Alloc));
  }

  static Error validate(MappedFileRegion &Region);

  DatabaseFile(MappedFileRegionArena &Alloc)
      : H(reinterpret_cast<Header *>(Alloc.data())), Alloc(Alloc) {}
  DatabaseFile(std::unique_ptr<MappedFileRegionArena> Alloc)
      : DatabaseFile(*Alloc) {
    OwnedAlloc = std::move(Alloc);
  }

  Header *H = nullptr;
  MappedFileRegionArena &Alloc;
  std::unique_ptr<MappedFileRegionArena> OwnedAlloc;
};

Error createTableConfigError(std::errc ErrC, StringRef Path,
                             StringRef TableName, const Twine &Msg);

Error checkTable(StringRef Label, size_t Expected, size_t Observed,
                 StringRef Path, StringRef TrieName);

} // namespace vm::core::cas::ondisk

#endif
