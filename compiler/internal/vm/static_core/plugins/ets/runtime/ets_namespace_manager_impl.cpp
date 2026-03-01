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

#include <cstring>
#include "plugins/ets/runtime/ets_namespace_manager_impl.h"
#include "libarkbase/utils/logger.h"

namespace ark::ets {

EtsNamespaceManagerImpl::~EtsNamespaceManagerImpl()
{
    os::memory::WriteLockHolder lock(lock_);
    namespaceNames_.clear();
}

EtsNamespaceManagerImpl &EtsNamespaceManagerImpl::GetInstance()
{
    static EtsNamespaceManagerImpl instance;
    return instance;
}

void EtsNamespaceManagerImpl::RegisterNamespaceName(const std::string &key, const std::string &value)
{
    os::memory::WriteLockHolder lock(lock_);
    namespaceNames_[key] = value;
}

Expected<EtsNativeLibrary, os::Error> EtsNamespaceManagerImpl::LoadNativeLibraryFromNs(const std::string &pathKey,
                                                                                       const char *name)
{
    LOG(INFO, RUNTIME) << "EtsNamespaceManagerImpl::LoadNativeLibraryFromNs pathKey :" << pathKey.c_str()
                       << " loading library name :" << name;
#if defined(PANDA_TARGET_OHOS)
    PandaString errInfo = "EtsNamespaceManagerImpl::LoadNativeLibraryFromNs: ";
    std::string abcPath = pathKey;
    std::string namespaceName;
    os::memory::ReadLockHolder lock(lock_);
    if (namespaceNames_.find(abcPath) != namespaceNames_.end()) {
        namespaceName = namespaceNames_[abcPath];
    } else {
        abcPath = "default";
        if (namespaceNames_.find(abcPath) != namespaceNames_.end()) {
            LOG(WARNING, RUNTIME) << "EtsNamespaceManagerImpl::LoadNativeLibraryFromNs: " << abcPath.c_str()
                                  << "not  find namespaceNames_, use `pathNS_default` ns";
            namespaceName = namespaceNames_[abcPath];
        } else {
            errInfo += "pathKey: " + abcPath + " and `default` not found in namespaceNames_";
            return Unexpected(os::Error(errInfo.c_str()));
        }
    }
    Dl_namespace ns;
    if (dlns_get(namespaceName.data(), &ns) != 0) {
        errInfo += "pathKey: " + abcPath + "namespaceName: " + namespaceName + " not found";
        return Unexpected(os::Error(errInfo.c_str()));
    }
    void *nativeHandle = nullptr;
    nativeHandle = dlopen_ns(&ns, name, RTLD_LAZY);
    if (nativeHandle == nullptr) {
        char *dlerr = dlerror();
        auto dlerrMsg = dlerr != nullptr ? dlerr
                                         : "Error loading path " + std::string(name) + "in namespace" +
                                               std::string(namespaceName) + ":No such file or directory";
        errInfo += "load app library failed. " + std::string(dlerrMsg);
        return Unexpected(os::Error(errInfo.c_str()));
    }
    os::library_loader::LibraryHandle handle(nativeHandle);
    return EtsNativeLibrary(PandaString(name), std::move(handle));
#endif
    return Unexpected(os::Error("not support platform"));
}
}  // namespace ark::ets