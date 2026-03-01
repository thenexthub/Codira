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
 * This file declares the Query and related classes.
 */

#ifndef CODIRA_AST_QUERY_H
#define CODIRA_AST_QUERY_H

#include <memory>
#include <string>
#include <unordered_set>

#include "Codira/Basic/Position.h"

namespace Codira {
/**
 * Operations on the Query node.
 */
enum class Operator {
    AND, /**< Condition1 && Condition2. */
    OR,  /**< Condition1 || Condition2. */
    NOT, /**< Condition1 !  Condition2. */
};

/**
 * The type of the Query node.
 */
enum class QueryType { OP, STRING, POS, NONE };

/**
 * Search term match kind.
 */
enum class MatchKind {
    PRECISE, /**< Query string "name: foo". */
    PREFIX,  /**< Query string "name: foo*". */
    SUFFIX   /**< Query string "name: *foo". */
};

/**
 * Query is a query tree, Leaf Node represent the query, none leaf node is query condition.
 */
struct Query {
    Query(std::string key, std::string value) : key(std::move(key)), value(std::move(value))
    {
    }
    Query(std::string key, std::string value, MatchKind matchKind)
        : key(std::move(key)), value(std::move(value)), matchKind(matchKind)
    {
    }
    explicit Query(Operator op) : op(op)
    {
        type = QueryType::OP;
    }
    Query() = default;
    std::string key;                         /**< Leaf node's key. */
    std::string value;                       /**< Leaf node's value. */
    std::unordered_set<uint64_t> fileHashes; /**< For filter certain files. */
    Position pos;                            /**< Save the Position value. */
    std::string sign{"="};                   /**< Only position filed support '=', '<', '<='. */
    QueryType type{QueryType::NONE};
    std::unique_ptr<Query> left;
    std::unique_ptr<Query> right;
    Operator op{Operator::AND};
    MatchKind matchKind{MatchKind::PRECISE};
    void PrettyPrint(std::string& result) const;
};
} // namespace Codira

#endif
