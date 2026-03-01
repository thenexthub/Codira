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

#include "libarkfile/external/file_ext.h"
#include "libarkfile/external/panda_file_external.h"
#include "libarkbase/os/mutex.h"
#include <dlfcn.h>
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"

namespace panda_api::panda_file {

decltype(OpenPandafileFromFdExt) *PandaFileWrapper::pOpenPandafileFromFdExt = nullptr;
decltype(OpenPandafileFromMemoryExt) *PandaFileWrapper::pOpenPandafileFromMemoryExt = nullptr;
decltype(QueryMethodSymByOffsetExt) *PandaFileWrapper::pQueryMethodSymByOffsetExt = nullptr;
decltype(QueryMethodSymAndLineByOffsetExt) *PandaFileWrapper::pQueryMethodSymAndLineByOffsetExt = nullptr;
decltype(QueryAllMethodSymsExt) *PandaFileWrapper::pQueryAllMethodSymsExt = nullptr;

using OnceCallback = void();
static void CallOnce(bool *onceFlag, OnceCallback callBack)
{
    static ark::os::memory::Mutex onceLock;
    ark::os::memory::LockHolder lock(onceLock);
    if (!(*onceFlag)) {
        callBack();
        *onceFlag = true;
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOAD_FUNC_OR_RETURN(CLASS, FUNC, HANDLE)                                           \
    do {                                                                                   \
        CLASS::p##FUNC = reinterpret_cast<decltype(CLASS::p##FUNC)>(dlsym(HANDLE, #FUNC)); \
        if (CLASS::p##FUNC == nullptr) {                                                   \
            ClearAllLoadedFuncs();                                                         \
            dlclose(HANDLE);                                                               \
            return;                                                                        \
        }                                                                                  \
    } while (0)

static void ClearAllLoadedFuncs()
{
    PandaFileWrapper::pOpenPandafileFromFdExt = nullptr;
    PandaFileWrapper::pOpenPandafileFromMemoryExt = nullptr;
    PandaFileWrapper::pQueryMethodSymByOffsetExt = nullptr;
    PandaFileWrapper::pQueryMethodSymAndLineByOffsetExt = nullptr;
    PandaFileWrapper::pQueryAllMethodSymsExt = nullptr;
}

void LoadPandFileExt()
{
    static bool dlopenOnce = false;
    CallOnce(&dlopenOnce, []() {
        const char pandafileext[] = "libarkfileExt.so";
        void *hd = dlopen(pandafileext, RTLD_NOW);
        if (hd == nullptr) {
            return;
        }

        LOAD_FUNC_OR_RETURN(PandaFileWrapper, OpenPandafileFromFdExt, hd);
        LOAD_FUNC_OR_RETURN(PandaFileWrapper, OpenPandafileFromMemoryExt, hd);
        LOAD_FUNC_OR_RETURN(PandaFileWrapper, QueryMethodSymByOffsetExt, hd);
        LOAD_FUNC_OR_RETURN(PandaFileWrapper, QueryMethodSymAndLineByOffsetExt, hd);
        LOAD_FUNC_OR_RETURN(PandaFileWrapper, QueryAllMethodSymsExt, hd);
    });
}
#undef LOAD_FUNC_OR_RETURN

}  // namespace panda_api::panda_file
