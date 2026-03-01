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

#include "helpers_common.h"

#include <cassert>
#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/statuses.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/helpers_common.h"
#include "scoped_timer.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/metadata_inspect_dynamic.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_ARKTS_TARGET(m)                                                     \
    if ((m)->target != ABCKIT_TARGET_ARK_TS_V1 && (m)->target != ABCKIT_TARGET_ARK_TS_V2) { \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);                      \
        /* CC-OFFNXT(G.PRE.05) code generation */                                           \
        return nullptr;                                                                     \
    }

namespace libabckit {

// ========================================
// File
// ========================================

// ========================================
// Module
// ========================================

extern "C" AbckitCoreModule *ArktsModuleToCoreModule(AbckitArktsModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    return m->core;
}

extern "C" AbckitArktsModule *CoreModuleToArktsModule(AbckitCoreModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(m);
    return m->GetArkTSImpl();
}

/* ========================================
 * Namespace
 * ======================================== */

extern "C" AbckitCoreNamespace *ArktsNamespaceToCoreNamespace(AbckitArktsNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    return n->core;
}

extern "C" AbckitArktsNamespace *CoreNamespaceToArktsNamespace(AbckitCoreNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(n->owningModule);
    return n->GetArkTSImpl();
}

extern "C" AbckitArktsFunction *ArktsV1NamespaceGetConstructor(AbckitArktsNamespace *n)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(n, nullptr);
    if (n->core->owningModule->target != ABCKIT_TARGET_ARK_TS_V1) {
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET);
        return nullptr;
    }
    return n->f->GetArkTSImpl();
}

// ========================================
// ImportDescriptor
// ========================================

extern "C" AbckitCoreImportDescriptor *ArktsImportDescriptorToCoreImportDescriptor(AbckitArktsImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    return i->core;
}

extern "C" AbckitArktsImportDescriptor *CoreImportDescriptorToArktsImportDescriptor(AbckitCoreImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(i, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(i->importingModule);
    return i->GetArkTSImpl();
}

// ========================================
// ExportDescriptor
// ========================================

extern "C" AbckitCoreExportDescriptor *ArktsExportDescriptorToCoreExportDescriptor(AbckitArktsExportDescriptor *e)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(e, nullptr);
    return e->core;
}

extern "C" AbckitArktsExportDescriptor *CoreExportDescriptorToArktsExportDescriptor(AbckitCoreExportDescriptor *e)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(e, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(e->exportingModule);
    return e->GetArkTSImpl();
}

// ========================================
// Class
// ========================================

extern "C" AbckitCoreClass *ArktsClassToCoreClass(AbckitArktsClass *c)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(c, nullptr);
    return c->core;
}

extern "C" AbckitArktsClass *CoreClassToArktsClass(AbckitCoreClass *c)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(c, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(c->owningModule);
    return c->GetArkTSImpl();
}

extern "C" bool ArktsClassIsFinal(AbckitArktsClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, false);
    return ClassIsFinalStatic(klass->core);
}

extern "C" bool ArktsClassIsAbstract(AbckitArktsClass *klass)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(klass, false);
    return ClassIsAbstractStatic(klass->core);
}

/* ========================================
 * Interface
 * ======================================== */

extern "C" AbckitCoreInterface *ArktsInterfaceToCoreInterface(AbckitArktsInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, nullptr);
    return iface->core;
}

extern "C" AbckitArktsInterface *CoreInterfaceToArktsInterface(AbckitCoreInterface *iface)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(iface, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(iface->owningModule);
    return iface->GetArkTSImpl();
}

/* ========================================
 * Enum
 * ======================================== */

extern "C" AbckitCoreEnum *ArktsEnumToCoreEnum(AbckitArktsEnum *enm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, nullptr);
    return enm->core;
}

extern "C" AbckitArktsEnum *CoreEnumToArktsEnum(AbckitCoreEnum *enm)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(enm, nullptr);
    return enm->GetArkTSImpl();
}

/* ========================================
 * Module Field
 * ======================================== */

extern "C" AbckitCoreModuleField *ArktsModuleFieldToCoreModuleField(AbckitArktsModuleField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->core;
}

extern "C" AbckitArktsModuleField *CoreModuleFieldToArktsModuleField(AbckitCoreModuleField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->GetArkTSImpl();
}

