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
 * This file instantiate functions for CHIR pass Function Inline and Devirtualization
 */

#ifndef CODIRA_CHIR_TRANSFORMATION_BLOCK_GROUP_COPY_HELPER_H
#define CODIRA_CHIR_TRANSFORMATION_BLOCK_GROUP_COPY_HELPER_H

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Expression/Terminator.h"

namespace Codira::CHIR {
/**
 * helper class for copying blockgroup, do instantiation if instMap is set from apply
 * using in two pass now:
 *  1. devirtalizaion
 *  2. function inline
 */
class BlockGroupCopyHelper {
public:
    /**
     * @brief constructor of block group copy helper class
     * @param builder CHIR builder for generating IR
     */
    explicit BlockGroupCopyHelper(CHIRBuilder& builder) : builder(builder)
    {
    }

    /**
     * @brief main entry for copying block group, return result block group and its return value.
     * @param other block group copy from
     * @param parentFunc parent func where copy block in
     * @return
     */
    std::pair<BlockGroup*, LocalVar*> CloneBlockGroup(const BlockGroup& other, Func& parentFunc);

    /**
     * @brief get instantiation map from apply call, using for function instantiation.
     * @param apply input apply expression to get instantiation map
     * @param newBodyOuterFunction func that encloses new body to decide this type conversion,
     *  use apply's top level function if nullptr
     */
    void GetInstMapFromApply(const Apply& apply, const FuncBase* newBodyOuterFunction = nullptr);

    /**
     * @brief replace value with extra value map, such as parameter value map from old block group.
     * @param block block to replace value
     * @param valueMap extra value map to replace value
     */
    void SubstituteValue(Ptr<BlockGroup> block, std::unordered_map<Value*, Value*>& valueMap);
private:
    void InstBlockGroup(Ptr<BlockGroup> group);

    static void CollectValueMap(const Lambda& oldLambda, const Lambda& newLambda,
        std::unordered_map<Value*, Value*>& valueMap, std::unordered_set<Expression*>& newDebugs);
    static void CollectValueMap(const Block& oldBlk, const Block& newBlk,
        std::unordered_map<Value*, Value*>& valueMap, std::unordered_set<Expression*>& newDebugs);
    static void CollectValueMap(const BlockGroup& oldBG, const BlockGroup& newBG,
        std::unordered_map<Value*, Value*>& valueMap, std::unordered_set<Expression*>& newDebugs);

    static void ReplaceExprOperands(const Block& block, const std::unordered_map<Value*, Value*>& valueMap);
    static void ReplaceExprOperands(const BlockGroup& bg, const std::unordered_map<Value*, Value*>& valueMap);

    // map for block group instantiation
    std::unordered_map<const GenericType*, Type*> instMap;
    // mark this type if need replace this type
    Type* thisType = nullptr;
    CHIRBuilder& builder;
};

void FixCastProblemAfterInst(Ptr<BlockGroup> group, CHIRBuilder& builder);
}  // namespace Codira::CHIR

#endif
