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
#ifndef LIBABCKIT_TESTS_INVALID_HELPERS
#define LIBABCKIT_TESTS_INVALID_HELPERS

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/extensions/js/metadata_js.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/c/ir_core.h"

namespace libabckit::test::helpers_nullptr {

void TestNullptr(void (*apiToCheck)(AbckitFile *));
void TestNullptr(void (*apiToCheck)(AbckitGraph *));
void TestNullptr(AbckitFile *(*apiToCheck)(const char *, size_t));
void TestNullptr(void (*apiToCheck)(AbckitFile *, const char *, size_t));
void TestNullptr(AbckitCoreAnnotationElement *(*apiToCheck)(AbckitArktsAnnotationElement *));
void TestNullptr(AbckitCoreAnnotationInterfaceField *(*apiToCheck)(AbckitArktsAnnotationInterfaceField *));
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitArktsAnnotationInterface *));
void TestNullptr(AbckitCoreAnnotation *(*apiToCheck)(AbckitArktsAnnotation *));
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitArktsClass *));
void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitArktsExportDescriptor *));
void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitArktsImportDescriptor *));
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitArktsFunction *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitArktsModule *));
void TestNullptr(AbckitArktsAnnotationElement *(*apiToCheck)(AbckitCoreAnnotationElement *));
void TestNullptr(AbckitArktsAnnotationInterfaceField *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *));
void TestNullptr(AbckitArktsAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotationInterface *));
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitCoreAnnotation *));
void TestNullptr(AbckitArktsClass *(*apiToCheck)(AbckitCoreClass *));
void TestNullptr(AbckitArktsExportDescriptor *(*apiToCheck)(AbckitCoreExportDescriptor *));
void TestNullptr(AbckitArktsImportDescriptor *(*apiToCheck)(AbckitCoreImportDescriptor *));
void TestNullptr(AbckitArktsFunction *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitArktsModule *(*apiToCheck)(AbckitCoreModule *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsFunction *));
void TestNullptr(AbckitArktsAnnotationElement *(*apiToCheck)(AbckitArktsAnnotation *,
                                                             AbckitArktsAnnotationElementCreateParams *));
void TestNullptr(AbckitArktsAnnotationInterfaceField *(*apiToCheck)(
    AbckitArktsAnnotationInterface *, const AbckitArktsAnnotationInterfaceFieldCreateParams *));
void TestNullptr(void (*apiToCheck)(AbckitArktsAnnotationInterface *, AbckitArktsAnnotationInterfaceField *));
void TestNullptr(void (*apiToCheck)(AbckitArktsAnnotation *, AbckitArktsAnnotationElement *));
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitArktsClass *, const AbckitArktsAnnotationCreateParams *));
void TestNullptr(void (*apiToCheck)(AbckitArktsClass *, AbckitArktsAnnotation *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, const char *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsClassField *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsModule *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, AbckitArktsInterface *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, AbckitArktsFunction *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, AbckitArktsModule *));
void TestNullptr(AbckitArktsAnnotation *(*apiToCheck)(AbckitArktsFunction *,
                                                      const AbckitArktsAnnotationCreateParams *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsFunction *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, const char *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterface *, AbckitArktsInterfaceField *));
void TestNullptr(void (*apiToCheck)(AbckitArktsFunction *, AbckitArktsAnnotation *));
void TestNullptr(AbckitArktsAnnotationInterface *(*apiToCheck)(AbckitArktsModule *,
                                                               const AbckitArktsAnnotationInterfaceCreateParams *));
void TestNullptr(AbckitArktsExportDescriptor *(*apiToCheck)(AbckitArktsModule *, AbckitArktsModule *,
                                                            const AbckitArktsDynamicModuleExportCreateParams *));
void TestNullptr(AbckitArktsImportDescriptor *(*apiToCheck)(AbckitArktsModule *, AbckitArktsModule *,
                                                            const AbckitArktsImportFromDynamicModuleCreateParams *));
