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

#ifndef LIBABCKIT_METADATA_ARKTS_H
#define LIBABCKIT_METADATA_ARKTS_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif /* __cplusplus */

#include "../../declarations.h"
#include "../../api_version.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Struct that holds the pointers to the non-modifying API for Arkts-specific Abckit types.
 */
struct CAPI_EXPORT AbckitArktsInspectApi {
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
     * @brief Convert an instance of type `AbckitArktsModule` to the instance of type `AbckitCoreModule`, which can be
     * used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `m`.
     * @param [ in ] m - Module to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `m` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreModule *(*arktsModuleToCoreModule)(AbckitArktsModule *m);

    /**
     * @brief Convert an instance of type `AbckitCoreModule` to the instance of type `AbckitArktsModule`, which can be
     * used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `m`.
     * @param [ in ] m - Module to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `m` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsModule *(*coreModuleToArktsModule)(AbckitCoreModule *m);

    /* ========================================
     * Namespace
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsNamespace` to the instance of type `AbckitCoreNamespace`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `ns`.
     * @param [ in ] ns - Namespace to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ns` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ns` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreNamespace *(*arktsNamespaceToCoreNamespace)(AbckitArktsNamespace *ns);

    /**
     * @brief Convert an instance of type `AbckitCoreNamespace` to the instance of type `AbckitArktsNamespace`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `ns`.
     * @param [ in ] ns - Namespace to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ns` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ns` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsNamespace *(*coreNamespaceToArktsNamespace)(AbckitCoreNamespace *ns);

    /**
     * @brief Returns constructor function for namespace.
     * @return Function that is invoked upon namespace `ns` construction.
     * @param [ in ] ns - Namespace to get the constructor function for.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ns` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ns` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    AbckitArktsFunction *(*arktsV1NamespaceGetConstructor)(AbckitArktsNamespace *ns);

    /* ========================================
     * ImportDescriptor
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsImportDescriptorToCoreImportDescrip` to the instance of type `r`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `id`.
     * @param [ in ] id - Import descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `id` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    AbckitCoreImportDescriptor *(*arktsImportDescriptorToCoreImportDescriptor)(AbckitArktsImportDescriptor *id);

    /**
     * @brief Convert an instance of type `AbckitCoreImportDescriptorToArktsImportDescrip` to the instance of type `r`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `id`.
     * @param [ in ] id - Import descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `id` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    AbckitArktsImportDescriptor *(*coreImportDescriptorToArktsImportDescriptor)(AbckitCoreImportDescriptor *id);

    /* ========================================
     * ExportDescriptor
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsExportDescriptorToCoreExportDescrip` to the instance of type `r`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `ed`.
     * @param [ in ] ed - Export descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ed` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    AbckitCoreExportDescriptor *(*arktsExportDescriptorToCoreExportDescriptor)(AbckitArktsExportDescriptor *ed);

    /**
     * @brief Convert an instance of type `AbckitCoreExportDescriptorToArktsExportDescrip` to the instance of type `r`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `ed`.
     * @param [ in ] ed - Export descriptor to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ed` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    AbckitArktsExportDescriptor *(*coreExportDescriptorToArktsExportDescriptor)(AbckitCoreExportDescriptor *ed);

    /* ========================================
     * Class
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsClass` to the instance of type `AbckitCoreClass`, which can be
     * used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `klass`.
     * @param [ in ] klass - Class to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreClass *(*arktsClassToCoreClass)(AbckitArktsClass *klass);

    /**
     * @brief Convert an instance of type `AbckitCoreClass` to the instance of type `AbckitArktsClass`, which can be
     * used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `klass`.
     * @param [ in ] klass - Class to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsClass *(*coreClassToArktsClass)(AbckitCoreClass *klass);

    /**
     * @brief Returns whether class `klass` is final.
     * @return `true` if class `klass` is final.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*arktsClassIsFinal)(AbckitArktsClass *klass);

    /**
     * @brief Returns whether class `klass` is abstract.
     * @return `true` if class `klass` is abstract.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*arktsClassIsAbstract)(AbckitArktsClass *klass);

    /* ========================================
     * Interface
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsInterface` to the instance of type `AbckitCoreInterface`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `iface`.
     * @param [ in ] iface - Interface to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreInterface *(*arktsInterfaceToCoreInterface)(AbckitArktsInterface *iface);

    /**
     * @brief Convert an instance of type `AbckitCoreInterface` to the instance of type `AbckitArktsInterface`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `iface`.
     * @param [ in ] iface - Interface to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsInterface *(*coreInterfaceToArktsInterface)(AbckitCoreInterface *iface);

    /* ========================================
     * Enum
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsEnum` to the instance of type `AbckitCoreEnum`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `enm`.
     * @param [ in ] enm - Enum to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `enm` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreEnum *(*arktsEnumToCoreEnum)(AbckitArktsEnum *enm);

    /**
     * @brief Convert an instance of type `AbckitCoreEnum` to the instance of type `AbckitArktsEnum`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `enm`.
     * @param [ in ] enm - Enum to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `enm` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsEnum *(*coreEnumToArktsEnum)(AbckitCoreEnum *enm);

    /* ========================================
     * Module Field
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsModuleField` to the instance of type `AbckitCoreModuleField`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `field`.
     * @param [ in ] field - Module field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreModuleField *(*arktsModuleFieldToCoreModuleField)(AbckitArktsModuleField *field);

    /**
     * @brief Convert an instance of type `AbckitCoreModuleField` to the instance of type `AbckitArktsModuleField`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `field`.
     * @param [ in ] field - Module field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsModuleField *(*coreModuleFieldToArktsModuleField)(AbckitCoreModuleField *field);

    /* ========================================
     * Namespace Field
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitCoreNamespaceField` to the instance of type
     * `AbckitArktsNamespaceField`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `field`.
     * @param [ in ] field - Namespace field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsNamespaceField *(*coreNamespaceFieldToArktsNamespaceField)(AbckitCoreNamespaceField *field);

    /* ========================================
     * Class Field
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsClassField` to the instance of type `AbckitCoreClassField`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `field`.
     * @param [ in ] field - Class field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreClassField *(*arktsClassFieldToCoreClassField)(AbckitArktsClassField *field);

    /**
     * @brief Convert an instance of type `AbckitCoreClassField` to the instance of type `AbckitArktsClassField`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `field`.
     * @param [ in ] field - Class field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsClassField *(*coreClassFieldToArktsClassField)(AbckitCoreClassField *field);

    /* ========================================
     * Interface Field
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsInterfaceField` to the instance of type
     * `AbckitCoreInterfaceField`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `field`.
     * @param [ in ] field - Interface field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have`ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreInterfaceField *(*arktsInterfaceFieldToCoreInterfaceField)(AbckitArktsInterfaceField *field);

    /**
     * @brief Convert an instance of type `AbckitCoreInterfaceField` to the instance of type
     * `AbckitArktsInterfaceField`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `field`.
     * @param [ in ] field - Interface field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsInterfaceField *(*coreInterfaceFieldToArktsInterfaceField)(AbckitCoreInterfaceField *field);

    /**
     * @brief Returns whether interface field `field` is readonly.
     * @return `true` if field `field` is readonly.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*arktsInterfaceFieldIsReadonly)(AbckitArktsInterfaceField *field);

    /* ========================================
     * Enum Field
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsEnumField` to the instance of type `AbckitCoreEnumField`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `field`.
     * @param [ in ] field - Enum field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreEnumField *(*arktsEnumFieldToCoreEnumField)(AbckitArktsEnumField *field);

    /**
     * @brief Convert an instance of type `AbckitCoreEnumField` to the instance of type `AbckitArktsEnumField`,
     * which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `field`.
     * @param [ in ] field - Enum field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsEnumField *(*coreEnumFieldToArktsEnumField)(AbckitCoreEnumField *field);

    /* ========================================
     * Function
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsFunction` to the instance of type `AbckitCoreFunction`, which can
     * be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `function`.
     * @param [ in ] function - Function to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `function` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreFunction *(*arktsFunctionToCoreFunction)(AbckitArktsFunction *function);

    /**
     * @brief Convert an instance of type `AbckitCoreFunction` to the instance of type `AbckitArktsFunction`, which can
     * be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `function`.
     * @param [ in ] function - Function to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `function` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsFunction *(*coreFunctionToArktsFunction)(AbckitCoreFunction *function);

    /**
     * @brief Check whether the `function` is native.
     * @return `true` if `function` is native, `false` otherwise.
     * @param [ in ] function - Function to inspect.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `function` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*functionIsNative)(AbckitArktsFunction *function);

    /**
     * @brief Check whether the `function` is async.
     * @return `true` if `function` is async, `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `function` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*functionIsAsync)(AbckitArktsFunction *function);

    /**
     * @brief Check whether the `function` is final.
     * @return `true` if `function` is final, `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `function` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*functionIsFinal)(AbckitArktsFunction *function);

    /**
     * @brief Check whether the `function` is abstract.
     * @return `true` if `function` is abstract, `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `function` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*functionIsAbstract)(AbckitArktsFunction *function);

    /* ========================================
     * Annotation
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsAnnotation` to the instance of type `AbckitCoreAnnotation`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `anno`.
     * @param [ in ] anno - Annotation to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `anno` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreAnnotation *(*arktsAnnotationToCoreAnnotation)(AbckitArktsAnnotation *anno);

    /**
     * @brief Convert an instance of type `AbckitCoreAnnotation` to the instance of type `AbckitArktsAnnotation`, which
     * can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `anno`.
     * @param [ in ] anno - Annotation to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `anno` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotation *(*coreAnnotationToArktsAnnotation)(AbckitCoreAnnotation *anno);

    /* ========================================
     * AnnotationElement
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsAnnotationElement` to the instance of type
     * `AbckitCoreAnnotationElement`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `annoElem`.
     * @param [ in ] annoElem - Annotation element to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoElem` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoElem` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreAnnotationElement *(*arktsAnnotationElementToCoreAnnotationElement)(
        AbckitArktsAnnotationElement *annoElem);

    /**
     * @brief Convert an instance of type `AbckitCoreAnnotationElement` to the instance of type
     * `AbckitArktsAnnotationElement`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `annoElem`.
     * @param [ in ] annoElem - Annotation element to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoElem` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoElem` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationElement *(*coreAnnotationElementToArktsAnnotationElement)(
        AbckitCoreAnnotationElement *annoElem);

    /* ========================================
     * AnnotationInterface
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsAnnotationInterface` to the instance of type
     * `AbckitCoreAnnotationInterface`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `annoInterface`.
     * @param [ in ] annoInterface - Annotataion interface to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoInterface` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoInterface` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreAnnotationInterface *(*arktsAnnotationInterfaceToCoreAnnotationInterface)(
        AbckitArktsAnnotationInterface *annoInterface);

    /**
     * @brief Convert an instance of type `AbckitCoreAnnotationInterface` to the instance of type
     * `AbckitArktsAnnotationInterface`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `annoInterface`.
     * @param [ in ] annoInterface - Annotataion interface to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoInterface` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoInterface` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationInterface *(*coreAnnotationInterfaceToArktsAnnotationInterface)(
        AbckitCoreAnnotationInterface *annoInterface);

    /* ========================================
     * AnnotationInterfaceField
     * ======================================== */

