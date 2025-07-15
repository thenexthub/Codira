/*===-- toolchain-c/Support.h - C Interface Types declarations ---------*- C -*-===*\
|*                                                                            *|
|* Part of the LLVM Project, under the Apache License v2.0 with LLVM          *|
|* Exceptions.                                                                *|
|* See https://toolchain.org/LICENSE.txt for license information.                  *|
|* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception                    *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This file defines types used by the C interface to LLVM.                   *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef TOOLCHAIN_C_TYPES_H
#define TOOLCHAIN_C_TYPES_H

#include "toolchain-c/DataTypes.h"
#include "toolchain-c/ExternC.h"

TOOLCHAIN_C_EXTERN_C_BEGIN

/**
 * @defgroup LLVMCSupportTypes Types and Enumerations
 *
 * @{
 */

typedef int LLVMBool;

/* Opaque types. */

/**
 * LLVM uses a polymorphic type hierarchy which C cannot represent, therefore
 * parameters must be passed as base types. Despite the declared types, most
 * of the functions provided operate only on branches of the type hierarchy.
 * The declared parameter names are descriptive and specify which type is
 * required. Additionally, each type hierarchy is documented along with the
 * functions that operate upon it. For more detail, refer to LLVM's C++ code.
 * If in doubt, refer to Core.cpp, which performs parameter downcasts in the
 * form unwrap<RequiredType>(Param).
 */

/**
 * Used to pass regions of memory through LLVM interfaces.
 *
 * @see toolchain::MemoryBuffer
 */
typedef struct LLVMOpaqueMemoryBuffer *LLVMMemoryBufferRef;

/**
 * The top-level container for all LLVM global data. See the LLVMContext class.
 */
typedef struct LLVMOpaqueContext *LLVMContextRef;

/**
 * The top-level container for all other LLVM Intermediate Representation (IR)
 * objects.
 *
 * @see toolchain::Module
 */
typedef struct LLVMOpaqueModule *LLVMModuleRef;

/**
 * Each value in the LLVM IR has a type, an LLVMTypeRef.
 *
 * @see toolchain::Type
 */
typedef struct LLVMOpaqueType *LLVMTypeRef;

/**
 * Represents an individual value in LLVM IR.
 *
 * This models toolchain::Value.
 */
typedef struct LLVMOpaqueValue *LLVMValueRef;

/**
 * Represents a basic block of instructions in LLVM IR.
 *
 * This models toolchain::BasicBlock.
 */
typedef struct LLVMOpaqueBasicBlock *LLVMBasicBlockRef;

/**
 * Represents an LLVM Metadata.
 *
 * This models toolchain::Metadata.
 */
typedef struct LLVMOpaqueMetadata *LLVMMetadataRef;

/**
 * Represents an LLVM Named Metadata Node.
 *
 * This models toolchain::NamedMDNode.
 */
typedef struct LLVMOpaqueNamedMDNode *LLVMNamedMDNodeRef;

/**
 * Represents an entry in a Global Object's metadata attachments.
 *
 * This models std::pair<unsigned, MDNode *>
 */
typedef struct LLVMOpaqueValueMetadataEntry LLVMValueMetadataEntry;

/**
 * Represents an LLVM basic block builder.
 *
 * This models toolchain::IRBuilder.
 */
typedef struct LLVMOpaqueBuilder *LLVMBuilderRef;

/**
 * Represents an LLVM debug info builder.
 *
 * This models toolchain::DIBuilder.
 */
typedef struct LLVMOpaqueDIBuilder *LLVMDIBuilderRef;

/**
 * Interface used to provide a module to JIT or interpreter.
 * This is now just a synonym for toolchain::Module, but we have to keep using the
 * different type to keep binary compatibility.
 */
typedef struct LLVMOpaqueModuleProvider *LLVMModuleProviderRef;

/** @see toolchain::PassManagerBase */
typedef struct LLVMOpaquePassManager *LLVMPassManagerRef;

/**
 * Used to get the users and usees of a Value.
 *
 * @see toolchain::Use */
typedef struct LLVMOpaqueUse *LLVMUseRef;

/**
 * Used to represent an attributes.
 *
 * @see toolchain::Attribute
 */
typedef struct LLVMOpaqueAttributeRef *LLVMAttributeRef;

/**
 * @see toolchain::DiagnosticInfo
 */
typedef struct LLVMOpaqueDiagnosticInfo *LLVMDiagnosticInfoRef;

/**
 * @see toolchain::Comdat
 */
typedef struct LLVMComdat *LLVMComdatRef;

/**
 * @see toolchain::Module::ModuleFlagEntry
 */
typedef struct LLVMOpaqueModuleFlagEntry LLVMModuleFlagEntry;

/**
 * @see toolchain::JITEventListener
 */
typedef struct LLVMOpaqueJITEventListener *LLVMJITEventListenerRef;

/**
 * @see toolchain::object::Binary
 */
typedef struct LLVMOpaqueBinary *LLVMBinaryRef;

/**
 * @}
 */

TOOLCHAIN_C_EXTERN_C_END

#endif
