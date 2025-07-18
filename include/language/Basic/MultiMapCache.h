//===--- MultiMapCache.h --------------------------------------------------===//
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

#ifndef LANGUAGE_BASIC_MULTIMAPCACHE_H
#define LANGUAGE_BASIC_MULTIMAPCACHE_H

#include "language/Basic/Toolchain.h"
#include "language/Basic/STLExtras.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include <optional>

namespace language {

/// A write-once multi-map cache that can be small. It uses a DenseMap
/// internally, so it can be used as a cache without needing to be frozen like
/// FrozenMultiMap (which uses only a vector internally). The cached value is
/// computed by a passed in std::function. The std::function is able to map
/// multiple values to a specific key via the out array.
///
/// NOTE: We store the (size, length) of each ArrayRef<ValueTy> instead of
/// storing the ArrayRef to avoid data invalidation issues caused by SmallVector
/// switching from small to large representations.
///
/// For an example of a subclass implementation see:
/// unittests/Basic/MultiMapCacheTest.cpp.
template <
    typename KeyTy, typename ValueTy,
    typename MapTy =
        toolchain::DenseMap<KeyTy, std::optional<std::tuple<unsigned, unsigned>>>,
    typename VectorTy = std::vector<ValueTy>, typename VectorTyImpl = VectorTy>
class MultiMapCache {
  std::function<bool(const KeyTy &, VectorTyImpl &)> function;
  MapTy valueToDataOffsetIndexMap;
  VectorTy data;

  constexpr static unsigned ArrayStartOffset = 0;
  constexpr static unsigned ArrayLengthOffset = 1;

public:
  MultiMapCache(std::function<bool(const KeyTy &, VectorTyImpl &)> function)
      : function(function) {}

  void clear() {
    valueToDataOffsetIndexMap.clear();
    data.clear();
  }

  bool empty() const { return valueToDataOffsetIndexMap.empty(); }
  unsigned size() const { return valueToDataOffsetIndexMap.size(); }

  std::optional<ArrayRef<ValueTy>> get(const KeyTy &key) {
    auto iter = valueToDataOffsetIndexMap.try_emplace(key, std::nullopt);

    // If we already have a cached value, just return the cached value.
    if (!iter.second) {

      return language::transform(
          iter.first->second,
          [&](std::tuple<unsigned, unsigned> startLengthRange) {
            return toolchain::ArrayRef(data).slice(
                std::get<ArrayStartOffset>(startLengthRange),
                std::get<ArrayLengthOffset>(startLengthRange));
          });
    }

    // Otherwise, try to compute the value. If we failed conservatively, return
    // None. The iter value already had None by default set to it, this just
    // makes it so that we do not need to have a memory dependency and can just
    // exit.
    unsigned initialOffset = data.size();

    // We assume that constructValuesForKey /only/ inserts to the end of data
    // and does not inspect any other values in the data array.
    if (!function(key, data)) {
      return std::nullopt;
    }

    // Otherwise, compute our length, compute our initial ArrayRef<ValueTy>,
    // update the map with the start, length, and return the resulting ArrayRef.
    unsigned length = data.size() - initialOffset;
    iter.first->second = std::make_tuple(initialOffset, length);
    auto result = toolchain::ArrayRef(data).slice(initialOffset, length);
    return result;
  }
};

template <typename KeyTy, typename ValueTy>
using SmallMultiMapCache =
    MultiMapCache<KeyTy, ValueTy,
                  toolchain::SmallDenseMap<
                      KeyTy, std::optional<std::tuple<unsigned, unsigned>>, 8>,
                  SmallVector<ValueTy, 32>, SmallVectorImpl<ValueTy>>;

} // namespace language

#endif
