//===--- USRFinder.h - Clang refactoring library --------------------------===//
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
/// Methods for determining the USR of a symbol at a location in source
/// code.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRFINDER_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRFINDER_H

#include "language/Core/AST/AST.h"
#include "language/Core/AST/ASTContext.h"
#include <string>
#include <vector>

namespace language::Core {

class ASTContext;
class Decl;
class SourceLocation;
class NamedDecl;

namespace tooling {

// Given an AST context and a point, returns a NamedDecl identifying the symbol
// at the point. Returns null if nothing is found at the point.
const NamedDecl *getNamedDeclAt(const ASTContext &Context,
                                const SourceLocation Point);

// Given an AST context and a fully qualified name, returns a NamedDecl
// identifying the symbol with a matching name. Returns null if nothing is
// found for the name.
const NamedDecl *getNamedDeclFor(const ASTContext &Context,
                                 const std::string &Name);

// Converts a Decl into a USR.
std::string getUSRForDecl(const Decl *Decl);

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRFINDER_H
