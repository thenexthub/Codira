//===-- DynamicCheckerFunctions.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_EXPRESSION_DYNAMICCHECKERFUNCTIONS_H
#define LLDB_EXPRESSION_DYNAMICCHECKERFUNCTIONS_H

#include "lldb/lldb-types.h"

#include "llvm/Support/Error.h"

namespace lldb_private {

class DiagnosticManager;
class ExecutionContext;

/// Encapsulates dynamic check functions used by expressions.
///
/// Each of the utility functions encapsulated in this class is responsible
/// for validating some data that an expression is about to use.  Examples
/// are:
///
/// a = *b;     // check that b is a valid pointer
/// [b init];   // check that b is a valid object to send "init" to
///
/// The class installs each checker function into the target process and makes
/// it available to IRDynamicChecks to use.
class DynamicCheckerFunctions {
public:
  enum DynamicCheckerFunctionsKind {
    DCF_Clang,
  };

  DynamicCheckerFunctions(DynamicCheckerFunctionsKind kind) : m_kind(kind) {}
  virtual ~DynamicCheckerFunctions() = default;

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
  virtual llvm::Error Install(DiagnosticManager &diagnostic_manager,
                              ExecutionContext &exe_ctx) = 0;
  virtual bool DoCheckersExplainStop(lldb::addr_t addr, Stream &message) = 0;

  DynamicCheckerFunctionsKind GetKind() const { return m_kind; }

private:
  const DynamicCheckerFunctionsKind m_kind;
};
} // namespace lldb_private

#endif // LLDB_EXPRESSION_DYNAMICCHECKERFUNCTIONS_H
