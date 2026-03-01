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

#pragma once

#include <type_traits>
#include <utility>

namespace sqldb {

template <typename Callable>
class [[nodiscard]] ScopeExit {
    static_assert(std::is_nothrow_invocable_v<std::decay_t<Callable>>);

public:
    ScopeExit(Callable &&F) noexcept : F(std::forward<Callable>(F)) {}

    ScopeExit(const ScopeExit &) = delete;
    void operator=(const ScopeExit &) = delete;

    ~ScopeExit() noexcept { F(); }

private:
    std::decay_t<Callable> F;
};

} // namespace sqldb
