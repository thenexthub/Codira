//===-- language/Compability/Parser/unparse.h --------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_PARSER_UNPARSE_H_
#define LANGUAGE_COMPABILITY_PARSER_UNPARSE_H_

#include "char-block.h"
#include "characters.h"
#include <functional>
#include <iosfwd>

namespace toolchain {
class raw_ostream;
}

namespace language::Compability::common {
class LangOptions;
}

namespace language::Compability::evaluate {
struct GenericExprWrapper;
struct GenericAssignmentWrapper;
class ProcedureRef;
} // namespace language::Compability::evaluate

namespace language::Compability::parser {

struct Program;
struct Expr;

// A function called before each Statement is unparsed.
using preStatementType =
    std::function<void(const CharBlock &, toolchain::raw_ostream &, int)>;

// Functions to handle unparsing of analyzed expressions and related
// objects rather than their original parse trees.
struct AnalyzedObjectsAsFortran {
  std::function<void(toolchain::raw_ostream &, const evaluate::GenericExprWrapper &)>
      expr;
  std::function<void(
      toolchain::raw_ostream &, const evaluate::GenericAssignmentWrapper &)>
      assignment;
  std::function<void(toolchain::raw_ostream &, const evaluate::ProcedureRef &)> call;
};

// Converts parsed program (or fragment) to out as Fortran.
template <typename A>
void Unparse(toolchain::raw_ostream &out, const A &root,
    const common::LangOptions &langOpts, Encoding encoding = Encoding::UTF_8,
    bool capitalizeKeywords = true, bool backslashEscapes = true,
    preStatementType *preStatement = nullptr,
    AnalyzedObjectsAsFortran * = nullptr);

extern template void Unparse(toolchain::raw_ostream &out, const Program &program,
    const common::LangOptions &langOpts, Encoding encoding,
    bool capitalizeKeywords, bool backslashEscapes,
    preStatementType *preStatement, AnalyzedObjectsAsFortran *);
extern template void Unparse(toolchain::raw_ostream &out, const Expr &expr,
    const common::LangOptions &langOpts, Encoding encoding,
    bool capitalizeKeywords, bool backslashEscapes,
    preStatementType *preStatement, AnalyzedObjectsAsFortran *);
} // namespace language::Compability::parser

#endif
