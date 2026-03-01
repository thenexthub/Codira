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

#ifndef CODIRA_CHIR_TRANSFORMATION_ARRAYLIST_CONST_START_OPT_H
#define CODIRA_CHIR_TRANSFORMATION_ARRAYLIST_CONST_START_OPT_H


#include "Codira/CHIR/Transformation/FunctionInline.h"


namespace Codira::CHIR {
/**
 * CHIR Opt Pass: optimization for codira array list loop and start point.
 *     1. inline special func with array loop function.
 *     2. replace start point call with const zero.
 */
class ArrayListConstStartOpt {
public:
    /**
     * @brief constructor of optimization for codira array list loop and start point.
     * @param builder CHIR builder for generating IR.
     * @param opts global options of codira inputs.
     * @param pass inline opt pass.
     */
    explicit ArrayListConstStartOpt(CHIRBuilder& builder, const GlobalOptions& opts, FunctionInline& pass)
        : builder(builder), opts(opts), pass(pass)
    {
    }

    /**
     * @brief run array list start optimization on a certain package CHIR IR.
     * @param package package to do optimization.
     */
    void RunOnPackage(const Ptr<const Package>& package);

    /**
     * @brief Get effect map after this pass.
     * @return effect map affected by this pass.
     */
    const OptEffectCHIRMap& GetEffectMap() const;

private:
    bool CheckNeedRewrite(const Apply& apply) const;
    bool IsStartAddIndexExpression(const Field& field, bool isIteratorFunc) const;
    void RewriteStartWithConstZero(Expression& oldExpr) const;
    CHIRBuilder& builder;
    const std::string optPassName{"ArrayListConstStartOpt Inline"};
    const GlobalOptions& opts;
    FunctionInline& pass;
    OptEffectCHIRMap effectMap;
};
} // namespace Codira::CHIR

#endif
