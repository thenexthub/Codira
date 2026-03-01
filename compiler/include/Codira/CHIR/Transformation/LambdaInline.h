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

#ifndef CODIRA_CHIR_TRANSFORMATION_LAMBDA_INLINE_H
#define CODIRA_CHIR_TRANSFORMATION_LAMBDA_INLINE_H

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Transformation/FunctionInline.h"
#include "Codira/Option/Option.h"

namespace Codira::CHIR {

/**
 * @brief inline lambda expression if meet condition as blow:
 *   1. only have one consumer as a callee to apply expression.
 *   2. only have one consumer as a parameter to apply expression, which will not escape in new function.
 */
class LambdaInline {
public:
    /**
     * @brief lambda inline constructor.
     * @param builder chir builder to create IR.
     * @param opts options to indicate whether to do optimization.
     */
    LambdaInline(CHIRBuilder& builder, const GlobalOptions& opts);

    /**
     * @brief interface to do lambda inline.
     * @param funcs all lambda functions in the package.
     */
    void InlineLambda(const std::vector<Lambda*>& funcs);

private:
    /// run on single lambda
    void RunOnLambda(Lambda& lambda);

    /// judge whether you can do optimization ob a lambda if it is passed to a new easy function.
    bool IsLambdaPassToEasyFunc(const Lambda& lambda) const;

    const GlobalOptions& opts;
    /// function inline pass
    FunctionInline inlinePass;
};

}  // namespace Codira::CHIR


#endif
