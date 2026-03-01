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

#ifndef CODIRACODECHECK_REGEXRULE_H
#define CODIRACODECHECK_REGEXRULE_H
#include <fstream>
#include <iostream>
#include <regex>
#include "common/ConfigContext.h"
#include "StructuralRule.h"

namespace Codira::CodeCheck {
enum class HardcodeType {
    IP = 0,
    URL = 1,
    EMAIL = 2
};

struct ResultInfo {
    int type = -1;
    int line = -1;
    int column = -1;
    int endLine = -1;
    int endColumn = -1;
    std::string result;
    std::string filepath;
};

struct RegexInfo {
    std::map<int, std::regex> regex;
    std::string filepath = "";
};

class RegexRule : public StructuralRule {
public:
    std::vector<ResultInfo> resultVector;
    explicit RegexRule(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~RegexRule() override = default;
    std::vector<ResultInfo> InitRegexInfo(Ptr<Codira::AST::Node> node, const std::map<int, std::regex> reg);

protected:
    void MatchPattern(ASTContext &ctx, Ptr<Codira::AST::Node> node) override = 0;

private:
    ResultInfo resultInfo;
    RegexInfo regexInfo;
    void GetFileFromNode(Ptr<Codira::AST::Node> node);
    void GetMatchedPatternFromFile();
};
} // namespace Codira::CodeCheck
#endif // CODIRACODECHECK_REGEXRULE_H
