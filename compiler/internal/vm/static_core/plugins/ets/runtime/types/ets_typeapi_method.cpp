/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "plugins/ets/runtime/types/ets_typeapi_method.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {

EtsTypeAPIMethod *EtsTypeAPIMethod::Create(EtsCoroutine *etsCoroutine)
{
    EtsClass *klass = PlatformTypes(etsCoroutine)->coreMethod;
    EtsObject *etsObject = EtsObject::Create(etsCoroutine, klass);
    return reinterpret_cast<EtsTypeAPIMethod *>(etsObject);
}

EtsMethod *EtsTypeAPIMethod::GetEtsMethod()
{
    return EtsMethod::FromTypeDescriptor(methodType_->GetRuntimeTypeDescriptor()->GetMutf8(),
                                         methodType_->GetContextLinker());
}

}  // namespace ark::ets
