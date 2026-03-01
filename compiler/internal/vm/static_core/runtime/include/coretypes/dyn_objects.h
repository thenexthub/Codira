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

#ifndef PANDA_RUNTIME_CORETYPES_DYNOBJECTS_H_
#define PANDA_RUNTIME_CORETYPES_DYNOBJECTS_H_

#include "runtime/include/hclass.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/vm_handle.h"

namespace ark::coretypes {
// Class for objects in dynamic languages like JavaScript
class DynClass : public ObjectHeader {
public:
    static DynClass *Cast(ObjectHeader *object)
    {
        ASSERT(object->ClassAddr<HClass>()->IsDynamicClass());
        return reinterpret_cast<DynClass *>(object);
    }

    HClass *GetHClass()
    {
        return reinterpret_cast<HClass *>(klass_);
    }

    static constexpr size_t GetHClassOffset()
    {
        return MEMBER_OFFSET(DynClass, klass_);
    }

private:
    DynClass() : ObjectHeader() {}

    // Actually HClass is located here.
    __extension__ uint8_t klass_[0];  // NOLINT(modernize-avoid-c-arrays)
};

class DynObject : public ObjectHeader {
public:
    static DynObject *Cast(ObjectHeader *object)
    {
        ASSERT(object->ClassAddr<HClass>()->IsDynamicClass());
        return reinterpret_cast<DynObject *>(object);
    }

    bool IsBuiltinsConstructor()
    {
        return ClassAddr<HClass>()->IsBuiltinsConstructor();
    }

    bool IsCallable()
    {
        return ClassAddr<HClass>()->IsCallable();
    }
};

}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_DYNOBJECTS_H_
