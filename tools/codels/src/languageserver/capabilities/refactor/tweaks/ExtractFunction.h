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

#ifndef CODIRA_LSP_EXTRACTFUNCTION_H
#define CODIRA_LSP_EXTRACTFUNCTION_H

#include "../Tweak.h"

namespace ark {

struct ExtractedFunction {
    struct Param {
        // param name
        std::string name;

        // param type
        std::string type;

        // rename param name
        std::optional<std::string> newName;

        bool isGeneric = false;

        bool operator==(const Param& other) const
        {
            return name == other.name && type == other.type;
        }

        bool operator!=(const Param& other) const
        {
            return !(*this == other);
        }

        bool operator<(const Param& other) const
        {
            return std::tie(name, type) < std::tie(other.name, other.type);
        }
    };

    struct ReturnValue {
        // return name
        std::string name;

        // the function whether need add return statement at the last line of body
        bool addReturnOnFuncBody = false;

        // need define a variable to receive the return value,
        // or use the existing variable to receive the return value
        bool needDeclVar = false;
    };

    // function name
    std::string name = "extracted";

    // param list
    std::set<Param> params;

    std::unordered_set<std::string> generics;

    // Return type (node) of the function
    std::optional<ReturnValue> returnValue;

    // function body
    std::string body;

    // is static function
    bool isStatic = false;

    bool isConst = false;

    bool isMut = false;

    // selected range
    Range replacedRange;

    std::string GetFunctionDeclaration() const;
};

class ExtractFunction : public Tweak {
public:
    enum class ExtractFunctionError {
        INVALID_CODE_SEGMENT = 2,
        PARTIAL_SELECTION,
        MULTI_RETURN_VALUE,
        PARTIAL_IF_EXPR,
        PARTIAL_JUMP_EXPR,
        MULTI_EXIT_POINT,
        GLOBAL_MEMBER_VAR,
        CONTAIN_MEMBER_VAR_INIT,
        PARTIAL_TRY_EXPR,
        PARTIAL_MATCH_EXPR
    };

    const std::string Id() const override
    {
        return "ExtractFunction";
    };

    std::string Title() const override
    {
        return "Extract to function";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

private:
    void GetExtractedFunction(const Tweak::Selection &sel, ExtractedFunction &function);

    void GetParam(ExtractedFunction& function, Codira::AST::RefExpr* refExpr, const Tweak::Selection &sel);

    void GetFunctionParamsList(const Tweak::Selection &sel, ExtractedFunction &function);

    void GetFunctionReturnValue(const Tweak::Selection &sel, ExtractedFunction &function);

    void GetFunctionBody(const Tweak::Selection &sel, ExtractedFunction &function);

    void GetFunctionModifier(const Tweak::Selection &sel, ExtractedFunction &function);

    std::string AssignParam2MutVarAndRemove(const Tweak::Selection &sel, ExtractedFunction &function);

    void AddMutParamVariable(std::string &mutParams, Codira::AST::AssignExpr* assignExpr,
        ExtractedFunction &function, const Tweak::Selection &sel, std::string &scopeName);

    bool ExistSameNameParam(const std::set<ExtractedFunction::Param>& params, const std::string& name);

    void CollectGenerics(Ty &ty, std::unordered_set<std::string> &generics);

    TextEdit InsertDeclaration(const Tweak::Selection &sel, ExtractedFunction &function);

    TextEdit ReplaceBlockWithCall(const Tweak::Selection &sel, ExtractedFunction &function);
};
} // namespace ark

#endif // CODIRA_LSP_EXTRACTFUNCTION_H