    /**
     * @brief Convert an instance of type `AbckitArktsAnnotationInterfaceField` to the instance of type
     * `AbckitCoreAnnotationInterfaceField`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-independent representation of the `annoInterfaceField`.
     * @param [ in ] annoInterfaceField - Annotation inteface field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoInterfaceField` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoInterfaceField` is does not have `ABCKIT_TARGET_ARK_TS_V1`
     * or `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitCoreAnnotationInterfaceField *(*arktsAnnotationInterfaceFieldToCoreAnnotationInterfaceField)(
        AbckitArktsAnnotationInterfaceField *annoInterfaceField);

    /**
     * @brief Convert an instance of type `AbckitCoreAnnotationInterfaceField` to the instance of type
     * `AbckitArktsAnnotationInterfaceField`, which can be used to invoke the corresponding APIs.
     * @return Pointer to the language-dependent representation of the `annoInterfaceField`.
     * @param [ in ] annoInterfaceField - Annotation inteface field to convert.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `annoInterfaceField` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `annoInterfaceField` is does not have `ABCKIT_TARGET_ARK_TS_V1`
     * or `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationInterfaceField *(*coreAnnotationInterfaceFieldToArktsAnnotationInterfaceField)(
        AbckitCoreAnnotationInterfaceField *annoInterfaceField);
};

/**
 * @brief Struct that is used to create new annotation interfaces.
 */
struct AbckitArktsAnnotationInterfaceCreateParams {
    /**
     * @brief Name of the created annotation interface.
     */
    const char *name;
};

/**
 * @brief Struct that is used to create new annotation interface fields.
 */
struct AbckitArktsAnnotationInterfaceFieldCreateParams {
    /**
     * @brief Name of the created annotation interface field.
     */
    const char *name;
    /**
     * @brief Type of the created annotation interface field.
     */
    AbckitType *type;
    /**
     * @brief Default value of the created annotation interface field. Leave as NULL for no default value.
     */
    AbckitValue *defaultValue;
};

/**
 * @brief Struct that is used to create new annotations.
 */
struct AbckitArktsAnnotationCreateParams {
    /**
     * @brief Annotation interface that created annotation instantiates.
     */
    AbckitArktsAnnotationInterface *ai;
};

/**
 * @brief Struct that is used to create new annotation elements.
 */
struct AbckitArktsAnnotationElementCreateParams {
    /**
     * @brief Name of the created annotation element. Must be equal to one of the fields of the corresponding annotation
     * interface.
     */
    const char *name;
    /**
     * @brief Value that should be assigned to the annotation element.
     */
    AbckitValue *value;
};

enum AbckitArktsFieldVisibility {
    PUBLIC,
    PRIVATE,
    PROTECTED,
};

/**
 * @brief Struct that is used to create new field.
 */
struct AbckitArktsFieldCreateParams {
    /**
     * @brief Name of the created field.
     */
    const char *name;
    /**
     * @brief Type of the created field.
     */
    AbckitType *type;
    /**
     * @brief Value of the created field.
     */
    AbckitValue *value;
    /**
     * @brief field is static.
     */
    bool isStatic;
    /**
     * @brief Enumeration access control levels of fields.
     */
    enum AbckitArktsFieldVisibility fieldVisibility;
};

/**
 * @brief Struct that is used to create new field.
 */
struct AbckitArktsInterfaceFieldCreateParams {
    /**
     * @brief Name of the created field.
     */
    const char *name;
    /**
     * @brief Type of the created field.
     */
    AbckitType *type;
    /**
     * @brief field is readonly.
     */
    bool isReadOnly;
    /**
     * @brief Enumeration access control levels of fields.
     */
    enum AbckitArktsFieldVisibility fieldVisibility;
};
/**
 * @brief Struct that is used to create new function parameters.
 */
struct AbckitArktsFunctionParamCreatedParams {
    /**
     * @brief Name of the created function parameter.
     */
    const char *name;
    /**
     * @brief Type of the created function parameter.
     */
    AbckitType *type;
    /**
     * @brief Default value of the created function parameter. Leave as NULL for no default value.
     */
    AbckitValue *defaultValue;
};
/**
 * @brief Struct that is used to create new imports.
 */
struct AbckitArktsImportFromDynamicModuleCreateParams {
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
struct AbckitArktsDynamicModuleExportCreateParams {
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
struct AbckitArktsV1ExternalModuleCreateParams {
    /**
     * @brief Name of the created external module
     */
    const char *name;
};

/**
 * @brief Struct that is used to create new functions.
 */
struct AbckitArktsFunctionCreateParams {
    /**
     * @brief Name of the created function.
     */
    const char *name;
    /**
     * @brief Parameters of the created function.
     */
    AbckitArktsFunctionParam **params;
    /**
     * @brief Return type of the created function.
     */
    AbckitType *returnType;
    /**
     * @brief Whether the created function is async.
     */
    bool isAsync;
};

/**
 * @brief Enum that is used to specify the visibility of a method.
 */
enum ArktsMethodVisibility {
    ABCKIT_ARKTS_METHOD_VISIBILITY_PUBLIC,
    ABCKIT_ARKTS_METHOD_VISIBILITY_PROTECTED,
    ABCKIT_ARKTS_METHOD_VISIBILITY_PRIVATE,
};

/**
 * @brief Struct that is used to create new methods.
 */
struct ArktsMethodCreateParams {
    /**
     * @brief Name of the created method
     */
    const char *name;
    /**
     * @brief Parameters of the created method.
     */
    AbckitArktsFunctionParam **params;
    /**
     * @brief Return type of the created method.
     */
    AbckitType *returnType;
    /**
     * @brief Whether the created method is static.
     */
    bool isStatic;
    /**
     * @brief Whether the created method is async.
     */
    bool isAsync;
    /**
     * @brief Visibility of the created method.
     */
    enum ArktsMethodVisibility methodVisibility;
};

/**
 * @brief Struct that holds the pointers to the modifying API for Arkts-specific Abckit types.
 */
struct CAPI_EXPORT AbckitArktsModifyApi {
    /* ========================================
     * File
     * ======================================== */

