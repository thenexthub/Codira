//===----- FormatStringParsing.h - Format String Parsing --------*- C++ -*-===//
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
// This provides some shared functions between printf and scanf format string
// parsing code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_ANALYSIS_FORMATSTRINGPARSING_H
#define LANGUAGE_CORE_LIB_ANALYSIS_FORMATSTRINGPARSING_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Type.h"
#include "language/Core/AST/FormatString.h"

namespace language::Core {

class LangOptions;

template <typename T>
class UpdateOnReturn {
  T &ValueToUpdate;
  const T &ValueToCopy;
public:
  UpdateOnReturn(T &valueToUpdate, const T &valueToCopy)
    : ValueToUpdate(valueToUpdate), ValueToCopy(valueToCopy) {}

  ~UpdateOnReturn() {
    ValueToUpdate = ValueToCopy;
  }
};

namespace analyze_format_string {

OptionalAmount ParseAmount(const char *&Beg, const char *E);
OptionalAmount ParseNonPositionAmount(const char *&Beg, const char *E,
                                      unsigned &argIndex);

OptionalAmount ParsePositionAmount(FormatStringHandler &H,
                                   const char *Start, const char *&Beg,
                                   const char *E, PositionContext p);

bool ParseFieldWidth(FormatStringHandler &H,
                     FormatSpecifier &CS,
                     const char *Start, const char *&Beg, const char *E,
                     unsigned *argIndex);

bool ParseArgPosition(FormatStringHandler &H,
                      FormatSpecifier &CS, const char *Start,
                      const char *&Beg, const char *E);

bool ParseVectorModifier(FormatStringHandler &H,
                         FormatSpecifier &FS, const char *&Beg, const char *E,
                         const LangOptions &LO);

/// Returns true if a LengthModifier was parsed and installed in the
/// FormatSpecifier& argument, and false otherwise.
bool ParseLengthModifier(FormatSpecifier &FS, const char *&Beg, const char *E,
                         const LangOptions &LO, bool IsScanf = false);

/// Returns true if the invalid specifier in \p SpecifierBegin is a UTF-8
/// string; check that it won't go further than \p FmtStrEnd and write
/// up the total size in \p Len.
bool ParseUTF8InvalidSpecifier(const char *SpecifierBegin,
                               const char *FmtStrEnd, unsigned &Len);

template <typename T> class SpecifierResult {
  T FS;
  const char *Start;
  bool Stop;
public:
  SpecifierResult(bool stop = false)
  : Start(nullptr), Stop(stop) {}
  SpecifierResult(const char *start,
                  const T &fs)
  : FS(fs), Start(start), Stop(false) {}

  const char *getStart() const { return Start; }
  bool shouldStop() const { return Stop; }
  bool hasValue() const { return Start != nullptr; }
  const T &getValue() const {
    assert(hasValue());
    return FS;
  }
  const T &getValue() { return FS; }
};

} // end analyze_format_string namespace
} // end clang namespace

#endif
