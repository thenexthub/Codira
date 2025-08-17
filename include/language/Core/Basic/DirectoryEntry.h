//===- clang/Basic/DirectoryEntry.h - Directory references ------*- C++ -*-===//
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
///
/// \file
/// Defines interfaces for language::Core::DirectoryEntry and language::Core::DirectoryEntryRef.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_DIRECTORYENTRY_H
#define LANGUAGE_CORE_BASIC_DIRECTORYENTRY_H

#include "language/Core/Basic/CustomizableOptional.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/DenseMapInfo.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/ErrorOr.h"

#include <optional>
#include <utility>

namespace language::Core {
namespace FileMgr {

template <class RefTy> class MapEntryOptionalStorage;

} // end namespace FileMgr

/// Cached information about one directory (either on disk or in
/// the virtual file system).
class DirectoryEntry {
  DirectoryEntry() = default;
  DirectoryEntry(const DirectoryEntry &) = delete;
  DirectoryEntry &operator=(const DirectoryEntry &) = delete;
  friend class FileManager;
  friend class FileEntryTestHelper;
};

/// A reference to a \c DirectoryEntry  that includes the name of the directory
/// as it was accessed by the FileManager's client.
class DirectoryEntryRef {
public:
  const DirectoryEntry &getDirEntry() const { return *ME->getValue(); }

  StringRef getName() const { return ME->getKey(); }

  /// Hash code is based on the DirectoryEntry, not the specific named
  /// reference.
  friend toolchain::hash_code hash_value(DirectoryEntryRef Ref) {
    return toolchain::hash_value(&Ref.getDirEntry());
  }

  using MapEntry = toolchain::StringMapEntry<toolchain::ErrorOr<DirectoryEntry &>>;

  const MapEntry &getMapEntry() const { return *ME; }

  /// Check if RHS referenced the file in exactly the same way.
  bool isSameRef(DirectoryEntryRef RHS) const { return ME == RHS.ME; }

  DirectoryEntryRef() = delete;
  explicit DirectoryEntryRef(const MapEntry &ME) : ME(&ME) {}

  /// Allow DirectoryEntryRef to degrade into 'const DirectoryEntry*' to
  /// facilitate incremental adoption.
  ///
  /// The goal is to avoid code churn due to dances like the following:
  /// \code
  /// // Old code.
  /// lvalue = rvalue;
  ///
  /// // Temporary code from an incremental patch.
  /// lvalue = &rvalue.getDirectoryEntry();
  ///
  /// // Final code.
  /// lvalue = rvalue;
  /// \endcode
  ///
  /// FIXME: Once DirectoryEntryRef is "everywhere" and DirectoryEntry::getName
  /// has been deleted, delete this implicit conversion.
  operator const DirectoryEntry *() const { return &getDirEntry(); }

private:
  friend class FileMgr::MapEntryOptionalStorage<DirectoryEntryRef>;
  struct optional_none_tag {};

  // Private constructor for use by OptionalStorage.
  DirectoryEntryRef(optional_none_tag) : ME(nullptr) {}
  bool hasOptionalValue() const { return ME; }

  friend struct toolchain::DenseMapInfo<DirectoryEntryRef>;
  struct dense_map_empty_tag {};
  struct dense_map_tombstone_tag {};

  // Private constructors for use by DenseMapInfo.
  DirectoryEntryRef(dense_map_empty_tag)
      : ME(toolchain::DenseMapInfo<const MapEntry *>::getEmptyKey()) {}
  DirectoryEntryRef(dense_map_tombstone_tag)
      : ME(toolchain::DenseMapInfo<const MapEntry *>::getTombstoneKey()) {}
  bool isSpecialDenseMapKey() const {
    return isSameRef(DirectoryEntryRef(dense_map_empty_tag())) ||
           isSameRef(DirectoryEntryRef(dense_map_tombstone_tag()));
  }