    /**
     * @brief Creates an external Arkts module with target `ABCKIT_TARGET_ARK_TS_V1` and adds it to the file `file`.
     * @return Pointer to the newly created module.
     * @param [ in ] file - Binary file to .
     * @param [ in ] params - Data that is used to create the external module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `params` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AbckitArktsModule *(*fileAddExternalModuleArktsV1)(AbckitFile *file,
                                                       const struct AbckitArktsV1ExternalModuleCreateParams *params);

    /**
     * @brief Creates an external Arkts module with target `ABCKIT_TARGET_ARK_TS_V2` and adds it to the file `file`.
     * @return Pointer to the newly created module.
     * @param [ in ] file - Binary file to .
     * @param [ in ] moduleName - The name of the external module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `moduleName` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `file` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Allocates
     */
    AbckitArktsModule *(*fileAddExternalModuleArktsV2)(AbckitFile *file, const char *moduleName);

    /* ========================================
     * Module
     * ======================================== */

    /**
     * @brief Sets name for module `m`.
     * @return `true` on success.
     * @param [ in ] m - Module to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `m` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*moduleSetName)(AbckitArktsModule *m, const char *name);

    /**
     * @brief Adds import from one Arkts module to another Arkts module.
     * @return Pointer to the newly created import descriptor.
     * @param [ in ] importing - Importing module.
     * @param [ in ] imported - The module the `importing` module imports from.
     * @param [ in ] params - Data that is used to create the import.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `importing` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `imported` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `importing` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `imported` is does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AbckitArktsImportDescriptor *(*moduleAddImportFromArktsV1ToArktsV1)(
        AbckitArktsModule *importing, AbckitArktsModule *imported,
        const struct AbckitArktsImportFromDynamicModuleCreateParams *params);
    /**
     * @brief Adds import from one Arkts module to another Arkts module.
     * @return Pointer to the newly created import descriptor.
     * @param [ in ] externalModule - The external module to import the class from.
     * @param [ in ] className - The name of the class to import.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `externalModule` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `className` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `externalModule` is does not have `ABCKIT_TARGET_ARK_TS_V2`
     * target.
     * @note Allocates
     */
    AbckitArktsClass *(*moduleImportClassFromArktsV2toArktsV2)(AbckitArktsModule *externalModule,
                                                               const char *className);

