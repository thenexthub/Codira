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

#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CAPI_EXPORT
#ifdef PANDA_TARGET_WINDOWS
#define CAPI_EXPORT __declspec(dllexport)
#else
#define CAPI_EXPORT __attribute__((visibility("default")))
#endif
#endif

struct AbckitFile;
struct AbckitString;
struct AbckitType;
struct AbckitValue;
struct AbckitLiteral;
struct AbckitLiteralArray;

struct AbckitCoreModule;
struct AbckitCoreNamespace;
struct AbckitCoreClass;
struct AbckitCoreFunction;
struct AbckitCoreAnnotation;
struct AbckitCoreAnnotationElement;
struct AbckitCoreAnnotationInterface;
struct AbckitCoreAnnotationInterfaceField;
struct AbckitCoreImportDescriptor;
struct AbckitCoreExportDescriptor;
struct AbckitCoreInterface;
struct AbckitCoreEnum;
struct AbckitCoreModuleField;
struct AbckitCoreNamespaceField;
struct AbckitCoreClassField;
struct AbckitCoreInterfaceField;
struct AbckitCoreEnumField;
struct AbckitCoreFunctionParam;

struct AbckitArktsModule;
struct AbckitArktsNamespace;
struct AbckitArktsClass;
struct AbckitArktsInterface;
struct AbckitArktsEnum;
struct AbckitArktsFunction;
struct AbckitArktsFunctionParam;
struct AbckitArktsModuleField;
struct AbckitArktsNamespaceField;
struct AbckitArktsClassField;
struct AbckitArktsInterfaceField;
struct AbckitArktsEnumField;
struct AbckitArktsAnnotation;
struct AbckitArktsAnnotationElement;
struct AbckitArktsAnnotationInterface;
struct AbckitArktsAnnotationInterfaceField;
struct AbckitArktsImportDescriptor;
struct AbckitArktsExportDescriptor;

struct AbckitJsModule;
struct AbckitJsClass;
struct AbckitJsFunction;
struct AbckitJsImportDescriptor;
struct AbckitJsExportDescriptor;

struct AbckitGraph;
struct AbckitBasicBlock;
struct AbckitInst;

#ifndef __cplusplus
typedef uint8_t *AbckitFileVersion;

typedef struct AbckitFile AbckitFile;
typedef struct AbckitString AbckitString;
typedef struct AbckitType AbckitType;
typedef struct AbckitValue AbckitValue;
typedef struct AbckitLiteral AbckitLiteral;
typedef struct AbckitLiteralArray AbckitLiteralArray;

typedef struct AbckitCoreModule AbckitCoreModule;
typedef struct AbckitCoreNamespace AbckitCoreNamespace;
typedef struct AbckitCoreClass AbckitCoreClass;
typedef struct AbckitCoreFunction AbckitCoreFunction;
typedef struct AbckitCoreAnnotation AbckitCoreAnnotation;
typedef struct AbckitCoreAnnotationElement AbckitCoreAnnotationElement;
typedef struct AbckitCoreAnnotationInterface AbckitCoreAnnotationInterface;
typedef struct AbckitCoreAnnotationInterfaceField AbckitCoreAnnotationInterfaceField;
typedef struct AbckitCoreImportDescriptor AbckitCoreImportDescriptor;
typedef struct AbckitCoreExportDescriptor AbckitCoreExportDescriptor;
typedef struct AbckitCoreInterface AbckitCoreInterface;
typedef struct AbckitCoreEnum AbckitCoreEnum;
typedef struct AbckitCoreModuleField AbckitCoreModuleField;
typedef struct AbckitCoreNamespaceField AbckitCoreNamespaceField;
typedef struct AbckitCoreClassField AbckitCoreClassField;
typedef struct AbckitCoreInterfaceField AbckitCoreInterfaceField;
typedef struct AbckitCoreEnumField AbckitCoreEnumField;
typedef struct AbckitCoreFunctionParam AbckitCoreFunctionParam;

typedef struct AbckitArktsModule AbckitArktsModule;
typedef struct AbckitArktsNamespace AbckitArktsNamespace;
typedef struct AbckitArktsClass AbckitArktsClass;
typedef struct AbckitArktsInterface AbckitArktsInterface;
typedef struct AbckitArktsEnum AbckitArktsEnum;
typedef struct AbckitArktsFunction AbckitArktsFunction;
typedef struct AbckitArktsFunctionParam AbckitArktsFunctionParam;
typedef struct AbckitArktsModuleField AbckitArktsModuleField;
typedef struct AbckitArktsNamespaceField AbckitArktsNamespaceField;
typedef struct AbckitArktsClassField AbckitArktsClassField;
typedef struct AbckitArktsInterfaceField AbckitArktsInterfaceField;
typedef struct AbckitArktsEnumField AbckitArktsEnumField;
typedef struct AbckitArktsAnnotation AbckitArktsAnnotation;
typedef struct AbckitArktsAnnotationElement AbckitArktsAnnotationElement;
typedef struct AbckitArktsAnnotationInterface AbckitArktsAnnotationInterface;
typedef struct AbckitArktsAnnotationInterfaceField AbckitArktsAnnotationInterfaceField;
typedef struct AbckitArktsImportDescriptor AbckitArktsImportDescriptor;
typedef struct AbckitArktsExportDescriptor AbckitArktsExportDescriptor;

typedef struct AbckitJsModule AbckitJsModule;
typedef struct AbckitJsClass AbckitJsClass;
typedef struct AbckitJsFunction AbckitJsFunction;
typedef struct AbckitJsImportDescriptor AbckitJsImportDescriptor;
typedef struct AbckitJsExportDescriptor AbckitJsExportDescriptor;

typedef struct AbckitGraph AbckitGraph;
typedef struct AbckitBasicBlock AbckitBasicBlock;
typedef struct AbckitInst AbckitInst;
#else
using AbckitFileVersion = uint8_t *;
#endif

#ifdef __cplusplus
}
#endif

#endif
