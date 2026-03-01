//===-- ClangDiagnostic.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGDIAGNOSTIC_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGDIAGNOSTIC_H

#include <vector>

#include "clang/Basic/Diagnostic.h"

#include "lldb/lldb-defines.h"
#include "lldb/lldb-types.h"

#include "lldb/Expression/DiagnosticManager.h"

namespace lldb_private {

class ClangDiagnostic : public Diagnostic {
public:
  typedef std::vector<clang::FixItHint> FixItList;

  static inline bool classof(const ClangDiagnostic *) { return true; }
  static inline bool classof(const Diagnostic *diag) {
    return diag->getKind() == eDiagnosticOriginClang;
  }

  ClangDiagnostic(DiagnosticDetail detail, uint32_t compiler_id)
      : Diagnostic(eDiagnosticOriginClang, compiler_id, detail) {}

  ~ClangDiagnostic() override = default;

  bool HasFixIts() const override { return !m_fixit_vec.empty(); }

  void AddFixitHint(const clang::FixItHint &fixit) {
    m_fixit_vec.push_back(fixit);
  }

  const FixItList &FixIts() const { return m_fixit_vec; }
private:
  FixItList m_fixit_vec;
};

} // namespace lldb_private
#endif // LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGDIAGNOSTIC_H
