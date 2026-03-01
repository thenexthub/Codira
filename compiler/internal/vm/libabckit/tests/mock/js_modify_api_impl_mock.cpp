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

#include "include/libabckit/c/extensions/js/metadata_js.h"

#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock {

// NOLINTBEGIN(readability-identifier-naming)

inline AbckitJsModule *FileAddExternalModule(AbckitFile *file, const struct AbckitJsExternalModuleCreateParams *params)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    return DEFAULT_JS_MODULE;
}

inline AbckitJsImportDescriptor *ModuleAddImportFromJsToJs(
    AbckitJsModule *importing, AbckitJsModule *imported,
    const struct AbckitJsImportFromDynamicModuleCreateParams *params)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(importing == DEFAULT_JS_MODULE);
    EXPECT_TRUE(imported == DEFAULT_JS_MODULE);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    return DEFAULT_JS_IMPORT_DESCRIPTOR;
}

inline void ModuleRemoveImport(AbckitJsModule *removeFrom, AbckitJsImportDescriptor *toRemove)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(removeFrom == DEFAULT_JS_MODULE);
    EXPECT_TRUE(toRemove == DEFAULT_JS_IMPORT_DESCRIPTOR);
}

inline AbckitJsExportDescriptor *ModuleAddExportFromJsToJs(AbckitJsModule *importing, AbckitJsModule *imported,
                                                           const struct AbckitJsDynamicModuleExportCreateParams *params)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(importing == DEFAULT_JS_MODULE);
    EXPECT_TRUE(imported == DEFAULT_JS_MODULE);
    EXPECT_TRUE(strncmp(params->name, DEFAULT_CONST_CHAR, DEFAULT_CONST_CHAR_SIZE) == 0);
    return DEFAULT_JS_EXPORT_DESCRIPTOR;
}

void ModuleRemoveExport(AbckitJsModule *removeFrom, AbckitJsExportDescriptor *toRemove)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(removeFrom == DEFAULT_JS_MODULE);
    EXPECT_TRUE(toRemove == DEFAULT_JS_EXPORT_DESCRIPTOR);
}

AbckitJsModifyApi g_jsModifyApiImpl = {

    // ========================================
    // File
    // ========================================

    FileAddExternalModule, ModuleAddImportFromJsToJs, ModuleRemoveImport, ModuleAddExportFromJsToJs,
    ModuleRemoveExport};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock

AbckitJsModifyApi const *AbckitGetMockJsModifyApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::g_jsModifyApiImpl;
}