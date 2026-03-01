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
#ifndef PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_IMPL_H
#define PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_IMPL_H

#include <map>
#include "libarkbase/os/mutex.h"
#include <string>
#include <cstddef>
#include <libarkbase/macros.h>
#include <functional>

#if defined(PANDA_TARGET_OHOS)
#include <dlfcn.h>
#if defined(PANDA_CMAKE_SDK)
// NOTE(ivagin): cmake uses toolchain/sysroot from ohos sdk, and ohos sdk doesn't have below functions
// This should be fixed when panda sdk will use gn instead of cmake
extern "C" __attribute__((weak)) int dlns_get(const char *, Dl_namespace *);
extern "C" __attribute__((weak)) void *dlopen_ns(Dl_namespace *, const char *, int);
#endif
#endif

#include "libarkbase/os/library_loader.h"
#include "plugins/ets/runtime/ets_native_library.h"
using CreateNamespaceCallback = std::function<bool(const std::string &bundleModuleName, std::string &namespaceName)>;
using ExtensionApiCheckCallback = std::function<bool(const std::string &className, const std::string &fileName)>;
namespace ark::ets {
class EtsNamespaceManagerImpl {
public:
    static EtsNamespaceManagerImpl &GetInstance();

    Expected<EtsNativeLibrary, os::Error> LoadNativeLibraryFromNs(const std::string &pathKey, const char *name);
    void RegisterNamespaceName(const std::string &key, const std::string &value);

    void SetExtensionApiCheckCallback(const ExtensionApiCheckCallback &cb)
    {
        os::memory::WriteLockHolder wlh(lock_);
        checkLibraryPermissionCallback_ = cb;
    }

    ExtensionApiCheckCallback GetExtensionApiCheckCallback() const
    {
        os::memory::ReadLockHolder rlh(lock_);
        return checkLibraryPermissionCallback_;
    }

    virtual ~EtsNamespaceManagerImpl();

    NO_COPY_SEMANTIC(EtsNamespaceManagerImpl);

    NO_MOVE_SEMANTIC(EtsNamespaceManagerImpl);

private:
    EtsNamespaceManagerImpl() = default;

    mutable os::memory::RWLock lock_;
    std::map<std::string, std::string> namespaceNames_ GUARDED_BY(lock_);
    ExtensionApiCheckCallback checkLibraryPermissionCallback_ {nullptr};
};
}  // namespace ark::ets
#endif  // !PLUGINS_ETS_RUNTIME_NAMESPACE_MANAGER_IMPL_H