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

#include "default_plugin.h"
#include "abs_int_inl_compat_checks.h"
#include <libarkfile/include/source_lang_enum.h>
#include "verifier_messages.h"
#include "runtime/include/mtmanaged_thread.h"
#include "runtime/include/runtime.h"

namespace ark::verifier::plugin {

ManagedThread *DefaultPlugin::CreateManagedThread() const
{
    return MTManagedThread::Create(Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
}

void DefaultPlugin::DestroyManagedThread(ManagedThread *thr) const
{
    MTManagedThread::CastFromThread(thr)->Destroy();
}

void DefaultPlugin::TypeSystemSetup(TypeSystem *types) const
{
    types->SetSupertypeOfArray(types->Object());
}

Type DefaultPlugin::NormalizeType(Type type, TypeSystem *types) const
{
    auto integral32 = Type {Type::Builtin::INTEGRAL32};
    auto integral64 = Type {Type::Builtin::INTEGRAL64};
    auto float32 = Type {Type::Builtin::FLOAT32};
    auto float64 = Type {Type::Builtin::FLOAT64};
    Type result;
    if (IsSubtype(type, integral32, types)) {
        result = integral32;
    } else if (IsSubtype(type, integral64, types)) {
        result = integral64;
    } else if (IsSubtype(type, float32, types)) {
        result = float32;
    } else if (IsSubtype(type, float64, types)) {
        result = float64;
    } else {
        result = type;
    }
    return result;
}

}  // namespace ark::verifier::plugin