/* ========================================
 * Namespace Field
 * ======================================== */

extern "C" AbckitArktsNamespaceField *CoreNamespaceFieldToArktsNamespaceField(AbckitCoreNamespaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->GetArkTSImpl();
}

/* ========================================
 * Class Field
 * ======================================== */

extern "C" AbckitCoreClassField *ArktsClassFieldToCoreClassField(AbckitArktsClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->core;
}

extern "C" AbckitArktsClassField *CoreClassFieldToArktsClassField(AbckitCoreClassField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->GetArkTSImpl();
}

/* ========================================
 * Interface Field
 * ======================================== */

extern "C" AbckitCoreInterfaceField *ArktsInterfaceFieldToCoreInterfaceField(AbckitArktsInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->core;
}

extern "C" AbckitArktsInterfaceField *CoreInterfaceFieldToArktsInterfaceField(AbckitCoreInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->GetArkTSImpl();
}

extern "C" bool ArktsInterfaceFieldIsReadonly(AbckitArktsInterfaceField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, false);
    return InterfaceFieldIsReadonlyStatic(field->core);
}

/* ========================================
 * Enum Field
 * ======================================== */

extern "C" AbckitCoreEnumField *ArktsEnumFieldToCoreEnumField(AbckitArktsEnumField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->core;
}

extern "C" AbckitArktsEnumField *CoreEnumFieldToArktsEnumField(AbckitCoreEnumField *field)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(field, nullptr);
    return field->GetArkTSImpl();
}

// ========================================
// Function
// ========================================

extern "C" AbckitCoreFunction *ArktsFunctionToCoreFunction(AbckitArktsFunction *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    return m->core;
}

extern "C" AbckitArktsFunction *CoreFunctionToArktsFunction(AbckitCoreFunction *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(m->owningModule);
    return m->GetArkTSImpl();
}