    /**
     * @brief Import static function from external module (e.g., std.math.ETSGLOBAL.abs).
     * @return Pointer to the newly created function.
     * @param [ in ] externalModule - The external module to import the function from.
     * @param [ in ] functionName - The name of the static function to import.
     * @param [ in ] returnType - Return type of the function.
     * @param [ in ] params - Array of parameter type strings.
     * @param [ in ] paramCount - Number of parameters in the params array.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if any parameter is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `externalModule` is does not have `ABCKIT_TARGET_ARK_TS_V2`
     * target.
     * @note Allocates
     */
    AbckitArktsFunction *(*moduleImportStaticFunctionFromArktsV2ToArktsV2)(AbckitArktsModule *externalModule,
                                                                           const char *functionName,
                                                                           const char *returnType,
                                                                           const char *const *params,
                                                                           size_t paramCount);

    /**
     * @brief Import class instance method from external module (e.g., std.core.Console.log).
     * Automatically imports the class if needed and adds class field to module.
     * @return Pointer to the newly created function.
     * @param [ in ] externalModule - The external module to import the method from.
     * @param [ in ] className - The name of the class containing the method.
     * @param [ in ] methodName - The name of the instance method to import.
     * @param [ in ] returnType - Return type of the method.
     * @param [ in ] params - Array of parameter type strings (excluding 'this' parameter).
     * @param [ in ] paramCount - Number of parameters in the params array.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if any parameter is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `externalModule` is does not have `ABCKIT_TARGET_ARK_TS_V2`
     * target.
     * @note Allocates
     */
    AbckitArktsFunction *(*moduleImportClassMethodFromArktsV2ToArktsV2)(AbckitArktsModule *externalModule,
                                                                        const char *className, const char *methodName,
                                                                        const char *returnType,
                                                                        const char *const *params, size_t paramCount);
    /**
     * @brief Removes import `id` from module `m`.
     * @return None.
     * @param [ in ] m - Module to remove the import `id` from.
     * @param [ in ] id - Import to remove from the module `m`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module `m` does not have the import descriptor `id`.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module `m` does not have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    void (*moduleRemoveImport)(AbckitArktsModule *m, AbckitArktsImportDescriptor *id);

    /**
     * @brief Adds export from one Arkts module to another Arkts module.
     * @return Pointer to the newly created export descriptor.
     * @param [ in ] exporting - The module to add export to.
     * @param [ in ] exported - The module the entity is exported from. In case of local export is the same as
     * `exporting`.
     * @param [ in ] params - Data that is used to create the export.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exporting` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `exported` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module `m` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     * @note Allocates
     */
    AbckitArktsExportDescriptor *(*moduleAddExportFromArktsV1ToArktsV1)(
        AbckitArktsModule *exporting, AbckitArktsModule *exported,
        const struct AbckitArktsDynamicModuleExportCreateParams *params);

