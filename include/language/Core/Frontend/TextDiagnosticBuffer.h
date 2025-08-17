//===- TextDiagnosticBuffer.h - Buffer Text Diagnostics ---------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
// This is a concrete diagnostic client, which buffers the diagnostic messages.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_FRONTEND_TEXTDIAGNOSTICBUFFER_H
#define LANGUAGE_CORE_FRONTEND_TEXTDIAGNOSTICBUFFER_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/SourceLocation.h"
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace language::Core {

class TextDiagnosticBuffer : public DiagnosticConsumer {
public:
  using DiagList = std::vector<std::pair<SourceLocation, std::string>>;
  using iterator = DiagList::iterator;
  using const_iterator = DiagList::const_iterator;

private:
  DiagList Errors, Warnings, Remarks, Notes;

  /// All - All diagnostics in the order in which they were generated.  That
  /// order likely doesn't correspond to user input order, but it at least
  /// keeps notes in the right places.  Each pair in the vector is a diagnostic
  /// level and an index into the corresponding DiagList above.
  std::vector<std::pair<DiagnosticsEngine::Level, size_t>> All;

public:
  const_iterator err_begin() const { return Errors.begin(); }
  const_iterator err_end() const { return Errors.end(); }

  const_iterator warn_begin() const { return Warnings.begin(); }
  const_iterator warn_end() const { return Warnings.end(); }

  const_iterator remark_begin() const { return Remarks.begin(); }
  const_iterator remark_end() const { return Remarks.end(); }

  const_iterator note_begin() const { return Notes.begin(); }
  const_iterator note_end() const { return Notes.end(); }

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override;

  /// FlushDiagnostics - Flush the buffered diagnostics to an given
  /// diagnostic engine.
  void FlushDiagnostics(DiagnosticsEngine &Diags) const;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_FRONTEND_TEXTDIAGNOSTICBUFFER_H
