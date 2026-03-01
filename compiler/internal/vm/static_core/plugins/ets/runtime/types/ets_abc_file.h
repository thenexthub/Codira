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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_FILE_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_FILE_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

namespace test {
class EtsAbcRuntimeLinkerTest;
}  // namespace test

class EtsAbcFile : public EtsObject {
public:
    EtsAbcFile() = delete;
    ~EtsAbcFile() = delete;

    NO_COPY_SEMANTIC(EtsAbcFile);
    NO_MOVE_SEMANTIC(EtsAbcFile);

    static EtsAbcFile *FromEtsObject(EtsObject *obj)
    {
        return reinterpret_cast<EtsAbcFile *>(obj);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    panda_file::File *GetPandaFile()
    {
        return reinterpret_cast<panda_file::File *>(fileHandlePtr_);
    }

    void SetPandaFile(const panda_file::File *file)
    {
        fileHandlePtr_ = reinterpret_cast<EtsLong>(file);
    }

private:
    // ets.AbcFile fields BEGIN
    EtsLong fileHandlePtr_;
    // ets.AbcFile fields END

    friend class test::EtsAbcRuntimeLinkerTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ABC_FILE_H
