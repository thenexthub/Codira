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
#include "plugins/ets/runtime/ets_namespace_manager.h"
#include "plugins/ets/runtime/ets_namespace_manager_impl.h"
#include "libarkbase/utils/logger.h"

namespace ark::ets {

PANDA_PUBLIC_API void EtsNamespaceManager::SetAppLibPaths(const AppBundleModuleNamePathMap &appModuleNames,
                                                          CreateNamespaceCallback &cb)
{
    LOG(INFO, RUNTIME) << "set ets app library path";
    if (appModuleNames.empty()) {
        LOG(ERROR, RUNTIME) << "no ets app library path to set";
        return;
    }
    ASSERT(cb != nullptr);
    EtsNamespaceManagerImpl &instance = EtsNamespaceManagerImpl::GetInstance();
    for (const auto &bundleModuleName : appModuleNames) {
        std::string namespaceName;
        if (cb(bundleModuleName.second, namespaceName)) {
            instance.RegisterNamespaceName(bundleModuleName.first, namespaceName);
        }
    }
}

PANDA_PUBLIC_API void EtsNamespaceManager::SetExtensionApiCheckCallback(const ExtensionApiCheckCallback &cb)
{
    LOG(INFO, RUNTIME) << "Set ExtensionApiCheckCallback begins";
    EtsNamespaceManagerImpl &instance = EtsNamespaceManagerImpl::GetInstance();
    instance.SetExtensionApiCheckCallback(cb);
}

}  // namespace ark::ets