    /**
     * @brief Removes export `ed` from module `m`.
     * @return None.
     * @param [ in ] m - Module to remove the export `e` from.
     * @param [ in ] ed - Export to remove from the module `m`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module `m` does not have the export descriptor `ed`.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module `m` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    void (*moduleRemoveExport)(AbckitArktsModule *m, AbckitArktsExportDescriptor *ed);

    /**
     * @brief Adds new annotation interface to the module `m`.
     * @return Pointer to the newly constructed annotation interface.
     * @param [ in ] m - Module to add annotation interface to.
     * @param [ in ] params - Data that is used to create the annotation interface.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if module `m` doesn't have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Allocates
     */
    AbckitArktsAnnotationInterface *(*moduleAddAnnotationInterface)(
        AbckitArktsModule *m, const struct AbckitArktsAnnotationInterfaceCreateParams *params);

    /**
     * @brief Create and adds field to the list of fields to the module `m`.
     * @return return Pointer to the created module field.
     * @param [ in ] module - Module to be inspected.
     * @param [ in ] params - param of field to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `module` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Only modified the module record.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `m` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsModuleField *(*moduleAddField)(AbckitArktsModule *m, const struct AbckitArktsFieldCreateParams *params);

    /* ========================================
     * Namespace
     * ======================================== */

    /**
     * @brief Sets name for module `ns`.
     * @return `true` on success.
     * @param [ in ] ns - Namespace to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ns` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ns` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*namespaceSetName)(AbckitArktsNamespace *ns, const char *name);

    /* ========================================
     * Class
     * ======================================== */

    /**
     * @brief Add annotation to the class declaration.
     * @return Pointer to the newly created annotation.
     * @param [ in ] klass - Class to add annotation to.
     * @param [ in ] params - Data that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if class `klass` doesn't have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Allocates
     */
    AbckitArktsAnnotation *(*classAddAnnotation)(AbckitArktsClass *klass,
                                                 const struct AbckitArktsAnnotationCreateParams *params);

