/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_WRAPPERS_CACHE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_WRAPPERS_CACHE_H_

#include "libarkbase/macros.h"

#include <memory>
#include <unordered_map>

namespace ark::ets::interop::js::ets_proxy {

template <typename Key, typename Wrapper>
class WrappersCache {
public:
    WrappersCache() = default;
    ~WrappersCache() = default;
    NO_COPY_SEMANTIC(WrappersCache);
    NO_MOVE_SEMANTIC(WrappersCache);

    Wrapper *Lookup(Key key)
    {
        auto it = cache_.find(key);
        if (UNLIKELY(it == cache_.end())) {
            return nullptr;
        }
        return it->second.get();
    }

    Wrapper *Insert(Key key, std::unique_ptr<Wrapper> &&wrapper)
    {
        auto [it, inserted] = cache_.insert_or_assign(key, std::move(wrapper));
        ASSERT(inserted);
        return it->second.get();
    }

    std::unique_ptr<Wrapper> Steal(Key key)
    {
        auto nh = cache_.extract(key);
        return nh ? std::move(nh.mapped()) : nullptr;
    }

private:
    std::unordered_map<Key, std::unique_ptr<Wrapper>> cache_;
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_WRAPPERS_CACHE_H_
