//===-- ClangExpressionHelper.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGEXPRESSIONHELPER_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGEXPRESSIONHELPER_H

#include <map>
#include <string>
#include <vector>

#include "lldb/Expression/ExpressionTypeSystemHelper.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-private.h"

namespace clang {
class ASTConsumer;
}

namespace lldb_private {

class ClangExpressionDeclMap;

// ClangExpressionHelper
class ClangExpressionHelper
    : public llvm::RTTIExtends<ClangExpressionHelper,
                               ExpressionTypeSystemHelper> {
public:
  // LLVM RTTI support
  static char ID;

  /// Return the object that the parser should use when resolving external
  /// values.  May be NULL if everything should be self-contained.
  virtual ClangExpressionDeclMap *DeclMap() = 0;

  /// Return the object that the parser should allow to access ASTs.
  /// May be NULL if the ASTs do not need to be transformed.
  ///
  /// \param[in] passthrough
  ///     The ASTConsumer that the returned transformer should send
  ///     the ASTs to after transformation.
  virtual clang::ASTConsumer *
  ASTTransformer(clang::ASTConsumer *passthrough) = 0;

  virtual void CommitPersistentDecls() {}
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGEXPRESSIONHELPER_H
