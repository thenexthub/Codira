//===- clang/Basic/PrettyStackTrace.h - Pretty Crash Handling --*- C++ -*-===//
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
///
/// \file
/// Defines the PrettyStackTraceEntry class, which is used to make
/// crashes give more contextual information about what the program was doing
/// when it crashed.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_PRETTYSTACKTRACE_H
#define LANGUAGE_CORE_BASIC_PRETTYSTACKTRACE_H

#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/Support/PrettyStackTrace.h"

namespace language::Core {

  /// If a crash happens while one of these objects are live, the message
  /// is printed out along with the specified source location.
  class PrettyStackTraceLoc : public toolchain::PrettyStackTraceEntry {
    SourceManager &SM;
    SourceLocation Loc;
    const char *Message;
  public:
    PrettyStackTraceLoc(SourceManager &sm, SourceLocation L, const char *Msg)
      : SM(sm), Loc(L), Message(Msg) {}
    void print(raw_ostream &OS) const override;
  };
}

#endif
