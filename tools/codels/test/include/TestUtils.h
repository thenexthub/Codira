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

#ifndef LSPSERVER_TESTUTILS_H
#define LSPSERVER_TESTUTILS_H

#ifdef __linux__
#include <linux/limits.h>
#endif
#include <sstream>
#include "../../src/json-rpc/Protocol.h"
#include "../../src/json-rpc/URI.h"
#include "../../src/languageserver/ArkServer.h"

namespace TestUtils {
#ifdef _WIN32
    const std::string TEST_FILE_SEPERATOR = "\\";
#else
    const std::string TEST_FILE_SEPERATOR = "/";
#endif

    const std::string syscapFolder = "sysconfig";
    const std::string syscapPath = "syscap_api_config.json";
    const std::string phonePath = "phone.json";
    const std::string phonehmosPath = "phone-hmos.json";
    const std::string MACRO_PATH = "/Macro/";
    const std::string CRLF = "\\r\\n";
    const std::string LF = "\\n";
    const int INDENT_LEN = 2;
    const int MAX_LEN = 1024 * 1024;

    struct SemanticTokensFormat {
        int line;
        int startChar;
        int length;
        int tokenType;
        int tokenTypeModifier;

        bool operator==(const SemanticTokensFormat &right) const;

        bool operator<(const SemanticTokensFormat &right) const;
    };

    std::string ChangeMessageUrlOfString(const std::string &projectPath, const std::string &tmp,
                                            std::string &rootUri,
                                            bool &isMultiModule);

    std::string replaceCrLf(const std::string &str);

    void PrintJson(const nlohmann::json &exp, const std::string &prefix);

    std::optional<std::string> GetRelativePathForTest(const std::string &basePath, const std::string &path);

    void ChangeMessageUrlOfTextDocument(const std::string &projectPath, nlohmann::json &root,
                                        std::string &rootUri,
                                        bool &isMultiModule);

    template<typename T>
    bool CheckResultCount(const std::vector<T> &exp, const std::vector<T> &act, bool needCheckEmpty = true) {
        if (needCheckEmpty) {
            return exp.size() == act.size() && !exp.empty();
        }
        return exp.size() == act.size();
    }

    std::vector<ark::CodeAction> CreateCodeActionStruct(const nlohmann::json& exp);

    bool CheckCodeActionResult(const nlohmann::json& expect, const nlohmann::json& actual, std::string &reason);

    ark::ApplyWorkspaceEditParams CreateApplyEditStruct(const nlohmann::json& exp);

    bool CheckApplyEditResult(const nlohmann::json& expect, const nlohmann::json& actual, std::string &reason);
    
    ark::Command CreateCommandStruct(const nlohmann::json &exp);
    
    ark::Range CreateRangeStruct(const nlohmann::json &exp);
    
    std::set<ark::ExecutableRange> CreateExecutableRangeStruct(const nlohmann::json &exp);
    
    std::string GetFileUrl(const std::string &file);

    bool CompareCodeLen(std::vector<ark::CodeLens> exp, std::vector<ark::CodeLens> act,
                        std::string &reason, int i);

    template<typename T>
    bool CompareRange(const T &letf, const T &right) {
        return letf.range < right.range;
    }                             
} // test::common
#endif // LSPSERVER_TESTUTILS_H
