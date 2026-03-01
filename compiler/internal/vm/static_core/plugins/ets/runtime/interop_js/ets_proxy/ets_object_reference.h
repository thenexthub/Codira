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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_OBJECT_REFERENCE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_OBJECT_REFERENCE_H_

#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include <memory>

namespace ark::mem {
class Reference;
}  // namespace ark::mem

namespace ark::ets::interop::js::ets_proxy {

class EtsObjectReferenceDeleter {
public:
    constexpr EtsObjectReferenceDeleter() noexcept = default;
    ~EtsObjectReferenceDeleter() noexcept = default;
    EtsObjectReferenceDeleter(const EtsObjectReferenceDeleter &) noexcept = default;
    EtsObjectReferenceDeleter &operator=(const EtsObjectReferenceDeleter &) noexcept = default;
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(EtsObjectReferenceDeleter);

    void operator()(mem::Reference *ref) const noexcept
    {
        INTEROP_LOG(FATAL) << "UniqueEtsObjectReference shouldn't delete automaticly, ref=" << ref;
    }
};

using UniqueEtsObjectReference = std::unique_ptr<mem::Reference, EtsObjectReferenceDeleter>;

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_OBJECT_REFERENCE_H_
