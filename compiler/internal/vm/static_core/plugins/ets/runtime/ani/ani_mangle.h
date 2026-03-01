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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_MANGLE_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_MANGLE_H

#include "runtime/include/mem/panda_string.h"

namespace ark::ets {
class EtsMethodSignature;
}  // namespace ark::ets

namespace ark::ets::ani {

class Mangle {
public:
    static std::optional<PandaString> ConvertDescriptor(const std::string_view descriptor, bool allowArray = false);
    static std::optional<PandaString> ConvertSignature(const std::string_view descriptor);
    static void ConvertSignatureToProto(std::optional<EtsMethodSignature> &methodSignature,
                                        const std::string_view descriptor);
};

}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_MANGLE_H
