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

#ifndef PANDA_PLUGINS_ETS_STATIC_TYPE_CONVERTER_H
#define PANDA_PLUGINS_ETS_STATIC_TYPE_CONVERTER_H

#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/static_type_converter_interface.h"

namespace ark::ets {
class StaticTypeConverter : public common::StaticTypeConverterInterface {
public:
    static void Initialize();

    common::BoxedValue PUBLIC_API WrapBoxed(common::BaseType value) override;

    common::BaseType PUBLIC_API UnwrapBoxed(common::BoxedValue value) override;

private:
    static StaticTypeConverter stcTypeConverter_;
};
}  // namespace ark::ets
#endif  // PANDA_PLUGINS_ETS_STATIC_TYPE_CONVERTER_H
