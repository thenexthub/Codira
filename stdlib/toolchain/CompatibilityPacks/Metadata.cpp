//===--- Metadata.cpp -----------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Backward deployment of language_allocateMetadataPack() and
// language_allocateWitnessTablePack() runtime entry points.
//
//===----------------------------------------------------------------------===//

#include "../../public/runtime/MetadataCache.h"

using namespace language;

/// Copy and paste a symbol that needs to exist.
void *MetadataAllocator::Allocate(size_t size, size_t alignment) {
  return malloc(size);
}

/// Avoid depending on non-inline parts of toolchain::hashing.
inline toolchain::hash_code our_hash_integer_value(uint64_t value) {
  const char *s = reinterpret_cast<const char *>(&value);
  const uint64_t a = toolchain::hashing::detail::fetch32(s);
  return toolchain::hashing::detail::hash_16_bytes(
      (a << 3), toolchain::hashing::detail::fetch32(s + 4));
}

static inline toolchain::hash_code our_hash_combine(toolchain::hash_code seed, toolchain::hash_code v) {
  return seed ^ (v + 0x9e3779b9 + (seed<<6) + (seed>>2));
}

/// Copy and paste from Metadata.cpp.

namespace {

template<typename PackType>
class PackCacheEntry {
public:
  size_t Count;

  const PackType * const * getElements() const {
    return reinterpret_cast<const PackType * const *>(this + 1);
  }

  const PackType ** getElements() {
    return reinterpret_cast<const PackType **>(this + 1);
  }

  struct Key {
    const PackType *const *Data;
    const size_t Count;

    size_t getCount() const {
      return Count;
    }

    const PackType *getElement(size_t index) const {
      assert(index < Count);
      return Data[index];
    }

    friend toolchain::hash_code hash_value(const Key &key) {
      toolchain::hash_code hash = 0;
      for (size_t i = 0; i != key.getCount(); ++i) {
        hash = our_hash_combine(hash, our_hash_integer_value(
            reinterpret_cast<uint64_t>(key.getElement(i))));
      }
      return hash;
    }
  };

  PackCacheEntry(const Key &key);

  intptr_t getKeyIntValueForDump() {
    return 0; // No single meaningful value here.
  }

  bool matchesKey(const Key &key) const {
    if (key.getCount() != Count)
      return false;
    for (unsigned i = 0; i != Count; ++i) {
      if (key.getElement(i) != getElements()[i])
        return false;
    }
    return true;
  }

  friend toolchain::hash_code hash_value(const PackCacheEntry<PackType> &value) {
    toolchain::hash_code hash = 0;
    for (size_t i = 0; i != value.Count; ++i) {
      hash = our_hash_combine(hash, our_hash_integer_value(
          reinterpret_cast<uint64_t>(value.getElements()[i])));
    }
    return hash;
  }

  static size_t getExtraAllocationSize(const Key &key) {
    return getExtraAllocationSize(key.Count);
  }

  size_t getExtraAllocationSize() const {
    return getExtraAllocationSize(Count);
  }

  static size_t getExtraAllocationSize(unsigned count) {
    return count * sizeof(const Metadata * const *);
  }
};

template<typename PackType>
PackCacheEntry<PackType>::PackCacheEntry(
    const typename PackCacheEntry<PackType>::Key &key) {
  Count = key.getCount();

  for (unsigned i = 0; i < Count; ++i)
    getElements()[i] = key.getElement(i);
}

} // end anonymous namespace

/// The uniquing structure for metadata packs.
static SimpleGlobalCache<PackCacheEntry<Metadata>,
                         MetadataPackTag> MetadataPacks;

LANGUAGE_RUNTIME_EXPORT LANGUAGE_CC(language)
const Metadata * const *
language_allocateMetadataPack(const Metadata * const *ptr, size_t count) {
  if (MetadataPackPointer(reinterpret_cast<uintptr_t>(ptr)).getLifetime()
        == PackLifetime::OnHeap)
    return ptr;

  PackCacheEntry<Metadata>::Key key{ptr, count};
  auto bytes = MetadataPacks.getOrInsert(key).first->getElements();

  MetadataPackPointer pack(bytes, PackLifetime::OnHeap);
  return pack.getPointer();
}

/// The uniquing structure for witness table packs.
static SimpleGlobalCache<PackCacheEntry<WitnessTable>,
                         WitnessTablePackTag> WitnessTablePacks;

LANGUAGE_RUNTIME_EXPORT LANGUAGE_CC(language)
const WitnessTable * const *
language_allocateWitnessTablePack(const WitnessTable * const *ptr, size_t count) {
  if (WitnessTablePackPointer(reinterpret_cast<uintptr_t>(ptr)).getLifetime()
        == PackLifetime::OnHeap)
    return ptr;

  PackCacheEntry<WitnessTable>::Key key{ptr, count};
  auto bytes = WitnessTablePacks.getOrInsert(key).first->getElements();

  WitnessTablePackPointer pack(bytes, PackLifetime::OnHeap);
  return pack.getPointer();
}
