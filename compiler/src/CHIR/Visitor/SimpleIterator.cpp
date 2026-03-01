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
 * This file implements the SimpleIterator Class in CHIR.
 */

#include "Codira/CHIR/Visitor/SimpleIterator.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Value.h"
#include "Codira/CHIR/CHIRCasting.h"

using namespace Codira::CHIR;

std::vector<BlockGroup*> SimpleIterator::Iterate(const Expression& expr)
{
    auto kind{expr.GetExprKind()};
    if (kind == ExprKind::IF || kind == ExprKind::LOOP || Is<ForIn>(expr)) {
        return expr.GetBlockGroups();
    }
    return {};
}

std::vector<Block*> SimpleIterator::Iterate(const BlockGroup& blockGroup)
{
    return blockGroup.GetBlocks();
}

std::vector<Expression*> SimpleIterator::Iterate(const Block& block)
{
    return block.GetExpressions();
}
