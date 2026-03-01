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

#ifndef CODIRA_CHIR_TRANSFORMATION_ARRAY_LAMBDA_OPT_H
#define CODIRA_CHIR_TRANSFORMATION_ARRAY_LAMBDA_OPT_H

#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {
/**
 * CHIR Opt Pass: optimize CHIR IR from lambda function init to value init.
 */
class ArrayLambdaOpt {
public:
    /**
     * @brief constructor of array lambda optimization pass.
     * @param builder CHIR builder for generating IR.
     */
    explicit ArrayLambdaOpt(CHIRBuilder& builder);

    /**
     * @brief run array lambda optimization on a certain package CHIR IR.
     * @param package package to do optimization.
     * @param isDebug flag whether print debug log.
     */
    void RunOnPackage(const Ptr<const Package>& package, bool isDebug);

private:
    void RunOnFunc(const Ptr<Func>& func, bool isDebug);

    Ptr<Constant> CheckCanRewriteLambda(const Ptr<Expression>& expr) const;

    Ptr<Constant> CheckIfLambdaReturnConst(const Lambda& lambda) const;

    void RewriteArrayInitFunc(Apply& apply, const Ptr<const Constant>& constant);

    Ptr<Intrinsic> CheckCanRewriteZeroValue(const Ptr<Expression>& expr) const;

    void RewriteZeroValue(const Ptr<RawArrayInitByValue>& init, const Ptr<Intrinsic>& zeroVal) const;

    CHIRBuilder& builder;
};
} // namespace Codira::CHIR

#endif
