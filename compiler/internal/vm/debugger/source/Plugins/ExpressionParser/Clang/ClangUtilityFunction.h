//===-- ClangUtilityFunction.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGUTILITYFUNCTION_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGUTILITYFUNCTION_H

#include <map>
#include <string>
#include <vector>

#include "ClangExpressionHelper.h"

#include "lldb/Expression/UtilityFunction.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

/// \class ClangUtilityFunction ClangUtilityFunction.h
/// "lldb/Expression/ClangUtilityFunction.h" Encapsulates a single expression
/// for use with Clang
///
/// LLDB uses expressions for various purposes, notably to call functions
/// and as a backend for the expr command.  ClangUtilityFunction encapsulates
/// a self-contained function meant to be used from other code.  Utility
/// functions can perform error-checking for ClangUserExpressions, or can
/// simply provide a way to push a function into the target for the debugger
/// to call later on.
class ClangUtilityFunction : public UtilityFunction {
  // LLVM RTTI support
  static char ID;

public:
  bool isA(const void *ClassID) const override {
    return ClassID == &ID || UtilityFunction::isA(ClassID);
  }
  static bool classof(const Expression *obj) { return obj->isA(&ID); }

  /// Constructor
  ///
  /// \param[in] text
  ///     The text of the function.  Must be a full translation unit.
  ///
  /// \param[in] name
  ///     The name of the function, as used in the text.
  ///
  /// \param[in] enable_debugging
  ///     Enable debugging of this function.
  ClangUtilityFunction(ExecutionContextScope &exe_scope, std::string text,
                       std::string name, bool enable_debugging);

  ~ClangUtilityFunction() override;

  ExpressionTypeSystemHelper *GetTypeSystemHelper() override {
    return &m_type_system_helper;
  }

  ClangExpressionDeclMap *DeclMap() { return m_type_system_helper.DeclMap(); }

  void ResetDeclMap() { m_type_system_helper.ResetDeclMap(); }

  void ResetDeclMap(ExecutionContext &exe_ctx, bool keep_result_in_memory) {
    m_type_system_helper.ResetDeclMap(exe_ctx, keep_result_in_memory);
  }

  bool Install(DiagnosticManager &diagnostic_manager,
               ExecutionContext &exe_ctx) override;

private:
  class ClangUtilityFunctionHelper
      : public llvm::RTTIExtends<ClangUtilityFunctionHelper,
                                 ClangExpressionHelper> {
  public:
    // LLVM RTTI support
    static char ID;

    /// Return the object that the parser should use when resolving external
    /// values.  May be NULL if everything should be self-contained.
    ClangExpressionDeclMap *DeclMap() override {
      return m_expr_decl_map_up.get();
    }

    void ResetDeclMap() { m_expr_decl_map_up.reset(); }

    void ResetDeclMap(ExecutionContext &exe_ctx, bool keep_result_in_memory);

    /// Return the object that the parser should allow to access ASTs. May be
    /// nullptr if the ASTs do not need to be transformed.
    ///
    /// \param[in] passthrough
    ///     The ASTConsumer that the returned transformer should send
    ///     the ASTs to after transformation.
    clang::ASTConsumer *
    ASTTransformer(clang::ASTConsumer *passthrough) override {
      return nullptr;
    }

  private:
    std::unique_ptr<ClangExpressionDeclMap> m_expr_decl_map_up;
  };

  /// The map to use when parsing and materializing the expression.
  ClangUtilityFunctionHelper m_type_system_helper;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGUTILITYFUNCTION_H
