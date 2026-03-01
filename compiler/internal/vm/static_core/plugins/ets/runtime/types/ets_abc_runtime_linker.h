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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_RUNTIME_LINKER_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_RUNTIME_LINKER_H

#include "include/object_accessor.h"
#include "include/object_header.h"
#include "libarkbase/mem/object_pointer.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"

namespace ark::ets {

namespace test {
class EtsAbcRuntimeLinkerTest;
}  // namespace test

class EtsAbcRuntimeLinker : public EtsRuntimeLinker {
public:
    EtsAbcRuntimeLinker() = delete;
    ~EtsAbcRuntimeLinker() = delete;

    NO_COPY_SEMANTIC(EtsAbcRuntimeLinker);
    NO_MOVE_SEMANTIC(EtsAbcRuntimeLinker);

    static EtsAbcRuntimeLinker *FromEtsObject(EtsObject *obj)
    {
        return reinterpret_cast<EtsAbcRuntimeLinker *>(obj);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    EtsObjectArray *GetAbcFiles()
    {
        return EtsObjectArray::FromCoreType(
            ObjectAccessor::GetObject(this, MEMBER_OFFSET(EtsAbcRuntimeLinker, abcFiles_)));
    }

    void SetAbcFiles(EtsObjectArray *abcFiles)
    {
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsAbcRuntimeLinker, abcFiles_), abcFiles->GetCoreType());
    }

    EtsRuntimeLinker *GetParentLinker()
    {
        return EtsRuntimeLinker::FromCoreType(
            ObjectAccessor::GetObject(this, MEMBER_OFFSET(EtsAbcRuntimeLinker, parentLinker_)));
    }

    EtsObjectArray *GetSharedLibraryRuntimeLinkers()
    {
        return EtsObjectArray::FromCoreType(
            ObjectAccessor::GetObject(this, MEMBER_OFFSET(EtsAbcRuntimeLinker, sharedRuntimeLinkers_)));
    }

private:
    // ets.AbcRuntimeLinker fields BEGIN
    ObjectPointer<EtsObject> parentLinker_;
    ObjectPointer<EtsObjectArray> abcFiles_;
    ObjectPointer<EtsObjectArray> sharedRuntimeLinkers_;
    // ets.AbcRuntimeLinker fields END

    friend class test::EtsAbcRuntimeLinkerTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_RUNTIME_LINKER_H
