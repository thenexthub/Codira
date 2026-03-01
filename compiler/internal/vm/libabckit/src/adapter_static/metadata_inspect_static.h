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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_METADATA_INSPECT_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_METADATA_INSPECT_STATIC_H

#include "libabckit/c/metadata_core.h"

namespace libabckit {

// ========================================
// Module
// ========================================

bool ModuleEnumerateAnonymousFunctionsStatic(AbckitCoreModule *m, void *data,
                                             bool (*cb)(AbckitCoreFunction *function, void *data));

// ========================================
// Namespace
// ========================================

AbckitString *NamespaceGetNameStatic(AbckitCoreNamespace *ns);

bool NamespaceIsExternalStatic(AbckitCoreNamespace *ns);

// ========================================
// Class
// ========================================

AbckitString *ClassGetNameStatic(AbckitCoreClass *klass);

bool ClassIsExternalStatic(AbckitCoreClass *klass);

bool ClassIsFinalStatic(AbckitCoreClass *klass);

bool ClassIsAbstractStatic(AbckitCoreClass *klass);

// ========================================
// Interface
// ========================================

AbckitString *InterfaceGetNameStatic(AbckitCoreInterface *iface);

bool InterfaceIsExternalStatic(AbckitCoreInterface *iface);

// ========================================
// Enum
// ========================================

AbckitString *EnumGetNameStatic(AbckitCoreEnum *enm);

bool EnumIsExternalStatic(AbckitCoreEnum *enm);

// ========================================
// Field
// ========================================

AbckitString *ModuleFieldGetNameStatic(AbckitCoreModuleField *field);
AbckitString *NamespaceFieldGetNameStatic(AbckitCoreNamespaceField *field);
AbckitString *ClassFieldGetNameStatic(AbckitCoreClassField *field);
AbckitString *EnumFieldGetNameStatic(AbckitCoreEnumField *field);
bool ClassFieldIsPublicStatic(AbckitCoreClassField *field);
bool ClassFieldIsProtectedStatic(AbckitCoreClassField *field);
bool ClassFieldIsPrivateStatic(AbckitCoreClassField *field);
bool ClassFieldIsInternalStatic(AbckitCoreClassField *field);
bool ClassFieldIsStaticStatic(AbckitCoreClassField *field);
bool ClassFieldIsFinalStatic(AbckitCoreClassField *field);
bool InterfaceFieldIsReadonlyStatic(AbckitCoreInterfaceField *field);

// ========================================
// Function
// ========================================

AbckitString *FunctionGetNameStatic(AbckitCoreFunction *function);
AbckitGraph *CreateGraphFromFunctionStatic(AbckitCoreFunction *function);
bool FunctionIsStaticStatic(AbckitCoreFunction *function);
bool FunctionIsCtorStatic(AbckitCoreFunction *function);
bool FunctionIsCctorStatic(AbckitCoreFunction *function);
bool FunctionIsAnonymousStatic(AbckitCoreFunction *function);
bool FunctionIsNativeStatic(AbckitCoreFunction *function);
bool FunctionIsPublicStatic(AbckitCoreFunction *function);
bool FunctionIsProtectedStatic(AbckitCoreFunction *function);
bool FunctionIsPrivateStatic(AbckitCoreFunction *function);
bool FunctionIsInternalStatic(AbckitCoreFunction *function);
bool FunctionIsExternalStatic(AbckitCoreFunction *function);
bool FunctionIsAbstractStatic(AbckitCoreFunction *function);
bool FunctionIsFinalStatic(AbckitCoreFunction *function);
bool FunctionIsOverrideStatic(AbckitCoreFunction *function);
bool FunctionIsAsyncStatic(AbckitCoreFunction *function);
AbckitType *FunctionGetReturnTypeStatic(AbckitCoreFunction *function);

// ========================================
// Annotation
// ========================================

// ========================================
// AnnotationInterface
// ========================================

AbckitString *AnnotationInterfaceGetNameStatic(AbckitCoreAnnotationInterface *ai);

// ========================================
// String
// ========================================

void StringToStringStatic(AbckitFile *file, AbckitString *value, char *out, size_t *len);

// ========================================
// ImportDescriptor
// ========================================

// ========================================
// ImportDescriptor
// ========================================

// ========================================
// Literal
// ========================================

bool LiteralGetBoolStatic(AbckitLiteral *lit);
uint8_t LiteralGetU8Static(AbckitLiteral *lit);
uint16_t LiteralGetU16Static(AbckitLiteral *lit);
uint16_t LiteralGetMethodAffiliateStatic(AbckitLiteral *lit);
uint32_t LiteralGetU32Static(AbckitLiteral *lit);
uint64_t LiteralGetU64Static(AbckitLiteral *lit);
float LiteralGetFloatStatic(AbckitLiteral *lit);
double LiteralGetDoubleStatic(AbckitLiteral *lit);
AbckitString *LiteralGetStringStatic(AbckitLiteral *lit);
AbckitLiteralTag LiteralGetTagStatic(AbckitLiteral *lit);

// ========================================
// Type
// ========================================
bool TypeIsUnionStatic(AbckitType *type);
bool TypeEnumerateUnionTypesStatic(AbckitType *type, void *data, bool (*cb)(AbckitType *type, void *data));

// ========================================
// Value
// ========================================

AbckitType *ValueGetTypeStatic(AbckitValue *value);
bool ValueGetU1Static(AbckitValue *value);
int ValueGetIntStatic(AbckitValue *value);
double ValueGetDoubleStatic(AbckitValue *value);
AbckitString *ValueGetStringStatic(AbckitValue *value);
AbckitLiteralArray *ArrayValueGetLiteralArrayStatic(AbckitValue *value);

}  // namespace libabckit

#endif
