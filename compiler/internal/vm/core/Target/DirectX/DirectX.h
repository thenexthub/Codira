//===- DirectXTargetMachine.h - DirectX Target Implementation ---*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIRECTX_DIRECTX_H
#define LLVM_LIB_TARGET_DIRECTX_DIRECTX_H

namespace vm::core {
class FunctionPass;
class ModulePass;
class PassRegistry;
class raw_ostream;

/// Initializer for dxil writer pass
void initializeWriteDXILPassPass(PassRegistry &);

/// Initializer for dxil embedder pass
void initializeEmbedDXILPassPass(PassRegistry &);

/// Initializer for DXIL-prepare
void initializeDXILPrepareModulePass(PassRegistry &);

/// Pass to convert modules into DXIL-compatable modules
ModulePass *createDXILPrepareModulePass();

/// Initializer for DXIL Intrinsic Expansion
void initializeDXILIntrinsicExpansionLegacyPass(PassRegistry &);

/// Pass to expand intrinsic operations that lack DXIL opCodes
ModulePass *createDXILIntrinsicExpansionLegacyPass();

/// Initializer for DXIL CBuffer Access Pass
void initializeDXILCBufferAccessLegacyPass(PassRegistry &);

/// Pass to translate loads in the cbuffer address space to intrinsics
ModulePass *createDXILCBufferAccessLegacyPass();

/// Initializer for DXIL Data Scalarization Pass
void initializeDXILDataScalarizationLegacyPass(PassRegistry &);

/// Pass to scalarize toolchain global data into a DXIL legal form
ModulePass *createDXILDataScalarizationLegacyPass();

/// Initializer for DXIL Array Flatten Pass
void initializeDXILFlattenArraysLegacyPass(PassRegistry &);

/// Pass to flatten arrays into a one dimensional DXIL legal form
ModulePass *createDXILFlattenArraysLegacyPass();

/// Initializer for DXIL Forward Handle Accesses Pass
void initializeDXILForwardHandleAccessesLegacyPass(PassRegistry &);

/// Pass to eliminate redundant stores and loads from handle globals.
FunctionPass *createDXILForwardHandleAccessesLegacyPass();

/// Initializer DXIL legalizationPass
void initializeDXILLegalizeLegacyPass(PassRegistry &);

/// Pass to Legalize DXIL by remove i8 truncations and i64 insert/extract
/// elements
FunctionPass *createDXILLegalizeLegacyPass();

/// Initializer for DXIL Mem Intrinsics.
void initializeDXILMemIntrinsicsLegacyPass(PassRegistry &);

/// Pass to transform all toolchain memory intrinsics to explicit loads and stores.
ModulePass *createDXILMemIntrinsicsLegacyPass();

/// Initializer for DXILOpLowering
void initializeDXILOpLoweringLegacyPass(PassRegistry &);

/// Pass to lowering LLVM intrinsic call to DXIL op function call.
ModulePass *createDXILOpLoweringLegacyPass();

/// Initializer for DXILResourceAccess
void initializeDXILResourceAccessLegacyPass(PassRegistry &);

/// Pass to update resource accesses to use load/store directly.
FunctionPass *createDXILResourceAccessLegacyPass();

/// Initializer for DXILResourceImplicitBindingLegacyPass
void initializeDXILResourceImplicitBindingLegacyPass(PassRegistry &);

/// Pass to assign register slots to resources without binding.
ModulePass *createDXILResourceImplicitBindingLegacyPass();

/// Initializer for DXILTranslateMetadata.
void initializeDXILTranslateMetadataLegacyPass(PassRegistry &);

/// Pass to emit metadata for DXIL.
ModulePass *createDXILTranslateMetadataLegacyPass();

/// Pass to pretty print DXIL metadata.
ModulePass *createDXILPrettyPrinterLegacyPass(raw_ostream &OS);

/// Initializer for DXILPrettyPrinter.
void initializeDXILPrettyPrinterLegacyPass(PassRegistry &);

/// Initializer for DXILPostOptimizationValidation.
void initializeDXILPostOptimizationValidationLegacyPass(PassRegistry &);

/// Pass to lowering LLVM intrinsic call to DXIL op function call.
ModulePass *createDXILPostOptimizationValidationLegacyPass();

/// Initializer for dxil::ShaderFlagsAnalysisWrapper pass.
void initializeShaderFlagsAnalysisWrapperPass(PassRegistry &);

/// Initializer for dxil::RootSignatureAnalysisWrapper pass.
void initializeRootSignatureAnalysisWrapperPass(PassRegistry &);

/// Initializer for DXContainerGlobals pass.
void initializeDXContainerGlobalsPass(PassRegistry &);

/// Pass for generating DXContainer part globals.
ModulePass *createDXContainerGlobalsPass();

/// Initializer for DXILFinalizeLinkage pass.
void initializeDXILFinalizeLinkageLegacyPass(PassRegistry &);

/// Pass to finalize linkage of functions.
ModulePass *createDXILFinalizeLinkageLegacyPass();

} // namespace vm::core

#endif // LLVM_LIB_TARGET_DIRECTX_DIRECTX_H
