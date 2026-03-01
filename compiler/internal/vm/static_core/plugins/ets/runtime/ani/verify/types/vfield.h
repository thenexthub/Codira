/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VFIELD_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VFIELD_H

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"

namespace ark::ets {
class EtsField;
}  // namespace ark::ets

namespace ark::ets::ani::verify {

namespace impl {

class VField {
public:
    enum class ANIFieldType {
        FIELD,
        STATIC_FIELD,
        VARIABLE,
    };
    explicit VField(EtsField *field) : field_(field) {}
    ~VField() = default;

    EtsField *GetEtsField() const
    {
        return field_;
    }

    ANIFieldType GetType() const;

    DEFAULT_COPY_SEMANTIC(VField);
    DEFAULT_MOVE_SEMANTIC(VField);

private:
    EtsField *field_;
};

}  // namespace impl

class VField : public impl::VField {
public:
    ani_field GetField() const;
};

class VStaticField : public impl::VField {
public:
    ani_static_field GetField() const;
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_FIELD_H
