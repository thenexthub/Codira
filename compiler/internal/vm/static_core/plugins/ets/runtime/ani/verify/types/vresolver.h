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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VRESOLVER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VRESOLVER_H

#include "plugins/ets/runtime/ani/ani.h"

namespace ark::ets::ani::verify {

class VResolver {
public:
    explicit VResolver(ani_resolver resolver) : resolver_(resolver) {}

    ani_resolver GetResolver()
    {
        return resolver_;
    }

private:
    ani_resolver resolver_;
};

}  // namespace ark::ets::ani::verify
#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VRESOLVER_H
