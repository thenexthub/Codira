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

#ifndef CODIRA_CHIR_TRANSFORMATION_FLAT_FORIN_EXPR_H
#define CODIRA_CHIR_TRANSFORMATION_FLAT_FORIN_EXPR_H

#include "Codira/CHIR/CHIRContext.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"

namespace Codira::CHIR {
/**
 * CHIR normaol Pass: mainly flat ForIn Expression, generate standard CHIR IR to replace ForIn Expr.
 */
class FlatForInExpr {
public:
    /**
     * @brief constructor to flat ForIn Expression.
     * @param builder CHIR builder for generating IR.
     */
    explicit FlatForInExpr(CHIRBuilder& builder);

    /**
     * @brief Main process to flat ForIn Expression.
     * @param package package to do optimization.
     */
    void RunOnPackage(const Package& package);

private:
    CHIRBuilder& builder;

    void RunOnFunc(Func& func);

    void RunOnBlockGroup(BlockGroup& blockGroup);

    using ExprIt = std::vector<Expression*>::iterator;
    // for-in-iter before translation
    // %0 = Apply(iterator, for-value)
    // %1 = Allocate(Int64) // delay exit signal
    // %2 = Allocate(Enum-Option<...>) // iterator var
    // %3 = Allocate(Bool) // cond var
    // Store(Constant(0), %1)
    // Store(Tuple(Constant(1)), %2) // init value is None, unused
    // Store(Constant(true), %3) // init value is true, unused
    // %4 = For(%2, %3) {
    //     #2: body
    //         do body things...
    //     #3: latch
    //         %5 = Apply(next, %0)
    //         Store(%5, %2)
    //     #4: cond
    //         %6 = TypeCast(Enum-Option<...>, %5)
    //         %7 = Field(%6, 0)
    //         %8 = Not(%7)
    //         Store(%8, %3)
    // }
    // GoTo(delay-exit-true-block)

    // after translation
    // for-in-iter after translation
    // %0 = Apply(iterator, for-value)
    // %1 = Allocate(Int64) // delay exit signal
    // %2 = Allocate(Enum-Option<...>) // iterator var
    // %3 = Allocate(Bool) // cond var
    // Store(Constant(0), %1)
    // Store(Tuple(Constant(1)), %2) // init value is None, unused
    // Store(Constant(true), %3) // init value is true, unused
    // GoTo(#3)
    // #3: latch
    //     %5 = Apply(next, %0)
    //     Store(%5, %2)
    //     GoTo(#4)
    // #4: cond
    //     %6 = TypeCast(Enum-Option<...>, %5)
    //     %7 = Field(%6, 0)
    //     %8 = Not(%7)
    //     Store(%8, %3)
    //     GoTo(#5) // new block, named jump block
    // #2: body
    //     do body things...
    //     GoTo(#3)
    // #5: jump block
    //     %9 = Load(%3)
    //     Branch(%9, #2, delay-exit-true-block)
    /// \param it the iterator to forIn
    /// \param end the iterator of the end of the block containing the forIn expression
    void FlatternForInExpr(ExprIt it, ExprIt end);
    void FlatternForInClosedRange(ExprIt it, ExprIt end);
};
} // namespace Codira::CHIR
#endif
