//===- ExtractAPI/TypedefUnderlyingTypeResolver.h ---------------*- C++ -*-===//
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
/// This file defines the UnderlyingTypeResolver which is a helper type for
/// resolving the undelrying type for a given QualType and exposing that
/// information in various forms.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_UNDERLYING_TYPE_RESOLVER_H
#define LANGUAGE_CORE_UNDERLYING_TYPE_RESOLVER_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/ExtractAPI/API.h"

#include <string>

namespace language::Core {
namespace extractapi {

struct TypedefUnderlyingTypeResolver {
  /// Gets the underlying type declaration.
  const NamedDecl *getUnderlyingTypeDecl(QualType Type) const;

  /// Get a SymbolReference for the given type.
  SymbolReference getSymbolReferenceForType(QualType Type, APISet &API) const;

  /// Get a USR for the given type.
  std::string getUSRForType(QualType Type) const;

  explicit TypedefUnderlyingTypeResolver(ASTContext &Context)
      : Context(Context) {}

private:
  ASTContext &Context;
};

} // namespace extractapi
} // namespace language::Core

#endif // LANGUAGE_CORE_UNDERLYING_TYPE_RESOLVER_H
