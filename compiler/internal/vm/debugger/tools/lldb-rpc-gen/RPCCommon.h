//===-- RPCCommon.h -------------------------------------------------------===//
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

#ifndef LLDB_RPC_GEN_RPCCOMMON_H
#define LLDB_RPC_GEN_RPCCOMMON_H

#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

using namespace clang;

namespace lldb_rpc_gen {
QualType GetUnderlyingType(QualType T);
QualType GetUnqualifiedUnderlyingType(QualType T);
std::string GetMangledName(ASTContext &Context, CXXMethodDecl *MDecl);

bool TypeIsFromLLDBPrivate(QualType T);
bool TypeIsSBClass(QualType T);
bool TypeIsConstCharPtr(QualType T);
bool TypeIsConstCharPtrPtr(QualType T);
bool TypeIsDisallowedClass(QualType T);
bool TypeIsCallbackFunctionPointer(QualType T);

bool MethodIsDisallowed(ASTContext &Context, CXXMethodDecl *MDecl);

std::string ReplaceLLDBNamespaceWithRPCNamespace(std::string Name);
std::string StripLLDBNamespace(std::string Name);
bool SBClassRequiresDefaultCtor(const std::string &ClassName);
bool SBClassRequiresCopyCtorAssign(const std::string &ClassName);
bool SBClassInheritsFromObjectRef(const std::string &ClassName);
std::string GetSBClassNameFromType(QualType T);
struct Param {
  std::string Name;
  QualType Type;
  std::string DefaultValueText;
  bool IsFollowedByLen;
};

enum GenerationKind : bool { eServer, eLibrary };

struct Method {
  enum Type { eOther, eConstructor, eDestructor };

  Method(CXXMethodDecl *MDecl, const PrintingPolicy &Policy,
         ASTContext &Context);

  // Adding a '<' allows us to use Methods in ordered containers.
  // The ordering is on memory addresses.
  bool operator<(const lldb_rpc_gen::Method &rhs) const;
  const PrintingPolicy &Policy;
  const ASTContext &Context;
  std::string QualifiedName;
  std::string BaseName;
  std::string MangledName;
  QualType ReturnType;
  QualType ThisType;
  std::vector<Param> Params;
  bool IsConst = false;
  bool IsInstance = false;
  bool IsCtor = false;
  bool IsCopyCtor = false;
  bool IsCopyAssign = false;
  bool IsMoveCtor = false;
  bool IsMoveAssign = false;
  bool IsDtor = false;
  bool IsConversionMethod = false;
  bool IsExplicitCtorOrConversionMethod = false;
  bool ContainsFunctionPointerParameter = false;

  std::string CreateParamListAsString(GenerationKind Generation,
                                      bool IncludeDefaultValue = false) const;

  bool RequiresConnectionParameter() const;
};

std::string
GetDefaultArgumentsForConstructor(std::string ClassName,
                                  const lldb_rpc_gen::Method &method);

class FileEmitter {
protected:
  FileEmitter(std::unique_ptr<llvm::ToolOutputFile> &&OutputFile)
      : OutputFile(std::move(OutputFile)), IndentLevel(0) {}
  void EmitLine(const std::string &line) {
    for (auto i = 0; i < IndentLevel; i++)
      OutputFile->os() << "  ";

    OutputFile->os() << line << "\n";
  }

  void EmitNewLine() { OutputFile->os() << "\n"; }

  std::unique_ptr<llvm::ToolOutputFile> OutputFile;
  uint8_t IndentLevel;
};
} // namespace lldb_rpc_gen
#endif // LLDB_RPC_GEN_RPCCOMMON_H
