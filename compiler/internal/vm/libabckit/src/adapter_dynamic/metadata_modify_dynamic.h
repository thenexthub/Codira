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

#ifndef LIBABCKIT_SRC_ADAPTER_DYNAMIC_METADATA_MODIFY_DYNAMIC_H
#define LIBABCKIT_SRC_ADAPTER_DYNAMIC_METADATA_MODIFY_DYNAMIC_H

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/extensions/js/metadata_js.h"

namespace libabckit {

AbckitString *CreateStringDynamic(AbckitFile *file, const char *value, size_t len);
void FunctionSetGraphDynamic(AbckitCoreFunction *function, AbckitGraph *graph);
void FunctionSetGraphDynamicSync(AbckitCoreFunction *function, AbckitGraph *graph);
AbckitJsModule *FileAddExternalJsModule(AbckitFile *file, const struct AbckitJsExternalModuleCreateParams *params);
AbckitArktsModule *FileAddExternalArkTsV1Module(AbckitFile *file,
                                                const struct AbckitArktsV1ExternalModuleCreateParams *params);
void ModuleRemoveImportDynamic(AbckitCoreModule *m, AbckitArktsImportDescriptor *i);
void ModuleRemoveImportDynamic(AbckitCoreModule *m, AbckitJsImportDescriptor *i);
AbckitArktsImportDescriptor *ModuleAddImportFromDynamicModuleDynamic(
    AbckitCoreModule *importing, AbckitCoreModule *imported,
    const AbckitArktsImportFromDynamicModuleCreateParams *params);
AbckitJsImportDescriptor *ModuleAddImportFromDynamicModuleDynamic(
    AbckitCoreModule *importing, AbckitCoreModule *imported, const AbckitJsImportFromDynamicModuleCreateParams *params);
AbckitArktsExportDescriptor *DynamicModuleAddExportDynamic(AbckitCoreModule *exporting, AbckitCoreModule *exported,
                                                           const AbckitArktsDynamicModuleExportCreateParams *params);
AbckitJsExportDescriptor *DynamicModuleAddExportDynamic(AbckitCoreModule *exporting, AbckitCoreModule *exported,
                                                        const AbckitJsDynamicModuleExportCreateParams *params);
void ModuleRemoveExportDynamic(AbckitCoreModule *m, AbckitArktsExportDescriptor *i);
void ModuleRemoveExportDynamic(AbckitCoreModule *m, AbckitJsExportDescriptor *i);
AbckitLiteral *FindOrCreateLiteralBoolDynamic(AbckitFile *file, bool value);
AbckitLiteral *FindOrCreateLiteralU8Dynamic(AbckitFile *file, uint8_t value);
AbckitLiteral *FindOrCreateLiteralU16Dynamic(AbckitFile *file, uint16_t value);
AbckitLiteral *FindOrCreateLiteralMethodAffiliateDynamic(AbckitFile *file, uint16_t value);
AbckitLiteral *FindOrCreateLiteralU32Dynamic(AbckitFile *file, uint32_t value);
AbckitLiteral *FindOrCreateLiteralU64Dynamic(AbckitFile *file, uint64_t value);
AbckitLiteral *FindOrCreateLiteralFloatDynamic(AbckitFile *file, float value);
AbckitLiteral *FindOrCreateLiteralDoubleDynamic(AbckitFile *file, double value);
AbckitLiteral *FindOrCreateLiteralStringDynamic(AbckitFile *file, const char *value, size_t len);
AbckitLiteral *FindOrCreateLiteralMethodDynamic(AbckitFile *file, AbckitCoreFunction *function);
AbckitLiteral *FindOrCreateLiteralLiteralArrayDynamic(AbckitFile *file, AbckitLiteralArray *litarr);
AbckitLiteral *FindOrCreateLiteralNullValueDynamic(AbckitFile *file);
AbckitLiteralArray *CreateLiteralArrayDynamic(AbckitFile *file, AbckitLiteral **value, size_t size);

AbckitValue *FindOrCreateValueU1Dynamic(AbckitFile *file, bool value);
AbckitValue *FindOrCreateValueIntDynamic(AbckitFile *file, int value);
AbckitValue *FindOrCreateValueDoubleDynamic(AbckitFile *file, double value);
AbckitValue *FindOrCreateValueStringDynamic(AbckitFile *file, const char *value, size_t len);
AbckitValue *FindOrCreateLiteralArrayValueDynamic(AbckitFile *file, AbckitValue **value, size_t size);

AbckitArktsAnnotationInterface *ModuleAddAnnotationInterfaceDynamic(
    AbckitCoreModule *m, const AbckitArktsAnnotationInterfaceCreateParams *params);

AbckitArktsAnnotation *ClassAddAnnotationDynamic(AbckitCoreClass *klass,
                                                 const AbckitArktsAnnotationCreateParams *params);
void ClassRemoveAnnotationDynamic(AbckitCoreClass *klass, AbckitCoreAnnotation *anno);
AbckitArktsAnnotation *FunctionAddAnnotationDynamic(AbckitCoreFunction *function,
                                                    const AbckitArktsAnnotationCreateParams *params);
void FunctionRemoveAnnotationDynamic(AbckitCoreFunction *function, AbckitCoreAnnotation *anno);
AbckitArktsAnnotationElement *AnnotationAddAnnotationElementDynamic(AbckitCoreAnnotation *anno,
                                                                    AbckitArktsAnnotationElementCreateParams *params);
void AnnotationRemoveAnnotationElementDynamic(AbckitCoreAnnotation *anno, AbckitCoreAnnotationElement *elem);
AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddFieldDynamic(
    AbckitCoreAnnotationInterface *ai, const AbckitArktsAnnotationInterfaceFieldCreateParams *params);
void AnnotationInterfaceRemoveFieldDynamic(AbckitCoreAnnotationInterface *ai,
                                           AbckitCoreAnnotationInterfaceField *field);
}  // namespace libabckit

#endif
