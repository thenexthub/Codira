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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLRELATIVETIMEFORMATCACHE_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLRELATIVETIMEFORMATCACHE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <unicode/reldatefmt.h>
#include "libarkbase/os/mutex.h"

namespace ark::ets::stdlib::intl {

class IntlRelativeTimeFormatCache {
public:
    IntlRelativeTimeFormatCache();
    ~IntlRelativeTimeFormatCache() = default;

    IntlRelativeTimeFormatCache(const IntlRelativeTimeFormatCache &) = delete;
    IntlRelativeTimeFormatCache &operator=(const IntlRelativeTimeFormatCache &) = delete;
    IntlRelativeTimeFormatCache(IntlRelativeTimeFormatCache &&) = delete;
    IntlRelativeTimeFormatCache &operator=(IntlRelativeTimeFormatCache &&) = delete;

    icu::RelativeDateTimeFormatter *GetOrCreateFormatter(const std::string &cacheKey, const icu::Locale &locale,
                                                         UDateRelativeDateTimeFormatterStyle style);

private:
    void EraseRandFmtsGroupByEraseRatio() REQUIRES(mtx_);

    os::memory::Mutex mtx_;
    std::unordered_map<std::string, std::unique_ptr<icu::RelativeDateTimeFormatter>> cache_ GUARDED_BY(mtx_);
};

}  // namespace ark::ets::stdlib::intl
#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLRELATIVETIMEFORMATCACHE_H