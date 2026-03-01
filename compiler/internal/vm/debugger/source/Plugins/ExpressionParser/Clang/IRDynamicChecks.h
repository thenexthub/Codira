//===-- IRDynamicChecks.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_IRDYNAMICCHECKS_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_IRDYNAMICCHECKS_H

#include "lldb/Expression/DynamicCheckerFunctions.h"
#include "lldb/lldb-types.h"
#include "llvm/Pass.h"

namespace llvm {
class BasicBlock;
class Module;
}

namespace lldb_private {

class ExecutionContext;
class Stream;

class ClangDynamicCheckerFunctions
    : public lldb_private::DynamicCheckerFunctions {
public:
  /// Constructor
  ClangDynamicCheckerFunctions();

  /// Destructor
  ~ClangDynamicCheckerFunctions() override;

  static bool classof(const DynamicCheckerFunctions *checker_funcs) {
    return checker_funcs->GetKind() == DCF_Clang;
  }

  /// Install the utility functions into a process.  This binds the instance
  /// of DynamicCheckerFunctions to that process.
  ///
  /// \param[in] diagnostic_manager
  ///     A diagnostic manager to report errors to.
  ///
  /// \param[in] exe_ctx
  ///     The execution context to install the functions into.
  ///
  /// \return
  ///     Either llvm::ErrorSuccess or Error with llvm::ErrorInfo
  ///
  llvm::Error Install(DiagnosticManager &diagnostic_manager,
                      ExecutionContext &exe_ctx) override;

  bool DoCheckersExplainStop(lldb::addr_t addr, Stream &message) override;

  std::shared_ptr<UtilityFunction> m_objc_object_check;
};

/// \class IRDynamicChecks IRDynamicChecks.h
/// "lldb/Expression/IRDynamicChecks.h" Adds dynamic checks to a user-entered
/// expression to reduce its likelihood of crashing
///
/// When an IR function is executed in the target process, it may cause
/// crashes or hangs by dereferencing NULL pointers, trying to call
/// Objective-C methods on objects that do not respond to them, and so forth.
///
/// IRDynamicChecks adds calls to the functions in DynamicCheckerFunctions to
/// appropriate locations in an expression's IR.
class IRDynamicChecks : public llvm::ModulePass {
public:
  /// Constructor
  ///
  /// \param[in] checker_functions
  ///     The checker functions for the target process.
  ///
  /// \param[in] func_name
  ///     The name of the function to prepare for execution in the target.
  IRDynamicChecks(ClangDynamicCheckerFunctions &checker_functions,
                  const char *func_name = "$__lldb_expr");

  /// Destructor
  ~IRDynamicChecks() override;

  /// Run this IR transformer on a single module
  ///
  /// \param[in] M
  ///     The module to run on.  This module is searched for the function
  ///     $__lldb_expr, and that function is passed to the passes one by
  ///     one.
  ///
  /// \return
  ///     True on success; false otherwise
  bool runOnModule(llvm::Module &M) override;

  /// Interface stub
  void assignPassManager(
      llvm::PMStack &PMS,
      llvm::PassManagerType T = llvm::PMT_ModulePassManager) override;

  /// Returns PMT_ModulePassManager
  llvm::PassManagerType getPotentialPassManagerType() const override;

private:
  /// A basic block-level pass to find all pointer dereferences and
  /// validate them before use.

  /// The top-level pass implementation
  ///
  /// \param[in] M
  ///     The module currently being processed.
  ///
  /// \param[in] BB
  ///     The basic block currently being processed.
  ///
  /// \return
  ///     True on success; false otherwise
  bool FindDataLoads(llvm::Module &M, llvm::BasicBlock &BB);

  std::string m_func_name; ///< The name of the function to add checks to
  ClangDynamicCheckerFunctions
      &m_checker_functions; ///< The checker functions for the process
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_IRDYNAMICCHECKS_H
