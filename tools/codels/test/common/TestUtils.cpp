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

#include "TestUtils.h"
#include "SingleInstance.h"

namespace {
bool CheckCommandArguments(std::vector<ark::CodeAction> &exp, std::vector<ark::CodeAction>& act,
                            std::string &reason, int i)
{
    if (!exp[i].command->arguments.begin()->extraOptions.empty() && !act[i].command->arguments.begin()->extraOptions.empty()) {
        const auto& expMap = exp[i].command->arguments.begin()->extraOptions;
        const auto& actMap = act[i].command->arguments.begin()->extraOptions;
        auto expIt = expMap.find("ErrorCode");
        auto actIt = actMap.find("ErrorCode");
        if (expIt == expMap.end() && actIt == actMap.end()) {
            return true;
        } else if (expIt == expMap.end() || actIt == actMap.end()) {
            reason = "the expect and actual CodeAction " + std::to_string(i) + " command.arguments.extraOptions is different";
            return false;
        } else if (expIt->second != actIt->second) {
            reason = "the expect and actual CodeAction " + std::to_string(i) + " command.arguments.extraOptions is different";
            return false;
        }
    }
    return true;
}                           

bool CheckCodeActionCommand(std::vector<ark::CodeAction> &exp, std::vector<ark::CodeAction>& act,
                            std::string &reason, int i)
{
    if (exp[i].command->command != act[i].command->command) {
        reason = "the expect and actual CodeAction " + std::to_string(i) + " command.command is different";
        return false;
    }
    if (exp[i].command->title != act[i].command->title) {
        reason = "the expect and actual CodeAction " + std::to_string(i) + " command.title is different";
        return false;
    }
    if (exp[i].command->arguments.size() != act[i].command->arguments.size()) {
        reason = "the expect and actual CodeAction " + std::to_string(i) + " command.arguments count is different";
        return false;
    }
    if (exp[i].command->arguments.begin()->range != act[i].command->arguments.begin()->range) {
        reason = "the expect and actual CodeAction " + std::to_string(i) + " command.arguments.range count is different";
        return false;
    }
    bool extraCheck = (!exp[i].command->arguments.begin()->extraOptions.empty() && act[i].command->arguments.begin()->extraOptions.empty())
                        || (exp[i].command->arguments.begin()->extraOptions.empty() && !act[i].command->arguments.begin()->extraOptions.empty());
    if (extraCheck) {
        reason = "the expect and actual CodeAction " + std::to_string(i) + " command.arguments.extraOptions is different";
        return false;
    }
    if (!CheckCommandArguments(exp, act, reason, i)) {
        return false;
    }
    return true;
}
}

