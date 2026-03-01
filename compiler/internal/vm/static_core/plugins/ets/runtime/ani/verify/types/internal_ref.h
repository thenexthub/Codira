/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_INTERNAL_REF_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_INTERNAL_REF_H

#include "plugins/ets/runtime/ani/verify/types/vref.h"

namespace ark::ets::ani::verify {

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class InternalRef {
public:
    explicit InternalRef(ani_ref ref) : ref_(ref) {}

    static VRef *CastToVRef(InternalRef *iref);
    static InternalRef *CastFromVRef(VRef *vref);

    static bool IsStackVRef(VRef *vref);
    static bool IsUndefinedStackRef(VRef *vref);

    ani_ref GetRef()
    {
        return ref_;
    }

    DEFAULT_COPY_SEMANTIC(InternalRef);
    DEFAULT_MOVE_SEMANTIC(InternalRef);

private:
    ani_ref ref_;

    static constexpr uintptr_t TYPE_MASK = static_cast<uintptr_t>(1U);
    static constexpr uintptr_t TYPE_VREF = static_cast<uintptr_t>(1U);
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_INTERNAL_REF_H
