//===-- language/Compability/Semantics/unparse-with-symbols.h ----------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_UNPARSE_WITH_SYMBOLS_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_UNPARSE_WITH_SYMBOLS_H_

#include "language/Compability/Parser/characters.h"
#include <iosfwd>

namespace toolchain {
class raw_ostream;
}

namespace language::Compability::common {
class LangOptions;
}

namespace language::Compability::parser {
struct Program;
}

namespace language::Compability::semantics {
class SemanticsContext;
void UnparseWithSymbols(toolchain::raw_ostream &, const parser::Program &,
    const common::LangOptions &,
    parser::Encoding encoding = parser::Encoding::UTF_8);
void UnparseWithModules(toolchain::raw_ostream &, SemanticsContext &,
    const parser::Program &,
    parser::Encoding encoding = parser::Encoding::UTF_8);
}

#endif // FORTRAN_SEMANTICS_UNPARSE_WITH_SYMBOLS_H_