void TestNullptr(void (*apiToCheck)(AbckitArktsModule *, AbckitArktsExportDescriptor *));
void TestNullptr(void (*apiToCheck)(AbckitArktsModule *, AbckitArktsImportDescriptor *));
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitInst *));
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *, AbckitInst *, bool));
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *));
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *));
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *));
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitGraph *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitBasicBlock *, size_t, ...));
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, uint32_t));
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, int32_t));
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitBasicBlock *));
void TestNullptr(AbckitGraph *(*apiToCheck)(AbckitBasicBlock *));
void TestNullptr(uint32_t (*apiToCheck)(AbckitBasicBlock *));
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitBasicBlock *, uint32_t));
void TestNullptr(uint64_t (*apiToCheck)(AbckitBasicBlock *));
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *, uint32_t));
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *));
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitInst *, bool));
void TestNullptr(bool (*apiToCheck)(AbckitBasicBlock *, void *, bool (*cb)(AbckitBasicBlock *, void *)));
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, void *, void (*cb)(AbckitBasicBlock *, void *)));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, double));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, int32_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, int64_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t));
void TestNullptr(void (*apiToCheck)(AbckitGraph *, int32_t));
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitGraph *, uint32_t));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitGraph *));
void TestNullptr(uint32_t (*apiToCheck)(AbckitGraph *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint32_t));
void TestNullptr(void (*apiToCheck)(AbckitBasicBlock *, AbckitBasicBlock *, AbckitBasicBlock *, AbckitBasicBlock *));
void TestNullptr(AbckitIsaType (*apiToCheck)(AbckitGraph *));
void TestNullptr(bool (*apiToCheck)(AbckitGraph *, void *, bool (*cb)(AbckitBasicBlock *, void *)));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitInst *));
void TestNullptr(bool (*apiToCheck)(AbckitInst *, AbckitInst *));
void TestNullptr(bool (*apiToCheck)(AbckitInst *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, int32_t));
void TestNullptr(AbckitBasicBlock *(*apiToCheck)(AbckitInst *));
void TestNullptr(double (*apiToCheck)(AbckitInst *));
void TestNullptr(int64_t (*apiToCheck)(AbckitInst *));
void TestNullptr(int32_t (*apiToCheck)(AbckitInst *));
void TestNullptr(uint64_t (*apiToCheck)(AbckitInst *));
void TestNullptr(uint32_t (*apiToCheck)(AbckitInst *));
void TestNullptr(uint64_t (*apiToCheck)(AbckitInst *, size_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitInst *, uint32_t));
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitInst *));
void TestNullptr(void (*apiToCheck)(AbckitInst *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, size_t, uint64_t));
void TestNullptr(AbckitBitImmSize (*apiToCheck)(AbckitInst *, size_t));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitInst *, uint32_t));
void TestNullptr(void (*apiToCheck)(AbckitInst *, size_t, ...));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitLiteralArray *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreFunction *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitString *));
void TestNullptr(bool (*apiToCheck)(AbckitInst *, void *, bool (*cb)(AbckitInst *, size_t, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitInst *, void *, bool (*cb)(AbckitInst *, void *)));
void TestNullptr(AbckitCoreAnnotation *(*apiToCheck)(AbckitCoreAnnotationElement *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationElement *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationElement *));
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitCoreAnnotationElement *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreAnnotation *, void *, bool (*cb)(AbckitCoreAnnotationElement *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreAnnotation *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotation *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotation *));
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotation *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreAnnotationInterface *, void *,
                                    bool (*cb)(AbckitCoreAnnotationInterfaceField *, void *)));
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *));
void TestNullptr(AbckitCoreAnnotationInterface *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreAnnotationInterfaceField *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreAnnotationInterface *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreAnnotationInterface *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreAnnotationInterface *));
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitValue *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreFunction *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreClassField *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreClass *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *, void *, bool (*cb)(AbckitCoreInterface *, void *)));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreClass *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreClass *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreClass *));
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitCoreClass *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreClass *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreExportDescriptor *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreExportDescriptor *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreExportDescriptor *));
void TestNullptr(void (*apiToCheck)(AbckitFile *, void *, bool (*cb)(AbckitCoreModule *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitFile *, void *, bool (*cb)(AbckitCoreModule *, void *)));
void TestNullptr(AbckitFileVersion (*apiToCheck)(AbckitFile *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreImportDescriptor *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreImportDescriptor *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreImportDescriptor *));
void TestNullptr(bool (*apiToCheck)(AbckitLiteralArray *, void *, bool (*cb)(AbckitFile *, AbckitLiteral *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitLiteral *));
void TestNullptr(double (*apiToCheck)(AbckitLiteral *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitLiteral *));
void TestNullptr(float (*apiToCheck)(AbckitLiteral *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitLiteral *));
void TestNullptr(AbckitLiteralTag (*apiToCheck)(AbckitLiteral *));
void TestNullptr(uint16_t (*apiToCheck)(AbckitLiteral *));
void TestNullptr(uint32_t (*apiToCheck)(AbckitLiteral *));
void TestNullptr(uint64_t (*apiToCheck)(AbckitLiteral *));
void TestNullptr(uint8_t (*apiToCheck)(AbckitLiteral *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *, bool (*cb)(AbckitCoreFunctionParam *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *, bool (*cb)(AbckitCoreClass *, void *)));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitGraph *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreFunctionParam *));
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitCoreClass *));
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreAnnotationInterface *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreFunction *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreModuleField *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreClass *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreInterface *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreEnum *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreExportDescriptor *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreImportDescriptor *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *, void *, bool (*cb)(AbckitCoreNamespace *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitArktsModuleField *, const char *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsModuleField *, AbckitType *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsModuleField *, AbckitValue *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, const struct AbckitArktsAnnotationCreateParams *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, AbckitArktsAnnotation *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, const char *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, AbckitType *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClassField *, AbckitValue *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterfaceField *, const struct AbckitArktsAnnotationCreateParams *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterfaceField *, AbckitArktsAnnotation *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterfaceField *, const char *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsInterfaceField *, AbckitType *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsEnumField *, const char *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreModule *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreModule *));
void TestNullptr(AbckitTarget (*apiToCheck)(AbckitCoreModule *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreModule *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreInterface *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreInterface *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreInterface *));
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreInterface *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreFunction *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreInterfaceField *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreInterface *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreClass *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterface *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreEnum *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreEnum *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitCoreEnum *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreEnum *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreEnum *, void *, bool (*cb)(AbckitCoreFunction *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreEnum *, void *, bool (*cb)(AbckitCoreEnumField *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreClass *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreInterface *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreEnum *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreNamespaceField *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreNamespace *, void *)));
void TestNullptr(bool (*apiToCheck)(AbckitCoreNamespace *, void *, bool (*cb)(AbckitCoreFunction *, void *)));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitCoreModuleField *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreModuleField *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreModuleField *));
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitCoreClassField *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreClassField *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreClassField *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreClassField *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreClassField *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)));
void TestNullptr(AbckitCoreInterface *(*apiToCheck)(AbckitCoreInterfaceField *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreInterfaceField *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreInterfaceField *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterfaceField *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreInterfaceField *, void *, bool (*cb)(AbckitCoreAnnotation *, void *)));
void TestNullptr(AbckitCoreEnum *(*apiToCheck)(AbckitCoreEnumField *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreEnumField *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreEnumField *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreNamespace *));
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitType *));
void TestNullptr(AbckitTypeId (*apiToCheck)(AbckitType *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitType *));
void TestNullptr(size_t (*apiToCheck)(AbckitType *));
void TestNullptr(bool (*apiToCheck)(AbckitType *));
void TestNullptr(bool (*apiToCheck)(AbckitType *, void *, bool (*cb)(AbckitType *, void *)));
void TestNullptr(double (*apiToCheck)(AbckitValue *));
void TestNullptr(AbckitFile *(*apiToCheck)(AbckitValue *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitValue *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitValue *));
void TestNullptr(bool (*apiToCheck)(AbckitValue *));
void TestNullptr(const char *(*apiToCheck)(AbckitString *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, size_t, ...));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, uint64_t, ...));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, AbckitLiteralArray *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, uint64_t, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, AbckitLiteralArray *, uint64_t,
                                           AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, uint64_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t, uint64_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitInst *,
                                           AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitBasicBlock *, size_t, ...));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitLiteralArray *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, uint64_t, AbckitInst *, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitString *, uint64_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitString *, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, uint64_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreFunction *, uint64_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreModule *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitIsaApiDynamicConditionCode));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitString *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreImportDescriptor *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreExportDescriptor *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, uint64_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitString *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, size_t, ...));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreExportDescriptor *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, uint64_t));
