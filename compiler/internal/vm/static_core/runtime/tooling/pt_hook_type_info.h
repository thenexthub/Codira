/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_PT_HOOK_TYPE_INFO_H
#define PANDA_TOOLING_PT_HOOK_TYPE_INFO_H

#include <array>
#include "runtime/include/tooling/debug_interface.h"

namespace ark::tooling {
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class PtHookTypeInfo {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit PtHookTypeInfo(bool defalutValue)
    {
        for (auto &v : isEnabled_) {
            v = defalutValue;
        }
    }

    bool IsEnabled(const PtHookType type) const
    {
        return isEnabled_.at(ToIndex(type));
    }

    void Enable(const PtHookType type)
    {
        isEnabled_[ToIndex(type)] = true;
    }

    void Disable(const PtHookType type)
    {
        isEnabled_[ToIndex(type)] = false;
    }

    void EnableAll()
    {
        isEnabled_.fill(true);
    }

    void DisableAll()
    {
        isEnabled_.fill(false);
    }

    ~PtHookTypeInfo() = default;
    DEFAULT_COPY_SEMANTIC(PtHookTypeInfo);
    DEFAULT_MOVE_SEMANTIC(PtHookTypeInfo);

private:
    static constexpr size_t ToIndex(const PtHookType type)
    {
        auto index = static_cast<size_t>(type);
        ASSERT(index < HOOKS_COUNT);
        return index;
    }

    static constexpr size_t HOOKS_COUNT = static_cast<size_t>(PtHookType::PT_HOOK_TYPE_COUNT);
    std::array<bool, HOOKS_COUNT> isEnabled_;
};
}  // namespace ark::tooling

#endif  // PANDA_TOOLING_PT_HOOK_TYPE_INFO_H
