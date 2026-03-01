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
 * This file declares the Simple Iterator in CHIR.
 */

#ifndef CODIRA_CHIR_SIMPLEITERATOR_H
#define CODIRA_CHIR_SIMPLEITERATOR_H

#include <vector>

namespace Codira::CHIR {
class Expression;
class Block;
class BlockGroup;
/*
 * @brief Simple Iterator for CHIR node.
 *
 */
class SimpleIterator {
public:
    /**
     * @brief Iterates over an expression and returns a vector of block groups.
     *
     * @param expr The expression to iterate over.
     * @return A vector of block groups.
     */
    static std::vector<BlockGroup*> Iterate(const Expression& expr);
    
    /**
     * @brief Iterates over a block group and returns a vector of blocks.
     *
     * @param blockGroup The block group to iterate over.
     * @return A vector of blocks.
     */
    static std::vector<Block*> Iterate(const BlockGroup& blockGroup);
    
    /**
     * @brief Iterates over a block and returns a vector of expressions.
     *
     * @param block The block to iterate over.
     * @return A vector of expressions.
     */
    static std::vector<Expression*> Iterate(const Block& block);
};

} // namespace Codira::CHIR
#endif // CODIRA_CHIR_SIMPLEITERATOR_H
