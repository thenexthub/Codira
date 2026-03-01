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

#ifndef PANDA_ARK_NAPI_HELPER_H
#define PANDA_ARK_NAPI_HELPER_H

#include <js_native_api_types.h>
#include "common_interfaces/objects/base_type.h"

namespace ark::ets::interop::js {

using common::BaseObject;
using common::TaggedType;

class PUBLIC_API ArkNapiHelper {
public:
    static inline TaggedType GetTaggedType([[maybe_unused]] napi_value napiValue)
    {
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
        return *(reinterpret_cast<TaggedType *>(napiValue));
#else
        INTEROP_LOG(FATAL) << "unable to preform raw napi invocation without ohos";
        return 0;
#endif
    }

    static inline BaseObject *ToBaseObject(napi_value napiValue)
    {
        return BaseObject::Cast(static_cast<common::MAddress>(GetTaggedType(napiValue)));
    }

    static inline napi_value ToNapiValue([[maybe_unused]] TaggedType *taggedType)
    {
#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
        return reinterpret_cast<napi_value>(taggedType);
#else
        INTEROP_LOG(FATAL) << "unable to preform raw napi invocation without ohos";
        return nullptr;
#endif
    }
};
}  // namespace ark::ets::interop::js
#endif  // PANDA_ARK_NAPI_HELPER_H
