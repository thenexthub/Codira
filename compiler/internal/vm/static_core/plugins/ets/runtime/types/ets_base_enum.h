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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_BASE_ENUM_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_BASE_ENUM_H

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

namespace test {
class EtsBaseEnumMembers;
}  // namespace test

class EtsBaseEnum : public EtsObject {
public:
    static EtsBaseEnum *FromEtsObject(EtsObject *etsObj)
    {
        ASSERT(etsObj->GetClass()->IsEtsEnum());
        return reinterpret_cast<EtsBaseEnum *>(etsObj);
    }

    EtsObject *GetValue()
    {
        return reinterpret_cast<EtsObject *>(GetFieldObject(GetValueOffset()));
    }

    static constexpr size_t GetValueOffset()
    {
        return MEMBER_OFFSET(EtsBaseEnum, value_);
    }

    EtsBaseEnum() = delete;
    ~EtsBaseEnum() = delete;

private:
    NO_COPY_SEMANTIC(EtsBaseEnum);
    NO_MOVE_SEMANTIC(EtsBaseEnum);

    ObjectPointer<EtsObject> value_;

    friend class test::EtsBaseEnumMembers;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_BASE_ENUM_H
