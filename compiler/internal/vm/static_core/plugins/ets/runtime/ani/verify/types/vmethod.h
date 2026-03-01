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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VMETHOD_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VMETHOD_H

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"

namespace ark::ets {
class EtsMethod;
}  // namespace ark::ets

namespace ark::ets::ani::verify {

namespace impl {

class VMethod {
public:
    enum class ANIMethodType {
        FUNCTION,
        METHOD,
        STATIC_METHOD,
    };
    explicit VMethod(EtsMethod *method) : method_(method) {}
    ~VMethod() = default;

    EtsMethod *GetEtsMethod() const
    {
        return method_;
    }

    ANIMethodType GetType() const;

    DEFAULT_COPY_SEMANTIC(VMethod);
    DEFAULT_MOVE_SEMANTIC(VMethod);

private:
    EtsMethod *method_;
};

}  // namespace impl

class VMethod : public impl::VMethod {
public:
    ani_method GetMethod() const;
};

class VStaticMethod : public impl::VMethod {
public:
    ani_static_method GetMethod() const;
};

class VFunction : public impl::VMethod {
public:
    ani_function GetMethod() const;
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VMETHOD_H
