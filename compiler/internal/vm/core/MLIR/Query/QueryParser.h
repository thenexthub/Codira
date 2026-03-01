//===--- QueryParser.h - ----------------------------------------*- C++ -*-===//
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

#ifndef MLIR_TOOLS_MLIRQUERY_QUERYPARSER_H
#define MLIR_TOOLS_MLIRQUERY_QUERYPARSER_H

#include "Matcher/Parser.h"
#include "mlir/Query/Query.h"
#include "mlir/Query/QuerySession.h"

#include "vm/core/ADT/StringRef.h"
#include "vm/core/LineEditor/LineEditor.h"

namespace mlir::query {

class QuerySession;

class QueryParser {
public:
  // Parse line as a query and return a QueryRef representing the query, which
  // may be an InvalidQuery.
  static QueryRef parse(toolchain::StringRef line, const QuerySession &qs);

  static std::vector<toolchain::LineEditor::Completion>
  complete(toolchain::StringRef line, size_t pos, const QuerySession &qs);

private:
  QueryParser(toolchain::StringRef line, const QuerySession &qs)
      : line(line), completionPos(nullptr), qs(qs) {}

  toolchain::StringRef lexWord();

  template <typename T>
  struct LexOrCompleteWord;

  QueryRef completeMatcherExpression();

  QueryRef endQuery(QueryRef queryRef);

  // Parse [begin, end) and returns a reference to the parsed query object,
  // which may be an InvalidQuery if a parse error occurs.
  QueryRef doParse();

  toolchain::StringRef line;

  const char *completionPos;
  std::vector<toolchain::LineEditor::Completion> completions;

  const QuerySession &qs;
};

} // namespace mlir::query

#endif // MLIR_TOOLS_MLIRQUERY_QUERYPARSER_H
