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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_TYPE_CHECK_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_TYPE_CHECK_H

#include "ani.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets::ani {

inline bool IsInstanceField(EtsField *field)
{
    return !field->IsStatic() && !field->GetDeclaringClass()->IsModule();
}

inline bool IsStaticField(EtsField *field)
{
    return field->IsStatic() && !field->GetDeclaringClass()->IsModule();
}

inline bool IsVariable(EtsField *field)
{
    return field->IsStatic() && field->GetDeclaringClass()->IsModule();
}

inline bool IsInstanceMethod(EtsMethod *method)
{
    return !method->IsStatic() && !method->GetClass()->IsModule();
}

inline bool IsStaticMethod(EtsMethod *method)
{
    return method->IsStatic() && !method->GetClass()->IsModule();
}

inline bool IsFunction(EtsMethod *method)
{
    return method->IsStatic() && method->GetClass()->IsModule();
}

}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_TYPE_CHECK_H