namespace TestUtils {
using namespace Codira::FileUtil;

std::string ChangeMessageUrlOfString(const std::string &projectPath, const std::string &tmp,
                                        std::string &rootUri,
                                        bool &isMultiModule) {
    std::string relativePath;
    std::string uri;
    if (isMultiModule) {
        relativePath = GetRelativePathForTest(rootUri, tmp).value();
    } else {
        auto index = tmp.find("src");
        if (index == std::string::npos) {
            return "";
        }
        relativePath = tmp.substr(tmp.find("src"));
    }
    uri = ark::PathWindowsToLinux(JoinPath(projectPath, relativePath));
    auto found = uri.find(':');
    if (found != std::string::npos) {
        uri.replace(found, 1, "%3A");
    }
    if (!uri.empty() && uri.back() == '/') {
        uri.pop_back();
    }
    if (TEST_FILE_SEPERATOR == "\\") {
        uri = "file:///" + uri;
    } else {
        uri = "file://" + uri;
    }
    return uri;
}

std::string replaceCrLf(const std::string &str) {
    std::string result = str;
    size_t start_pos = 0;
    while ((start_pos = result.find(CRLF, start_pos)) != std::string::npos) {
        result.replace(start_pos, CRLF.length(), LF);
        start_pos += 1;
    }
    return result;
}

void PrintJson(const nlohmann::json &exp, const std::string &prefix) {
    std::ostringstream actOs;
    actOs << exp.dump(-1);
    std::string lines = actOs.str();
    printf("%s=%s \n", prefix.c_str(), lines.c_str());
}

std::optional<std::string> GetRelativePathForTest(const std::string &basePath, const std::string &path) {
    if (basePath == path) {
        return "";
    }
    auto found = path.find(basePath);
    if (found != std::string::npos) {
        return path.substr(found + basePath.size());
    }
    return "";
}

void ChangeMessageUrlOfTextDocument(const std::string &projectPath, nlohmann::json &root,
                                    std::string &rootUri,
                                    bool &isMultiModule) {
    if (root.empty()) {
        return;
    }
    std::string tmp = root.get<std::string>();
    if (tmp.empty()) {
        return;
    }
    std::string temp = ChangeMessageUrlOfString(projectPath, tmp, rootUri, isMultiModule);
    if (temp.empty()) {
        return;
    }
    root = temp;
}

bool SemanticTokensFormat::operator==(const SemanticTokensFormat &right) const
{
    return this->line == right.line && this->startChar == right.startChar &&
            this->length == right.length && this->tokenType == right.tokenType;
}

bool SemanticTokensFormat::operator<(const SemanticTokensFormat &right) const
{
    if (this->line < right.line) {
        return true;
    }
    if (this->line == right.line && (this->startChar < right.startChar ||
                                        this->startChar == right.startChar && this->length < right.length)) {
        return true;
    }
    return false;
}

std::vector<ark::CodeAction> CreateCodeActionStruct(const nlohmann::json& exp)
{
    std::vector<ark::CodeAction> result;
    ark::CodeAction item;
    if (!exp.contains("result")) {
        return result;
    }

    if (!exp["result"].empty()) {
        for (int i = 0; i < exp["result"].size(); i++) {
            item.kind = exp["result"][i].value("kind", "");
            item.title = exp["result"][i].value("title", "");
            item.command = CreateCommandStruct(exp["result"][i]["command"]);
            result.push_back(item);
        }
    }
    return result;
}

bool CompareCodeAction(std::vector<ark::CodeAction> &exp, std::vector<ark::CodeAction>& act,
                        std::string &reason, int i)
{
    if (exp[i].kind != act[i].kind) {
        reason = "the expect and actual CodeAction " + std::to_string(i) + " kind is different";
        return false;
    }
    if (exp[i].title != act[i].title) {
        reason = "the expect and actual CodeAction " + std::to_string(i) + " title is different";
        return false;
    }

    if (exp[i].command.has_value() && act[i].command.has_value()) {
        if (!CheckCodeActionCommand(exp, act, reason, i)) {
            return false;
        }
    } else if (!exp[i].command.has_value() || !act[i].command.has_value()) {
        reason = "the expect and actual CodeAction " + std::to_string(i) + " command is different";
        return false;
    }
    return true;
}

bool CheckCodeActionResult(const nlohmann::json& expect, const nlohmann::json& actual, std::string &reason)
{
    if (!expect.contains("result") || !actual.contains("result")) {
        reason = "expect or actual has no result";
        return false;
    }

    std::vector<ark::CodeAction> exp = CreateCodeActionStruct(expect);
    std::vector<ark::CodeAction> act = CreateCodeActionStruct(actual);

    std::sort(exp.begin(), exp.end(), [](const ark::CodeAction &a, const ark::CodeAction &b) {
        return a.title < b.title;
    });
    std::sort(act.begin(), act.end(), [](const ark::CodeAction &a, const ark::CodeAction &b) {
        return a.title < b.title;
    });

    if (!CheckResultCount(exp, act, false)) {
        reason = "expect and actual CodeAction number is different";
        return false;
    }

    for (int i = 0; i < exp.size(); i++) {
        if (!CompareCodeAction(exp, act, reason, i)) {
            return false;
        }
    }
    return true;
}

ark::ApplyWorkspaceEditParams CreateApplyEditStruct(const nlohmann::json& exp)
{
    ark::ApplyWorkspaceEditParams result;
    if (!exp.contains("params")) {
        return result;
    }

    if (exp["params"]["edit"]["changes"].empty()) {
        return result;
    }

    for (const auto &[key, value] : exp["params"]["edit"]["changes"].items()) {
        std::vector<ark::TextEdit> textEdits;
        for (const auto& item : value) {
            ark::TextEdit textEdit;
            textEdit.range = CreateRangeStruct(item["range"]);
            textEdit.newText = item["newText"];
            textEdits.push_back(std::move(textEdit));
        }
        result.edit.changes[key] = textEdits;
    }
    return result;
}

bool CheckApplyEditResult(const nlohmann::json& expect, const nlohmann::json& actual, std::string &reason)
{
    if (!expect.contains("params") || !actual.contains("params")) {
        reason = "expect or actual has no params";
        return false;
    }

    ark::ApplyWorkspaceEditParams exp = CreateApplyEditStruct(expect);
    ark::ApplyWorkspaceEditParams act = CreateApplyEditStruct(actual);
    if (exp.edit.changes.size() != act.edit.changes.size()) {
        reason = "the expect and actual changes size is different";
        return false;
    }

    for (const auto &[key, value] : exp.edit.changes) {
        auto actValue = act.edit.changes[key];
        if (value.size() != actValue.size()) {
            reason = "the expect and actual changes.textEdit size is different";
            return false;
        }
        for (int i = 0; i < value.size(); i++) {
            if (value[i].newText != actValue[i].newText) {
                reason = "the expect and actual changes.textEdit newText is different";
                return false;
            }
            if (value[i].range != actValue[i].range) {
                reason = "the expect and actual changes.textEdit range is different";
                return false;
            }
        }
    }
    return true;
}

ark::Command CreateCommandStruct(const nlohmann::json &exp) {
    ark::Command result;
    if (exp.contains("command")) {
        result.command = exp.value("command", "");
        result.title = exp.value("title", "");
        result.arguments = CreateExecutableRangeStruct(exp["arguments"][0]);
    }
    return result;
}


ark::Range CreateRangeStruct(const nlohmann::json &exp) {
    ark::Range result;
    if (exp.contains("range")) {
        result.start.column = exp["range"]["start"].value("character", -1);
        result.start.line = exp["range"]["start"].value("line", -1);
        result.end.column = exp["range"]["end"].value("character", -1);
        result.end.line = exp["range"]["end"].value("line", -1);
    }
    if (exp.contains("selection")) {
        result.start.column = exp["selection"]["start"].value("character", -1);
        result.start.line = exp["selection"]["start"].value("line", -1);
        result.end.column = exp["selection"]["end"].value("character", -1);
        result.end.line = exp["selection"]["end"].value("line", -1);
    }
    return result;
}

std::set<ark::ExecutableRange> CreateExecutableRangeStruct(const nlohmann::json &exp) {
    std::set<ark::ExecutableRange> result;
    ark::ExecutableRange executableRange;
    executableRange.uri = GetFileUrl(exp.value("uri", ""));
    executableRange.projectName = exp.value("projectName", "");
    executableRange.packageName = exp.value("packageName", "");
    executableRange.className = exp.value("className", "");
    executableRange.functionName = exp.value("functionName", "");
    executableRange.range = CreateRangeStruct(exp);
    executableRange.tweakId = exp.value("tweakID", "");
    if (exp.contains("ErrorCode")) {
        executableRange.extraOptions.emplace("ErrorCode", exp.value("ErrorCode", ""));
    }
    result.insert(executableRange);
    return result;
}

std::string GetFileUrl(const std::string &file) {
    std::string filePath;
    auto loc = file.find("test/testChr");
    if (loc == std::string::npos) {
        if (file.find("output/bin/stdlib/codedecl/") != std::string::npos) {
            return file;
        }
        return filePath;
    }

    std::string subUri = "/" + file.substr(loc, file.length());
    filePath = SingleInstance::GetInstance()->workPath + subUri;
    return filePath;
}

bool CompareCodeLen(std::vector<ark::CodeLens> exp, std::vector<ark::CodeLens> act,
                    std::string &reason, int i)
{
    if ((exp[i].range.start.column != act[i].range.start.column) ||
        (exp[i].range.end.column != act[i].range.end.column)) {
        reason = "the expect and actual " + std::to_string(i) + " range start or end column is different";
        return false;
    }

    if ((exp[i].range.start.line != act[i].range.start.line) ||
        (exp[i].range.end.line != act[i].range.end.line)) {
        reason = "the expect and actual " + std::to_string(i) + " range start or end line is different";
        return false;
    }

    if (exp[i].command.title != act[i].command.title) {
        reason = "the expect and actual " + std::to_string(i) + " command title is different";
        return false;
    }

    if (exp[i].command.command != act[i].command.command) {
        reason = "the expect and actual " + std::to_string(i) + " command command is different";
        return false;
    }

    if (exp[i].command.arguments.size() != act[i].command.arguments.size() ||
        exp[i].command.arguments.size() == 0 || act[i].command.arguments.size() == 0) {
        reason = "the expect and actual " + std::to_string(i) + " command arguments is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->uri != act[i].command.arguments.begin()->uri) {
        reason = "the expect and actual " + std::to_string(i) + " arguments uri is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->projectName != act[i].command.arguments.begin()->projectName) {
        reason = "the expect and actual " + std::to_string(i) + " arguments projectName is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->packageName != act[i].command.arguments.begin()->packageName) {
        reason = "the expect and actual " + std::to_string(i) + " arguments packageName is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->className != act[i].command.arguments.begin()->className) {
        reason = "the expect and actual " + std::to_string(i) + " arguments className is different";
        return false;
    }

    if (exp[i].command.arguments.begin()->functionName != act[i].command.arguments.begin()->functionName) {
        reason = "the expect and actual " + std::to_string(i) + " arguments functionName is different";
        return false;
    }
    return true;
}
} // namespace TestUtils