    /**
     * @brief Remove annotation from the class declaration.
     * @return None.
     * @param [ in ] klass - Class to remove annotation from.
     * @param [ in ] anno - Annotation to remove.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if class `klass` doesn't have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if class `klass` doesn't have the annotation `anno`.
     */
    void (*classRemoveAnnotation)(AbckitArktsClass *klass, AbckitArktsAnnotation *anno);

    /**
     * @brief Adds interface `iface` to the list of interfaces that class `klass` implements.
     * @return `true` on success.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in ] iface - Interface to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classAddInterface)(AbckitArktsClass *klass, AbckitArktsInterface *iface);

    /**
     * @brief Removes interface `iface` from the list of interfaces that class `klass` implements.
     * @return `true` on success.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in ] iface - Interface to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is not an interface of class `klass`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classRemoveInterface)(AbckitArktsClass *klass, AbckitArktsInterface *iface);

    /**
     * @brief Set super class `superClass` for class `klass`.
     * @return `true` on success.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in ] superClass - Super class to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `superClass` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classSetSuperClass)(AbckitArktsClass *klass, AbckitArktsClass *superClass);

    /**
     * @brief Set name for class `klass`.
     * @return `true` on success.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classSetName)(AbckitArktsClass *klass, const char *name);

    /**
     * @brief Create and adds field to the list of fields to the class `klass`.
     * @return return Pointer to the created class field.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in ] params -  param of field to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Only modified the class record.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    AbckitArktsClassField *(*classAddField)(AbckitArktsClass *klass, const struct AbckitArktsFieldCreateParams *params);

    /**
     * @brief Removes field `field` from the list of fields for class `klass`.
     * @return `true` on success.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in ] field - Field to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is not a field of class `klass`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classRemoveField)(AbckitArktsClass *klass, AbckitArktsClassField *field);

    /**
     * @brief Removes method `method` from the list of methods for class `klass`.
     * @return `true` on success.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in ] method - Method to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `method` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `method` is not a method of class `klass`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classRemoveMethod)(AbckitArktsClass *klass, AbckitArktsFunction *method);

    /**
     * @brief Creates new class with name `name`.
     * @return Pointer to the created class.
     * @param [ in ] name - Name of the class to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `m` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    AbckitArktsClass *(*createClass)(AbckitArktsModule *m, const char *name);

    /**
     * @brief Sets owning module for class `klass`.
     * @return `true` on success.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in ] module - Module to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `module` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classSetOwningModule)(AbckitArktsClass *klass, AbckitArktsModule *module);

    /* ========================================
     * Interface
     * ======================================== */

    /**
     * @brief Adds super interface `iface` to the list of super interfaces for interface `iface`.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in ] superInterface - Super interface to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `superInterface` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceAddSuperInterface)(AbckitArktsInterface *iface, AbckitArktsInterface *superInterface);

    /**
     * @brief Removes super interface `iface` from the list of super interfaces for interface `iface`.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in ] superInterface - Super interface to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `superInterface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `superInterface` is not a super interface of `iface`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceRemoveSuperInterface)(AbckitArktsInterface *iface, AbckitArktsInterface *superInterface);

    /**
     * @brief Sets name for interface `iface`.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceSetName)(AbckitArktsInterface *iface, const char *name);

    /**
     * @brief Create and adds field to the list of fields to the interface `iface`.
     * @return return Pointer to the created interface field.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in ] params - param of field to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    AbckitArktsInterfaceField *(*interfaceAddField)(AbckitArktsInterface *iface,
                                                    const struct AbckitArktsInterfaceFieldCreateParams *params);
    /**
     * @brief Removes field `field` from the list of fields for interface `iface`.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in ] field - Field to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is not a field of interface `iface`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceRemoveField)(AbckitArktsInterface *iface, AbckitArktsInterfaceField *field);

    /**
     * @brief Adds method `method` to the list of methods for interface `iface`.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in ] method - Method to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `method` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceAddMethod)(AbckitArktsInterface *iface, AbckitArktsFunction *method);

    /**
     * @brief Removes method `method` from the list of methods for interface `iface`.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in ] method - Method to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `method` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `method` is not a method of interface `iface`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceRemoveMethod)(AbckitArktsInterface *iface, AbckitArktsFunction *method);

    /**
     * @brief Sets owning module for interface `iface`.
     * @return `true` on success.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in ] module - Module to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `module` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `iface` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceSetOwningModule)(AbckitArktsInterface *iface, AbckitArktsModule *module);

    /**
     * @brief Creates new interface with name `name`.
     * @return Pointer to the created interface.
     * @param [ in ] name - Name of the interface to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `m` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    AbckitArktsInterface *(*createInterface)(AbckitArktsModule *m, const char *name);

    /* ========================================
     * Enum
     * ======================================== */

