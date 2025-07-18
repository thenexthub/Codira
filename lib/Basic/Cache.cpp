//===--- Cache.cpp - Caching mechanism implementation ---------------------===//
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

#if defined(__APPLE__)
#include "Darwin/Cache-Mac.inc"
#else

//  This file implements a default caching implementation that never evicts
//  its entries.

#include "language/Basic/Cache.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/Mutex.h"

using namespace language::sys;
using toolchain::StringRef;

namespace {
struct DefaultCacheKey {
  void *Key = nullptr;
  CacheImpl::CallBacks *CBs = nullptr;

  //DefaultCacheKey() = default;
  DefaultCacheKey(void *Key, CacheImpl::CallBacks *CBs) : Key(Key), CBs(CBs) {}
};

struct DefaultCache {
  toolchain::sys::Mutex Mux;
  CacheImpl::CallBacks CBs;
  toolchain::DenseMap<DefaultCacheKey, void *> Entries;

  explicit DefaultCache(CacheImpl::CallBacks CBs) : CBs(std::move(CBs)) { }
};
} // end anonymous namespace

namespace toolchain {
template<> struct DenseMapInfo<DefaultCacheKey> {
  static inline DefaultCacheKey getEmptyKey() {
    return { DenseMapInfo<void*>::getEmptyKey(), nullptr };
  }
  static inline DefaultCacheKey getTombstoneKey() {
    return { DenseMapInfo<void*>::getTombstoneKey(), nullptr };
  }
  static unsigned getHashValue(const DefaultCacheKey &Val) {
    uintptr_t Hash = Val.CBs->keyHashCB(Val.Key, nullptr);
    return DenseMapInfo<uintptr_t>::getHashValue(Hash);
  }
  static bool isEqual(const DefaultCacheKey &LHS, const DefaultCacheKey &RHS) {
    if (LHS.Key == RHS.Key)
      return true;
    if (LHS.Key == DenseMapInfo<void*>::getEmptyKey() ||
        LHS.Key == DenseMapInfo<void*>::getTombstoneKey() ||
        RHS.Key == DenseMapInfo<void*>::getEmptyKey() ||
        RHS.Key == DenseMapInfo<void*>::getTombstoneKey())
      return false;
    return LHS.CBs->keyIsEqualCB(LHS.Key, RHS.Key, nullptr);
  }
};
} // namespace toolchain

CacheImpl::ImplTy CacheImpl::create(StringRef Name, const CallBacks &CBs) {
  return new DefaultCache(CBs);
}

void CacheImpl::setAndRetain(void *Key, void *Value, size_t Cost) {
  DefaultCache &DCache = *static_cast<DefaultCache*>(Impl);
  toolchain::sys::ScopedLock L(DCache.Mux);

  DefaultCacheKey CKey(Key, &DCache.CBs);
  auto Entry = DCache.Entries.find(CKey);

  // If there is no existing entry, retain the value and insert the entry.
  if (Entry == DCache.Entries.end()) {
    DCache.CBs.valueRetainCB(Value, nullptr);
    DCache.Entries[CKey] = Value;
    return;
  }

  // If there is an existing entry, the original key and the new key are ==.
  // Swap the new key into the map and destroy the original key.
  std::swap(Entry->first.Key, Key);
  DCache.CBs.keyDestroyCB(Key, nullptr);

  // Replace the value, if necessary.
  if (Entry->second != Value) {
    DCache.CBs.valueRetainCB(Value, nullptr);
    std::swap(Entry->second, Value);
    DCache.CBs.valueReleaseCB(Value, nullptr);
  }

  // FIXME: Not thread-safe! It should avoid deleting the value until
  // 'releaseValue is called on it.
}

bool CacheImpl::getAndRetain(const void *Key, void **Value_out) {
  DefaultCache &DCache = *static_cast<DefaultCache*>(Impl);
  toolchain::sys::ScopedLock L(DCache.Mux);

  DefaultCacheKey CKey(const_cast<void*>(Key), &DCache.CBs);
  auto Entry = DCache.Entries.find(CKey);
  if (Entry != DCache.Entries.end()) {
    // FIXME: Not thread-safe! It should avoid deleting the value until
    // 'releaseValue is called on it.
    *Value_out = Entry->second;
    return true;
  }
  return false;
}

void CacheImpl::releaseValue(void *Value) {
  // FIXME: Implementation.
}

bool CacheImpl::remove(const void *Key) {
  DefaultCache &DCache = *static_cast<DefaultCache*>(Impl);
  toolchain::sys::ScopedLock L(DCache.Mux);

  DefaultCacheKey CKey(const_cast<void*>(Key), &DCache.CBs);
  auto Entry = DCache.Entries.find(CKey);
  if (Entry != DCache.Entries.end()) {
    DCache.CBs.keyDestroyCB(Entry->first.Key, nullptr);
    DCache.CBs.valueReleaseCB(Entry->second, nullptr);
    DCache.Entries.erase(Entry);
    return true;
  }
  return false;
}

void CacheImpl::removeAll() {
  DefaultCache &DCache = *static_cast<DefaultCache*>(Impl);
  toolchain::sys::ScopedLock L(DCache.Mux);

  for (auto Entry : DCache.Entries) {
    DCache.CBs.keyDestroyCB(Entry.first.Key, nullptr);
    DCache.CBs.valueReleaseCB(Entry.second, nullptr);
  }
  DCache.Entries.clear();
}

void CacheImpl::destroy() {
  removeAll();
  delete static_cast<DefaultCache*>(Impl);
}

#endif // finish default implementation
