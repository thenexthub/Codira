//===--- HLSLBuiltinTypeDeclBuilder.h - HLSL Builtin Type Decl Builder  ---===//
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
// Helper classes for creating HLSL builtin class types. Used by external HLSL
// sema source.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_HLSLBUILTINTYPEDECLBUILDER_H
#define LANGUAGE_CORE_SEMA_HLSLBUILTINTYPEDECLBUILDER_H

#include "language/Core/AST/Type.h"
#include "language/Core/Sema/Sema.h"
#include "toolchain/ADT/StringMap.h"

using toolchain::hlsl::ResourceClass;

namespace language::Core {

class ClassTemplateDecl;
class NamespaceDecl;
class CXXRecordDecl;
class FieldDecl;

namespace hlsl {

// Builder for builtin HLSL class types such as HLSL resource classes.
// Allows creating declaration of builtin types using the builder pattern
// like this:
//
//   Decl = BuiltinTypeDeclBuilder(Sema, Namespace, "BuiltinClassName")
//           .addSimpleTemplateParams({"T"}, Concept)
//           .finalizeForwardDeclaration();
//
// And then completing the type like this:
//
//   BuiltinTypeDeclBuilder(Sema, Decl)
//          .addDefaultHandleConstructor();
//          .addLoadMethods()
//          .completeDefinition();
//
class BuiltinTypeDeclBuilder {
private:
  Sema &SemaRef;
  CXXRecordDecl *Record = nullptr;
  ClassTemplateDecl *Template = nullptr;
  ClassTemplateDecl *PrevTemplate = nullptr;
  NamespaceDecl *HLSLNamespace = nullptr;
  toolchain::StringMap<FieldDecl *> Fields;

public:
  friend struct TemplateParameterListBuilder;
  friend struct BuiltinTypeMethodBuilder;

  BuiltinTypeDeclBuilder(Sema &SemaRef, CXXRecordDecl *R);
  BuiltinTypeDeclBuilder(Sema &SemaRef, NamespaceDecl *Namespace,
                         StringRef Name);
  ~BuiltinTypeDeclBuilder();

  BuiltinTypeDeclBuilder &addSimpleTemplateParams(ArrayRef<StringRef> Names,
                                                  ConceptDecl *CD);
  CXXRecordDecl *finalizeForwardDeclaration() { return Record; }
  BuiltinTypeDeclBuilder &completeDefinition();

  BuiltinTypeDeclBuilder &
  addMemberVariable(StringRef Name, QualType Type, toolchain::ArrayRef<Attr *> Attrs,
                    AccessSpecifier Access = AccessSpecifier::AS_private);

  BuiltinTypeDeclBuilder &
  addHandleMember(ResourceClass RC, bool IsROV, bool RawBuffer,
                  AccessSpecifier Access = AccessSpecifier::AS_private);
  BuiltinTypeDeclBuilder &addArraySubscriptOperators();

  // Builtin types constructors
  BuiltinTypeDeclBuilder &addDefaultHandleConstructor();
  BuiltinTypeDeclBuilder &addHandleConstructorFromBinding();
  BuiltinTypeDeclBuilder &addHandleConstructorFromImplicitBinding();

  // Builtin types methods
  BuiltinTypeDeclBuilder &addLoadMethods();
  BuiltinTypeDeclBuilder &addIncrementCounterMethod();
  BuiltinTypeDeclBuilder &addDecrementCounterMethod();
  BuiltinTypeDeclBuilder &addHandleAccessFunction(DeclarationName &Name,
                                                  bool IsConst, bool IsRef);
  BuiltinTypeDeclBuilder &addAppendMethod();
  BuiltinTypeDeclBuilder &addConsumeMethod();

private:
  FieldDecl *getResourceHandleField() const;
  QualType getFirstTemplateTypeParam();
  QualType getHandleElementType();
  Expr *getConstantIntExpr(int value);
  HLSLAttributedResourceType::Attributes getResourceAttrs() const;
};

} // namespace hlsl

} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_HLSLBUILTINTYPEDECLBUILDER_H
