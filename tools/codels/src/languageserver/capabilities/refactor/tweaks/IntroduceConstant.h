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

#ifndef CODIRA_LSP_INTRODUCECONSTANT_H
#define CODIRA_LSP_INTRODUCECONSTANT_H

#include "../Tweak.h"

namespace ark {
class IntroduceConstant : public Tweak {
public:
    enum class IntroduceConstantError {
        INVALID_EXPR = 2,
        INVALID_CONST_EXPR,
        PARTIAL_SELECTION
    };

    const std::string Id() const override
    {
        return "IntroduceConstant";
    };

    std::string Title() const override
    {
        return "Introduce expression to constant variable";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

    std::map<std::string, std::string> ExtraOptions() override;

    static TextEdit InsertDeclaration(const Selection &sel, Range &range, std::string &varName);

    static TextEdit ReplaceExprWithVar(const Selection &sel, Range &range, std::string &varName);
};
} // namespace ark

#endif // CODIRA_LSP_INTRODUCECONSTANT_H