  const MapEntry *ME;
};

using OptionalDirectoryEntryRef = CustomizableOptional<DirectoryEntryRef>;

namespace FileMgr {

/// Customized storage for refs derived from map entires in FileManager, using
/// the private optional_none_tag to keep it to the size of a single pointer.
template <class RefTy> class MapEntryOptionalStorage {
  using optional_none_tag = typename RefTy::optional_none_tag;
  RefTy MaybeRef;

public:
  MapEntryOptionalStorage() : MaybeRef(optional_none_tag()) {}

  template <class... ArgTypes>
  explicit MapEntryOptionalStorage(std::in_place_t, ArgTypes &&...Args)
      : MaybeRef(std::forward<ArgTypes>(Args)...) {}

  void reset() { MaybeRef = optional_none_tag(); }

  bool has_value() const { return MaybeRef.hasOptionalValue(); }

  RefTy &value() & {
    assert(has_value());
    return MaybeRef;
  }
  RefTy const &value() const & {
    assert(has_value());
    return MaybeRef;
  }
  RefTy &&value() && {
    assert(has_value());
    return std::move(MaybeRef);
  }

  template <class... Args> void emplace(Args &&...args) {
    MaybeRef = RefTy(std::forward<Args>(args)...);
  }

  MapEntryOptionalStorage &operator=(RefTy Ref) {
    MaybeRef = Ref;
    return *this;
  }
};

} // end namespace FileMgr

namespace optional_detail {

/// Customize OptionalStorage<DirectoryEntryRef> to use DirectoryEntryRef and
/// its optional_none_tag to keep it the size of a single pointer.
template <>
class OptionalStorage<language::Core::DirectoryEntryRef>
    : public language::Core::FileMgr::MapEntryOptionalStorage<language::Core::DirectoryEntryRef> {
  using StorageImpl =
      language::Core::FileMgr::MapEntryOptionalStorage<language::Core::DirectoryEntryRef>;

public:
  OptionalStorage() = default;

  template <class... ArgTypes>
  explicit OptionalStorage(std::in_place_t, ArgTypes &&...Args)
      : StorageImpl(std::in_place_t{}, std::forward<ArgTypes>(Args)...) {}

  OptionalStorage &operator=(language::Core::DirectoryEntryRef Ref) {
    StorageImpl::operator=(Ref);
    return *this;
  }
};

static_assert(sizeof(OptionalDirectoryEntryRef) == sizeof(DirectoryEntryRef),
              "OptionalDirectoryEntryRef must avoid size overhead");

static_assert(std::is_trivially_copyable<OptionalDirectoryEntryRef>::value,
              "OptionalDirectoryEntryRef should be trivially copyable");

} // end namespace optional_detail
} // namespace language::Core

namespace toolchain {

template <> struct PointerLikeTypeTraits<language::Core::DirectoryEntryRef> {
  static inline void *getAsVoidPointer(language::Core::DirectoryEntryRef Dir) {
    return const_cast<language::Core::DirectoryEntryRef::MapEntry *>(&Dir.getMapEntry());
  }

  static inline language::Core::DirectoryEntryRef getFromVoidPointer(void *Ptr) {
    return language::Core::DirectoryEntryRef(
        *reinterpret_cast<const language::Core::DirectoryEntryRef::MapEntry *>(Ptr));
  }

  static constexpr int NumLowBitsAvailable = PointerLikeTypeTraits<
      const language::Core::DirectoryEntryRef::MapEntry *>::NumLowBitsAvailable;
};

/// Specialisation of DenseMapInfo for DirectoryEntryRef.
template <> struct DenseMapInfo<language::Core::DirectoryEntryRef> {
  static inline language::Core::DirectoryEntryRef getEmptyKey() {
    return language::Core::DirectoryEntryRef(
        language::Core::DirectoryEntryRef::dense_map_empty_tag());
  }

  static inline language::Core::DirectoryEntryRef getTombstoneKey() {
    return language::Core::DirectoryEntryRef(
        language::Core::DirectoryEntryRef::dense_map_tombstone_tag());
  }

  static unsigned getHashValue(language::Core::DirectoryEntryRef Val) {
    return hash_value(Val);
  }

  static bool isEqual(language::Core::DirectoryEntryRef LHS,
                      language::Core::DirectoryEntryRef RHS) {
    // Catch the easy cases: both empty, both tombstone, or the same ref.
    if (LHS.isSameRef(RHS))
      return true;

    // Confirm LHS and RHS are valid.
    if (LHS.isSpecialDenseMapKey() || RHS.isSpecialDenseMapKey())
      return false;

    // It's safe to use operator==.
    return LHS == RHS;
  }
};

} // end namespace toolchain

#endif // LANGUAGE_CORE_BASIC_DIRECTORYENTRY_H
