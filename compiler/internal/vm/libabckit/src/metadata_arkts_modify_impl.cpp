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
#include <cstdint>
#include <cstring>
#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"

#include "libabckit/src/macros.h"
#include "scoped_timer.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "libabckit/src/adapter_static/abckit_static.h"

#include "helpers_common.h"

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_SAME_TARGET(target1, target2)                  \
    if ((target1) != (target2)) {                                      \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET); \
        /* CC-OFFNXT(G.PRE.05) code generation */                      \
        return nullptr;                                                \
    }

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_SAME_TARGET_VOID(target1, target2)             \
    if ((target1) != (target2)) {                                      \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET); \
        /* CC-OFFNXT(G.PRE.05) code generation */                      \
        return;                                                        \
    }

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_SAME_TARGET_RETURN(target1, target2, value)    \
    if ((target1) != (target2)) {                                      \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET); \
        /* CC-OFFNXT(G.PRE.05) code generation */                      \
        return value;                                                  \
    }

namespace libabckit {

// ========================================
// File
// ========================================

extern "C" AbckitArktsModule *FileAddExternalModuleArktsV1(AbckitFile *file,
                                                           const struct AbckitArktsV1ExternalModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(file, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);

    switch (file->frontend) {
        case Mode::DYNAMIC:
            return FileAddExternalArkTsV1Module(file, params);
        case Mode::STATIC:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitArktsModule *FileAddExternalModuleArktsV2(AbckitFile *file, const char *moduleName)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(file, nullptr);
    LIBABCKIT_BAD_ARGUMENT(moduleName, nullptr);

    switch (file->frontend) {
        case Mode::STATIC:
            return FileAddExternalModuleArktsV2Static(file, moduleName);
        case Mode::DYNAMIC:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Module
// ========================================
extern "C" bool ModuleSetName(AbckitArktsModule *m, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(m, false);
    LIBABCKIT_BAD_ARGUMENT(name, false);

    if (IsDynamic(m->core->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return ModuleSetNameStatic(m->core, name);
}

extern "C" AbckitArktsImportDescriptor *ModuleAddImportFromArktsV1ToArktsV1(
    AbckitArktsModule *importing, AbckitArktsModule *imported,
    const AbckitArktsImportFromDynamicModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(importing, nullptr);
    LIBABCKIT_BAD_ARGUMENT(imported, nullptr);

    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->alias, nullptr);

    LIBABCKIT_INTERNAL_ERROR(importing->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(imported->core, nullptr);

    if (importing->core->target != ABCKIT_TARGET_ARK_TS_V1) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }

    LIBABCKIT_CHECK_SAME_TARGET(importing->core->target, imported->core->target);

    return ModuleAddImportFromDynamicModuleDynamic(importing->core, imported->core, params);
}

extern "C" AbckitArktsClass *ModuleImportClassFromArktsV2ToArktsV2(AbckitArktsModule *externalModule,
                                                                   const char *className)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(externalModule, nullptr);
    LIBABCKIT_BAD_ARGUMENT(className, nullptr);

    if (externalModule->core->target != ABCKIT_TARGET_ARK_TS_V2) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }
    return ModuleImportClassFromArktsV2ToArktsV2Static(externalModule, className);
}

extern "C" AbckitArktsFunction *ModuleImportStaticFunctionFromArktsV2ToArktsV2(AbckitArktsModule *externalModule,
                                                                               const char *functionName,
                                                                               const char *returnType,
                                                                               const char *const *params,
                                                                               size_t paramCount)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(externalModule, nullptr);
    LIBABCKIT_BAD_ARGUMENT(functionName, nullptr);
    LIBABCKIT_BAD_ARGUMENT(returnType, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    if (externalModule->core->target != ABCKIT_TARGET_ARK_TS_V2) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }

    // Convert C-style array to std::vector for internal use
    std::vector<const char *> paramsVector(params, params + paramCount);
    return ModuleImportStaticFunctionStatic(externalModule, functionName, returnType, paramsVector);
}

extern "C" AbckitArktsFunction *ModuleImportClassMethodFromArktsV2ToArktsV2(
    AbckitArktsModule *externalModule, const char *className, const char *methodName, const char *returnType,
    const char *const *params, size_t paramCount)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(externalModule, nullptr);
    LIBABCKIT_BAD_ARGUMENT(className, nullptr);
    LIBABCKIT_BAD_ARGUMENT(methodName, nullptr);
    LIBABCKIT_BAD_ARGUMENT(returnType, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    if (externalModule->core->target != ABCKIT_TARGET_ARK_TS_V2) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }

    // Convert C-style array to std::vector for internal use
    std::vector<const char *> paramsVector(params, params + paramCount);
    return ModuleImportClassMethodStatic(externalModule, className, methodName, returnType, paramsVector);
}

extern "C" void ModuleRemoveImport(AbckitArktsModule *m, AbckitArktsImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(i)

    LIBABCKIT_INTERNAL_ERROR_VOID(m->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(i->core);

    auto mt = m->core->target;
    auto it = i->core->importingModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_VOID(mt, it);

    switch (mt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleRemoveImportDynamic(m->core, i);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitArktsExportDescriptor *ModuleAddExportFromArktsV1ToArktsV1(
    AbckitArktsModule *exporting, AbckitArktsModule *exported, const AbckitArktsDynamicModuleExportCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(exporting, nullptr);
    LIBABCKIT_BAD_ARGUMENT(exported, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);

    LIBABCKIT_INTERNAL_ERROR(exporting->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(exported->core, nullptr);

    if (exporting->core->target != ABCKIT_TARGET_ARK_TS_V1) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }

    LIBABCKIT_CHECK_SAME_TARGET(exporting->core->target, exported->core->target);

    return DynamicModuleAddExportDynamic(exporting->core, exported->core, params);
}

extern "C" AbckitArktsExportDescriptor *ModuleAddExportFromArktsV2ToArktsV2(
    AbckitArktsModule * /*exporting*/ /* assert(exporting.target === AbckitTarget_ArkTS_v2) */,
    AbckitArktsModule * /*exported*/ /* assert(exporting.target === AbckitTarget_ArkTS_v2) */,
    const AbckitArktsDynamicModuleExportCreateParams * /*params*/)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

extern "C" void ModuleRemoveExport(AbckitArktsModule *m, AbckitArktsExportDescriptor *e)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(e)

    LIBABCKIT_INTERNAL_ERROR_VOID(m->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(e->core);

    auto mt = m->core->target;
    auto et = e->core->exportingModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_VOID(mt, et);

    switch (mt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleRemoveExportDynamic(m->core, e);
        case ABCKIT_TARGET_ARK_TS_V2:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitArktsAnnotationInterface *ModuleAddAnnotationInterface(
    AbckitArktsModule *m, const AbckitArktsAnnotationInterfaceCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);

    LIBABCKIT_INTERNAL_ERROR(m->core, nullptr);

    switch (m->core->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleAddAnnotationInterfaceDynamic(m->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            return ModuleAddAnnotationInterfaceStatic(m->core, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitArktsModuleField *ModuleAddField(AbckitArktsModule *m,
                                                  const struct AbckitArktsFieldCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_INTERNAL_ERROR(m->core, nullptr);

    auto mt = m->core->target;
    switch (mt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ModuleAddFieldStatic(m->core, params);
        default:
            statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
            return nullptr;
    }
}

// ========================================
// Namespace
// ========================================

extern "C" bool NamespaceSetName(AbckitArktsNamespace *ns, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ns, false);
    LIBABCKIT_BAD_ARGUMENT(name, false);

    if (IsDynamic(ns->core->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return NamespaceSetNameStatic(ns->core, name);
}

// ========================================
// Class
// ========================================

extern "C" AbckitArktsAnnotation *ClassAddAnnotation(AbckitArktsClass *klass,
                                                     const AbckitArktsAnnotationCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->ai, nullptr);

    LIBABCKIT_INTERNAL_ERROR(klass->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(params->ai->core, nullptr);

    auto kt = klass->core->owningModule->target;
    auto at = params->ai->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET(kt, at);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ClassAddAnnotationDynamic(klass->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassAddAnnotationStatic(klass->core, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void ClassRemoveAnnotation(AbckitArktsClass *klass, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(klass);
    LIBABCKIT_BAD_ARGUMENT_VOID(anno);

    LIBABCKIT_INTERNAL_ERROR_VOID(klass->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(anno->core);

    auto kt = klass->core->owningModule->target;
    auto at = GetAnnotationOwningModule(anno->core)->target;

    LIBABCKIT_CHECK_SAME_TARGET_VOID(kt, at);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ClassRemoveAnnotationDynamic(klass->core, anno->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassRemoveAnnotationStatic(klass->core, anno->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassAddInterface(AbckitArktsClass *klass, AbckitArktsInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(klass, false);
    LIBABCKIT_BAD_ARGUMENT(iface, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule->file, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule->file, false);

    auto kt = klass->core->owningModule->target;
    auto it = iface->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, it, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassAddInterfaceStatic(klass, iface);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassRemoveInterface(AbckitArktsClass *klass, AbckitArktsInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(klass, false);
    LIBABCKIT_BAD_ARGUMENT(iface, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule->file, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule->file, false);

    auto kt = klass->core->owningModule->target;
    auto it = iface->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, it, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassRemoveInterfaceStatic(klass, iface);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassSetSuperClass(AbckitArktsClass *klass, AbckitArktsClass *superClass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(klass, false);
    LIBABCKIT_BAD_ARGUMENT(superClass, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core, false);
    LIBABCKIT_INTERNAL_ERROR(superClass->core, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(superClass->core->owningModule, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule->file, false);
    LIBABCKIT_INTERNAL_ERROR(superClass->core->owningModule->file, false);

    auto kt = klass->core->owningModule->target;
    auto st = superClass->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, st, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassSetSuperClassStatic(klass, superClass);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassSetName(AbckitArktsClass *klass, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(name, false)

    if (IsDynamic(klass->core->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return ClassSetNameStatic(klass->core, name);
}

extern "C" AbckitArktsClassField *ClassAddField(AbckitArktsClass *klass,
                                                const struct AbckitArktsFieldCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_INTERNAL_ERROR(klass->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule, nullptr);

    auto kt = klass->core->owningModule->target;
    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassAddFieldStatic(klass->core, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassRemoveField(AbckitArktsClass *klass, AbckitArktsClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(klass, false);
    LIBABCKIT_BAD_ARGUMENT(field, false);

    LIBABCKIT_INTERNAL_ERROR(klass->core, false);
    LIBABCKIT_INTERNAL_ERROR(field->core, false);

    auto kt = klass->core->owningModule->target;
    auto at = field->core->owner->owningModule->target;
    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, at, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassRemoveFieldStatic(klass, field->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassRemoveMethod(AbckitArktsClass *klass, AbckitArktsFunction *method)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(klass, false);
    LIBABCKIT_BAD_ARGUMENT(method, false);
    LIBABCKIT_INTERNAL_ERROR(klass->core, false);
    LIBABCKIT_INTERNAL_ERROR(method->core, false);
    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(method->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule->file, false);
    LIBABCKIT_INTERNAL_ERROR(method->core->owningModule->file, false);

    auto kt = klass->core->owningModule->target;
    auto mt = method->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, mt, false);
    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            break;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassRemoveMethodStatic(klass->core, method->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
    return false;
}

extern "C" AbckitArktsClass *CreateClass(AbckitArktsModule *m, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_BAD_ARGUMENT(name, nullptr);
    LIBABCKIT_INTERNAL_ERROR(m->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(m->core->file, nullptr);

    switch (m->core->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            break;
        case ABCKIT_TARGET_ARK_TS_V2:
            return CreateClassStatic(m->core, name);
        default:
            LIBABCKIT_UNREACHABLE;
    }
    return nullptr;
}

extern "C" bool ClassSetOwningModule(AbckitArktsClass *klass, AbckitArktsModule *module)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, false);
    LIBABCKIT_BAD_ARGUMENT(module, false);
    LIBABCKIT_INTERNAL_ERROR(klass->core, false);
    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(module->core, false);

    auto kt = klass->core->owningModule->target;
    auto at = module->core->target;
    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, at, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassSetOwningModuleStatic(klass->core, module->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Interface
// ========================================

extern "C" bool InterfaceAddSuperInterface(AbckitArktsInterface *iface, AbckitArktsInterface *superIface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(superIface, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(superIface->core, false);
    LIBABCKIT_INTERNAL_ERROR(superIface->core->owningModule, false);

    auto kt = iface->core->owningModule->target;
    auto at = superIface->core->owningModule->target;
    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, at, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceAddSuperInterfaceStatic(iface->core, superIface->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool InterfaceRemoveSuperInterface(AbckitArktsInterface *iface, AbckitArktsInterface *superIface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(superIface, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(superIface->core, false);
    LIBABCKIT_INTERNAL_ERROR(superIface->core->owningModule, false);

    auto kt = iface->core->owningModule->target;
    auto at = superIface->core->owningModule->target;
    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, at, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceRemoveSuperInterfaceStatic(iface->core, superIface->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool InterfaceSetName(AbckitArktsInterface *iface, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(iface, false)
    LIBABCKIT_BAD_ARGUMENT(name, false)

    if (IsDynamic(iface->core->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return InterfaceSetNameStatic(iface->core, name);
}

extern "C" AbckitArktsInterfaceField *InterfaceAddField(AbckitArktsInterface *iface,
                                                        const struct AbckitArktsInterfaceFieldCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(iface, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    LIBABCKIT_INTERNAL_ERROR(iface->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, nullptr);

    auto kt = iface->core->owningModule->target;
    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceAddFieldStatic(iface, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool InterfaceRemoveField(AbckitArktsInterface *iface, AbckitArktsInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(field, false);

    LIBABCKIT_INTERNAL_ERROR(iface->core, false);
    LIBABCKIT_INTERNAL_ERROR(field->core, false);

    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner->owningModule, false);

    auto kt = iface->core->owningModule->target;
    auto at = field->core->owner->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, at, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceRemoveFieldStatic(iface, field->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool InterfaceAddMethod(AbckitArktsInterface *iface, AbckitArktsFunction *method)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(method, false);

    LIBABCKIT_INTERNAL_ERROR(iface->core, false);
    LIBABCKIT_INTERNAL_ERROR(method->core, false);

    auto kt = iface->core->owningModule->target;
    auto at = method->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, at, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceAddMethodStatic(iface, method);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool InterfaceRemoveMethod(AbckitArktsInterface *iface, AbckitArktsFunction *method)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(method, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(method->core, false);
    LIBABCKIT_INTERNAL_ERROR(method->core->owningModule, false);

    auto kt = iface->core->owningModule->target;
    auto at = method->core->owningModule->target;
    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, at, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceRemoveMethodStatic(iface->core, method->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool InterfaceSetOwningModule(AbckitArktsInterface *iface, AbckitArktsModule *module)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(module, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(module->core, false);

    auto kt = iface->core->owningModule->target;
    auto at = module->core->target;
    LIBABCKIT_CHECK_SAME_TARGET_RETURN(kt, at, false);

    switch (kt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceSetOwningModuleStatic(iface->core, module->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitArktsInterface *CreateInterface(AbckitArktsModule *m, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_BAD_ARGUMENT(name, nullptr);
    LIBABCKIT_INTERNAL_ERROR(m->core, nullptr);

    switch (m->core->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        case ABCKIT_TARGET_ARK_TS_V2:
            return CreateInterfaceStatic(m, name);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Enum
// ========================================

extern "C" bool EnumSetName(AbckitArktsEnum *enm, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(enm, false);
    LIBABCKIT_BAD_ARGUMENT(name, false);

    if (IsDynamic(enm->core->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return EnumSetNameStatic(enm->core, name);
}

extern "C" AbckitArktsEnumField *EnumAddField(AbckitArktsEnum *enm, const struct AbckitArktsFieldCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(enm, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_INTERNAL_ERROR(enm->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(enm->core->owningModule, nullptr);

    auto mt = enm->core->owningModule->target;
    switch (mt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        case ABCKIT_TARGET_ARK_TS_V2:
            return EnumAddFieldStatic(enm->core, params);
        default:
            statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
            return nullptr;
    }
}

// ========================================
// Module Field
// ========================================

extern "C" bool ModuleFieldSetName(AbckitArktsModuleField *field, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(field, false)
    LIBABCKIT_BAD_ARGUMENT(name, false)

    if (IsDynamic(field->core->owner->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return ModuleFieldSetNameStatic(field->core, name);
}

extern "C" bool ModuleFieldSetType(AbckitArktsModuleField *field, AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(type, false);

    LIBABCKIT_INTERNAL_ERROR(field->core, false);

    auto fieldt = field->core->owner->target;
    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ModuleFieldSetTypeStatic(field, type);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ModuleFieldSetValue(AbckitArktsModuleField *field, AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(value, false);

    LIBABCKIT_INTERNAL_ERROR(field->core, false);

    auto fieldt = field->core->owner->target;
    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ModuleFieldSetValueStatic(field, value);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// NamespaceField
// ========================================

extern "C" bool NamespaceFieldSetName(AbckitArktsNamespaceField *field, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(name, false);

    if (IsDynamic(field->core->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return NamespaceFieldSetNameStatic(field->core, name);
}

// ========================================
// Class Field
// ========================================

extern "C" bool ClassFieldAddAnnotation(AbckitArktsClassField *field,
                                        const struct AbckitArktsAnnotationCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(params, false);
    LIBABCKIT_INTERNAL_ERROR(field->core, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner->owningModule, false);

    LIBABCKIT_INTERNAL_ERROR(params->ai, false);
    LIBABCKIT_INTERNAL_ERROR(params->ai->core, false);

    auto fieldt = field->core->owner->owningModule->target;
    auto annot = params->ai->core->owningModule->target;
    if (fieldt != annot) {
        LIBABCKIT_LOG(ERROR) << "field target not equal anno target\n";
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return false;
    }
    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassFieldAddAnnotationStatic(field, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassFieldRemoveAnnotation(AbckitArktsClassField *field, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(anno, false);
    LIBABCKIT_INTERNAL_ERROR(field->core, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(anno->core, false);
    LIBABCKIT_INTERNAL_ERROR(anno->core->ai, false);
    LIBABCKIT_INTERNAL_ERROR(anno->core->ai->owningModule, false);

    auto fieldt = field->core->owner->owningModule->target;
    auto annot = anno->core->ai->owningModule->target;
    if (fieldt != annot) {
        LIBABCKIT_LOG(ERROR) << "field target not equal anno target\n";
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return false;
    }

    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassFieldRemoveAnnotationStatic(field, anno);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassFieldSetName(AbckitArktsClassField *field, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(name, false);

    if (IsDynamic(field->core->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return ClassFieldSetNameStatic(field->core, name);
}

extern "C" bool ClassFieldSetType(AbckitArktsClassField *field, AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(type, false);

    LIBABCKIT_INTERNAL_ERROR(field->core, false);

    auto fieldt = field->core->owner->owningModule->target;
    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassFieldSetTypeStatic(field, type);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool ClassFieldSetValue(AbckitArktsClassField *field, AbckitValue *value)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(value, false);

    LIBABCKIT_INTERNAL_ERROR(field->core, false);

    auto fieldt = field->core->owner->owningModule->target;
    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassFieldSetValueStatic(field, value);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Interface Field
// ========================================

extern "C" bool InterfaceFieldAddAnnotation(AbckitArktsInterfaceField *field,
                                            const struct AbckitArktsAnnotationCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(params, false);
    LIBABCKIT_INTERNAL_ERROR(field->core, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(params->ai, false);
    LIBABCKIT_INTERNAL_ERROR(params->ai->core, false);

    auto fieldt = field->core->owner->owningModule->target;
    auto annot = params->ai->core->owningModule->target;
    if (fieldt != annot) {
        LIBABCKIT_LOG(ERROR) << "field target not equal anno target\n";
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return false;
    }

    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceFieldAddAnnotationStatic(field, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool InterfaceFieldRemoveAnnotation(AbckitArktsInterfaceField *field, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(anno, false);
    LIBABCKIT_INTERNAL_ERROR(field->core, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(anno->core, false);
    LIBABCKIT_INTERNAL_ERROR(anno->core->ai, false);
    LIBABCKIT_INTERNAL_ERROR(anno->core->ai->owningModule, false);

    auto fieldt = field->core->owner->owningModule->target;
    auto annot = anno->core->ai->owningModule->target;
    if (fieldt != annot) {
        LIBABCKIT_LOG(ERROR) << "field target not equal anno target\n";
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return false;
    }

    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceFieldRemoveAnnotationStatic(field, anno);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool InterfaceFieldSetName(AbckitArktsInterfaceField *field, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(field, false)
    LIBABCKIT_BAD_ARGUMENT(name, false)

    if (IsDynamic(field->core->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    } else {
        InterfaceFieldSetNameStatic(field->core, name);
    }

    return true;
}

extern "C" bool InterfaceFieldSetType(AbckitArktsInterfaceField *field, AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;

    LIBABCKIT_BAD_ARGUMENT(field, false);
    LIBABCKIT_BAD_ARGUMENT(type, false);

    LIBABCKIT_INTERNAL_ERROR(field->core, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner, false);
    LIBABCKIT_INTERNAL_ERROR(field->core->owner->owningModule, false);

    auto fieldt = field->core->owner->owningModule->target;
    switch (fieldt) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return false;
        case ABCKIT_TARGET_ARK_TS_V2:
            return InterfaceFieldSetTypeStatic(field, type);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Enum Field
// ========================================

extern "C" bool EnumFieldSetName(AbckitArktsEnumField *field, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(field, false)
    LIBABCKIT_BAD_ARGUMENT(name, false)

    if (IsDynamic(field->core->owner->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return EnumFieldSetNameStatic(field->core, name);
}

// ========================================
// AnnotationInterface
// ========================================

extern "C" bool AnnotationInterfaceSetName(AbckitArktsAnnotationInterface *ai, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ai, false);
    LIBABCKIT_BAD_ARGUMENT(name, false);

    if (IsDynamic(ai->core->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return AnnotationInterfaceSetNameStatic(ai->core, name);
}

extern "C" AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddField(
    AbckitArktsAnnotationInterface *ai, const AbckitArktsAnnotationInterfaceFieldCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(ai, nullptr);

    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->type, nullptr);

    LIBABCKIT_INTERNAL_ERROR(ai->core, nullptr);

    switch (ai->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return AnnotationInterfaceAddFieldDynamic(ai->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            return AnnotationInterfaceAddFieldStatic(ai->core, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void AnnotationInterfaceRemoveField(AbckitArktsAnnotationInterface *ai,
                                               AbckitArktsAnnotationInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(ai);
    LIBABCKIT_BAD_ARGUMENT_VOID(field);

    LIBABCKIT_INTERNAL_ERROR_VOID(ai->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(field->core);

    switch (ai->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return AnnotationInterfaceRemoveFieldDynamic(ai->core, field->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            return AnnotationInterfaceRemoveFieldStatic(ai->core, field->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Function
// ========================================

extern "C" AbckitArktsAnnotation *FunctionAddAnnotation(AbckitArktsFunction *function,
                                                        const AbckitArktsAnnotationCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    LIBABCKIT_INTERNAL_ERROR(function->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(params->ai->core, nullptr);

    auto ft = function->core->owningModule->target;
    auto at = params->ai->core->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET(ft, at);

    switch (ft) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionAddAnnotationDynamic(function->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionAddAnnotationStatic(function->core, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void FunctionRemoveAnnotation(AbckitArktsFunction *function, AbckitArktsAnnotation *anno)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(function);
    LIBABCKIT_BAD_ARGUMENT_VOID(anno);

    LIBABCKIT_INTERNAL_ERROR_VOID(function->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(anno->core);

    auto ft = function->core->owningModule->target;
    auto at = anno->core->ai->owningModule->target;

    LIBABCKIT_CHECK_SAME_TARGET_VOID(ft, at);

    switch (ft) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionRemoveAnnotationDynamic(function->core, anno->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionRemoveAnnotationStatic(function->core, anno->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool FunctionSetName(AbckitArktsFunction *function, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(function, false);
    LIBABCKIT_BAD_ARGUMENT(name, false);

    LIBABCKIT_INTERNAL_ERROR(function->core, false);
    if (IsDynamic(function->core->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return FunctionSetNameStatic(function->core, name);
}

extern "C" AbckitArktsFunctionParam *CreateFunctionParameter(AbckitFile *file,
                                                             const AbckitArktsFunctionParamCreatedParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(file, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->type, nullptr);

    // Delegate to static implementation
    return CreateFunctionParameterStatic(file, params);
}

extern "C" bool FunctionAddParameter(AbckitArktsFunction *func, AbckitArktsFunctionParam *param)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(func, false);
    LIBABCKIT_BAD_ARGUMENT(param, false);

    LIBABCKIT_INTERNAL_ERROR(func->core, false);
    LIBABCKIT_INTERNAL_ERROR(param->core, false);
    return FunctionAddParameterStatic(func, param);
}

extern "C" bool FunctionRemoveParameter(AbckitArktsFunction *func, size_t index)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(func, false);

    LIBABCKIT_INTERNAL_ERROR(func->core, false);
    return FunctionRemoveParameterStatic(func, index);
}

extern "C" bool FunctionSetReturnType(AbckitArktsFunction *func, AbckitType *type)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(func, false);
    LIBABCKIT_BAD_ARGUMENT(type, false);

    LIBABCKIT_INTERNAL_ERROR(func->core, false);

    return FunctionSetReturnTypeStatic(func, type);
}

extern "C" AbckitArktsFunction *ModuleAddFunction(AbckitArktsModule *module,
                                                  struct AbckitArktsFunctionCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(module, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    switch (module->core->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ModuleAddFunctionStatic(module, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" AbckitArktsFunction *ClassAddMethod(AbckitArktsClass *klass, struct ArktsMethodCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    LIBABCKIT_INTERNAL_ERROR(klass->core, nullptr);
    switch (klass->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
            return nullptr;
        case ABCKIT_TARGET_ARK_TS_V2:
            return ClassAddMethodStatic(klass, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Annotation
// ========================================

extern "C" bool AnnotationSetName(AbckitArktsAnnotation *anno, const char *name)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, false)
    LIBABCKIT_BAD_ARGUMENT(name, false)

    if (IsDynamic(anno->core->ai->owningModule->target)) {
        statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }

    return AnnotationSetNameStatic(anno->core, name);
}

extern "C" AbckitArktsAnnotationElement *AnnotationAddAnnotationElement(
    AbckitArktsAnnotation *anno, AbckitArktsAnnotationElementCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(anno, nullptr);

    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->value, nullptr);

    LIBABCKIT_INTERNAL_ERROR(anno->core, nullptr);
    AbckitCoreModule *module = GetAnnotationOwningModule(anno->core);

    LIBABCKIT_INTERNAL_ERROR(module, nullptr);
    LIBABCKIT_INTERNAL_ERROR(module->file, nullptr);

    switch (module->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return AnnotationAddAnnotationElementDynamic(anno->core, params);
        case ABCKIT_TARGET_ARK_TS_V2:
            return AnnotationAddAnnotationElementStatic(anno->core, params);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" void AnnotationRemoveAnnotationElement(AbckitArktsAnnotation *anno, AbckitArktsAnnotationElement *elem)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(anno);
    LIBABCKIT_BAD_ARGUMENT_VOID(elem);

    AbckitCoreModule *module = GetAnnotationOwningModule(anno->core);
    LIBABCKIT_INTERNAL_ERROR_VOID(module);
    LIBABCKIT_INTERNAL_ERROR_VOID(module->file);

    switch (module->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return AnnotationRemoveAnnotationElementDynamic(anno->core, elem->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            return AnnotationRemoveAnnotationElementStatic(anno->core, elem->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Type
// ========================================

// ========================================
// Value
// ========================================

// ========================================
// String
// ========================================

// ========================================
// LiteralArray
// ========================================

AbckitArktsModifyApi g_arktsModifyApiImpl = {

    // ========================================
    // File
    // ========================================

    FileAddExternalModuleArktsV1, FileAddExternalModuleArktsV2,

    // ========================================
    // Module
    // ========================================

    ModuleSetName, ModuleAddImportFromArktsV1ToArktsV1, ModuleImportClassFromArktsV2ToArktsV2,
    ModuleImportStaticFunctionFromArktsV2ToArktsV2, ModuleImportClassMethodFromArktsV2ToArktsV2, ModuleRemoveImport,
    ModuleAddExportFromArktsV1ToArktsV1, ModuleRemoveExport, ModuleAddAnnotationInterface, ModuleAddField,

    // ========================================
    // Namespace
    // ========================================

    NamespaceSetName,

    // ========================================
    // Class
    // ========================================

    ClassAddAnnotation, ClassRemoveAnnotation, ClassAddInterface, ClassRemoveInterface, ClassSetSuperClass,
    ClassSetName, ClassAddField, ClassRemoveField, ClassRemoveMethod, CreateClass, ClassSetOwningModule,

    // ========================================
    // Interface
    // ========================================

    InterfaceAddSuperInterface, InterfaceRemoveSuperInterface, InterfaceSetName, InterfaceAddField,
    InterfaceRemoveField, InterfaceAddMethod, InterfaceRemoveMethod, InterfaceSetOwningModule, CreateInterface,

    // ========================================
    // Enum
    // ========================================

    EnumSetName, EnumAddField,

    // ========================================
    // Module Field
    // ========================================

    ModuleFieldSetName, ModuleFieldSetType, ModuleFieldSetValue,

    // ========================================
    // Namespace Field
    // ========================================

    NamespaceFieldSetName,

    // ========================================
    // Class Field
    // ========================================

    ClassFieldAddAnnotation, ClassFieldRemoveAnnotation, ClassFieldSetName, ClassFieldSetType, ClassFieldSetValue,

    // ========================================
    // Interface Field
    // ========================================

    InterfaceFieldAddAnnotation, InterfaceFieldRemoveAnnotation, InterfaceFieldSetName, InterfaceFieldSetType,

    // ========================================
    // Enum Field
    // ========================================

    EnumFieldSetName,

    // ========================================
    // AnnotationInterface
    // ========================================

    AnnotationInterfaceSetName, AnnotationInterfaceAddField, AnnotationInterfaceRemoveField,

    // ========================================
    // Function
    // ========================================

    FunctionAddAnnotation, FunctionRemoveAnnotation, FunctionSetName, CreateFunctionParameter, FunctionAddParameter,
    FunctionRemoveParameter, FunctionSetReturnType, ModuleAddFunction, ClassAddMethod,

    // ========================================
    // Annotation
    // ========================================

    AnnotationSetName, AnnotationAddAnnotationElement, AnnotationRemoveAnnotationElement,

    // ========================================
    // Type
    // ========================================

    // ========================================
    // Value
    // ========================================

    // ========================================
    // String
    // ========================================

    // ========================================
    // LiteralArray
    // ========================================

    // ========================================
    // LiteralArray
    // ========================================
};

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitArktsModifyApi const *AbckitGetArktsModifyApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockArktsModifyApiImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_arktsModifyApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
