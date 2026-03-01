/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tooling/dynamic/client/manager/source_manager.h"

#include "tooling/dynamic/client/session/session.h"

using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
namespace OHOS::ArkCompiler::Toolchain {
void SourceManager::SendRequeSource(int scriptId)
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    uint32_t id = session->GetMessageId();
    scriptIdMap_.emplace(std::make_pair(id, scriptId));

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.getScriptSource");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("scriptId", std::to_string(scriptId).c_str());
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return;
}

void SourceManager::EnableReply(const std::unique_ptr<PtJson> json)
{
    if (json == nullptr) {
        LOGE("arkdb: json parse error");
        return;
    }

    if (!json->IsObject()) {
        LOGE("arkdb: json parse format error");
        json->ReleaseRoot();
        return;
    }

    std::unique_ptr<PtJson> params;
    Result ret = json->GetObject("params", &params);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find params error");
        return;
    }

    std::string scriptIdStr;
    ret = params->GetString("scriptId", &scriptIdStr);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find scriptId error");
        return;
    }

    std::string fileName;
    ret = params->GetString("url", &fileName);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find fileName error");
        return;
    }
    int scriptId = std::atoi(scriptIdStr.c_str());
    SetFileName(scriptId, fileName);
    SendRequeSource(scriptId);
    return;
}

void SourceManager::SetFileName(int scriptId, const std::string& fileName)
{
    fileSource_.insert(std::make_pair(scriptId, std::make_pair(fileName, std::vector<std::string> {})));
    return;
}

void SourceManager::GetFileName()
{
    for (auto it = fileSource_.begin(); it != fileSource_.end(); it++) {
        std::cout << "scriptID : " << it->first;
        std::cout << " fileName : " << it->second.first <<std::endl;
    }
    return;
}

void SourceManager::SetFileSource(int scriptIdIndex, const std::string& fileSource)
{
    const int IGNOR_NEWLINE_FLAG = 2;
    auto scriptIdIt = scriptIdMap_.find(scriptIdIndex);
    if (scriptIdIt == scriptIdMap_.end()) {
        return;
    }
    int scriptId = scriptIdIt->second;

    auto it = fileSource_.find(scriptId);
    if (it != fileSource_.end() && it->second.second.empty()) {
        std::string::size_type startPos = 0;
        std::string::size_type endPos = fileSource.find("\r\n");
        while (endPos != std::string::npos) {
            std::string line = fileSource.substr(startPos, endPos - startPos);
            it->second.second.push_back(line);
            startPos = endPos + IGNOR_NEWLINE_FLAG;  // ignore "\r\n"
            endPos = fileSource.find("\r\n", startPos);
        }
        it->second.second.push_back(fileSource.substr(startPos));
    }
    return;
}

std::vector<std::string> SourceManager::GetFileSource(int scriptId)
{
    int linNum = 0;
    auto it = fileSource_.find(scriptId);
    if (it != fileSource_.end()) {
        std::cout << "fileSource : " <<std::endl;
        for (const std::string& value : it->second.second) {
            std::cout << ++linNum << "    " << value << std::endl;
        }
        return it->second.second;
    }
    return std::vector<std::string> {};
}

void SourceManager::GetDebugSources(const std::unique_ptr<PtJson> json)
{
    std::string funcName;
    Result ret = json->GetString("functionName", &funcName);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: get functionName error");
        return;
    }

    std::unique_ptr<PtJson> location;
    ret = json->GetObject("location", &location);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: get location error");
        return;
    }

    std::string scriptIdStr;
    ret = location->GetString("scriptId", &scriptIdStr);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: get scriptId error");
        return;
    }
    scriptId_ = std::atoi(scriptIdStr.c_str());

    ret = location->GetInt("lineNumber", &debugLineNum_);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: get lineNumber error");
        return;
    }

    LOGE("arkdb: callFrames : funcName %{public}s, scriptid %{public}s, lineNum %{public}d",
        funcName.c_str(), scriptIdStr.c_str(), debugLineNum_);
    auto it = fileSource_.find(scriptId_);
    if (it != fileSource_.end()) {
        std::cout << (debugLineNum_ + 1) << "    " << it->second.second[debugLineNum_] << std::endl;
        std::cout << ">>> ";
        fflush(stdout);
    }
    return;
}

