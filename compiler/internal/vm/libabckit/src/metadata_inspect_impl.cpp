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

#include <cassert>
#include "libabckit/c/abckit.h"
#include "libabckit/c/metadata_core.h"

#include "libabckit/src/helpers_common.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/macros.h"
#include "scoped_timer.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/metadata_arkts_inspect_impl.h"
#include "libabckit/src/metadata_js_inspect_impl.h"
#include "libabckit/src/metadata_unknown_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/abckit_dynamic.h"
#include "libabckit/src/adapter_dynamic/metadata_inspect_dynamic.h"
#include "libabckit/src/adapter_static/abckit_static.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"

namespace libabckit {

// ========================================
// File
// ========================================

extern "C" AbckitFileVersion FileGetVersion(AbckitFile *file)
{
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(file, nullptr);

    return file->version;
}

extern "C" bool FileEnumerateModules(AbckitFile *file, void *data, bool (*cb)(AbckitCoreModule *module, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(file, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    bool isNormalExit = true;
    for (auto &[moduleName, module] : file->localModules) {
        isNormalExit &= cb(module.get(), data);
    }
    for (auto &[moduleName, module] : file->externalModules) {
        isNormalExit &= cb(module.get(), data);
    }
    return isNormalExit;
}

extern "C" bool FileEnumerateExternalModules(AbckitFile *file, void *data,
                                             bool (*cb)(AbckitCoreModule *module, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(file, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    bool isNormalExit = true;
    for (auto &[moduleName, module] : file->externalModules) {
        isNormalExit &= cb(module.get(), data);
    }
    return isNormalExit;
}

// ========================================
// Module
// ========================================

extern "C" AbckitFile *ModuleGetFile(AbckitCoreModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    return m->file;
}

extern "C" AbckitTarget ModuleGetTarget(AbckitCoreModule *m)
{
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, ABCKIT_TARGET_UNKNOWN);

    return m->target;
}

extern "C" AbckitString *ModuleGetName(AbckitCoreModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    return m->moduleName;
}

extern "C" bool ModuleIsExternal(AbckitCoreModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, false);
    return m->isExternal;
}

extern "C" bool ModuleEnumerateImports(AbckitCoreModule *m, void *data,
                                       bool (*cb)(AbckitCoreImportDescriptor *i, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(m, false);
    LIBABCKIT_BAD_ARGUMENT(cb, false);
    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateImports(m, data, cb);
        case ABCKIT_TARGET_JS:
            return JsModuleEnumerateImports(m, data, cb);
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateExports(AbckitCoreModule *m, void *data,
                                       bool (*cb)(AbckitCoreExportDescriptor *e, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateExports(m, data, cb);
        case ABCKIT_TARGET_JS:
            return JsModuleEnumerateExports(m, data, cb);
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateNamespaces(AbckitCoreModule *m, void *data,
                                          bool (*cb)(AbckitCoreNamespace *n, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateNamespaces(m, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateClasses(m, data, cb);
        case ABCKIT_TARGET_JS:
            return JsModuleEnumerateClasses(m, data, cb);
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateInterfaces(AbckitCoreModule *m, void *data,
                                          bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateInterfaces(m, data, cb);
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_JS:
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateEnums(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateEnums(m, data, cb);
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_JS:
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateFields(AbckitCoreModule *m, void *data,
                                      bool (*cb)(AbckitCoreModuleField *field, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateFields(m, data, cb);
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_JS:
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                                 bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateTopLevelFunctions(m, data, cb);
        case ABCKIT_TARGET_JS:
            return JsModuleEnumerateTopLevelFunctions(m, data, cb);
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                                  bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateAnonymousFunctions(m, data, cb);
        case ABCKIT_TARGET_JS:
            return JsModuleEnumerateAnonymousFunctions(m, data, cb);
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                                    bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSModuleEnumerateAnnotationInterfaces(m, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        case ABCKIT_TARGET_UNKNOWN:
            return UnknownModuleEnumerateStub(m);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Namespace
// ========================================

extern "C" AbckitCoreModule *NamespaceGetModule(AbckitCoreNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    return n->owningModule;
}

extern "C" AbckitString *NamespaceGetName(AbckitCoreNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    switch (n->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return NamespaceGetNameDynamic(n);
        case ABCKIT_TARGET_ARK_TS_V2:
            return NamespaceGetNameStatic(n);
        case ABCKIT_TARGET_JS:
            statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool NamespaceIsExternal(AbckitCoreNamespace *ns)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(ns, false);

    if (IsDynamic(ns->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return NamespaceIsExternalStatic(ns);
}

extern "C" AbckitCoreNamespace *NamespaceGetParentNamespace(AbckitCoreNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    return n->parentNamespace;
}

extern "C" bool NamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                             bool (*cb)(AbckitCoreNamespace *klass, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(n->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSNamespaceEnumerateNamespaces(n, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool NamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data,
                                          bool (*cb)(AbckitCoreClass *klass, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(n->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSNamespaceEnumerateClasses(n, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool NamespaceEnumerateInterfaces(AbckitCoreNamespace *n, void *data,
                                             bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(n->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSNamespaceEnumerateInterfaces(n, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool NamespaceEnumerateEnums(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(n->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSNamespaceEnumerateEnums(n, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool NamespaceEnumerateFields(AbckitCoreNamespace *n, void *data,
                                         bool (*cb)(AbckitCoreNamespaceField *field, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(n->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSNamespaceEnumerateFields(n, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool NamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                                    bool (*cb)(AbckitCoreFunction *func, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(n->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSNamespaceEnumerateTopLevelFunctions(n, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool NamespaceEnumerateAnnotationInterfaces(AbckitCoreNamespace *n, void *data,
                                                       bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(n, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(n->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSNamespaceEnumerateAnnotationInterfaces(n, data, cb);
        case ABCKIT_TARGET_JS:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// ImportDescriptor
// ========================================

extern "C" AbckitFile *ImportDescriptorGetFile(AbckitCoreImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    return i->importingModule->file;
}

extern "C" AbckitCoreModule *ImportDescriptorGetImportedModule(AbckitCoreImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    return i->importedModule;
}

extern "C" AbckitCoreModule *ImportDescriptorGetImportingModule(AbckitCoreImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    return i->importingModule;
}

extern "C" AbckitString *ImportDescriptorGetName(AbckitCoreImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    if (IsDynamic(i->importingModule->target)) {
        return ImportDescriptorGetNameDynamic(i);
    }
    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

extern "C" AbckitString *ImportDescriptorGetAlias(AbckitCoreImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    if (IsDynamic(i->importingModule->target)) {
        return ImportDescriptorGetAliasDynamic(i);
    }
    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

// ========================================
// ExportDescriptor
// ========================================

extern "C" AbckitFile *ExportDescriptorGetFile(AbckitCoreExportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    return i->exportingModule->file;
}

extern "C" AbckitCoreModule *ExportDescriptorGetExportingModule(AbckitCoreExportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    return i->exportingModule;
}

extern "C" AbckitCoreModule *ExportDescriptorGetExportedModule(AbckitCoreExportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    return i->exportedModule;
}

extern "C" AbckitString *ExportDescriptorGetName(AbckitCoreExportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    if (IsDynamic(i->exportingModule->target)) {
        return ExportDescriptorGetNameDynamic(i);
    }
    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

extern "C" AbckitString *ExportDescriptorGetAlias(AbckitCoreExportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    if (IsDynamic(i->exportingModule->target)) {
        return ExportDescriptorGetAliasDynamic(i);
    }
    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

// ========================================
// Class
// ========================================

extern "C" AbckitFile *ClassGetFile(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    return klass->owningModule->file;
}

extern "C" AbckitCoreModule *ClassGetModule(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);

    return klass->owningModule;
}

extern "C" AbckitString *ClassGetName(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, nullptr)

    if (IsDynamic(klass->owningModule->target)) {
        return ClassGetNameDynamic(klass);
    }
    return ClassGetNameStatic(klass);
}

extern "C" bool ClassIsExternal(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, false);

    if (IsDynamic(klass->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return ClassIsExternalStatic(klass);
}

extern "C" bool ClassIsFinal(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, false);

    if (IsDynamic(klass->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return ClassIsFinalStatic(klass);
}

extern "C" bool ClassIsAbstract(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, false);

    if (IsDynamic(klass->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return ClassIsAbstractStatic(klass);
}

extern "C" AbckitCoreFunction *ClassGetParentFunction(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    return klass->parentFunction;
}

extern "C" AbckitCoreNamespace *ClassGetParentNamespace(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    return klass->parentNamespace;
}

extern "C" AbckitCoreClass *ClassGetSuperClass(AbckitCoreClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    return klass->superClass;
}

extern "C" bool ClassEnumerateMethods(AbckitCoreClass *klass, void *data,
                                      bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (klass->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSClassEnumerateMethods(klass, data, cb);
        case ABCKIT_TARGET_JS:
            return JsClassEnumerateMethods(klass, data, cb);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassEnumerateAnnotations(AbckitCoreClass *klass, void *data,
                                          bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(klass->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSClassEnumerateAnnotations(klass, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassEnumerateSubClasses(AbckitCoreClass *klass, void *data,
                                         bool (*cb)(AbckitCoreClass *subClass, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(klass->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSClassEnumerateSubClasses(klass, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool ClassEnumerateInterfaces(AbckitCoreClass *klass, void *data,
                                         bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(klass->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSClassEnumerateInterfaces(klass, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool ClassEnumerateFields(AbckitCoreClass *klass, void *data,
                                     bool (*cb)(AbckitCoreClassField *field, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(klass->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSClassEnumerateFields(klass, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

// ========================================
// Interface
// ========================================

extern "C" AbckitFile *InterfaceGetFile(AbckitCoreInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, nullptr)
    return iface->owningModule->file;
}

extern "C" AbckitCoreModule *InterfaceGetModule(AbckitCoreInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, nullptr)
    return iface->owningModule;
}

extern "C" AbckitString *InterfaceGetName(AbckitCoreInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, nullptr)

    if (IsDynamic(iface->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return nullptr;
    }
    return InterfaceGetNameStatic(iface);
}

extern "C" bool InterfaceIsExternal(AbckitCoreInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, false);

    if (IsDynamic(iface->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return InterfaceIsExternalStatic(iface);
}

extern "C" AbckitCoreNamespace *InterfaceGetParentNamespace(AbckitCoreInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, nullptr);
    return iface->parentNamespace;
}

extern "C" bool InterfaceEnumerateSuperInterfaces(AbckitCoreInterface *iface, void *data,
                                                  bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(iface->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSInterfaceEnumerateSuperInterfaces(iface, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool InterfaceEnumerateSubInterfaces(AbckitCoreInterface *iface, void *data,
                                                bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(iface->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSInterfaceEnumerateSubInterfaces(iface, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool InterfaceEnumerateClasses(AbckitCoreInterface *iface, void *data,
                                          bool (*cb)(AbckitCoreClass *klass, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(iface->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSInterfaceEnumerateClasses(iface, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool InterfaceEnumerateMethods(AbckitCoreInterface *iface, void *data,
                                          bool (*cb)(AbckitCoreFunction *func, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(iface->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSInterfaceEnumerateMethods(iface, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool InterfaceEnumerateAnnotations(AbckitCoreInterface *iface, void *data,
                                              bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(iface->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSInterfaceEnumerateAnnotations(iface, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool InterfaceEnumerateFields(AbckitCoreInterface *iface, void *data,
                                         bool (*cb)(AbckitCoreInterfaceField *field, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(iface->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSInterfaceEnumerateFields(iface, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

// ========================================
// Enum
// ========================================

extern "C" AbckitFile *EnumGetFile(AbckitCoreEnum *enm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, nullptr)
    return enm->owningModule->file;
}

extern "C" AbckitCoreModule *EnumGetModule(AbckitCoreEnum *enm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, nullptr)
    return enm->owningModule;
}

extern "C" AbckitString *EnumGetName(AbckitCoreEnum *enm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, nullptr)

    if (IsDynamic(enm->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return nullptr;
    }
    return EnumGetNameStatic(enm);
}

extern "C" bool EnumIsExternal(AbckitCoreEnum *enm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, false);

    if (IsDynamic(enm->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return EnumIsExternalStatic(enm);
}

extern "C" AbckitCoreNamespace *EnumGetParentNamespace(AbckitCoreEnum *enm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, nullptr);
    return enm->parentNamespace;
}

extern "C" bool EnumEnumerateMethods(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreFunction *func, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(enm->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSEnumEnumerateMethods(enm, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" bool EnumEnumerateFields(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreEnumField *field, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(enm->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSEnumEnumerateFields(enm, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

// ========================================
// Module Field
// ========================================

extern "C" AbckitCoreModule *ModuleFieldGetModule(AbckitCoreModuleField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->owner;
}

extern "C" AbckitString *ModuleFieldGetName(AbckitCoreModuleField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    if (IsDynamic(field->owner->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return nullptr;
    }
    return ModuleFieldGetNameStatic(field);
}

extern "C" AbckitType *ModuleFieldGetType(AbckitCoreModuleField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->type;
}

extern "C" AbckitValue *ModuleFieldGetValue(AbckitCoreModuleField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->value;
}

// ========================================
// Namespace Field
// ========================================

extern "C" AbckitCoreNamespace *NamespaceFieldGetNamespace(AbckitCoreNamespaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->owner;
}

extern "C" AbckitString *NamespaceFieldGetName(AbckitCoreNamespaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return nullptr;
    }
    return NamespaceFieldGetNameStatic(field);
}

extern "C" AbckitType *NamespaceFieldGetType(AbckitCoreNamespaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->type;
}

// ========================================
// Class Field
// ========================================

extern "C" AbckitCoreClass *ClassFieldGetClass(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->owner;
}

extern "C" AbckitString *ClassFieldGetName(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return nullptr;
    }
    return ClassFieldGetNameStatic(field);
}

extern "C" AbckitType *ClassFieldGetType(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->type;
}

extern "C" AbckitValue *ClassFieldGetValue(AbckitCoreClassField *field)
{
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return nullptr;
    }
    return field->value;
}

extern "C" bool ClassFieldIsPublic(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return ClassFieldIsPublicStatic(field);
}

extern "C" bool ClassFieldIsProtected(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return ClassFieldIsProtectedStatic(field);
}

extern "C" bool ClassFieldIsPrivate(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return ClassFieldIsPrivateStatic(field);
}

extern "C" bool ClassFieldIsInternal(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return ClassFieldIsInternalStatic(field);
}

extern "C" bool ClassFieldIsStatic(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return ClassFieldIsStaticStatic(field);
}

extern "C" bool ClassFieldIsFinal(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return ClassFieldIsFinalStatic(field);
}

extern "C" bool ClassFieldEnumerateAnnotations(AbckitCoreClassField *field, void *data,
                                               bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(field->owner->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSClassFieldEnumerateAnnotations(field, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

// ========================================
// Interface Field
// ========================================

extern "C" AbckitCoreInterface *InterfaceFieldGetInterface(AbckitCoreInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->owner;
}

extern "C" AbckitString *InterfaceFieldGetName(AbckitCoreInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->name;
}

extern "C" AbckitType *InterfaceFieldGetType(AbckitCoreInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->type;
}

extern "C" bool InterfaceFieldIsReadonly(AbckitCoreInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return InterfaceFieldIsReadonlyStatic(field);
}

extern "C" bool InterfaceFieldEnumerateAnnotations(AbckitCoreInterfaceField *field, void *data,
                                                   bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    switch (ModuleGetTarget(field->owner->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSInterfaceFieldEnumerateAnnotations(field, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

// ========================================
// Enum Field
// ========================================

extern "C" AbckitCoreEnum *EnumFieldGetEnum(AbckitCoreEnumField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->owner;
}

extern "C" AbckitString *EnumFieldGetName(AbckitCoreEnumField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    if (IsDynamic(field->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return nullptr;
    }
    return EnumFieldGetNameStatic(field);
}

extern "C" AbckitType *EnumFieldGetType(AbckitCoreEnumField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->type;
}

extern "C" AbckitValue *EnumFieldGetValue(AbckitCoreEnumField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->value;
}

// ========================================
// AnnotationInterface
// ========================================

extern "C" AbckitFile *AnnotationInterfaceGetFile(AbckitCoreAnnotationInterface *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);
    return anno->owningModule->file;
}

extern "C" AbckitCoreModule *AnnotationInterfaceGetModule(AbckitCoreAnnotationInterface *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);
    return anno->owningModule;
}

extern "C" AbckitCoreNamespace *AnnotationInterfaceGetParentNamespace(AbckitCoreAnnotationInterface *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);

    return anno->parentNamespace;
}

extern "C" AbckitString *AnnotationInterfaceGetName(AbckitCoreAnnotationInterface *ai)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ai, nullptr);
    if (IsDynamic(ai->owningModule->target)) {
        return AnnotationInterfaceGetNameDynamic(ai);
    }
    return AnnotationInterfaceGetNameStatic(ai);
}

extern "C" bool AnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                                   bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ai, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(ai->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSAnnotationInterfaceEnumerateFields(ai, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// AnnotationInterfaceField
// ========================================

extern "C" AbckitFile *AnnotationInterfaceFieldGetFile(AbckitCoreAnnotationInterfaceField *fld)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(fld, nullptr);

    return fld->ai->owningModule->file;
}

extern "C" AbckitCoreAnnotationInterface *AnnotationInterfaceFieldGetInterface(AbckitCoreAnnotationInterfaceField *fld)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(fld, nullptr);

    return fld->ai;
}

extern "C" AbckitString *AnnotationInterfaceFieldGetName(AbckitCoreAnnotationInterfaceField *fld)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(fld, nullptr);

    return fld->name;
}

extern "C" AbckitType *AnnotationInterfaceFieldGetType(AbckitCoreAnnotationInterfaceField *fld)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(fld, nullptr);

    return fld->type;
}

extern "C" AbckitValue *AnnotationInterfaceFieldGetDefaultValue(AbckitCoreAnnotationInterfaceField *fld)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(fld, nullptr);

    return fld->value;
}

// ========================================
// Function
// ========================================

extern "C" AbckitFile *FunctionGetFile(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, nullptr);
    return function->owningModule->file;
}

extern "C" AbckitCoreModule *FunctionGetModule(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, nullptr);

    return function->owningModule;
}

extern "C" AbckitString *FunctionGetName(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, nullptr);

    if (IsDynamic(function->owningModule->target)) {
        return FunctionGetNameDynamic(function);
    }
    return FunctionGetNameStatic(function);
}

extern "C" AbckitCoreFunction *FunctionGetParentFunction(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, nullptr);

    return function->parentFunction;
}

extern "C" AbckitCoreClass *FunctionGetParentClass(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, nullptr);

    if (IsDynamic(function->owningModule->target)) {
        return function->parentClass;
    }
    if (std::holds_alternative<AbckitCoreClass *>(function->parent)) {
        return std::get<AbckitCoreClass *>(function->parent);
    } else {
        return nullptr;
    }
}

extern "C" AbckitCoreNamespace *FunctionGetParentNamespace(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, nullptr);
    if (IsDynamic(function->owningModule->target)) {
        return function->parentNamespace;
    }
    if (std::holds_alternative<AbckitCoreNamespace *>(function->parent)) {
        return std::get<AbckitCoreNamespace *>(function->parent);
    } else {
        return nullptr;
    }
}

extern "C" bool FunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                                 bool (*cb)(AbckitCoreFunction *nestedFunc, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(function->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSFunctionEnumerateNestedFunctions(function, data, cb);
        case ABCKIT_TARGET_JS:
            return JsFunctionEnumerateNestedFunctions(function, data, cb);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool FunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                               bool (*cb)(AbckitCoreClass *nestedClass, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(function->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSFunctionEnumerateNestedClasses(function, data, cb);
        case ABCKIT_TARGET_JS:
            return JsFunctionEnumerateNestedClasses(function, data, cb);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool FunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                             bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(function->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSFunctionEnumerateAnnotations(function, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

AbckitGraph *getGraphFromCache(AbckitCoreFunction *function)
{
    std::unordered_map<AbckitCoreFunction *, FunctionStatus *> &functionsMap =
        function->owningModule->file->functionsMap;

    if (functionsMap.find(function) != functionsMap.end()) {
        bool reuse = true;
        for (auto &[_, status] : functionsMap) {
            if (!status->writeBack) {
                reuse = false;
            }
        }

        if (reuse) {
            return functionsMap[function]->graph;
        }

        for (auto &[func, status] : functionsMap) {
            if (!status->writeBack) {
                continue;
            }
            if (IsDynamic(function->owningModule->target)) {
                FunctionSetGraphDynamicSync(func, status->graph);
            } else {
                FunctionSetGraphStaticSync(func, status->graph);
            }
        }

        for (auto &[_, status] : functionsMap) {
            if (IsDynamic(function->owningModule->target)) {
                DestroyGraphDynamicSync(status->graph);
            } else {
                DestroyGraphStaticSync(status->graph);
            }
            delete status;
        }

        functionsMap.clear();
        function->owningModule->file->generateStatus.generated = false;
    }

    AbckitGraph *graph = nullptr;
    if (IsDynamic(function->owningModule->target)) {
        graph = CreateGraphFromFunctionDynamic(function);
    } else {
        graph = CreateGraphFromFunctionStatic(function);
    }
    functionsMap[function] = new FunctionStatus {graph, false};

    return graph;
}

extern "C" AbckitGraph *CreateGraphFromFunction(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, nullptr);

    if (function->owningModule->file->needOptimize) {
        return getGraphFromCache(function);
    }

    if (IsDynamic(function->owningModule->target)) {
        return CreateGraphFromFunctionDynamic(function);
    }
    return CreateGraphFromFunctionStatic(function);
}

extern "C" bool FunctionIsStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    if (IsDynamic(function->owningModule->target)) {
        return FunctionIsStaticDynamic(function);
    }
    return FunctionIsStaticStatic(function);
}

extern "C" bool FunctionIsCtor(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    if (IsDynamic(function->owningModule->target)) {
        return FunctionIsCtorDynamic(function);
    }
    return FunctionIsCtorStatic(function);
}

extern "C" bool FunctionIsCctor(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    if (IsDynamic(function->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return FunctionIsCctorStatic(function);
}

extern "C" bool FunctionIsAnonymous(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    if (IsDynamic(function->owningModule->target)) {
        return FunctionIsAnonymousDynamic(function);
    }
    return FunctionIsAnonymousStatic(function);
}

extern "C" bool FunctionIsPublic(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);
    if (IsDynamic(function->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return FunctionIsPublicStatic(function);
}

extern "C" bool FunctionIsProtected(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);
    if (IsDynamic(function->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return FunctionIsProtectedStatic(function);
}

extern "C" bool FunctionIsPrivate(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);
    if (IsDynamic(function->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return FunctionIsPrivateStatic(function);
}

extern "C" bool FunctionIsInternal(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);
    if (IsDynamic(function->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    return FunctionIsInternalStatic(function);
}

extern "C" bool FunctionIsExternal(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    if (IsDynamic(function->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return FunctionIsExternalStatic(function);
}

extern "C" bool FunctionEnumerateParameters(AbckitCoreFunction *function, void *data,
                                            bool (*cb)(AbckitCoreFunctionParam *param, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    switch (ModuleGetTarget(function->owningModule)) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSFunctionEnumerateParameters(function, data, cb);
        default:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

extern "C" AbckitType *FunctionGetReturnType(AbckitCoreFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, nullptr)

    if (IsDynamic(function->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return nullptr;
    }
    return FunctionGetReturnTypeStatic(function);
}

// ========================================
// FunctionParam
// ========================================

extern "C" AbckitType *FunctionParamGetType(AbckitCoreFunctionParam *param)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(param, nullptr);
    return param->type;
}

// ========================================
// Annotation
// ========================================

extern "C" AbckitFile *AnnotationGetFile(AbckitCoreAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);

    return anno->ai->owningModule->file;
}

extern "C" AbckitString *AnnotationGetName(AbckitCoreAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);
    if (!anno->ai) {
        return anno->name;
    }
    return AnnotationInterfaceGetName(anno->ai);
}

extern "C" AbckitCoreAnnotationInterface *AnnotationGetInterface(AbckitCoreAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);

    return anno->ai;
}

extern "C" bool AnnotationIsExternal(AbckitCoreAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(anno, false);
    return anno->ai == nullptr;
}

extern "C" bool AnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                            bool (*cb)(AbckitCoreAnnotationElement *ae, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    AbckitCoreModule *m = GetAnnotationOwningModule(anno);

    switch (ModuleGetTarget(m)) {
        case ABCKIT_TARGET_ARK_TS_V1:
        case ABCKIT_TARGET_ARK_TS_V2:
            return ArkTSAnnotationEnumerateElements(anno, data, cb);
        case ABCKIT_TARGET_JS:
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// AnnotationElement
// ========================================

extern "C" AbckitFile *AnnotationElementGetFile(AbckitCoreAnnotationElement *ae)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ae, nullptr);

    return ae->ann->ai->owningModule->file;
}

extern "C" AbckitCoreAnnotation *AnnotationElementGetAnnotation(AbckitCoreAnnotationElement *ae)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ae, nullptr);

    return ae->ann;
}

extern "C" AbckitString *AnnotationElementGetName(AbckitCoreAnnotationElement *ae)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ae, nullptr);

    return ae->name;
}

extern "C" AbckitValue *AnnotationElementGetValue(AbckitCoreAnnotationElement *ae)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ae, nullptr);

    return ae->value;
}

// ========================================
// Type
// ========================================

extern "C" AbckitTypeId TypeGetTypeId(AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(type, AbckitTypeId::ABCKIT_TYPE_ID_INVALID);

    return type->id;
}

extern "C" AbckitCoreClass *TypeGetReferenceClass(AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(type, nullptr);
    if (type->id != AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE) {
        return nullptr;
    }

    return type->GetClass();
}

extern "C" AbckitString *TypeGetName(AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(type, nullptr);

    return type->name;
}

extern "C" size_t TypeGetRank(AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(type, 0);

    return type->rank;
}

extern "C" bool TypeIsUnion(AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(type, false);

    switch (type->file->frontend) {
        case Mode::DYNAMIC:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case Mode::STATIC:
            return TypeIsUnionStatic(type);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool TypeEnumerateUnionTypes(AbckitType *type, void *data, bool (*cb)(AbckitType *type, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(type, false);
    LIBABCKIT_BAD_ARGUMENT(cb, false);

    switch (type->file->frontend) {
        case Mode::STATIC:
            return TypeEnumerateUnionTypesStatic(type, data, cb);
        case Mode::DYNAMIC:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
    }
}

// ========================================
// Value
// ========================================

extern "C" AbckitFile *ValueGetFile(AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(value, nullptr);

    return value->file;
}

extern "C" AbckitType *ValueGetType(AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(value, nullptr);

    switch (value->file->frontend) {
        case Mode::DYNAMIC:
            return ValueGetTypeDynamic(value);
        case Mode::STATIC:
            return ValueGetTypeStatic(value);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ValueGetU1(AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(value, false);

    switch (value->file->frontend) {
        case Mode::DYNAMIC:
            return ValueGetU1Dynamic(value);
        case Mode::STATIC:
            return ValueGetU1Static(value);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" int ValueGetInt(AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(value, 0);

    switch (value->file->frontend) {
        case Mode::STATIC:
            return ValueGetIntStatic(value);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" double ValueGetDouble(AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(value, 0);

    switch (value->file->frontend) {
        case Mode::DYNAMIC:
            return ValueGetDoubleDynamic(value);
        case Mode::STATIC:
            return ValueGetDoubleStatic(value);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitString *ValueGetString(AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(value, nullptr);

    switch (value->file->frontend) {
        case Mode::DYNAMIC:
            return ValueGetStringDynamic(value);
        case Mode::STATIC:
            return ValueGetStringStatic(value);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitLiteralArray *ArrayValueGetLiteralArray(AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(value, nullptr);

    switch (value->file->frontend) {
        case Mode::DYNAMIC:
            return ArrayValueGetLiteralArrayDynamic(value);
        case Mode::STATIC:
            return ArrayValueGetLiteralArrayStatic(value);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// String
// ========================================

extern "C" const char *AbckitStringToString(AbckitString *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(value, nullptr);

    return value->impl.data();
}

// ========================================
// LiteralArray
// ========================================

extern "C" bool LiteralArrayEnumerateElements(AbckitLiteralArray *litArr, void *data,
                                              bool (*cb)(AbckitFile *file, AbckitLiteral *v, void *data))
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(litArr, false);
    LIBABCKIT_BAD_ARGUMENT(cb, false);

    switch (litArr->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralArrayEnumerateElementsDynamic(litArr, data, cb);
        case Mode::STATIC:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Literal
// ========================================

extern "C" AbckitFile *LiteralGetFile(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, nullptr);

    return lit->file;
}

extern "C" AbckitLiteralTag LiteralGetTag(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, ABCKIT_LITERAL_TAG_INVALID);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetTagDynamic(lit);
        case Mode::STATIC:
            return LiteralGetTagStatic(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool LiteralGetBool(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, false);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetBoolDynamic(lit);
        case Mode::STATIC:
            return LiteralGetBoolStatic(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" uint8_t LiteralGetU8(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, 0);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetU8Dynamic(lit);
        case Mode::STATIC:
            return LiteralGetU8Static(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" uint16_t LiteralGetU16(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, 0);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetU16Dynamic(lit);
        case Mode::STATIC:
            return LiteralGetU16Static(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" uint16_t LiteralGetMethodAffiliate(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, 0);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetMethodAffiliateDynamic(lit);
        case Mode::STATIC:
            return LiteralGetMethodAffiliateStatic(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" uint32_t LiteralGetU32(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, 0);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetU32Dynamic(lit);
        case Mode::STATIC:
            return LiteralGetU32Static(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" uint64_t LiteralGetU64(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, 0);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetU64Dynamic(lit);
        case Mode::STATIC:
            return LiteralGetU64Static(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" float LiteralGetFloat(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, 0);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetFloatDynamic(lit);
        case Mode::STATIC:
            return LiteralGetFloatStatic(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" double LiteralGetDouble(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, 0);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetDoubleDynamic(lit);
        case Mode::STATIC:
            return LiteralGetDoubleStatic(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitLiteralArray *LiteralGetLiteralArray(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, nullptr);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetLiteralArrayDynamic(lit);
        case Mode::STATIC:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitString *LiteralGetString(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, nullptr);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetStringDynamic(lit);
        case Mode::STATIC:
            return LiteralGetStringStatic(lit);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitString *LiteralGetMethod(AbckitLiteral *lit)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(lit, nullptr);

    switch (lit->file->frontend) {
        case Mode::DYNAMIC:
            return LiteralGetStringDynamic(lit);
        case Mode::STATIC:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

AbckitInspectApi g_inspectApiImpl = {
    // ========================================
    // File
    // ========================================

    FileGetVersion,
    FileEnumerateModules,
    FileEnumerateExternalModules,

    // ========================================
    // String
    // ========================================

    AbckitStringToString,

    // ========================================
    // Type
    // ========================================

    TypeGetTypeId,
    TypeGetReferenceClass,
    TypeGetName,
    TypeGetRank,
    TypeIsUnion,
    TypeEnumerateUnionTypes,

    // ========================================
    // Value
    // ========================================

    ValueGetFile,
    ValueGetType,
    ValueGetU1,
    ValueGetInt,
    ValueGetDouble,
    ValueGetString,
    ArrayValueGetLiteralArray,

    // ========================================
    // Literal
    // ========================================

    LiteralGetFile,
    LiteralGetTag,
    LiteralGetBool,
    LiteralGetU8,
    LiteralGetU16,
    LiteralGetMethodAffiliate,
    LiteralGetU32,
    LiteralGetU64,
    LiteralGetFloat,
    LiteralGetDouble,
    LiteralGetLiteralArray,
    LiteralGetString,
    LiteralGetMethod,

    // ========================================
    // LiteralArray
    // ========================================

    LiteralArrayEnumerateElements,

    // ========================================
    // Module
    // ========================================

    ModuleGetFile,
    ModuleGetTarget,
    ModuleGetName,
    ModuleIsExternal,
    ModuleEnumerateImports,
    ModuleEnumerateExports,
    ModuleEnumerateNamespaces,
    ModuleEnumerateClasses,
    ModuleEnumerateInterfaces,
    ModuleEnumerateEnums,
    ModuleEnumerateFields,
    ModuleEnumerateTopLevelFunctions,
    ModuleEnumerateAnonymousFunctions,
    ModuleEnumerateAnnotationInterfaces,

    // ========================================
    // Namespace
    // ========================================

    NamespaceGetModule,
    NamespaceGetName,
    NamespaceIsExternal,
    NamespaceGetParentNamespace,
    NamespaceEnumerateNamespaces,
    NamespaceEnumerateClasses,
    NamespaceEnumerateInterfaces,
    NamespaceEnumerateEnums,
    NamespaceEnumerateFields,
    NamespaceEnumerateTopLevelFunctions,
    NamespaceEnumerateAnnotationInterfaces,

    // ========================================
    // ImportDescriptor
    // ========================================

    ImportDescriptorGetFile,
    ImportDescriptorGetImportingModule,
    ImportDescriptorGetImportedModule,
    ImportDescriptorGetName,
    ImportDescriptorGetAlias,

    // ========================================
    // ExportDescriptor
    // ========================================

    ExportDescriptorGetFile,
    ExportDescriptorGetExportingModule,
    ExportDescriptorGetExportedModule,
    ExportDescriptorGetName,
    ExportDescriptorGetAlias,

    // ========================================
    // Class
    // ========================================

    ClassGetFile,
    ClassGetModule,
    ClassGetName,
    ClassIsExternal,
    ClassIsFinal,
    ClassIsAbstract,
    ClassGetParentFunction,
    ClassGetParentNamespace,
    ClassGetSuperClass,
    ClassEnumerateMethods,
    ClassEnumerateAnnotations,
    ClassEnumerateSubClasses,
    ClassEnumerateInterfaces,
    ClassEnumerateFields,

    // ========================================
    // Interface
    // ========================================

    InterfaceGetFile,
    InterfaceGetModule,
    InterfaceGetName,
    InterfaceIsExternal,
    InterfaceGetParentNamespace,
    InterfaceEnumerateSuperInterfaces,
    InterfaceEnumerateSubInterfaces,
    InterfaceEnumerateClasses,
    InterfaceEnumerateMethods,
    InterfaceEnumerateAnnotations,
    InterfaceEnumerateFields,

    // ========================================
    // Enum
    // ========================================

    EnumGetFile,
    EnumGetModule,
    EnumGetName,
    EnumIsExternal,
    EnumGetParentNamespace,
    EnumEnumerateMethods,
    EnumEnumerateFields,

    // ========================================
    // Module Field
    // ========================================

    ModuleFieldGetModule,
    ModuleFieldGetName,
    ModuleFieldGetType,
    ModuleFieldGetValue,

    // ========================================
    // Namespace Field
    // ========================================

    NamespaceFieldGetNamespace,
    NamespaceFieldGetName,
    NamespaceFieldGetType,

    // ========================================
    // Class Field
    // ========================================

    ClassFieldGetClass,
    ClassFieldGetName,
    ClassFieldGetType,
    ClassFieldGetValue,
    ClassFieldIsPublic,
    ClassFieldIsProtected,
    ClassFieldIsPrivate,
    ClassFieldIsInternal,
    ClassFieldIsStatic,
    ClassFieldIsFinal,
    ClassFieldEnumerateAnnotations,

    // ========================================
    // Interface Field
    // ========================================

    InterfaceFieldGetInterface,
    InterfaceFieldGetName,
    InterfaceFieldGetType,
    InterfaceFieldIsReadonly,
    InterfaceFieldEnumerateAnnotations,

    // ========================================
    // Enum Field
    // ========================================

    EnumFieldGetEnum,
    EnumFieldGetName,
    EnumFieldGetType,
    EnumFieldGetValue,

    // ========================================
    // Function
    // ========================================

    FunctionGetFile,
    FunctionGetModule,
    FunctionGetName,
    FunctionGetParentFunction,
    FunctionGetParentClass,
    FunctionGetParentNamespace,
    FunctionEnumerateNestedFunctions,
    FunctionEnumerateNestedClasses,
    FunctionEnumerateAnnotations,
    CreateGraphFromFunction,
    FunctionIsStatic,
    FunctionIsCtor,
    FunctionIsCctor,
    FunctionIsAnonymous,
    FunctionIsPublic,
    FunctionIsProtected,
    FunctionIsPrivate,
    FunctionIsInternal,
    FunctionIsExternal,
    FunctionEnumerateParameters,
    FunctionGetReturnType,

    // ========================================
    // FunctionParam
    // ========================================

    FunctionParamGetType,

    // ========================================
    // Annotation
    // ========================================

    AnnotationGetFile,
    AnnotationGetName,
    AnnotationGetInterface,
    AnnotationIsExternal,
    AnnotationEnumerateElements,
    AnnotationElementGetFile,
    AnnotationElementGetAnnotation,
    AnnotationElementGetName,
    AnnotationElementGetValue,

    // ========================================
    // AnnotationInterface
    // ========================================

    AnnotationInterfaceGetFile,
    AnnotationInterfaceGetModule,
    AnnotationInterfaceGetParentNamespace,
    AnnotationInterfaceGetName,
    AnnotationInterfaceEnumerateFields,

    // ========================================
    // AnnotationInterfaceField
    // ========================================

    AnnotationInterfaceFieldGetFile,
    AnnotationInterfaceFieldGetInterface,
    AnnotationInterfaceFieldGetName,
    AnnotationInterfaceFieldGetType,
    AnnotationInterfaceFieldGetDefaultValue,
};

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitInspectApi const *AbckitGetInspectApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockInspectApiImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_inspectApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