extern "C" bool FunctionIsNative(AbckitArktsFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    switch (function->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionIsNativeDynamic(function->core);
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionIsNativeStatic(function->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool FunctionIsAsync(AbckitArktsFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    switch (function->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionIsAsyncStatic(function->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool FunctionIsFinal(AbckitArktsFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    switch (function->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionIsFinalStatic(function->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

extern "C" bool FunctionIsAbstract(AbckitArktsFunction *function)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_BAD_ARGUMENT(function, false);

    switch (function->core->owningModule->target) {
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionIsAbstractStatic(function->core);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

// ========================================
// Annotation
// ========================================

extern "C" AbckitCoreAnnotation *ArktsAnnotationToCoreAnnotation(AbckitArktsAnnotation *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    return a->core;
}

extern "C" AbckitArktsAnnotation *CoreAnnotationToArktsAnnotation(AbckitCoreAnnotation *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);

    AbckitCoreModule *module = GetAnnotationOwningModule(a);
    if (module == nullptr) {
        libabckit::statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    LIBABCKIT_CHECK_ARKTS_TARGET(module);
    return a->GetArkTSImpl();
}

// ========================================
// AnnotationElement
// ========================================

extern "C" AbckitCoreAnnotationElement *ArktsAnnotationElementToCoreAnnotationElement(AbckitArktsAnnotationElement *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    return a->core;
}

extern "C" AbckitArktsAnnotationElement *CoreAnnotationElementToArktsAnnotationElement(AbckitCoreAnnotationElement *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    LIBABCKIT_BAD_ARGUMENT(a->ann, nullptr);

    AbckitCoreModule *module = GetAnnotationOwningModule(a->ann);
    if (module == nullptr) {
        libabckit::statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    LIBABCKIT_CHECK_ARKTS_TARGET(module);
    return a->GetArkTSImpl();
}

// ========================================
// AnnotationInterface
// ========================================

extern "C" AbckitCoreAnnotationInterface *ArktsAnnotationInterfaceToCoreAnnotationInterface(
    AbckitArktsAnnotationInterface *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    return a->core;
}

extern "C" AbckitArktsAnnotationInterface *CoreAnnotationInterfaceToArktsAnnotationInterface(
    AbckitCoreAnnotationInterface *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    LIBABCKIT_CHECK_ARKTS_TARGET(a->owningModule);
    return a->GetArkTSImpl();
}

// ========================================
// AnnotationInterfaceField
// ========================================

extern "C" AbckitCoreAnnotationInterfaceField *ArktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField(
    AbckitArktsAnnotationInterfaceField *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);
    return a->core;
}

extern "C" AbckitArktsAnnotationInterfaceField *CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField(
    AbckitCoreAnnotationInterfaceField *a)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(a, nullptr);

    if (a->ai == nullptr) {
        libabckit::statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    LIBABCKIT_CHECK_ARKTS_TARGET(a->ai->owningModule);
    return a->GetArkTSImpl();
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

// ========================================
// Literal
// ========================================

AbckitArktsInspectApi g_arktsInspectApiImpl = {
    // ========================================
    // File
    // ========================================

    // ========================================
    // Module
    // ========================================

    ArktsModuleToCoreModule, CoreModuleToArktsModule,

    /* ========================================
     * Namespace
     * ======================================== */

    ArktsNamespaceToCoreNamespace, CoreNamespaceToArktsNamespace, ArktsV1NamespaceGetConstructor,

    // ========================================
    // ImportDescriptor
    // ========================================

    ArktsImportDescriptorToCoreImportDescriptor, CoreImportDescriptorToArktsImportDescriptor,

    // ========================================
    // ExportDescriptor
    // ========================================

    ArktsExportDescriptorToCoreExportDescriptor, CoreExportDescriptorToArktsExportDescriptor,

    // ========================================
    // Class
    // ========================================

    ArktsClassToCoreClass, CoreClassToArktsClass, ArktsClassIsFinal, ArktsClassIsAbstract,

    // ========================================
    // Interface
    // ========================================

    ArktsInterfaceToCoreInterface, CoreInterfaceToArktsInterface,

    // ========================================
    // Enum
    // ========================================

    ArktsEnumToCoreEnum, CoreEnumToArktsEnum,

    // ========================================
    // Module Field
    // ========================================

    ArktsModuleFieldToCoreModuleField, CoreModuleFieldToArktsModuleField,

    // ========================================
    // Namespace Field
    // ========================================

    CoreNamespaceFieldToArktsNamespaceField,

    // ========================================
    // Class Field
    // ========================================

    ArktsClassFieldToCoreClassField, CoreClassFieldToArktsClassField,

    // ========================================
    // Interface Field
    // ========================================

    ArktsInterfaceFieldToCoreInterfaceField, CoreInterfaceFieldToArktsInterfaceField, ArktsInterfaceFieldIsReadonly,

    // ========================================
    // Enum Field
    // ========================================

    ArktsEnumFieldToCoreEnumField, CoreEnumFieldToArktsEnumField,

    // ========================================
    // Function
    // ========================================

    ArktsFunctionToCoreFunction, CoreFunctionToArktsFunction, FunctionIsNative, FunctionIsAsync, FunctionIsFinal,
    FunctionIsAbstract,

    // ========================================
    // Annotation
    // ========================================

    ArktsAnnotationToCoreAnnotation, CoreAnnotationToArktsAnnotation,

    // ========================================
    // AnnotationElement
    // ========================================

    ArktsAnnotationElementToCoreAnnotationElement, CoreAnnotationElementToArktsAnnotationElement,

    // ========================================
    // AnnotationInterface
    // ========================================

    ArktsAnnotationInterfaceToCoreAnnotationInterface, CoreAnnotationInterfaceToArktsAnnotationInterface,

    // ========================================
    // AnnotationInterfaceField
    // ========================================

    ArktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField,
    CoreAnnotationInterfaceFieldToArktsAnnotationInterfaceField,

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
    // Literal
    // ========================================
};

// ========================================
// Enumerators
// ========================================

// ========================================
// Module
// ========================================

bool ArkTSModuleEnumerateImports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreImportDescriptor *i, void *data))
{
    for (auto &id : m->id) {
        if (!cb(id.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ArkTSModuleEnumerateExports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreExportDescriptor *e, void *data))
{
    for (auto &ed : m->ed) {
        if (!cb(ed.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ArkTSModuleEnumerateNamespaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *ns, void *data))
{
    return ModuleEnumerateNamespacesHelper(m, data, cb);
}

bool ArkTSModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool cb(AbckitCoreClass *klass, void *data))
{
    return ModuleEnumerateClassesHelper(m, data, cb);
}

bool ArkTSModuleEnumerateInterfaces(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    return ModuleEnumerateInterfacesHelper(m, data, cb);
}

bool ArkTSModuleEnumerateEnums(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data))
{
    return ModuleEnumerateEnumsHelper(m, data, cb);
}

bool ArkTSModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                           bool (*cb)(AbckitCoreFunction *function, void *data))
{
    return ModuleEnumerateTopLevelFunctionsHelper(m, data, cb);
}

bool ArkTSModuleEnumerateFields(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreModuleField *field, void *data))
{
    return ModuleEnumerateFieldsHelper(m, data, cb);
}

bool ArkTSModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                            bool (*cb)(AbckitCoreFunction *function, void *data))
{
    switch (m->target) {
        case ABCKIT_TARGET_ARK_TS_V1:
            return ModuleEnumerateAnonymousFunctionsDynamic(m, data, cb);
        case ABCKIT_TARGET_ARK_TS_V2:
            return ModuleEnumerateAnonymousFunctionsStatic(m, data, cb);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

bool ArkTSModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    return ModuleEnumerateAnnotationInterfacesHelper(m, data, cb);
}

// ========================================
// Namespace
// ========================================

bool ArkTSNamespaceEnumerateNamespaces(AbckitCoreNamespace *n, void *data,
                                       bool (*cb)(AbckitCoreNamespace *n, void *data))
{
    return NamespaceEnumerateNamespacesHelper(n, data, cb);
}

bool ArkTSNamespaceEnumerateClasses(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreClass *klass, void *data))
{
    return NamespaceEnumerateClassesHelper(n, data, cb);
}

bool ArkTSNamespaceEnumerateInterfaces(AbckitCoreNamespace *n, void *data,
                                       bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    return NamespaceEnumerateInterfacesHelper(n, data, cb);
}

bool ArkTSNamespaceEnumerateEnums(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data))
{
    return NamespaceEnumerateEnumsHelper(n, data, cb);
}

bool ArkTSNamespaceEnumerateFields(AbckitCoreNamespace *n, void *data,
                                   bool (*cb)(AbckitCoreNamespaceField *field, void *data))
{
    return NamespaceEnumerateFieldsHelper(n, data, cb);
}

bool ArkTSNamespaceEnumerateTopLevelFunctions(AbckitCoreNamespace *n, void *data,
                                              bool (*cb)(AbckitCoreFunction *func, void *data))
{
    for (auto &f : n->functions) {
        if (!cb(f.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ArkTSNamespaceEnumerateAnnotationInterfaces(AbckitCoreNamespace *n, void *data,
                                                 bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    return NamespaceEnumerateAnnotationInterfacesHelper(n, data, cb);
}

// ========================================
// Class
// ========================================

bool ArkTSClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *method, void *data))
{
    return ClassEnumerateMethodsHelper(klass, data, cb);
}

bool ArkTSClassEnumerateFields(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreClassField *field, void *data))
{
    return ClassEnumerateFieldsHelper(klass, data, cb);
}

bool ArkTSClassEnumerateAnnotations(AbckitCoreClass *klass, void *data,
                                    bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_BAD_ARGUMENT(klass, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    for (auto &annotation : klass->annotations) {
        if (!cb(annotation.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ArkTSClassEnumerateSubClasses(AbckitCoreClass *klass, void *data,
                                   bool (*cb)(AbckitCoreClass *subClass, void *data))
{
    return ClassEnumerateSubClassesHelper(klass, data, cb);
}

bool ArkTSClassEnumerateInterfaces(AbckitCoreClass *klass, void *data,
                                   bool (*cb)(AbckitCoreInterface *iface, void *data))
{
    return ClassEnumerateInterfacesHelper(klass, data, cb);
}

// ========================================
// Interface
// ========================================

bool ArkTSInterfaceEnumerateMethods(AbckitCoreInterface *iface, void *data,
                                    bool (*cb)(AbckitCoreFunction *method, void *data))
{
    return InterfaceEnumerateMethodsHelper(iface, data, cb);
}

bool ArkTSInterfaceEnumerateFields(AbckitCoreInterface *iface, void *data,
                                   bool (*cb)(AbckitCoreInterfaceField *field, void *data))
{
    return InterfaceEnumerateFieldsHelper(iface, data, cb);
}

bool ArkTSInterfaceEnumerateSuperInterfaces(AbckitCoreInterface *iface, void *data,
                                            bool (*cb)(AbckitCoreInterface *superInterface, void *data))
{
    return InterfaceEnumerateSuperInterfacesHelper(iface, data, cb);
}

bool ArkTSInterfaceEnumerateSubInterfaces(AbckitCoreInterface *iface, void *data,
                                          bool (*cb)(AbckitCoreInterface *subInterface, void *data))
{
    return InterfaceEnumerateSubInterfacesHelper(iface, data, cb);
}

bool ArkTSInterfaceEnumerateClasses(AbckitCoreInterface *iface, void *data,
                                    bool (*cb)(AbckitCoreClass *klass, void *data))
{
    return InterfaceEnumerateClassesHelper(iface, data, cb);
}

bool ArkTSInterfaceEnumerateAnnotations(AbckitCoreInterface *iface, void *data,
                                        bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    return InterfaceEnumerateAnnotationsHelper(iface, data, cb);
}

// ========================================
// Enum
// ========================================

bool ArkTSEnumEnumerateMethods(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreFunction *method, void *data))
{
    return EnumEnumerateMethodsHelper(enm, data, cb);
}

bool ArkTSEnumEnumerateFields(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreEnumField *field, void *data))
{
    return EnumEnumerateFieldsHelper(enm, data, cb);
}

// ========================================
// Field
// ========================================

bool ArkTSClassFieldEnumerateAnnotations(AbckitCoreClassField *field, void *data,
                                         bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    return ClassFieldEnumerateAnnotationsHelper(field, data, cb);
}

bool ArkTSInterfaceFieldEnumerateAnnotations(AbckitCoreInterfaceField *field, void *data,
                                             bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    return InterfaceFieldEnumerateAnnotationsHelper(field, data, cb);
}

// ========================================
// Function
// ========================================

bool ArkTSFunctionEnumerateNestedFunctions([[maybe_unused]] AbckitCoreFunction *function, [[maybe_unused]] void *data,
                                           [[maybe_unused]] bool (*cb)(AbckitCoreFunction *nestedFunc, void *data))
{
    return false;  // There is no nested functions in ArkTS
}

bool ArkTSFunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                         bool (*cb)(AbckitCoreClass *nestedClass, void *data))
{
    for (auto &c : function->nestedClasses) {
        if (!cb(c.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ArkTSFunctionEnumerateAnnotations(AbckitCoreFunction *function, void *data,
                                       bool (*cb)(AbckitCoreAnnotation *anno, void *data))
{
    LIBABCKIT_BAD_ARGUMENT(function, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    for (auto &annotation : function->annotations) {
        if (!cb(annotation.get(), data)) {
            return false;
        }
    }
    return true;
}

bool ArkTSFunctionEnumerateParameters(AbckitCoreFunction *function, void *data,
                                      bool (*cb)(AbckitCoreFunctionParam *param, void *data))
{
    LIBABCKIT_BAD_ARGUMENT(function, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    for (auto &param : function->parameters) {
        if (!cb(param.get(), data)) {
            return false;
        }
    }
    return true;
}

// ========================================
// Annotation
// ========================================

bool ArkTSAnnotationEnumerateElements(AbckitCoreAnnotation *anno, void *data,
                                      bool (*cb)(AbckitCoreAnnotationElement *ae, void *data))
{
    LIBABCKIT_BAD_ARGUMENT(anno, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    AbckitCoreModule *m = GetAnnotationOwningModule(anno);

    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_INTERNAL_ERROR(m->file, false)

    for (auto &elem : anno->elements) {
        if (!cb(elem.get(), data)) {
            return false;
        }
    }
    return true;
}

// ========================================
// AnnotationInterface
// ========================================

bool ArkTSAnnotationInterfaceEnumerateFields(AbckitCoreAnnotationInterface *ai, void *data,
                                             bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data))
{
    LIBABCKIT_BAD_ARGUMENT(ai, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    for (auto &ed : ai->fields) {
        if (!cb(ed.get(), data)) {
            return false;
        }
    }
    return true;
}

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitArktsInspectApi const *AbckitGetArktsInspectApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockArktsInspectApiImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_arktsInspectApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
