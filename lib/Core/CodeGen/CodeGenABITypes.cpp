//==--- CodeGenABITypes.cpp - Convert Clang types to LLVM types for ABI ----==//
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
//===----------------------------------------------------------------------===//

#include "language/Core/CodeGen/CodeGenABITypes.h"
#include "CGCXXABI.h"
#include "CGRecordLayout.h"
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "language/Core/CodeGen/CGFunctionInfo.h"

using namespace language::Core;
using namespace CodeGen;

void CodeGen::addDefaultFunctionDefinitionAttributes(CodeGenModule &CGM,
                                                     toolchain::AttrBuilder &attrs) {
  CGM.addDefaultFunctionDefinitionAttributes(attrs);
}

const CGFunctionInfo &
CodeGen::arrangeObjCMessageSendSignature(CodeGenModule &CGM,
                                         const ObjCMethodDecl *MD,
                                         QualType receiverType) {
  return CGM.getTypes().arrangeObjCMessageSendSignature(MD, receiverType);
}

const CGFunctionInfo &
CodeGen::arrangeFreeFunctionType(CodeGenModule &CGM,
                                 CanQual<FunctionProtoType> Ty) {
  return CGM.getTypes().arrangeFreeFunctionType(Ty);
}

const CGFunctionInfo &
CodeGen::arrangeFreeFunctionType(CodeGenModule &CGM,
                                 CanQual<FunctionNoProtoType> Ty) {
  return CGM.getTypes().arrangeFreeFunctionType(Ty);
}

const CGFunctionInfo &
CodeGen::arrangeCXXMethodType(CodeGenModule &CGM,
                              const CXXRecordDecl *RD,
                              const FunctionProtoType *FTP,
                              const CXXMethodDecl *MD) {
  return CGM.getTypes().arrangeCXXMethodType(RD, FTP, MD);
}

const CGFunctionInfo &CodeGen::arrangeCXXMethodCall(
    CodeGenModule &CGM, CanQualType returnType, ArrayRef<CanQualType> argTypes,
    FunctionType::ExtInfo info,
    ArrayRef<FunctionProtoType::ExtParameterInfo> paramInfos,
    RequiredArgs args) {
  return CGM.getTypes().arrangeLLVMFunctionInfo(
      returnType, FnInfoOpts::IsInstanceMethod, argTypes, info, paramInfos,
      args);
}

const CGFunctionInfo &CodeGen::arrangeFreeFunctionCall(
    CodeGenModule &CGM, CanQualType returnType, ArrayRef<CanQualType> argTypes,
    FunctionType::ExtInfo info,
    ArrayRef<FunctionProtoType::ExtParameterInfo> paramInfos,
    RequiredArgs args) {
  return CGM.getTypes().arrangeLLVMFunctionInfo(
      returnType, FnInfoOpts::None, argTypes, info, paramInfos, args);
}

ImplicitCXXConstructorArgs
CodeGen::getImplicitCXXConstructorArgs(CodeGenModule &CGM,
                                       const CXXConstructorDecl *D) {
  // We have to create a dummy CodeGenFunction here to pass to
  // getImplicitConstructorArgs(). In some cases (base and delegating
  // constructor calls), getImplicitConstructorArgs() can reach into the
  // CodeGenFunction to find parameters of the calling constructor to pass on to
  // the called constructor, but that can't happen here because we're asking for
  // the args for a complete, non-delegating constructor call.
  CodeGenFunction CGF(CGM, /* suppressNewContext= */ true);
  CGCXXABI::AddedStructorArgs addedArgs =
      CGM.getCXXABI().getImplicitConstructorArgs(CGF, D, Ctor_Complete,
                                                 /* ForVirtualBase= */ false,
                                                 /* Delegating= */ false);
  ImplicitCXXConstructorArgs implicitArgs;
  for (const auto &arg : addedArgs.Prefix) {
    implicitArgs.Prefix.push_back(arg.Value);
  }
  for (const auto &arg : addedArgs.Suffix) {
    implicitArgs.Suffix.push_back(arg.Value);
  }
  return implicitArgs;
}

toolchain::FunctionType *
CodeGen::convertFreeFunctionType(CodeGenModule &CGM, const FunctionDecl *FD) {
  assert(FD != nullptr && "Expected a non-null function declaration!");
  toolchain::Type *T = CGM.getTypes().ConvertType(FD->getType());

  if (auto FT = dyn_cast<toolchain::FunctionType>(T))
    return FT;

  return nullptr;
}

toolchain::Type *
CodeGen::convertTypeForMemory(CodeGenModule &CGM, QualType T) {
  return CGM.getTypes().ConvertTypeForMem(T);
}

unsigned CodeGen::getLLVMFieldNumber(CodeGenModule &CGM,
                                     const RecordDecl *RD,
                                     const FieldDecl *FD) {
  return CGM.getTypes().getCGRecordLayout(RD).getLLVMFieldNo(FD);
}

toolchain::Value *CodeGen::getCXXDestructorImplicitParam(
    CodeGenModule &CGM, toolchain::BasicBlock *InsertBlock,
    toolchain::BasicBlock::iterator InsertPoint, const CXXDestructorDecl *D,
    CXXDtorType Type, bool ForVirtualBase, bool Delegating) {
  CodeGenFunction CGF(CGM, /*suppressNewContext=*/true);
  CGF.CurCodeDecl = D;
  CGF.CurFuncDecl = D;
  CGF.CurFn = InsertBlock->getParent();
  CGF.Builder.SetInsertPoint(InsertBlock, InsertPoint);
  return CGM.getCXXABI().getCXXDestructorImplicitParam(
      CGF, D, Type, ForVirtualBase, Delegating);
}
