//===-- ClangFunctionCaller.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGFUNCTIONCALLER_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGFUNCTIONCALLER_H

#include "ClangExpressionHelper.h"

#include "lldb/Core/Address.h"
#include "lldb/Core/Value.h"
#include "lldb/Expression/FunctionCaller.h"
#include "lldb/Symbol/CompilerType.h"
#include "lldb/Target/Process.h"
#include "lldb/ValueObject/ValueObjectList.h"

namespace lldb_private {

class ASTStructExtractor;

/// \class ClangFunctionCaller ClangFunctionCaller.h
/// "lldb/Expression/ClangFunctionCaller.h" Encapsulates a function that can
/// be called.
///
/// A given ClangFunctionCaller object can handle a single function signature.
/// Once constructed, it can set up any number of concurrent calls to
/// functions with that signature.
///
/// It performs the call by synthesizing a structure that contains the pointer
/// to the function and the arguments that should be passed to that function,
/// and producing a special-purpose JIT-compiled function that accepts a void*
/// pointing to this struct as its only argument and calls the function in the
/// struct with the written arguments.  This method lets Clang handle the
/// vagaries of function calling conventions.
///
/// The simplest use of the ClangFunctionCaller is to construct it with a
/// function representative of the signature you want to use, then call
/// ExecuteFunction(ExecutionContext &, Stream &, Value &).
///
/// If you need to reuse the arguments for several calls, you can call
/// InsertFunction() followed by WriteFunctionArguments(), which will return
/// the location of the args struct for the wrapper function in args_addr_ref.
///
/// If you need to call the function on the thread plan stack, you can also
/// call InsertFunction() followed by GetThreadPlanToCallFunction().
///
/// Any of the methods that take arg_addr_ptr or arg_addr_ref can be passed a
/// pointer set to LLDB_INVALID_ADDRESS and new structure will be allocated
/// and its address returned in that variable.
///
/// Any of the methods that take arg_addr_ptr can be passed NULL, and the
/// argument space will be managed for you.
class ClangFunctionCaller : public FunctionCaller {
  friend class ASTStructExtractor;

  class ClangFunctionCallerHelper
      : public llvm::RTTIExtends<ClangFunctionCallerHelper,
                                 ClangExpressionHelper> {
  public:
    // LLVM RTTI support
    static char ID;

    ClangFunctionCallerHelper(ClangFunctionCaller &owner) : m_owner(owner) {}

    /// Return the object that the parser should use when resolving external
    /// values.  May be NULL if everything should be self-contained.
    ClangExpressionDeclMap *DeclMap() override { return nullptr; }

    /// Return the object that the parser should allow to access ASTs. May be
    /// NULL if the ASTs do not need to be transformed.
    ///
    /// \param[in] passthrough
    ///     The ASTConsumer that the returned transformer should send
    ///     the ASTs to after transformation.
    clang::ASTConsumer *
    ASTTransformer(clang::ASTConsumer *passthrough) override;

  private:
    ClangFunctionCaller &m_owner;
    std::unique_ptr<ASTStructExtractor> m_struct_extractor; ///< The class that
                                                            ///generates the
                                                            ///argument struct
                                                            ///layout.
  };

  // LLVM RTTI support
  static char ID;

public:
  bool isA(const void *ClassID) const override {
    return ClassID == &ID || FunctionCaller::isA(ClassID);
  }
  static bool classof(const Expression *obj) { return obj->isA(&ID); }

  /// Constructor
  ///
  /// \param[in] exe_scope
  ///     An execution context scope that gets us at least a target and
  ///     process.
  ///
  /// \param[in] return_type
  ///     A compiler type for the function result.  Should be
  ///     defined in ast_context.
  ///
  /// \param[in] function_address
  ///     The address of the function to call.
  ///
  /// \param[in] arg_value_list
  ///     The default values to use when calling this function.  Can
  ///     be overridden using WriteFunctionArguments().
  ClangFunctionCaller(ExecutionContextScope &exe_scope,
                      const CompilerType &return_type,
                      const Address &function_address,
                      const ValueList &arg_value_list, const char *name);

  ~ClangFunctionCaller() override;

  /// Compile the wrapper function
  ///
  /// \param[in] thread_to_use_sp
  ///     Compilation might end up calling functions.  Pass in the thread you
  ///     want the compilation to use.  If you pass in an empty ThreadSP it will
  ///     use the currently selected thread.
  ///
  /// \param[in] diagnostic_manager
  ///     The diagnostic manager to report parser errors to.
  ///
  /// \return
  ///     The number of errors.
  unsigned CompileFunction(lldb::ThreadSP thread_to_use_sp,
                           DiagnosticManager &diagnostic_manager) override;

  ExpressionTypeSystemHelper *GetTypeSystemHelper() override {
    return &m_type_system_helper;
  }

protected:
  const char *GetWrapperStructName() { return m_wrapper_struct_name.c_str(); }

private:
  // For ClangFunctionCaller only

  // Note: the parser needs to be destructed before the execution unit, so
  // declare the execution unit first.
  ClangFunctionCallerHelper m_type_system_helper;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGFUNCTIONCALLER_H
