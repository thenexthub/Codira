//===--- Parsing.h - Parsing library for Transformer ------------*- C++ -*-===//
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
///  \file
///  Defines parsing functions for Transformer types.
///  FIXME: Currently, only supports `RangeSelectors` but parsers for other
///  Transformer types are under development.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_TRANSFORMER_PARSING_H
#define LANGUAGE_CORE_TOOLING_TRANSFORMER_PARSING_H

#include "language/Core/ASTMatchers/ASTMatchFinder.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Tooling/Transformer/RangeSelector.h"
#include "toolchain/Support/Error.h"
#include <functional>

namespace language::Core {
namespace transformer {

/// Parses a string representation of a \c RangeSelector. The grammar of these
/// strings is closely based on the (sub)grammar of \c RangeSelectors as they'd
/// appear in C++ code. However, this language constrains the set of permissible
/// strings (for node ids) -- it does not support escapes in the
/// string. Additionally, the \c charRange combinator is not supported, because
/// there is no representation of values of type \c CharSourceRange in this
/// (little) language.
toolchain::Expected<RangeSelector> parseRangeSelector(toolchain::StringRef Input);

} // namespace transformer
} // namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_TRANSFORMER_PARSING_H
