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
#include "os/Loader.h"

#include <dlfcn.h>

#include "Base/Log.h"
namespace MapleRuntime {
namespace Os {
void* Loader::LoadBinaryFile(const char* path)
{
    if (path == nullptr) {
        return nullptr;
    }
    void* handler = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (handler == nullptr) {
        LOG(RTLOG_ERROR, dlerror());
    }
    return handler;
}

int Loader::UnloadBinaryFile(void* handler)
{
    if (handler == nullptr) {
        return -1;
    }
    int ret = dlclose(handler);
    return ret;
}

void* Loader::FindSymbol(void* handler, const char* symbolName)
{
    if (symbolName == nullptr) {
        return nullptr;
    }
    // handler can be nullptr
    return dlsym(handler, symbolName);
}

int Loader::GetBinaryInfoFromAddress(const void* address, BinaryInfo* binInfo)
{
    if (address == nullptr) {
        return 0;
    }
    Dl_info dlInfo;
    int ret = dladdr(address, &dlInfo);
    if (!ret) {
        return 0;
    }
    binInfo->filePathName = const_cast<char*>(dlInfo.dli_fname);
    binInfo->symbolName = const_cast<char*>(dlInfo.dli_sname);
    return ret;
}
} // namespace Os
} // namespace MapleRuntime
