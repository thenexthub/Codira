/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets {

EtsClass *EtsField::GetDeclaringClass() const
{
    return EtsClass::FromRuntimeClass(GetCoreType()->GetClass());
}

EtsClass *EtsField::GetType() const
{
    Class *klass = GetCoreType()->ResolveTypeClass();
    return klass != nullptr ? EtsClass::FromRuntimeClass(klass) : nullptr;
}

EtsString *EtsField::GetNameString() const
{
    panda_file::File::StringData nameData {};
    nameData = GetCoreType()->GetName();

    return EtsString::Resolve(nameData.data, nameData.utf16Length);
}

const char *EtsField::GetTypeDescriptor() const
{
    return utf::Mutf8AsCString(GetCoreType()->ResolveTypeClass()->GetDescriptor());
}

bool EtsField::IsDeclaredIn(const EtsClass *klass) const
{
    return this->GetDeclaringClass() == klass;
}

}  // namespace ark::ets