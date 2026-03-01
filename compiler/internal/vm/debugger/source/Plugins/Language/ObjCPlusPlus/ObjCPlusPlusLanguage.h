//===-- ObjCPlusPlusLanguage.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGE_OBJCPLUSPLUS_OBJCPLUSPLUSLANGUAGE_H
#define LLDB_SOURCE_PLUGINS_LANGUAGE_OBJCPLUSPLUS_OBJCPLUSPLUSLANGUAGE_H

#include "Plugins/Language/ClangCommon/ClangHighlighter.h"
#include "lldb/Target/Language.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class ObjCPlusPlusLanguage : public Language {
  ClangHighlighter m_highlighter;

public:
  ObjCPlusPlusLanguage() = default;

  ~ObjCPlusPlusLanguage() override = default;

  lldb::LanguageType GetLanguageType() const override {
    return lldb::eLanguageTypeObjC_plus_plus;
  }

  llvm::StringRef GetUserEntryPointName() const override { return "main"; }

  llvm::StringRef GetNilReferenceSummaryString() override { return "nil"; }

  bool IsSourceFile(llvm::StringRef file_path) const override;

  const Highlighter *GetHighlighter() const override { return &m_highlighter; }

  // Static Functions
  static void Initialize();

  static void Terminate();

  static lldb_private::Language *CreateInstance(lldb::LanguageType language);

  llvm::StringRef GetInstanceVariableName() override { return "self"; }

  virtual std::optional<bool>
  GetBooleanFromString(llvm::StringRef str) const override;

  static llvm::StringRef GetPluginNameStatic() { return "objcplusplus"; }

  // PluginInterface protocol
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGE_OBJCPLUSPLUS_OBJCPLUSPLUSLANGUAGE_H
