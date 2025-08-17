//==---- CodeGenABITypes.h - Convert Clang types to LLVM types for ABI -----==//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
// CodeGenABITypes is a simple interface for getting LLVM types for
// the parameters and the return value of a function given the Clang
// types.
//
// The class is implemented as a public wrapper around the private
// CodeGenTypes class in lib/CodeGen.
//
// It allows other clients, like LLDB, to determine the LLVM types that are
// actually used in function calls, which makes it possible to then determine
// the actual ABI locations (e.g. registers, stack locations, etc.) that
// these parameters are stored in.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_CODEGEN_CODEGENABITYPES_H
#define LANGUAGE_CORE_CODEGEN_CODEGENABITYPES_H

#include "language/Core/AST/CanonicalType.h"
#include "language/Core/AST/Type.h"
#include "language/Core/Basic/ABI.h"
#include "language/Core/CodeGen/CGFunctionInfo.h"
#include "toolchain/IR/BasicBlock.h"

namespace toolchain {
class AttrBuilder;
class Constant;
class Function;
class FunctionType;
class Type;
}

namespace language::Core {
class CXXConstructorDecl;
class CXXDestructorDecl;
class CXXRecordDecl;
class CXXMethodDecl;
class GlobalDecl;
class ObjCMethodDecl;
class ObjCProtocolDecl;

namespace CodeGen {
class CGFunctionInfo;
class CodeGenModule;

/// Additional implicit arguments to add to a constructor argument list.
struct ImplicitCXXConstructorArgs {
  /// Implicit arguments to add before the explicit arguments, but after the
  /// `*this` argument (which always comes first).
  SmallVector<toolchain::Value *, 1> Prefix;