    /**
     * @brief Sets name for enum `enm`.
     * @return `true` on success.
     * @param [ in ] enm - Enum to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `enm` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*enumSetName)(AbckitArktsEnum *enm, const char *name);

    /**
     * @brief Create and adds field to the list of fields to the enum `enm`.
     * @return return Pointer to the created enum field.
     * @param [ in ] enm - Enum to be inspected.
     * @param [ in ] params - param of field to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Only modified the enum record.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `enm` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    AbckitArktsEnumField *(*enumAddField)(AbckitArktsEnum *enm, const struct AbckitArktsFieldCreateParams *params);

    /* ========================================
     * Module Field
     * ======================================== */

    /**
     * @brief Sets name for module field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*moduleFieldSetName)(AbckitArktsModuleField *field, const char *name);

    /**
     * @brief Sets type for module field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] type - Type to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*moduleFieldSetType)(AbckitArktsModuleField *field, AbckitType *type);

    /**
     * @brief Sets value for module field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] value - Value to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*moduleFieldSetValue)(AbckitArktsModuleField *field, AbckitValue *value);

    /* ========================================
     * Namespace Field
     * ======================================== */

    /**
     * @brief Sets name for namespace field `field`.
     * @return `true` on success.
     * @param [ in ] field - NamespaceField to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*namespaceFieldSetName)(AbckitArktsNamespaceField *field, const char *name);

    /* ========================================
     * Class Field
     * ======================================== */

    /**
     * @brief Adds annotation `anno` to the list of annotations for class field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] params - Data that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classFieldAddAnnotation)(AbckitArktsClassField *field,
                                    const struct AbckitArktsAnnotationCreateParams *params);

    /**
     * @brief Removes annotation `anno` from the list of annotations for class field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] anno - Annotation to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is not an annotation of field `field`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classFieldRemoveAnnotation)(AbckitArktsClassField *field, AbckitArktsAnnotation *anno);

    /**
     * @brief Sets name for class field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classFieldSetName)(AbckitArktsClassField *field, const char *name);

    /**
     * @brief Sets type for class field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] type - Type to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classFieldSetType)(AbckitArktsClassField *field, AbckitType *type);

    /**
     * @brief Sets value for class field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] value - Value to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*classFieldSetValue)(AbckitArktsClassField *field, AbckitValue *value);

    /* ========================================
     * Interface Field
     * ======================================== */

    /**
     * @brief Adds annotation `anno` to the list of annotations for interface field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] params - Data that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceFieldAddAnnotation)(AbckitArktsInterfaceField *field,
                                        const struct AbckitArktsAnnotationCreateParams *params);

    /**
     * @brief Removes annotation `anno` from the list of annotations for interface field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] anno - Annotation to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is not an annotation of field `field`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceFieldRemoveAnnotation)(AbckitArktsInterfaceField *field, AbckitArktsAnnotation *anno);

    /**
     * @brief Sets name for interface field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceFieldSetName)(AbckitArktsInterfaceField *field, const char *name);

    /**
     * @brief Sets type for interface field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] type - Type to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*interfaceFieldSetType)(AbckitArktsInterfaceField *field, AbckitType *type);

    /* ========================================
     * Enum Field
     * ======================================== */

    /**
     * @brief Sets name for enum field `field`.
     * @return `true` on success.
     * @param [ in ] field - Field to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `field` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*enumFieldSetName)(AbckitArktsEnumField *field, const char *name);

    /* ========================================
     * AnnotationInterface
     * ======================================== */

    /**
     * @brief Sets name for annotation interface `ai`.
     * @return `true` on success.
     * @param [ in ] ai - AnnotationInterface to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `ai` is does not have `ABCKIT_TARGET_ARK_TS_V2` target
     */
    bool (*annotationInterfaceSetName)(AbckitArktsAnnotationInterface *ai, const char *name);

    /**
     * @brief Add new field to the annotation interface.
     * @return Pointer to the newly created annotation field.
     * @param [ in ] ai - Annotation interface to add new field to.
     * @param [ in ] params -  Data that is used to create the field of the annotation interface `ai`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation interface `ai` doesn't have `ABCKIT_TARGET_ARK_TS_V1`
     * or `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Allocates
     */
    AbckitArktsAnnotationInterfaceField *(*annotationInterfaceAddField)(
        AbckitArktsAnnotationInterface *ai, const struct AbckitArktsAnnotationInterfaceFieldCreateParams *params);

    /**
     * @brief Remove field from the annotation interface.
     * @return None.
     * @param [ in ] ai - Annotation interface to remove the field `field` from.
     * @param [ in ] field - Field to remove from the annotation interface `ai`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation interface `ai` doesn't have `ABCKIT_TARGET_ARK_TS_V1`
     * or `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if annotation interface `ai` does not have the field `field`.
     */
    void (*annotationInterfaceRemoveField)(AbckitArktsAnnotationInterface *ai,
                                           AbckitArktsAnnotationInterfaceField *field);

    /* ========================================
     * Function
     * ======================================== */

