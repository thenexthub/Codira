/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_STATIC_OBJECT_ACCESSOR_H
#define PANDA_PLUGINS_ETS_RUNTIME_STATIC_OBJECT_ACCESSOR_H

#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/static_object_accessor_interface.h"

namespace ark::ets {
class StaticObjectAccessor : public common::StaticObjectAccessorInterface {
public:
    bool HasProperty(common::ThreadHolder *thread, const common::BaseObject *obj, const char *name) override;

    common::BoxedValue GetProperty(common::ThreadHolder *thread, const common::BaseObject *obj,
                                   const char *name) override;

    bool SetProperty(common::ThreadHolder *thread, common::BaseObject *obj, const char *name,
                     common::BoxedValue value) override;

    bool HasElementByIdx(common::ThreadHolder *thread, const common::BaseObject *obj, const uint32_t index) override;

    common::BoxedValue GetElementByIdx(common::ThreadHolder *thread, const common::BaseObject *obj,
                                       const uint32_t index) override;

    bool SetElementByIdx(common::ThreadHolder *thread, common::BaseObject *obj, uint32_t index,
                         const common::BoxedValue value) override;

private:
    static StaticObjectAccessor stcObjAccessor_;
};
}  // namespace ark::ets
#endif  // PANDA_PLUGINS_ETS_RUNTIME_STATIC_OBJECT_ACCESSOR_H