void SourceManager::ListSourceCodeWithParameters(int startLine, int endLine)
{
    const int BLANK_LINE = std::numeric_limits<int>::max();
    const int STATR_LINE_OFFSET = 6;
    const int END_LINE_OFFSET = 4;
    const int END_LINE = 10;
    if (startLine != BLANK_LINE && endLine == BLANK_LINE) {
        auto it = fileSource_.find(scriptId_);
        if (it == fileSource_.end()) {
            LOGE("arkdb: get file source error");
            return;
        }
        if (startLine >= static_cast<int>(it->second.second.size()) + STATR_LINE_OFFSET ||
            startLine < 0) {
            std::cout << "Line number out of range, this file has " <<
            static_cast<int>(it->second.second.size()) << " lines" << std::endl;
            return;
        }
        int showLine = startLine - STATR_LINE_OFFSET;
        if (showLine < 0) {
            showLine = 0;
            endLine = END_LINE;
        } else {
            endLine = startLine + END_LINE_OFFSET;
        }
        if (endLine > static_cast<int>(it->second.second.size())) {
            endLine = static_cast<int>(it->second.second.size());
        }
        for (; showLine < endLine; showLine++) {
            std::cout << (showLine + 1) << "    " << it->second.second[showLine] << std::endl;
        }
    } else if (startLine != BLANK_LINE && endLine != BLANK_LINE) {
        auto it = fileSource_.find(scriptId_);
        if (it == fileSource_.end()) {
            LOGE("arkdb: get file source error");
            return;
        }
        if (startLine > static_cast<int>(it->second.second.size()) ||
            endLine > static_cast<int>(it->second.second.size()) || startLine < 1) {
            std::cout << "Line number out of range, this file has " <<
            static_cast<int>(it->second.second.size()) << " lines" << std::endl;
            return;
        }
        if (endLine > static_cast<int>(it->second.second.size())) {
            endLine = static_cast<int>(it->second.second.size());
        }
        for (int showLine = startLine - 1; showLine < endLine; showLine++) {
            std::cout << (showLine + 1) << "    " << it->second.second[showLine] << std::endl;
        }
    }
}

void SourceManager::ListSource(int startLine, int endLine)
{
    const int BLANK_LINE = std::numeric_limits<int>::max();
    const int STATR_LINE_OFFSET = 6;
    const int END_LINE_OFFSET = 4;
    const int END_LINE = 10;
    if (startLine == BLANK_LINE && endLine == BLANK_LINE) {
        auto it = fileSource_.find(scriptId_);
        if (it == fileSource_.end()) {
            LOGE("arkdb: get file source error");
            return;
        }
        int showLine = (debugLineNum_ + 1) - STATR_LINE_OFFSET;
        if (showLine < 0) {
            showLine = 0;
            endLine = END_LINE;
        } else {
            endLine = debugLineNum_ + END_LINE_OFFSET;
        }
        if (endLine > static_cast<int>(it->second.second.size())) {
            endLine = static_cast<int>(it->second.second.size());
        }
        for (; showLine <= endLine; showLine++) {
            std::cout << (showLine + 1) << "    " << it->second.second[showLine] << std::endl;
        }
    } else {
        ListSourceCodeWithParameters(startLine, endLine);
    }
}

void SourceManager::GetListSource(std::string startLine, std::string endLine)
{
    const int BLANK_LINE = std::numeric_limits<int>::max();
    int startline = startLine.empty() ? BLANK_LINE : std::stoi(startLine);
    int endline = endLine.empty() ? BLANK_LINE : std::stoi(endLine);
    ListSource(startline, endline);
    return;
}
}