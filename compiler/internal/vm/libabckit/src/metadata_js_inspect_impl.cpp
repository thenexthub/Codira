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
#include "libabckit/c/extensions/js/metadata_js.h"

#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/macros.h"
#include "scoped_timer.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/metadata_inspect_dynamic.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"
#include "libabckit/src/helpers_common.h"

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_JS_TARGET(m)                                   \
    if ((m)->target != ABCKIT_TARGET_JS) {                             \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET); \
        /* CC-OFFNXT(G.PRE.05) code generation */                      \
        return nullptr;                                                \
    }

namespace libabckit {

// ========================================
// File
// ========================================

// ========================================
// Module
// ========================================

extern "C" AbckitCoreModule *JsModuleToCoreModule(AbckitJsModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    return m->core;
}

extern "C" AbckitJsModule *CoreModuleToJsModule(AbckitCoreModule *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_CHECK_JS_TARGET(m);
    return m->GetJsImpl();
}

// ========================================
// ImportDescriptor
// ========================================

extern "C" AbckitCoreImportDescriptor *JsImportDescriptorToCoreImportDescriptor(AbckitJsImportDescriptor *id)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(id, nullptr);
    return id->core;
}

extern "C" AbckitJsImportDescriptor *CoreImportDescriptorToJsImportDescriptor(AbckitCoreImportDescriptor *id)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(id, nullptr);
    LIBABCKIT_CHECK_JS_TARGET(id->importingModule);
    return id->GetJsImpl();
}

// ========================================
// ExportDescriptor
// ========================================

extern "C" AbckitCoreExportDescriptor *JsExportDescriptorToCoreExportDescriptor(AbckitJsExportDescriptor *ed)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(ed, nullptr);
    return ed->core;
}

extern "C" AbckitJsExportDescriptor *CoreExportDescriptorToJsExportDescriptor(AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(ed, nullptr);
    LIBABCKIT_CHECK_JS_TARGET(ed->exportingModule);
    return ed->GetJsImpl();
}

// ========================================
// Class
// ========================================

extern "C" AbckitCoreClass *JsClassToCoreClass(AbckitJsClass *c)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(c, nullptr);
    return c->core;
}

extern "C" AbckitJsClass *CoreClassToJsClass(AbckitCoreClass *c)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(c, nullptr);
    LIBABCKIT_CHECK_JS_TARGET(c->owningModule);
    return c->GetJsImpl();
}

// ========================================
// AnnotationInterface
// ========================================

// ========================================
// AnnotationInterfaceField
// ========================================

// ========================================
// Function
// ========================================

extern "C" AbckitCoreFunction *JsFunctionToCoreFunction(AbckitJsFunction *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    return m->core;
}

extern "C" AbckitJsFunction *CoreFunctionToJsFunction(AbckitCoreFunction *m)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_CHECK_JS_TARGET(m->owningModule);
    return m->GetJsImpl();
}

// ========================================
// Annotation
// ========================================

// ========================================
// AnnotationElement
// ========================================

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

AbckitJsInspectApi g_jsInspectApiImpl = {
    // ========================================
    // File
    // ========================================

    // ========================================
    // Module
    // ========================================

    JsModuleToCoreModule, CoreModuleToJsModule,

    // ========================================
    // ImportDescriptor
    // ========================================

    JsImportDescriptorToCoreImportDescriptor, CoreImportDescriptorToJsImportDescriptor,

    // ========================================
    // ExportDescriptor
    // ========================================

    JsExportDescriptorToCoreExportDescriptor, CoreExportDescriptorToJsExportDescriptor,

    // ========================================
    // Class
    // ========================================

    JsClassToCoreClass, CoreClassToJsClass,

    // ========================================
    // AnnotationInterface
    // ========================================

    // ========================================
    // AnnotationInterfaceField
    // ========================================

    // ========================================
    // Function
    // ========================================

    JsFunctionToCoreFunction, CoreFunctionToJsFunction,

    // ========================================
    // Annotation
    // ========================================

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

bool JsModuleEnumerateImports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreImportDescriptor *i, void *data))
{
    for (auto &id : m->id) {
        if (!cb(id.get(), data)) {
            return false;
        }
    }
    return true;
}

bool JsModuleEnumerateExports(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreExportDescriptor *e, void *data))
{
    for (auto &ed : m->ed) {
        if (!cb(ed.get(), data)) {
            return false;
        }
    }
    return true;
}

bool JsModuleEnumerateClasses(AbckitCoreModule *m, void *data, bool cb(AbckitCoreClass *klass, void *data))
{
    return ModuleEnumerateClassesHelper(m, data, cb);
}

bool JsModuleEnumerateTopLevelFunctions(AbckitCoreModule *m, void *data,
                                        bool (*cb)(AbckitCoreFunction *function, void *data))
{
    return ModuleEnumerateTopLevelFunctionsHelper(m, data, cb);
}

bool JsModuleEnumerateAnonymousFunctions(AbckitCoreModule *m, void *data,
                                         bool (*cb)(AbckitCoreFunction *function, void *data))
{
    return ModuleEnumerateAnonymousFunctionsDynamic(m, data, cb);
}

bool JsModuleEnumerateAnnotationInterfaces(AbckitCoreModule *m, void *data,
                                           bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data))
{
    return ModuleEnumerateAnnotationInterfacesHelper(m, data, cb);
}

// ========================================
// Class
// ========================================

bool JsClassEnumerateMethods(AbckitCoreClass *klass, void *data, bool (*cb)(AbckitCoreFunction *function, void *data))
{
    return ClassEnumerateMethodsHelper(klass, data, cb);
}

// ========================================
// Function
// ========================================

bool JsFunctionEnumerateNestedFunctions(AbckitCoreFunction *function, void *data,
                                        bool (*cb)(AbckitCoreFunction *nestedFunc, void *data))
{
    for (auto &f : function->nestedFunction) {
        if (!cb(f.get(), data)) {
            return false;
        }
    }
    return true;
}

bool JsFunctionEnumerateNestedClasses(AbckitCoreFunction *function, void *data,
                                      bool (*cb)(AbckitCoreClass *nestedClass, void *data))
{
    for (auto &c : function->nestedClasses) {
        if (!cb(c.get(), data)) {
            return false;
        }
    }
    return true;
}

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitJsInspectApi const *AbckitGetJsInspectApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockJsInspectApiImpl(version);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_jsInspectApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
