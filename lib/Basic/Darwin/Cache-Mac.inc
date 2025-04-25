//===--- Cache-Mac.cpp - Caching mechanism implementation -----------------===//
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
//  This file implements the caching mechanism using darwin's libcache.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Cache.h"
#include "llvm/ADT/SmallString.h"
#include <cache.h>

using namespace language::sys;
using llvm::StringRef;

CacheImpl::ImplTy CacheImpl::create(StringRef Name, const CallBacks &CBs) {
  llvm::SmallString<32> NameBuf(Name);
  cache_attributes_t Attrs = {
    CACHE_ATTRIBUTES_VERSION_2,
    CBs.keyHashCB,
    CBs.keyIsEqualCB,
    nullptr,
    CBs.keyDestroyCB,
    CBs.valueReleaseCB,
    nullptr,
    nullptr,
    CBs.UserData,
    CBs.valueRetainCB,
  };

  cache_t *cache_out = nullptr;
  cache_create(NameBuf.c_str(), &Attrs, &cache_out);
  assert(cache_out);
  return cache_out;
}

void CacheImpl::setAndRetain(void *Key, void *Value, size_t Cost) {
  cache_set_and_retain(static_cast<cache_t*>(Impl), Key, Value, Cost);
}

bool CacheImpl::getAndRetain(const void *Key, void **Value_out) {
  int Ret = cache_get_and_retain(static_cast<cache_t*>(Impl),
                                 const_cast<void*>(Key), Value_out);
  return Ret == 0;
}

void CacheImpl::releaseValue(void *Value) {
  cache_release_value(static_cast<cache_t*>(Impl), Value);
}

bool CacheImpl::remove(const void *Key) {
  int Ret = cache_remove(static_cast<cache_t*>(Impl), const_cast<void*>(Key));
  return Ret == 0;
}

void CacheImpl::removeAll() {
  cache_remove_all(static_cast<cache_t*>(Impl));
}

void CacheImpl::destroy() {
  cache_destroy(static_cast<cache_t*>(Impl));
}
