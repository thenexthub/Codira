//===- OnDiskKeyValueDB.cpp -------------------------------------*- C++ -*-===//
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
/// This file implements OnDiskKeyValueDB, an ondisk key value database.
///
/// The KeyValue database file is named `actions.<version>` inside the CAS
/// directory. The database stores a mapping between a fixed-sized key and a
/// fixed-sized value, where the size of key and value can be configured when
/// opening the database.
///
//
//===----------------------------------------------------------------------===//

#include "vm/core/CAS/OnDiskKeyValueDB.h"
#include "OnDiskCommon.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/CAS/UnifiedOnDiskCache.h"
#include "vm/core/Support/Alignment.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/Path.h"

using namespace vm::core;
using namespace vm::core::cas;
using namespace vm::core::cas::ondisk;

static constexpr StringLiteral ActionCacheFile = "actions.";

Expected<ArrayRef<char>> OnDiskKeyValueDB::put(ArrayRef<uint8_t> Key,
                                               ArrayRef<char> Value) {
  if (LLVM_UNLIKELY(Value.size() != ValueSize))
    return createStringError(errc::invalid_argument,
                             "expected value size of " + itostr(ValueSize) +
                                 ", got: " + itostr(Value.size()));
  assert(Value.size() == ValueSize);
  auto ActionP = Cache.insertLazy(
      Key, [&](FileOffset TentativeOffset,
               OnDiskTrieRawHashMap::ValueProxy TentativeValue) {
        assert(TentativeValue.Data.size() == ValueSize);
        toolchain::copy(Value, TentativeValue.Data.data());
      });
  if (LLVM_UNLIKELY(!ActionP))
    return ActionP.takeError();
  return (*ActionP)->Data;
}

Expected<std::optional<ArrayRef<char>>>
OnDiskKeyValueDB::get(ArrayRef<uint8_t> Key) {
  // Check the result cache.
  OnDiskTrieRawHashMap::ConstOnDiskPtr ActionP = Cache.find(Key);
  if (ActionP) {
    assert(isAddrAligned(Align(8), ActionP->Data.data()));
    return ActionP->Data;
  }
  if (!UnifiedCache || !UnifiedCache->UpstreamKVDB)
    return std::nullopt;

  // Try to fault in from upstream.
  return UnifiedCache->faultInFromUpstreamKV(Key);
}

Expected<std::unique_ptr<OnDiskKeyValueDB>>
OnDiskKeyValueDB::open(StringRef Path, StringRef HashName, unsigned KeySize,
                       StringRef ValueName, size_t ValueSize,
                       UnifiedOnDiskCache *Cache) {
  if (std::error_code EC = sys::fs::create_directories(Path))
    return createFileError(Path, EC);

  SmallString<256> CachePath(Path);
  sys::path::append(CachePath, ActionCacheFile + CASFormatVersion);
  constexpr uint64_t MB = 1024ull * 1024ull;
  constexpr uint64_t GB = 1024ull * 1024ull * 1024ull;

  uint64_t MaxFileSize = GB;
  auto CustomSize = getOverriddenMaxMappingSize();
  if (!CustomSize)
    return CustomSize.takeError();
  if (*CustomSize)
    MaxFileSize = **CustomSize;

  std::optional<OnDiskTrieRawHashMap> ActionCache;
  if (Error E = OnDiskTrieRawHashMap::create(
                    CachePath,
                    "toolchain.actioncache[" + HashName + "->" + ValueName + "]",
                    KeySize * 8,
                    /*DataSize=*/ValueSize, MaxFileSize, /*MinFileSize=*/MB)
                    .moveInto(ActionCache))
    return std::move(E);

  return std::unique_ptr<OnDiskKeyValueDB>(
      new OnDiskKeyValueDB(ValueSize, std::move(*ActionCache), Cache));
}

Error OnDiskKeyValueDB::validate(CheckValueT CheckValue) const {
  if (UnifiedCache && UnifiedCache->UpstreamKVDB) {
    if (auto E = UnifiedCache->UpstreamKVDB->validate(CheckValue))
      return E;
  }
  return Cache.validate(
      [&](FileOffset Offset,
          OnDiskTrieRawHashMap::ConstValueProxy Record) -> Error {
        auto formatError = [&](Twine Msg) {
          return createStringError(
              toolchain::errc::illegal_byte_sequence,
              "bad cache value at 0x" +
                  utohexstr((unsigned)Offset.get(), /*LowerCase=*/true) + ": " +
                  Msg.str());
        };

        if (Record.Data.size() != ValueSize)
          return formatError("wrong cache value size");
        if (!isAddrAligned(Align(8), Record.Data.data()))
          return formatError("wrong cache value alignment");
        if (CheckValue)
          return CheckValue(Offset, Record.Data);
        return Error::success();
      });
}
