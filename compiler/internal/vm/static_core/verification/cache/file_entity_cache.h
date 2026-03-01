/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef PANDA_VERIFIER_CACHE_FILE_ENTITY_CACHE_H_
#define PANDA_VERIFIER_CACHE_FILE_ENTITY_CACHE_H_

#include "libarkbase/macros.h"

#include "verification/util/hash.h"
#include "verification/util/optional_ref.h"

#include "libarkfile/file.h"

#include "runtime/include/mem/panda_containers.h"

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace ark::verifier {

using TypeIndexT = uint8_t;

template <typename... Types>
constexpr TypeIndexT TYPE_INDEX = []() {
    static_assert(sizeof...(Types) > 1, "Type was not found in list");
    return 0;
}();

template <typename T, typename... Types>
constexpr TypeIndexT TYPE_INDEX<T, T, Types...> = 0;

template <typename T, typename U, typename... Types>
constexpr TypeIndexT TYPE_INDEX<T, U, Types...> = []() {
    constexpr TypeIndexT RESULT = 1 + TYPE_INDEX<T, Types...>;
    static_assert(RESULT != 0, "Overflow! TypeIndex used with more than 255 arguments");
    return RESULT;
}();

template <typename... CachedTypes>
class FileEntityCache {
    using FileId = uint64_t;
    using Key = std::tuple<FileId, panda_file::File::EntityId, TypeIndexT>;

    template <typename Entity>
    Key GetKey(const panda_file::File &pf, panda_file::File::EntityId id)
    {
        return {pf.GetUniqId(), id, TYPE_INDEX<Entity, CachedTypes...>};
    }

public:
    template <typename Entity>
    OptionalRef<Entity> GetCached(const panda_file::File &pf, panda_file::File::EntityId id)
    {
        const auto it = storage_.find(GetKey<Entity>(pf, id));
        if (it != storage_.cend()) {
            return *static_cast<Entity *>(it->second);
        }
        return {};
    }

    template <typename Entity>
    void AddToCache(const panda_file::File &pf, panda_file::File::EntityId id, Entity &entity)
    {
        storage_.insert_or_assign(GetKey<Entity>(pf, id), &entity);
    }

private:
    PandaUnorderedMap<Key, void *> storage_;
};

}  // namespace ark::verifier

#endif  // !PANDA_VERIFIER_CACHE_FILE_ENTITY_CACHE_H_
