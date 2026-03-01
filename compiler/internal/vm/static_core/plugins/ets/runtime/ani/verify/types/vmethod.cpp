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

#include "vmethod.h"

#include "plugins/ets/runtime/ani/ani_converters.h"

namespace ark::ets::ani::verify {

namespace impl {

VMethod::ANIMethodType VMethod::GetType() const
{
    if (IsFunction(method_)) {
        return ANIMethodType::FUNCTION;
    }
    if (IsStaticMethod(method_)) {
        return ANIMethodType::STATIC_METHOD;
    }
    return ANIMethodType::METHOD;
}

}  // namespace impl

ani_method VMethod::GetMethod() const
{
    return ToAniMethod(impl::VMethod::GetEtsMethod());
}

ani_static_method VStaticMethod::GetMethod() const
{
    return ToAniStaticMethod(impl::VMethod::GetEtsMethod());
}

ani_function VFunction::GetMethod() const
{
    return ToAniFunction(impl::VMethod::GetEtsMethod());
}

}  // namespace ark::ets::ani::verify
