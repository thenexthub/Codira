//===-- MSVCUndecoratedNameParser.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGE_CPLUSPLUS_MSVCUNDECORATEDNAMEPARSER_H
#define LLDB_SOURCE_PLUGINS_LANGUAGE_CPLUSPLUS_MSVCUNDECORATEDNAMEPARSER_H

#include <vector>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

class MSVCUndecoratedNameSpecifier {
public:
  MSVCUndecoratedNameSpecifier(llvm::StringRef full_name,
                               llvm::StringRef base_name)
      : m_full_name(full_name), m_base_name(base_name) {}

  llvm::StringRef GetFullName() const { return m_full_name; }
  llvm::StringRef GetBaseName() const { return m_base_name; }

private:
  llvm::StringRef m_full_name;
  llvm::StringRef m_base_name;
};

class MSVCUndecoratedNameParser {
public:
  explicit MSVCUndecoratedNameParser(llvm::StringRef name);

  llvm::ArrayRef<MSVCUndecoratedNameSpecifier> GetSpecifiers() const {
    return m_specifiers;
  }

  static bool IsMSVCUndecoratedName(llvm::StringRef name);
  static bool ExtractContextAndIdentifier(llvm::StringRef name,
                                          llvm::StringRef &context,
                                          llvm::StringRef &identifier);

  static llvm::StringRef DropScope(llvm::StringRef name);

private:
  std::vector<MSVCUndecoratedNameSpecifier> m_specifiers;
};

#endif
