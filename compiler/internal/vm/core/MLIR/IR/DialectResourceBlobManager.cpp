//===- DialectResourceBlobManager.cpp - Dialect Blob Management -----------===//
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

#include "mlir/IR/DialectResourceBlobManager.h"
#include "vm/core/ADT/SmallString.h"
#include <optional>

using namespace mlir;

//===----------------------------------------------------------------------===//
// DialectResourceBlobManager
//===---------------------------------------------------------------------===//

auto DialectResourceBlobManager::lookup(StringRef name) -> BlobEntry * {
  toolchain::sys::SmartScopedReader<true> reader(blobMapLock);

  auto it = blobMap.find(name);
  return it != blobMap.end() ? &it->second : nullptr;
}

void DialectResourceBlobManager::update(StringRef name,
                                        AsmResourceBlob &&newBlob) {
  BlobEntry *entry = lookup(name);
  assert(entry && "`update` expects an existing entry for the provided name");
  entry->setBlob(std::move(newBlob));
}

auto DialectResourceBlobManager::insert(StringRef name,
                                        std::optional<AsmResourceBlob> blob)
    -> BlobEntry & {
  toolchain::sys::SmartScopedWriter<true> writer(blobMapLock);

  // Functor used to attempt insertion with a given name.
  auto tryInsertion = [&](StringRef name) -> BlobEntry * {
    auto it = blobMap.try_emplace(name, BlobEntry());
    if (it.second) {
      it.first->second.initialize(it.first->getKey(), std::move(blob));
      return &it.first->second;
    }
    return nullptr;
  };

  // Try inserting with the name provided by the user.
  if (BlobEntry *entry = tryInsertion(name))
    return *entry;

  // If an entry already exists for the user provided name, tweak the name and
  // re-attempt insertion until we find one that is unique.
  toolchain::SmallString<32> nameStorage(name);
  nameStorage.push_back('_');
  size_t nameCounter = 1;
  do {
    Twine(nameCounter++).toVector(nameStorage);

    // Try inserting with the new name.
    if (BlobEntry *entry = tryInsertion(nameStorage))
      return *entry;
    nameStorage.resize(name.size() + 1);
  } while (true);
}

void DialectResourceBlobManager::getBlobMap(
    toolchain::function_ref<void(const toolchain::StringMap<BlobEntry> &)> accessor)
    const {
  toolchain::sys::SmartScopedReader<true> reader(blobMapLock);

  accessor(blobMap);
}
