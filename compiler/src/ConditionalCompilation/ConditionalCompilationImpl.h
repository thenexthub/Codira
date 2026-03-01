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
 * This file declares class ConditionalCompilationImpl.
 */

#ifndef CODIRA_CONDITIONALCOMPILATION_CONDITIONALCOMPILATIONIMPL_H
#define CODIRA_CONDITIONALCOMPILATION_CONDITIONALCOMPILATIONIMPL_H

#include <regex>
#include <optional>

#include "Codira/AST/Match.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Basic/Utils.h"
#include "Codira/Basic/Version.h"
#include "Codira/ConditionalCompilation/ConditionalCompilation.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Option/Option.h"
#include "Codira/Utils/FileUtil.h"

namespace Codira::AST {

const std::size_t CODEC_VERSION_LENGTH = 6; // length of codec version

class ConditionalCompilationImpl {
public:
    explicit ConditionalCompilationImpl(CompilerInstance* c);
    void HandleConditionalCompilation(const Package& root);
    void HandleFileConditionalCompilation(File& file);

private:
    inline std::string GetBackendType() const
    {
        if (backendType == Triple::BackendType::CODENATIVE) {
            return "codenative";
        } else {
            return Triple::BackendToString(backendType);
        }
    }
    inline std::string GetArchType() const
    {
        return triple.ArchToString();
    }
    inline std::string GetEnv() const
    {
        return triple.EnvironmentToSimpleString();
    }
    std::string GetOSType() const;
    inline std::string GetCODECVersion() const
    {
        std::string version = std::to_string(codecVersion);
        return RefreshVersionStr(version);
    }
    inline std::string GetDebug() const
    {
        return std::to_string(static_cast<int>(debug));
    }
    inline std::string GetTest() const
    {
        return std::to_string(static_cast<int>(test));
    }
    std::optional<std::string> GetUserDefinedInfoByName(const std::string& name) const;
    inline auto GetPassedValues() const
    {
        return passedCondition;
    }
    std::optional<std::string> GetRelatedInfo(const std::string& target) const;

    CompilerInstance* ci{nullptr};
    Triple::BackendType backendType;
    Triple::Info triple;
    uint32_t codecVersion;
    bool debug;
    bool test;
    std::unordered_map<std::string, std::string> passedCondition;

    bool EvalConditionExpr(const Expr& condition);

    bool ConditionCheck(const std::string& conditionStr, const Position& begin, const std::string& right);

    bool Eval(const BinaryExpr& expr, const std::string& left, const std::string& right) const;

    bool EvalBinaryExpr(const BinaryExpr& be);
    bool EvalParenExpr(const ParenExpr& pe);
    bool EvalUnaryExpr(const UnaryExpr& ue) const;
    bool EvalRefExpr(const RefExpr& re) const;

    bool EvalLogicBinaryExpr(const BinaryExpr& be);
    bool EvalJudgeBinaryExpr(const BinaryExpr& be);

    std::string RefreshVersionStr(std::string& version) const
    {
        while (version.size() < CODEC_VERSION_LENGTH) {
            version = "0" + version;
        }
        return version;
    }

    template <typename T> bool EvalNodeCondition(Ptr<T> node);
};
} // namespace Codira::AST
#endif
