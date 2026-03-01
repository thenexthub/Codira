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
#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/js/metadata_js.h"

#include "libabckit/src/macros.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "scoped_timer.h"

namespace libabckit {

// ========================================
// File
// ========================================

extern "C" AbckitJsModule *FileAddExternalModule(AbckitFile *file,
                                                 const struct AbckitJsExternalModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(file, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    if (file->frontend != Mode::DYNAMIC) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_MODE);
        return nullptr;
    }

    return FileAddExternalJsModule(file, params);
}

// ========================================
// Module
// ========================================

extern "C" AbckitJsImportDescriptor *ModuleAddImportFromJsToJs(
    AbckitJsModule *importing /* assert(importing.target === AbckitTarget_ArkTS_v2) */,
    AbckitJsModule *imported /* assert(importing.target === AbckitTarget_ArkTS_v2) */,
    const struct AbckitJsImportFromDynamicModuleCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(importing, nullptr);
    LIBABCKIT_BAD_ARGUMENT(imported, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    LIBABCKIT_INTERNAL_ERROR(importing->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(imported->core, nullptr);

    return ModuleAddImportFromDynamicModuleDynamic(importing->core, imported->core, params);
}

extern "C" void ModuleRemoveImportJs(AbckitJsModule *m, AbckitJsImportDescriptor *i)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(i)

    LIBABCKIT_INTERNAL_ERROR_VOID(m->core)

    return ModuleRemoveImportDynamic(m->core, i);
}

extern "C" AbckitJsExportDescriptor *ModuleAddExportFromJsToJs(AbckitJsModule *exporting, AbckitJsModule *exported,
                                                               const AbckitJsDynamicModuleExportCreateParams *params)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(exporting, nullptr);
    LIBABCKIT_BAD_ARGUMENT(exported, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);

    LIBABCKIT_INTERNAL_ERROR(exporting->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(exported->core, nullptr);

    return DynamicModuleAddExportDynamic(exporting->core, exported->core, params);
}

extern "C" void ModuleRemoveExportJs(AbckitJsModule *m, AbckitJsExportDescriptor *e)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(m)
    LIBABCKIT_BAD_ARGUMENT_VOID(e)

    LIBABCKIT_INTERNAL_ERROR_VOID(m->core);

    return ModuleRemoveExportDynamic(m->core, e);
}

// ========================================
// Class
// ========================================

// ========================================
// AnnotationInterface
// ========================================

// ========================================
// Function
// ========================================

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

AbckitJsModifyApi g_jsModifyApiImpl = {

    // ========================================
    // File
    // ========================================

    FileAddExternalModule,

    // ========================================
    // Module
    // ========================================

    ModuleAddImportFromJsToJs, ModuleRemoveImportJs, ModuleAddExportFromJsToJs, ModuleRemoveExportJs,

    // ========================================
    // Class
    // ========================================

    // ========================================
    // AnnotationInterface
    // ========================================

    // ========================================
    // Function
    // ========================================

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
    // LiteralArray
    // ========================================
};

}  // namespace libabckit

#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
#include "./mock/abckit_mock.h"
#endif

extern "C" AbckitJsModifyApi const *AbckitGetJsModifyApiImpl(AbckitApiVersion version)
{
#ifdef ABCKIT_ENABLE_MOCK_IMPLEMENTATION
    return AbckitGetMockJsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
#endif
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_jsModifyApiImpl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}
