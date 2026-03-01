//===-- ClangUtil.h ---------------------------------------------*- C++ -*-===//
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
// A collection of helper methods and data structures for manipulating clang
// types and decls.
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGUTIL_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGUTIL_H

#include "clang/AST/DeclBase.h"
#include "clang/AST/Type.h"

#include "lldb/Symbol/CompilerType.h"

namespace clang {
class TagDecl;
}

namespace lldb_private {
struct ClangUtil {
  static bool IsClangType(const CompilerType &ct);

  /// Returns the clang::Decl of the given CompilerDecl.
  /// CompilerDecl has to be valid and represent a clang::Decl.
  static clang::Decl *GetDecl(const CompilerDecl &decl);

  static clang::QualType GetQualType(const CompilerType &ct);

  static clang::QualType GetCanonicalQualType(const CompilerType &ct);

  static CompilerType RemoveFastQualifiers(const CompilerType &ct);

  static clang::TagDecl *GetAsTagDecl(const CompilerType &type);

  /// Returns a textual representation of the given Decl's AST. Does not
  /// deserialize any child nodes.
  static std::string DumpDecl(const clang::Decl *d);
  /// Returns a textual representation of the given type.
  static std::string ToString(const clang::Type *t);
  /// Returns a textual representation of the given CompilerType (assuming
  /// its underlying type is a Clang type).
  static std::string ToString(const CompilerType &c);
};
}

#endif