    /**
     * @brief Function to add annotation to.
     * @return Pointer to the newly created annotation.
     * @param [ in ] function - Function to add annotation to.
     * @param [ in ] params -  Data that is used to create the annotation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation interface `ai` doesn't have `ABCKIT_TARGET_ARK_TS_V1`
     * or `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Allocates
     */
    AbckitArktsAnnotation *(*functionAddAnnotation)(AbckitArktsFunction *function,
                                                    const struct AbckitArktsAnnotationCreateParams *params);

    /**
     * @brief Remove annotation from the function.
     * @return None.
     * @param [ in ] function - Function to remove annotation `anno` from.
     * @param [ in ] anno - Annotation to remove from the function `function`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if function `function` doesn't have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if function `function` doesn't have the annotation `anno`.
     */
    void (*functionRemoveAnnotation)(AbckitArktsFunction *function, AbckitArktsAnnotation *anno);

    /**
     * @brief Sets name for function `function`.
     * @return `true` on success.
     * @param [ in ] function - Function to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `function` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*functionSetName)(AbckitArktsFunction *function, const char *name);

    /**
     * @brief Creates a new function parameter with the specified parameters.
     * @return Pointer to the newly created function parameter.
     * @param [ in ] file - Binary file context for string creation.
     * @param [ in ] params - Data that is used to create the function parameter.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params->name` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params->type` is NULL.
     * @note Allocates
     */
    AbckitArktsFunctionParam *(*createFunctionParameter)(AbckitFile *file,
                                                         const struct AbckitArktsFunctionParamCreatedParams *params);

    /**
     * @brief Adds parameter `param` to the list of parameters for function `func`.
     * @return `true` on success.
     * @param [ in ] func - Function to be inspected.
     * @param [ in ] param - Parameter to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `param` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `func` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*functionAddParameter)(AbckitArktsFunction *func, AbckitArktsFunctionParam *param);

    /**
     * @brief Removes parameter `param` from the list of parameters for function `func`.
     * @return `true` on success.
     * @param [ in ] func - Function to be inspected.
     * @param [ in ] param - Parameter to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `param` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `param` is not a parameter of function `func`.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `func` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*functionRemoveParameter)(AbckitArktsFunction *func, size_t index);

    /**
     * @brief Sets return type for function `func`.
     * @return `true` on success.
     * @param [ in ] func - Function to be inspected.
     * @param [ in ] type - Type to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `func` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*functionSetReturnType)(AbckitArktsFunction *func, AbckitType *type);

    /**
     * @brief Sets return type for function `func`.
     * @return `true` on success.
     * @param [ in ] func - Function to be inspected.
     * @param [ in ] type - Type to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `module` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsFunction *(*moduleAddFunction)(AbckitArktsModule *module,
                                              struct AbckitArktsFunctionCreateParams *params);

    /**
     * @brief Creates new method with name `name`, return type `returnType`, isStatic `isStatic`, isAsync `isAsync` and
     * method visibility `methodVisibility` for class `klass`.
     * @return Pointer to the created method.
     * @param [ in ] klass - Class to be modified.
     * @param [ in ] params - Data that is used to create the method.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `klass` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsFunction *(*classAddMethod)(AbckitArktsClass *klass, struct ArktsMethodCreateParams *params);

    /* ========================================
     * Annotation
     * ======================================== */

    /**
     * @brief Sets name for annotation `anno`.
     * @return `true` on success.
     * @param [ in ] anno - AnnotationInterface to be inspected.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `anno` is does not have `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    bool (*annotationSetName)(AbckitArktsAnnotation *anno, const char *name);

    /**
     * @brief Add annotation element to the existing annotation.
     * @return Pointer to the newly created annotation element.
     * @param [ in ] anno - Annotation to add new element to.
     * @param [ in ] params -  Data that is used to create the annotation element.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `params` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation `anno` doesn't have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Allocates
     */
    AbckitArktsAnnotationElement *(*annotationAddAnnotationElement)(
        AbckitArktsAnnotation *anno, struct AbckitArktsAnnotationElementCreateParams *params);

    /**
     * @brief Remove annotation element `elem` from the annotation `anno`.
     * @return None.
     * @param [ in ] anno - Annotation to remove the annotataion element `elem` from.
     * @param [ in ] elem - Annotation element to remove from the annotation `anno`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `elem` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if annotation `anno` doesn't have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if annotation `anno` doesn't have the annotation element `elem`.
     */
    void (*annotationRemoveAnnotationElement)(AbckitArktsAnnotation *anno, AbckitArktsAnnotationElement *elem);

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
 * @brief Instantiates non-modifying API for Arkts-specific Abckit types.
 * @return Instance of the `AbckitApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitArktsInspectApi const *AbckitGetArktsInspectApiImpl(enum AbckitApiVersion version);

/**
 * @brief Instantiates modifying API for Arkts-specific Abckit types.
 * @return Instance of the `AbckitApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitArktsModifyApi const *AbckitGetArktsModifyApiImpl(enum AbckitApiVersion version);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBABCKIT_METADATA_ARKTS_H */
