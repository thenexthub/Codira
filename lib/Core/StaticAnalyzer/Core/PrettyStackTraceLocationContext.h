//==- PrettyStackTraceLocationContext.h - show analysis backtrace --*- C++ -*-//
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

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CORE_PRETTYSTACKTRACELOCATIONCONTEXT_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CORE_PRETTYSTACKTRACELOCATIONCONTEXT_H

#include "language/Core/Analysis/AnalysisDeclContext.h"

namespace language::Core {
namespace ento {

/// While alive, includes the current analysis stack in a crash trace.
///
/// Example:
/// \code
/// 0.     Program arguments: ...
/// 1.     <eof> parser at end of file
/// 2.     While analyzing stack:
///        #0 void inlined()
///        #1 void test()
/// 3.     crash-trace.c:6:3: Error evaluating statement
/// \endcode
class PrettyStackTraceLocationContext : public toolchain::PrettyStackTraceEntry {
  const LocationContext *LCtx;
public:
  PrettyStackTraceLocationContext(const LocationContext *LC) : LCtx(LC) {
    assert(LCtx);
  }

  void print(raw_ostream &Out) const override {
    Out << "While analyzing stack: \n";
    LCtx->dumpStack(Out);
  }
};

} // end ento namespace
} // end clang namespace

#endif
