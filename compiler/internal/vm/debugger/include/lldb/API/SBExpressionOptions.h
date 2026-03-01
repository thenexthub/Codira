//===-- SBExpressionOptions.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBEXPRESSIONOPTIONS_H
#define LLDB_API_SBEXPRESSIONOPTIONS_H

#include "lldb/API/SBDefines.h"
#include "lldb/API/SBLanguages.h"

#include <vector>

namespace lldb {

class LLDB_API SBExpressionOptions {
public:
  SBExpressionOptions();

  SBExpressionOptions(const lldb::SBExpressionOptions &rhs);

  ~SBExpressionOptions();

  const SBExpressionOptions &operator=(const lldb::SBExpressionOptions &rhs);

  bool GetCoerceResultToId() const;

  void SetCoerceResultToId(bool coerce = true);

  bool GetUnwindOnError() const;

  void SetUnwindOnError(bool unwind = true);

  bool GetIgnoreBreakpoints() const;

  void SetIgnoreBreakpoints(bool ignore = true);

  lldb::DynamicValueType GetFetchDynamicValue() const;

  void SetFetchDynamicValue(
      lldb::DynamicValueType dynamic = lldb::eDynamicCanRunTarget);

  uint32_t GetTimeoutInMicroSeconds() const;

  // Set the timeout for the expression, 0 means wait forever.
  void SetTimeoutInMicroSeconds(uint32_t timeout = 0);

  uint32_t GetOneThreadTimeoutInMicroSeconds() const;

  // Set the timeout for running on one thread, 0 means use the default
  // behavior. If you set this higher than the overall timeout, you'll get an
  // error when you try to run the expression.
  void SetOneThreadTimeoutInMicroSeconds(uint32_t timeout = 0);

  bool GetTryAllThreads() const;

  void SetTryAllThreads(bool run_others = true);

  bool GetStopOthers() const;

  void SetStopOthers(bool stop_others = true);

  bool GetTrapExceptions() const;

  void SetTrapExceptions(bool trap_exceptions = true);

  void SetLanguage(lldb::LanguageType language);
  /// Set the language using a pair of language code and version as
  /// defined by the DWARF 6 specification.
  /// WARNING: These codes may change until DWARF 6 is finalized.
  void SetLanguage(lldb::SBSourceLanguageName name, uint32_t version);

#ifndef SWIG
  void SetCancelCallback(lldb::ExpressionCancelCallback callback, void *baton);
#endif

  bool GetGenerateDebugInfo();

  void SetGenerateDebugInfo(bool b = true);

  bool GetSuppressPersistentResult();

  void SetSuppressPersistentResult(bool b = false);

  const char *GetPrefix() const;

  void SetPrefix(const char *prefix);

  void SetAutoApplyFixIts(bool b = true);

  bool GetAutoApplyFixIts();

  void SetRetriesWithFixIts(uint64_t retries);

  uint64_t GetRetriesWithFixIts();

  bool GetTopLevel();

  void SetTopLevel(bool b = true);

  // Gets whether we will JIT an expression if it cannot be interpreted
  bool GetAllowJIT();

  // Sets whether we will JIT an expression if it cannot be interpreted
  void SetAllowJIT(bool allow);

protected:
  lldb_private::EvaluateExpressionOptions *get() const;

  lldb_private::EvaluateExpressionOptions &ref() const;

  friend class SBFrame;
  friend class SBValue;
  friend class SBTarget;

private:
  // This auto_pointer is made in the constructor and is always valid.
  mutable std::unique_ptr<lldb_private::EvaluateExpressionOptions> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBEXPRESSIONOPTIONS_H
