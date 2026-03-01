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

#include "../../src/mock/abckit_mock.h"
#include "../../src/mock/mock_values.h"

#include "../include/libabckit/c/extensions/js/metadata_js.h"
#include "../../include/libabckit/c/metadata_core.h"

#include <cstring>
#include <gtest/gtest.h>

// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CHECK_JS_TARGET(m)                                   \
    if ((m)->target != ABCKIT_TARGET_JS) {                             \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_TARGET); \
        /* CC-OFFNXT(G.PRE.05) code generation */                      \
        return nullptr;                                                \
    }

namespace libabckit::mock {

// ========================================
// File
// ========================================

// ========================================
// Module
// ========================================

inline AbckitCoreModule *JsModuleToCoreModule(AbckitJsModule *m)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_JS_MODULE);
    return DEFAULT_CORE_MODULE;
}

inline AbckitJsModule *CoreModuleToJsModule(AbckitCoreModule *m)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_CORE_MODULE);
    return DEFAULT_JS_MODULE;
}

// ========================================
// ImportDescriptor
// ========================================

inline AbckitCoreImportDescriptor *JsImportDescriptorToCoreImportDescriptor(AbckitJsImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(id == DEFAULT_JS_IMPORT_DESCRIPTOR);
    return DEFAULT_CORE_IMPORT_DESCRIPTOR;
}

inline AbckitJsImportDescriptor *CoreImportDescriptorToJsImportDescriptor(AbckitCoreImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(id == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_JS_IMPORT_DESCRIPTOR;
}

// ========================================
// ExportDescriptor
// ========================================

inline AbckitCoreExportDescriptor *JsExportDescriptorToCoreExportDescriptor(AbckitJsExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ed == DEFAULT_JS_EXPORT_DESCRIPTOR);
    return DEFAULT_CORE_EXPORT_DESCRIPTOR;
}

inline AbckitJsExportDescriptor *CoreExportDescriptorToJsExportDescriptor(AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_JS_EXPORT_DESCRIPTOR;
}

// ========================================
// Class
// ========================================

inline AbckitCoreClass *JsClassToCoreClass(AbckitJsClass *c)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(c == DEFAULT_JS_CLASS);
    return DEFAULT_CORE_CLASS;
}

inline AbckitJsClass *CoreClassToJsClass(AbckitCoreClass *c)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(c == DEFAULT_CORE_CLASS);
    return DEFAULT_JS_CLASS;
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

inline AbckitCoreFunction *JsFunctionToCoreFunction(AbckitJsFunction *m)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_JS_FUNCTION);
    return DEFAULT_CORE_FUNCTION;
}

inline AbckitJsFunction *CoreFunctionToJsFunction(AbckitCoreFunction *m)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(m == DEFAULT_CORE_FUNCTION);
    return DEFAULT_JS_FUNCTION;
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

}  // namespace libabckit::mock

AbckitJsInspectApi const *AbckitGetMockJsInspectApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::g_jsInspectApiImpl;
}
