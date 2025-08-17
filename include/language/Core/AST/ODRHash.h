//===-- ODRHash.h - Hashing to diagnose ODR failures ------------*- C++ -*-===//
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
///
/// \file
/// This file contains the declaration of the ODRHash class, which calculates
/// a hash based on AST nodes, which is stable across different runs.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ODRHASH_H
#define LANGUAGE_CORE_AST_ODRHASH_H

#include "language/Core/AST/DeclarationName.h"
#include "language/Core/AST/Type.h"
#include "language/Core/AST/TemplateBase.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core {

class APValue;
class Decl;
class IdentifierInfo;
class NestedNameSpecifier;
class Stmt;
class TemplateParameterList;

// ODRHash is used to calculate a hash based on AST node contents that
// does not rely on pointer addresses.  This allows the hash to not vary
// between runs and is usable to detect ODR problems in modules.  To use,
// construct an ODRHash object, then call Add* methods over the nodes that
// need to be hashed.  Then call CalculateHash to get the hash value.
// Typically, only one Add* call is needed.  clear can be called to reuse the
// object.
class ODRHash {
  // Use DenseMaps to convert from DeclarationName and Type pointers
  // to an index value.
  toolchain::DenseMap<DeclarationName, unsigned> DeclNameMap;

  // Save space by processing bools at the end.
  toolchain::SmallVector<bool, 128> Bools;

  toolchain::FoldingSetNodeID ID;

public:
  ODRHash() {}

  // Use this for ODR checking classes between modules.  This method compares
  // more information than the AddDecl class.
  void AddCXXRecordDecl(const CXXRecordDecl *Record);

  // Use this for ODR checking records in C/Objective-C between modules. This
  // method compares more information than the AddDecl class.
  void AddRecordDecl(const RecordDecl *Record);

  // Use this for ODR checking ObjC interfaces. This
  // method compares more information than the AddDecl class.
  void AddObjCInterfaceDecl(const ObjCInterfaceDecl *Record);

  // Use this for ODR checking functions between modules.  This method compares
  // more information than the AddDecl class.  SkipBody will process the
  // hash as if the function has no body.
  void AddFunctionDecl(const FunctionDecl *Function, bool SkipBody = false);

  // Use this for ODR checking enums between modules.  This method compares
  // more information than the AddDecl class.
  void AddEnumDecl(const EnumDecl *Enum);

  // Use this for ODR checking ObjC protocols. This
  // method compares more information than the AddDecl class.
  void AddObjCProtocolDecl(const ObjCProtocolDecl *P);

  // Process SubDecls of the main Decl.  This method calls the DeclVisitor
  // while AddDecl does not.
  void AddSubDecl(const Decl *D);

  // Reset the object for reuse.
  void clear();

  // Add booleans to ID and uses it to calculate the hash.
  unsigned CalculateHash();

  // Add AST nodes that need to be processed.
  void AddDecl(const Decl *D);
  void AddType(const Type *T);
  void AddQualType(QualType T);
  void AddStmt(const Stmt *S);
  void AddIdentifierInfo(const IdentifierInfo *II);
  void AddNestedNameSpecifier(NestedNameSpecifier NNS);
  void AddDependentTemplateName(const DependentTemplateStorage &Name);
  void AddTemplateName(TemplateName Name);
  void AddDeclarationNameInfo(DeclarationNameInfo NameInfo,
                              bool TreatAsDecl = false);
  void AddDeclarationName(DeclarationName Name, bool TreatAsDecl = false) {
    AddDeclarationNameInfo(DeclarationNameInfo(Name, SourceLocation()),
                           TreatAsDecl);
  }

  void AddTemplateArgument(TemplateArgument TA);
  void AddTemplateParameterList(const TemplateParameterList *TPL);

  // Save booleans until the end to lower the size of data to process.
  void AddBoolean(bool value);

  void AddStructuralValue(const APValue &);

  static bool isSubDeclToBeProcessed(const Decl *D, const DeclContext *Parent);

private:
  void AddDeclarationNameInfoImpl(DeclarationNameInfo NameInfo);
};

}  // end namespace language::Core

#endif