  /// Implicit arguments to add after the explicit arguments.
  SmallVector<toolchain::Value *, 1> Suffix;
};

const CGFunctionInfo &arrangeObjCMessageSendSignature(CodeGenModule &CGM,
                                                      const ObjCMethodDecl *MD,
                                                      QualType receiverType);

const CGFunctionInfo &arrangeFreeFunctionType(CodeGenModule &CGM,
                                              CanQual<FunctionProtoType> Ty);

const CGFunctionInfo &arrangeFreeFunctionType(CodeGenModule &CGM,
                                              CanQual<FunctionNoProtoType> Ty);

const CGFunctionInfo &arrangeCXXMethodType(CodeGenModule &CGM,
                                           const CXXRecordDecl *RD,
                                           const FunctionProtoType *FTP,
                                           const CXXMethodDecl *MD);

const CGFunctionInfo &
arrangeCXXMethodCall(CodeGenModule &CGM, CanQualType returnType,
                     ArrayRef<CanQualType> argTypes, FunctionType::ExtInfo info,
                     ArrayRef<FunctionProtoType::ExtParameterInfo> paramInfos,
                     RequiredArgs args);

const CGFunctionInfo &arrangeFreeFunctionCall(
    CodeGenModule &CGM, CanQualType returnType, ArrayRef<CanQualType> argTypes,
    FunctionType::ExtInfo info,
    ArrayRef<FunctionProtoType::ExtParameterInfo> paramInfos,
    RequiredArgs args);

// An overload with an empty `paramInfos`
inline const CGFunctionInfo &
arrangeFreeFunctionCall(CodeGenModule &CGM, CanQualType returnType,
                        ArrayRef<CanQualType> argTypes,
                        FunctionType::ExtInfo info, RequiredArgs args) {
  return arrangeFreeFunctionCall(CGM, returnType, argTypes, info, {}, args);
}

/// Returns the implicit arguments to add to a complete, non-delegating C++
/// constructor call.
ImplicitCXXConstructorArgs
getImplicitCXXConstructorArgs(CodeGenModule &CGM, const CXXConstructorDecl *D);

toolchain::Value *
getCXXDestructorImplicitParam(CodeGenModule &CGM, toolchain::BasicBlock *InsertBlock,
                              toolchain::BasicBlock::iterator InsertPoint,
                              const CXXDestructorDecl *D, CXXDtorType Type,
                              bool ForVirtualBase, bool Delegating);

/// Returns null if the function type is incomplete and can't be lowered.
toolchain::FunctionType *convertFreeFunctionType(CodeGenModule &CGM,
                                            const FunctionDecl *FD);

toolchain::Type *convertTypeForMemory(CodeGenModule &CGM, QualType T);

/// Given a non-bitfield struct field, return its index within the elements of
/// the struct's converted type.  The returned index refers to a field number in
/// the complete object type which is returned by convertTypeForMemory.  FD must
/// be a field in RD directly (i.e. not an inherited field).
unsigned getLLVMFieldNumber(CodeGenModule &CGM,
                            const RecordDecl *RD, const FieldDecl *FD);

/// Return a declaration discriminator for the given global decl.
uint16_t getPointerAuthDeclDiscriminator(CodeGenModule &CGM, GlobalDecl GD);

/// Return a type discriminator for the given function type.
uint16_t getPointerAuthTypeDiscriminator(CodeGenModule &CGM,
                                         QualType FunctionType);

/// Given the language and code-generation options that Clang was configured
/// with, set the default LLVM IR attributes for a function definition.
/// The attributes set here are mostly global target-configuration and
/// pipeline-configuration options like the target CPU, variant stack
/// rules, whether to optimize for size, and so on.  This is useful for
/// frontends (such as Swift) that generally intend to interoperate with
/// C code and rely on Clang's target configuration logic.
///
/// As a general rule, this function assumes that meaningful attributes
/// haven't already been added to the builder.  It won't intentionally
/// displace any existing attributes, but it also won't check to avoid
/// overwriting them.  Callers should generally apply customizations after
/// making this call.
///
/// This function assumes that the caller is not defining a function that
/// requires special no-builtin treatment.
void addDefaultFunctionDefinitionAttributes(CodeGenModule &CGM,
                                            toolchain::AttrBuilder &attrs);

/// Returns the default constructor for a C struct with non-trivially copyable
/// fields, generating it if necessary. The returned function uses the `cdecl`
/// calling convention, returns void, and takes a single argument that is a
/// pointer to the address of the struct.
toolchain::Function *getNonTrivialCStructDefaultConstructor(CodeGenModule &GCM,
                                                       CharUnits DstAlignment,
                                                       bool IsVolatile,
                                                       QualType QT);

/// Returns the copy constructor for a C struct with non-trivially copyable
/// fields, generating it if necessary. The returned function uses the `cdecl`
/// calling convention, returns void, and takes two arguments: pointers to the
/// addresses of the destination and source structs, respectively.
toolchain::Function *getNonTrivialCStructCopyConstructor(CodeGenModule &CGM,
                                                    CharUnits DstAlignment,
                                                    CharUnits SrcAlignment,
                                                    bool IsVolatile,
                                                    QualType QT);

/// Returns the move constructor for a C struct with non-trivially copyable
/// fields, generating it if necessary. The returned function uses the `cdecl`
/// calling convention, returns void, and takes two arguments: pointers to the
/// addresses of the destination and source structs, respectively.
toolchain::Function *getNonTrivialCStructMoveConstructor(CodeGenModule &CGM,
                                                    CharUnits DstAlignment,
                                                    CharUnits SrcAlignment,
                                                    bool IsVolatile,
                                                    QualType QT);

/// Returns the copy assignment operator for a C struct with non-trivially
/// copyable fields, generating it if necessary. The returned function uses the
/// `cdecl` calling convention, returns void, and takes two arguments: pointers
/// to the addresses of the destination and source structs, respectively.
toolchain::Function *getNonTrivialCStructCopyAssignmentOperator(
    CodeGenModule &CGM, CharUnits DstAlignment, CharUnits SrcAlignment,
    bool IsVolatile, QualType QT);

/// Return the move assignment operator for a C struct with non-trivially
/// copyable fields, generating it if necessary. The returned function uses the
/// `cdecl` calling convention, returns void, and takes two arguments: pointers
/// to the addresses of the destination and source structs, respectively.
toolchain::Function *getNonTrivialCStructMoveAssignmentOperator(
    CodeGenModule &CGM, CharUnits DstAlignment, CharUnits SrcAlignment,
    bool IsVolatile, QualType QT);

/// Returns the destructor for a C struct with non-trivially copyable fields,
/// generating it if necessary. The returned function uses the `cdecl` calling
/// convention, returns void, and takes a single argument that is a pointer to
/// the address of the struct.
toolchain::Function *getNonTrivialCStructDestructor(CodeGenModule &CGM,
                                               CharUnits DstAlignment,
                                               bool IsVolatile, QualType QT);

/// Get a pointer to a protocol object for the given declaration, emitting it if
/// it hasn't already been emitted in this translation unit. Note that the ABI
/// for emitting a protocol reference in code (e.g. for a protocol expression)
/// in most runtimes is not as simple as just materializing a pointer to this
/// object.
toolchain::Constant *emitObjCProtocolObject(CodeGenModule &CGM,
                                       const ObjCProtocolDecl *p);
}  // end namespace CodeGen
}  // end namespace language::Core

#endif