void TestNullptr(AbckitIsaApiDynamicConditionCode (*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitIsaApiDynamicOpcode (*apiToCheck)(AbckitInst *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitIsaApiDynamicConditionCode));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreExportDescriptor *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreImportDescriptor *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreModule *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreFunction *, size_t, ...));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitCoreFunction *, size_t, ...));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitTypeId));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitType *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitIsaApiStaticConditionCode));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitTypeId));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreClass *, AbckitInst *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitCoreClass *));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *, AbckitTypeId));
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitIsaApiStaticConditionCode (*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitIsaApiStaticOpcode (*apiToCheck)(AbckitInst *));
void TestNullptr(AbckitTypeId (*apiToCheck)(AbckitInst *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitCoreClass *));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitIsaApiStaticConditionCode));
void TestNullptr(void (*apiToCheck)(AbckitInst *, AbckitTypeId));
void TestNullptr(AbckitJsClass *(*apiToCheck)(AbckitCoreClass *));
void TestNullptr(AbckitJsFunction *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitJsModule *(*apiToCheck)(AbckitCoreModule *));
void TestNullptr(AbckitCoreClass *(*apiToCheck)(AbckitJsClass *));
void TestNullptr(AbckitCoreFunction *(*apiToCheck)(AbckitJsFunction *));
void TestNullptr(AbckitCoreModule *(*apiToCheck)(AbckitJsModule *));
void TestNullptr(AbckitJsImportDescriptor *(*apiToCheck)(AbckitJsModule *, AbckitJsModule *,
                                                         const AbckitJsImportFromDynamicModuleCreateParams *));
