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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_CONVERTERS_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_CONVERTERS_H

#include "ani.h"
#include "plugins/ets/runtime/ani/ani_type_check.h"

namespace ark::ets::ani {

inline ani_field ToAniField(EtsField *field)
{
    ASSERT(field != nullptr);
    ASSERT(IsInstanceField(field));
    return reinterpret_cast<ani_field>(field);
}

inline EtsField *ToInternalField(ani_field field)
{
    auto *f = reinterpret_cast<EtsField *>(field);
    ASSERT(f != nullptr);
    ASSERT(IsInstanceField(f));
    return f;
}

inline ani_static_field ToAniStaticField(EtsField *field)
{
    ASSERT(field != nullptr);
    ASSERT(IsStaticField(field));
    return reinterpret_cast<ani_static_field>(field);
}

inline EtsField *ToInternalField(ani_static_field field)
{
    auto *f = reinterpret_cast<EtsField *>(field);
    ASSERT(f != nullptr);
    ASSERT(IsStaticField(f));
    return f;
}

inline ani_variable ToAniVariable(EtsField *variable)
{
    ASSERT(variable != nullptr);
    ASSERT(IsVariable(variable));
    return reinterpret_cast<ani_variable>(variable);
}

inline EtsField *ToInternalField(ani_variable variable)
{
    auto *v = reinterpret_cast<EtsField *>(variable);
    ASSERT(v != nullptr);
    ASSERT(IsVariable(v));
    return v;
}

inline EtsMethod *ToInternalMethod(ani_method method)
{
    auto *m = reinterpret_cast<EtsMethod *>(method);
    ASSERT(m != nullptr);
    ASSERT(IsInstanceMethod(m));
    return m;
}

inline ani_method ToAniMethod(EtsMethod *method)
{
    ASSERT(method != nullptr);
    ASSERT(IsInstanceMethod(method));
    return reinterpret_cast<ani_method>(method);
}

inline EtsMethod *ToInternalMethod(ani_static_method method)
{
    auto *m = reinterpret_cast<EtsMethod *>(method);
    ASSERT(m != nullptr);
    ASSERT(IsStaticMethod(m));
    return m;
}

inline ani_static_method ToAniStaticMethod(EtsMethod *method)
{
    ASSERT(method != nullptr);
    ASSERT(IsStaticMethod(method));
    return reinterpret_cast<ani_static_method>(method);
}

inline EtsMethod *ToInternalMethod(ani_function fn)
{
    auto *m = reinterpret_cast<EtsMethod *>(fn);
    ASSERT(m != nullptr);
    ASSERT(IsFunction(m));
    return m;
}

inline ani_function ToAniFunction(EtsMethod *method)
{
    ASSERT(method != nullptr);
    ASSERT(IsFunction(method));
    return reinterpret_cast<ani_function>(method);
}

}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_CONVERTERS_H
