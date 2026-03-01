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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VVM_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VVM_H

#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets::ani::verify {

class VVm final {
public:
    ani_vm *GetVm()
    {
        return reinterpret_cast<ani_vm *>(this);
    }

    static VVm *GetInstance()
    {
        ani_vm *vm = PandaEtsVM::GetCurrent();
        return reinterpret_cast<VVm *>(vm);
    }
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_VVM_H