void TestNullptr(void (*apiToCheck)(AbckitJsModule *, AbckitJsExportDescriptor *));
void TestNullptr(void (*apiToCheck)(AbckitJsModule *, AbckitJsImportDescriptor *));
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitFile *, AbckitLiteral **, size_t));
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, AbckitValue **, size_t));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, bool));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, double));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, float));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, const char *, size_t));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint16_t));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint32_t));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint64_t));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, uint8_t));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, AbckitCoreFunction *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitFile *, AbckitCoreClass *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitFile *, const char *, size_t));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitFile *, AbckitTypeId));
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, double));
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, const char *, size_t));
void TestNullptr(AbckitValue *(*apiToCheck)(AbckitFile *, bool));
void TestNullptr(AbckitArktsModule *(*apiToCheck)(AbckitFile *, const AbckitArktsV1ExternalModuleCreateParams *));
void TestNullptr(AbckitJsModule *(*apiToCheck)(AbckitFile *, const AbckitJsExternalModuleCreateParams *));
void TestNullptr(void (*apiToCheck)(AbckitCoreFunction *, AbckitGraph *));
void TestNullptr(AbckitJsExportDescriptor *(*apiToCheck)(AbckitJsModule *, AbckitJsModule *,
                                                         const AbckitJsDynamicModuleExportCreateParams *));
void TestNullptr(AbckitArktsNamespace *(*apiToCheck)(AbckitCoreNamespace *));
void TestNullptr(AbckitArktsFunction *(*apiToCheck)(AbckitArktsNamespace *));
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitArktsNamespace *));
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreNamespace *));
void TestNullptr(AbckitLiteralArray *(*apiToCheck)(AbckitLiteral *));
void TestNullptr(bool (*apiToCheck)(AbckitCoreFunction *, void *,
                                    bool (*cb)(AbckitCoreFunction *nestedFunc, void *data)));
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreFunction *));
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreClass *));
void TestNullptr(AbckitJsExportDescriptor *(*apiToCheck)(AbckitCoreExportDescriptor *));
void TestNullptr(AbckitCoreImportDescriptor *(*apiToCheck)(AbckitJsImportDescriptor *));
void TestNullptr(AbckitCoreExportDescriptor *(*apiToCheck)(AbckitJsExportDescriptor *));
void TestNullptr(AbckitJsImportDescriptor *(*apiToCheck)(AbckitCoreImportDescriptor *));
void TestNullptr(AbckitLiteral *(*apiToCheck)(AbckitFile *, AbckitLiteralArray *));
void TestNullptr(const AbckitArktsModifyApi *(*apiToCheck)(AbckitApiVersion version));
void TestNullptr(const AbckitArktsInspectApi *(*apiToCheck)(AbckitApiVersion version));
void TestNullptr(const AbckitModifyApi *(*apiToCheck)(AbckitApiVersion version));
void TestNullptr(AbckitCoreNamespace *(*apiToCheck)(AbckitCoreNamespaceField *));
void TestNullptr(AbckitString *(*apiToCheck)(AbckitCoreNamespaceField *));
void TestNullptr(AbckitType *(*apiToCheck)(AbckitCoreNamespaceField *));
void TestNullptr(const AbckitInspectApi *(*apiToCheck)(AbckitApiVersion version));
void TestNullptr(const AbckitApi *(*apiToCheck)(AbckitApiVersion version));
void TestNullptr(const AbckitIsaApiDynamic *(*apiToCheck)(AbckitApiVersion version));
void TestNullptr(AbckitArktsClass *(*apiToCheck)(AbckitArktsModule *, const char *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsInterface *));
void TestNullptr(bool (*apiToCheck)(AbckitArktsClass *, AbckitArktsClass *));
void TestNullptr(AbckitArktsInterface *(*apiToCheck)(AbckitArktsModule *, const char *));
void TestNullptr(AbckitArktsModuleField *(*apiToCheck)(AbckitArktsModule *,
                                                       const struct AbckitArktsFieldCreateParams *));
void TestNullptr(AbckitArktsClassField *(*apiToCheck)(AbckitArktsClass *, const struct AbckitArktsFieldCreateParams *));
void TestNullptr(AbckitArktsInterfaceField *(*apiToCheck)(AbckitArktsInterface *,
                                                          const struct AbckitArktsInterfaceFieldCreateParams *));
void TestNullptr(AbckitArktsEnumField *(*apiToCheck)(AbckitArktsEnum *, const struct AbckitArktsFieldCreateParams *));
void TestNullptr(void (*apiToCheck)(AbckitType *, const char *, size_t));
void TestNullptr(void (*apiToCheck)(AbckitType *, size_t));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *, AbckitInst *inputObj, AbckitString *field, AbckitTypeId));
void TestNullptr(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId,
                                           AbckitInst *value, AbckitTypeId typeId));

}  // namespace libabckit::test::helpers_nullptr

#endif /*LIBABCKIT_TESTS_INVALID_HELPERS */
