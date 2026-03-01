//===-- ClangREPL.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_REPL_CLANG_CLANGREPL_H
#define LLDB_SOURCE_PLUGINS_REPL_CLANG_CLANGREPL_H

#include "lldb/Expression/REPL.h"

namespace lldb_private {
/// Implements a Clang-based REPL for C languages on top of LLDB's REPL
/// framework.
class ClangREPL : public llvm::RTTIExtends<ClangREPL, REPL> {
public:
  // LLVM RTTI support
  static char ID;

  ClangREPL(lldb::LanguageType language, Target &target);

  ~ClangREPL() override;

  static void Initialize();

  static void Terminate();

  static lldb::REPLSP CreateInstance(Status &error, lldb::LanguageType language,
                                     Debugger *debugger, Target *target,
                                     const char *repl_options);

  static llvm::StringRef GetPluginNameStatic() { return "ClangREPL"; }

protected:
  Status DoInitialization() override;

  llvm::StringRef GetSourceFileBasename() override;

  const char *GetAutoIndentCharacters() override;

  bool SourceIsComplete(const std::string &source) override;

  lldb::offset_t GetDesiredIndentation(const StringList &lines,
                                       int cursor_position,
                                       int tab_size) override;

  lldb::LanguageType GetLanguage() override;

  bool PrintOneVariable(Debugger &debugger, lldb::StreamFileSP &output_sp,
                        lldb::ValueObjectSP &valobj_sp,
                        ExpressionVariable *var = nullptr) override;

  void CompleteCode(const std::string &current_code,
                    CompletionRequest &request) override;

private:
  /// The specific C language of this REPL.
  lldb::LanguageType m_language;
  /// A regex matching the implicitly created LLDB result variables.
  lldb_private::RegularExpression m_implicit_expr_result_regex;
};
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_REPL_CLANG_CLANGREPL_H
