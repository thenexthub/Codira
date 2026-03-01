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

#include "RegexRule.h"
#include "Codira/AST/Match.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace std;

void RegexRule::GetMatchedPatternFromFile()
{
    ifstream f;
    string filepath = regexInfo.filepath;

    f.open(filepath, ios::in);
    if (!f.is_open()) {
        Errorln("open file failed: " + filepath);
        return;
    }
    std::string tmp = "";
    int line = 0;
    while (getline(f, tmp, '\n')) {
        if (line + 1 < line) {
            line = 0;
        } else {
            ++line;
        }
        std::smatch pattern;
        /*
         * Not supported regex group now: (reg1)(reg2)
         * Now only get matched whole string, i.e., pattern[0]
         * sometimes, we want to get a specified grouping: e.g.,:
         * for regex \b([a-zA-Z]{0,1}00[1-9]\d{5,6})(\$|[^0-9]), we want get first group, i.e., pattern[1].
         * Unfortunately, not supported now!
         */
        for (auto& item : regexInfo.regex) {
            int offset = 1;
            std::string loop(tmp);
            while (regex_search(loop, pattern, item.second)) {
                resultInfo.type = item.first;
                resultInfo.line = line;
                resultInfo.column = (int)pattern.position() + offset;
                resultInfo.endLine = line;
                resultInfo.endColumn = resultInfo.column + (int)pattern.length();
                resultInfo.result = pattern[0]; // pattern[0] means matched complete string
                resultInfo.filepath = regexInfo.filepath;
                resultVector.push_back(resultInfo);
                if (loop == pattern.suffix().str()) {
                    break;
                }
                loop = pattern.suffix().str();
                offset += (int)pattern.position() + (int)pattern.length();
            }
        }
    }
    f.close();
}

void RegexRule::GetFileFromNode(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    if (node->astKind != ASTKind::PACKAGE) {
        return;
    }
    auto package = StaticAs<ASTKind::PACKAGE>(node);
    for (auto &it : package->files) {
        if (!it->filePath.empty()) {
            regexInfo.filepath = it->filePath;
            RegexRule::GetMatchedPatternFromFile();
        }
    }
}

std::vector<ResultInfo> RegexRule::InitRegexInfo(Ptr<Node> node, const std::map<int, std::regex> reg)
{
    this->regexInfo.regex = reg;
    RegexRule::GetFileFromNode(node);
    return this->resultVector;
}
} // namespace Codira::CodeCheck
