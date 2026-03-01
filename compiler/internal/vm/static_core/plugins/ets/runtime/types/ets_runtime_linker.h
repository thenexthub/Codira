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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_RUNTIME_LINKER_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_RUNTIME_LINKER_H

#include <cstdint>
#include "include/object_header.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark {
class ClassLinkerContext;
}  // namespace ark

namespace ark::ets {

namespace test {
class EtsRuntimeLinkerTest;
}  // namespace test

class EtsRuntimeLinker : public EtsObject {
public:
    EtsRuntimeLinker() = delete;
    ~EtsRuntimeLinker() = delete;

    NO_COPY_SEMANTIC(EtsRuntimeLinker);
    NO_MOVE_SEMANTIC(EtsRuntimeLinker);

    static EtsRuntimeLinker *FromEtsObject(EtsObject *obj)
    {
        return reinterpret_cast<EtsRuntimeLinker *>(obj);
    }

    static EtsRuntimeLinker *FromCoreType(ObjectHeader *obj)
    {
        return reinterpret_cast<EtsRuntimeLinker *>(obj);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    ClassLinkerContext *GetClassLinkerContext()
    {
        return reinterpret_cast<ClassLinkerContext *>(classLinkerCtxPtr_);
    }

    EtsClass *FindLoadedClass(const uint8_t *descriptor)
    {
        auto *ctx = GetClassLinkerContext();
        ASSERT(ctx != nullptr);
        auto *klass = ctx->FindClass(descriptor);
        return klass != nullptr ? EtsClass::FromRuntimeClass(klass) : nullptr;
    }

    void SetClassLinkerContext(ClassLinkerContext *ctx)
    {
        ASSERT(ctx != nullptr);
        classLinkerCtxPtr_ = reinterpret_cast<EtsLong>(ctx);
    }

private:
    // ets.RuntimeLinker fields BEGIN
    EtsLong classLinkerCtxPtr_;
    // ets.RuntimeLinker fields END

    friend class test::EtsRuntimeLinkerTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_RUNTIME_LINKER_H
