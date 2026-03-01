/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLCOLLATORCACHE_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLCOLLATORCACHE_H

#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include <ani.h>
#include <unicode/coll.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <random>

namespace ark::ets::stdlib::intl {

class IntlCollatorCache {
public:
    NO_COPY_SEMANTIC(IntlCollatorCache);
    NO_MOVE_SEMANTIC(IntlCollatorCache);

    IntlCollatorCache();
    ~IntlCollatorCache() = default;

    icu::Collator *GetOrCreateCollator(ani_env *env, const std::string &lang, const std::string &collation,
                                       const std::string &caseFirst);

private:
    using CacheUMap = std::unordered_map<std::string, std::unique_ptr<icu::Collator>>;
    using CacheUMapIterator = CacheUMap::iterator;

    void EraseRandFmtsGroupByEraseRatio();

    os::memory::RecursiveMutex mtx_;
    CacheUMap cache_ GUARDED_BY(mtx_);
    static constexpr uint32_t MAX_SIZE_CACHE = 512U;
    static constexpr double ERASE_RATIO = 0.1;  // Erase 10% of MAX_SIZE_CACHE
    static constexpr uint32_t ERASE_AMOUNT = std::max(1U, static_cast<uint32_t>(MAX_SIZE_CACHE *ERASE_RATIO));
};

}  // namespace ark::ets::stdlib::intl
#endif