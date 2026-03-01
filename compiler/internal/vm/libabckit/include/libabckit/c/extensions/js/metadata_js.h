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

#ifndef LIBABCKIT_METADATA_JS_H
#define LIBABCKIT_METADATA_JS_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#endif /* __cplusplus */

#include "libabckit/c/declarations.h"
#include "libabckit/c/api_version.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Struct that holds the pointers to the non-modifying API for Js-specific Abckit types.
 */
struct CAPI_EXPORT AbckitJsInspectApi {
    /* ========================================
     * Language-independent abstractions
     * ======================================== */

    /* ========================================
     * File
     * ======================================== */

    /* ========================================
     * String
     * ======================================== */

    /* ========================================
     * Type
     * ======================================== */

    /* ========================================
     * Value
     * ======================================== */

    /* ========================================
     * Literal
     * ======================================== */

    /* ========================================
     * LiteralArray
     * ======================================== */

    /* ========================================
     * Language-dependent abstractions
     * ======================================== */

    /* ========================================
     * Module
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitJsModule` to the instance of type `AbckitCoreModule`, which can be used
     * to invoke the corresponding APIs. The original pointer can still be used after the conversion.
     * @return Pointer to the language-independent representation of the `m`.
     * @param [ in ] m - Module to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     */
    AbckitCoreModule *(*jsModuleToCoreModule)(AbckitJsModule *m);

    /**
     * @brief Convert an instance of type `AbckitCoreModule` to the instance of type `AbckitJsModule`, which can be used
     * to invoke the corresponding APIs. The original pointer can still be used after the conversion.
     * @return Pointer to the language-dependent representation of the `m`.
     * @param [ in ] m - Module to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `m` is does not have `ABCKIT_TARGET_JS` target.
     */
    AbckitJsModule *(*coreModuleToJsModule)(AbckitCoreModule *m);

    /* ========================================
     * ImportDescriptor
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitJsImportDescriptor` to the instance of type
     * `AbckitCoreImportDescriptor`, which can be used to invoke the corresponding APIs. The original pointer can still
     * be used after the conversion.
     * @return Pointer to the language-independent representation of the `id`.
     * @param [ in ] id - Import descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     */
    AbckitCoreImportDescriptor *(*jsImportDescriptorToCoreImportDescriptor)(AbckitJsImportDescriptor *id);

    /**
     * @brief Convert an instance of type `AbckitCoreImportDescriptor` to the instance of type
     * `AbckitJsImportDescriptor`, which can be used to invoke the corresponding APIs. The original pointer can still be
     * used after the conversion.
     * @return Pointer to the language-dependent representation of the `id`.
     * @param [ in ] id - Import descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `id` is does not have `ABCKIT_TARGET_JS` target.
     */
    AbckitJsImportDescriptor *(*coreImportDescriptorToJsImportDescriptor)(AbckitCoreImportDescriptor *id);

    /* ========================================
     * ExportDescriptor
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitJsExportDescriptor` to the instance of type
     * `AbckitCoreExportDescriptor`, which can be used to invoke the corresponding APIs. The original pointer can still
     * be used after the conversion.
     * @return Pointer to the language-independent representation of the `ed`.
     * @param [ in ] ed - Export descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     */
    AbckitCoreExportDescriptor *(*jsExportDescriptorToCoreExportDescriptor)(AbckitJsExportDescriptor *ed);

    /**
     * @brief Convert an instance of type `AbckitCoreExportDescriptor` to the instance of type
     * `AbckitJsExportDescriptor`, which can be used to invoke the corresponding APIs. The original pointer can still be
     * used after the conversion.
     * @return Pointer to the language-dependent representation of the `ed`.
     * @param [ in ] ed - Export descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ed` is does not have `ABCKIT_TARGET_JS` target.
     */
    AbckitJsExportDescriptor *(*coreExportDescriptorToJsExportDescriptor)(AbckitCoreExportDescriptor *ed);

    /* ========================================
     * Class
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitJsClass` to the instance of type `AbckitCoreClass`, which can be used
     * to invoke the corresponding APIs. The original pointer can still be used after the conversion.
     * @return Pointer to the language-independent representation of the `c`.
     * @param [ in ] c - Class to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `c` is NULL.
     */
    AbckitCoreClass *(*jsClassToCoreClass)(AbckitJsClass *c);

    /**
     * @brief Convert an instance of type `AbckitCoreClass` to the instance of type `AbckitJsClass`, which can be used
     * to invoke the corresponding APIs. The original pointer can still be used after the conversion.
     * @return Pointer to the language-dependent representation of the `c`.
     * @param [ in ] c - Class to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `c` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `c` is does not have `ABCKIT_TARGET_JS` target.
     */
    AbckitJsClass *(*coreClassToJsClass)(AbckitCoreClass *c);

    /* ========================================
     * Function
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitJsFunction` to the instance of type `AbckitCoreFunction`, which can be
     * used to invoke the corresponding APIs. The original pointer can still be used after the conversion.
     * @return Pointer to the language-independent representation of the `f`.
     * @param [ in ] f - Function to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `f` is NULL.
     */
    AbckitCoreFunction *(*jsFunctionToCoreFunction)(AbckitJsFunction *f);

