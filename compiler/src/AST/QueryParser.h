/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file declares the QueryParser.
 */

#ifndef CODIRA_AST_QUERY_PARSER_H
#define CODIRA_AST_QUERY_PARSER_H

#include <optional>
#include <string>

#include "Codira/AST/Query.h"
#include "Codira/Parse/Parser.h"

namespace Codira {
/**
 * @brief The main class used for parsing query statements.
 */
class QueryParser : public Parser {
public:
    template <typename... Args> explicit QueryParser(Args&&... args) : Parser(std::forward<Args>(args)...)
    {
    }
    /**
     * @brief The main parse entry.
     */
    std::unique_ptr<Query> Parse();

private:
    /**
     * @brief Parse boolean clause with parens, like (a:b || c:d).
     */
    std::unique_ptr<Query> ParseParenClause();
    /**
     * @brief Parse boolean clause, like a:b && c:d.
     */
    std::unique_ptr<Query> ParseBooleanClause();
    /**
     * @brief Parse normal term, like a:b.
     */
    std::unique_ptr<Query> ParseNormalTerm();
    /**
     * @brief Parse position term, like _ < (1,2,3).
     */
    std::unique_ptr<Query> ParsePositionTerm();
    /**
     * @brief Parse comparator sign, '=', '>', '>=', '<', '<='.
     */
    std::optional<std::string> ParseComparator();
    /**
     * @brief Parse term entry.
     */
    std::unique_ptr<Query> ParseTerm();

    bool parsingParenClause{false};
};
} // namespace Codira

#endif
