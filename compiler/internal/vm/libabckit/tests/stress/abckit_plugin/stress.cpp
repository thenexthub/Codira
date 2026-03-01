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

#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/statuses.h"

#include <iostream>
#include <functional>

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static void TransformMethod(AbckitCoreFunction *method)
{
    if (g_implI->moduleGetTarget(g_implI->functionGetModule(method)) == ABCKIT_TARGET_ARK_TS_V2) {
        auto *arktsMethod = g_implArkI->coreFunctionToArktsFunction(method);
        if (g_implArkI->functionIsNative(arktsMethod)) {
            return;
        }
    }
    AbckitGraph *graph = g_implI->createGraphFromFunction(method);
    if (g_impl->getLastError() != ABCKIT_STATUS_NO_ERROR) {
        return;
    }
    g_implM->functionSetGraph(method, graph);
    g_impl->destroyGraph(graph);
}

static void EnumerateAllMethodsInModule(AbckitFile *file, std::function<bool(AbckitCoreNamespace *)> &cbNamespace,
                                        std::function<bool(AbckitCoreClass *)> &cbClass,
                                        std::function<bool(AbckitCoreFunction *)> &cbFunc, bool &hasError)
{
    std::function<bool(AbckitCoreModule *)> cbModule = [&](AbckitCoreModule *m) {
        g_implI->moduleEnumerateNamespaces(m, &cbNamespace, [](AbckitCoreNamespace *n, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreNamespace *)> *>(cb))(n);
        });
        g_implI->moduleEnumerateClasses(m, &cbClass, [](AbckitCoreClass *c, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreClass *)> *>(cb))(c);
        });
        g_implI->moduleEnumerateTopLevelFunctions(m, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(f);
        });
        return !hasError;
    };

    g_implI->fileEnumerateModules(file, &cbModule, [](AbckitCoreModule *m, void *cb) {
        return (*reinterpret_cast<std::function<bool(AbckitCoreModule *)> *>(cb))(m);
    });
}

extern "C" int Entry(AbckitFile *file)
{
    bool hasError = false;

    std::function<bool(AbckitCoreClass *)> cbClass;

    std::function<bool(AbckitCoreFunction *)> cbFunc = [&](AbckitCoreFunction *f) {
        g_implI->functionEnumerateNestedFunctions(f, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(f);
        });
        g_implI->functionEnumerateNestedClasses(f, &cbClass, [](AbckitCoreClass *c, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreClass *)> *>(cb))(c);
        });

        TransformMethod(f);
        if (g_impl->getLastError() != ABCKIT_STATUS_NO_ERROR) {
            hasError = true;
        }

        return !hasError;
    };

    cbClass = [&](AbckitCoreClass *c) {
        g_implI->classEnumerateMethods(c, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(m);
        });
        return !hasError;
    };

    std::function<bool(AbckitCoreNamespace *)> cbNamespace = [&](AbckitCoreNamespace *n) {
        g_implI->namespaceEnumerateNamespaces(n, &cbNamespace, [](AbckitCoreNamespace *n, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreNamespace *)> *>(cb))(n);
        });
        g_implI->namespaceEnumerateClasses(n, &cbClass, [](AbckitCoreClass *c, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreClass *)> *>(cb))(c);
        });
        g_implI->namespaceEnumerateTopLevelFunctions(n, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            return (*reinterpret_cast<std::function<bool(AbckitCoreFunction *)> *>(cb))(f);
        });
        return !hasError;
    };

    std::cout << "Stress plugin start" << std::endl;

    EnumerateAllMethodsInModule(file, cbNamespace, cbClass, cbFunc, hasError);

    std::cout << "Stress plugin end" << std::endl;

    return hasError ? 1 : 0;
}