    /**
     * @brief Convert an instance of type `AbckitCoreFunction` to the instance of type `AbckitJsFunction`, which can be
     * used to invoke the corresponding APIs. The original pointer can still be used after the conversion.
     * @return Pointer to the language-dependent representation of the `f`.
     * @param [ in ] f - Function to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `f` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `f` is does not have `ABCKIT_TARGET_JS` target.
     */
    AbckitJsFunction *(*coreFunctionToJsFunction)(AbckitCoreFunction *f);

    /* ========================================
     * Annotation
     * ======================================== */

    /* ========================================
     * AnnotationElement
     * ======================================== */

    /* ========================================
     * AnnotationInterface
     * ======================================== */

    /* ========================================
     * AnnotationInterfaceField
     * ======================================== */
};

/**
 * @brief Struct that is used to create new imports.
 */
struct AbckitJsImportFromDynamicModuleCreateParams {
    /**
     * @brief Import name. For namespace imports equals to "*". For default imports equals to "default". For regular
     * imports is the same as in user code.
     */
    const char *name;
    /**
     * @brief Alias name for the import. For namespace imports is the same as in user code. For delault import is the
     * same as the default import name in user code. For regular imports is the same as in user code.
     */
    const char *alias;
};

/**
 * @brief Struct that is used to create new exports.
 */
struct AbckitJsDynamicModuleExportCreateParams {
    /**
     * @brief Name of the entity that should be exported. For star exports equals to "*". For indirect exports is the
     * same as in user code. For local exports is the same as in user code.
     */
    const char *name;
    /**
     * @brief Alias under which entity should be exported. For star exports equals nullptr. For indirect exports is the
     * same as in user code. For local exports is the same as in user code.
     */
    const char *alias;
};

/**
 * @brief Struct that is used to create new external modules.
 */
struct AbckitJsExternalModuleCreateParams {
    /**
     * @brief Name of the external module.
     */
    const char *name;
};

/**
 * @brief Struct that holds the pointers to the modifying API for Js-specific Abckit types.
 */
struct CAPI_EXPORT AbckitJsModifyApi {
    /* ========================================
     * File
     * ======================================== */

    /**
     * @brief Creates an external Js module and adds it to the file `file`.
     * @return Pointer to the created module.
     * @param [ in ] file - Binary file to add the external module to.
     * @param [ in ] params - Data that is used to create the external module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Allocates
     */
    AbckitJsModule *(*fileAddExternalModule)(AbckitFile *file, const struct AbckitJsExternalModuleCreateParams *params);

    /* ========================================
     * Module
     * ======================================== */

    /**
     * @brief Adds import from one Js module to another Js module.
     * @return Pointer to the newly created import descriptor.
     * @param [ in ] importing - Importing module.
     * @param [ in ] imported - The module the `importing` module imports from.
     * @param [ in ] params - Data that is used to create the import.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `importing` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `imported` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Allocates
     */
    AbckitJsImportDescriptor *(*moduleAddImportFromJsToJs)(
        AbckitJsModule *importing, AbckitJsModule *imported,
        const struct AbckitJsImportFromDynamicModuleCreateParams *params);

    /**
     * @brief Removes import `id` from module `m`.
     * @return None.
     * @param [ in ] m - The module to remove the import `id` from.
     * @param [ in ] id - Import to remove from the module `m`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module `m` does not have the import descriptor `id`.
     */
    void (*moduleRemoveImport)(AbckitJsModule *m, AbckitJsImportDescriptor *id);

    /**
     * @brief Adds export to the Js module.
     * @return Pointer to the newly created export descriptor.
     * @param [ in ] exporting - The module to add export to.
     * @param [ in ] exported - The module the entity is exported from. In case of local export is the same as
     * `exporting`.
     * @param [ in ] params - Data that is used to create the export.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exporting` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exported` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Allocates
     */
    AbckitJsExportDescriptor *(*moduleAddExportFromJsToJs)(
        AbckitJsModule *exporting, AbckitJsModule *exported,
        const struct AbckitJsDynamicModuleExportCreateParams *params);

    /**
     * @brief Removes export `ed` from module `m`.
     * @return None.
     * @param [ in ] m - Module to remove the export `ed` from.
     * @param [ in ] ed - Export to remove from the module `m`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module `m` does not have the export descriptor `ed`.
     */
    void (*moduleRemoveExport)(AbckitJsModule *m, AbckitJsExportDescriptor *ed);

    /* ========================================
     * Class
     * ======================================== */

    /* ========================================
     * AnnotationInterface
     * ======================================== */

    /* ========================================
     * Function
     * ======================================== */

    /* ========================================
     * Annotation
     * ======================================== */

    /* ========================================
     * Type
     * ======================================== */

    /* ========================================
     * Value
     * ======================================== */

    /* ========================================
     * String
     * ======================================== */

    /* ========================================
     * LiteralArray
     * ======================================== */

    /* ========================================
     * Literal
     * ======================================== */
};

/**
 * @brief Instantiates non-modifying API for Js-specific Abckit types.
 * @return Instance of the `AbckitApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitJsInspectApi const *AbckitGetJsInspectApiImpl(enum AbckitApiVersion version);

/**
 * @brief Instantiates modifying API for Js-specific Abckit types.
 * @return Instance of the `AbckitApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitJsModifyApi const *AbckitGetJsModifyApiImpl(enum AbckitApiVersion version);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBABCKIT_METADATA_JS_H */
