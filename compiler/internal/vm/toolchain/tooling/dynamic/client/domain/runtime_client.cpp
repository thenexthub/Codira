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

#include "tooling/dynamic/client/domain/runtime_client.h"

#include "tooling/dynamic/client/session/session.h"

using PtJson = panda::ecmascript::tooling::PtJson;
namespace OHOS::ArkCompiler::Toolchain {
bool RuntimeClient::DispatcherCmd(const std::string &cmd)
{
    std::map<std::string, std::function<int()>> dispatcherTable {
        { "heapusage", std::bind(&RuntimeClient::HeapusageCommand, this)},
        { "runtime-enable", std::bind(&RuntimeClient::RuntimeEnableCommand, this)},
        { "runtime-disable", std::bind(&RuntimeClient::RuntimeDisableCommand, this)},
        { "print", std::bind(&RuntimeClient::GetPropertiesCommand, this)},
        { "run", std::bind(&RuntimeClient::RunIfWaitingForDebuggerCommand, this)},
    };

    auto entry = dispatcherTable.find(cmd);
    if (entry != dispatcherTable.end()) {
        entry->second();
        LOGI("RuntimeClient DispatcherCmd reqStr1: %{public}s", cmd.c_str());
        return true;
    } else {
        LOGI("Unknown commond: %{public}s", cmd.c_str());
        return false;
    }
}

int RuntimeClient::HeapusageCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idMethodMap_[id] = std::make_tuple("getHeapUsage", "");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Runtime.getHeapUsage");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Runtime");
    }
    return 0;
}

int RuntimeClient::RuntimeEnableCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idMethodMap_[id] = std::make_tuple("enable", "");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Runtime.enable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Runtime");
    }
    return 0;
}

int RuntimeClient::RuntimeDisableCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idMethodMap_[id] = std::make_tuple("disable", "");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Runtime.disable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Runtime");
    }
    return 0;
}

int RuntimeClient::RunIfWaitingForDebuggerCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idMethodMap_[id] = std::make_tuple("runIfWaitingForDebugger", "");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Runtime.runIfWaitingForDebugger");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Runtime");
    }
    WatchManager &watchManager = session->GetWatchManager();
    watchManager.DebugFalseState();
    return 0;
}

int RuntimeClient::GetPropertiesCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idMethodMap_[id] = std::make_tuple("getProperties", objectId_);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Runtime.getProperties");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("accessorPropertiesOnly", false);
    params->Add("generatePreview", true);
    params->Add("objectId", objectId_.c_str());
    params->Add("ownProperties", true);
    if (HasStartIndex() && HasGroupCount()) {
        params->Add("start", start_.value());
        params->Add("count", count_.value());
    }
    request->Add("params", params);
    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Runtime");
    }
    return 0;
}

int RuntimeClient::GetPropertiesCommand2()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idMethodMap_[id] = std::make_tuple("getProperties", objectId_);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Runtime.getProperties");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("accessorPropertiesOnly", true);
    params->Add("generatePreview", true);
    params->Add("objectId", "0");
    params->Add("ownProperties", false);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Runtime");
    }
    return 0;
}

void RuntimeClient::RecvReply(std::unique_ptr<PtJson> json)
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

    int replyId;
    Result ret = json->GetInt("id", &replyId);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find id error");
        return;
    }

    if (GetMethodById(replyId) == "getHeapUsage") {
        HandleHeapUsage(std::move(json));
    } else if (GetMethodById(replyId) == "getProperties") {
        HandleGetProperties(std::move(json), replyId);
    } else {
        LOGI("arkdb: Runtime replay message is %{public}s", json->Stringify().c_str());
    }
}

std::string RuntimeClient::GetMethodById(const int &id)
{
    auto it = idMethodMap_.find(id);
    if (it != idMethodMap_.end()) {
        return std::get<0>(it->second);
    }
    return "";
}

std::string RuntimeClient::GetRequestObjectIdById(const int &id)
{
    auto it = idMethodMap_.find(id);
    if (it != idMethodMap_.end()) {
        return std::get<1>(it->second);
    }
    return "";
}

void RuntimeClient::HandleGetProperties(std::unique_ptr<PtJson> json, const int &id)
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

    std::unique_ptr<PtJson> result;
    Result ret = json->GetObject("result", &result);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find result error");
        return;
    }

    std::unique_ptr<PtJson> innerResult;
    ret = result->GetArray("result", &innerResult);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find innerResult error");
        return;
    }

    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    StackManager &stackManager = session->GetStackManager();
    VariableManager &variableManager = session->GetVariableManager();
    std::map<int32_t, std::map<int32_t, std::string>> treeInfo = stackManager.GetScopeChainInfo();
    if (isInitializeTree_) {
        variableManager.ClearVariableInfo();
        variableManager.InitializeTree(treeInfo);
    }
    std::string requestObjectIdStr = GetRequestObjectIdById(id);
    TreeNode *node = nullptr;
    if (!isInitializeTree_) {
        int32_t requestObjectId;
        if (!Utils::StrToInt32(requestObjectIdStr, requestObjectId)) {
            LOGE("arkdb: convert 'requestObjectId' from string to int error");
            return;
        }
        node = variableManager.FindNodeWithObjectId(requestObjectId);
    } else {
        node = variableManager.FindNodeObjectZero();
    }

    for (int32_t i = 0; i < innerResult->GetSize(); i++) {
        std::unique_ptr<PropertyDescriptor> variableInfo = PropertyDescriptor::Create(*(innerResult->Get(i)));
        variableManager.AddVariableInfo(node, std::move(variableInfo));
    }

    std::cout << std::endl;
    variableManager.PrintVariableInfo();
}

void RuntimeClient::HandleHeapUsage(std::unique_ptr<PtJson> json)
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

    std::unique_ptr<PtJson> result;
    Result ret = json->GetObject("result", &result);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find result error");
        return;
    }

    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    VariableManager &variableManager = session->GetVariableManager();
    std::unique_ptr<GetHeapUsageReturns> heapUsageReturns = GetHeapUsageReturns::Create(*result);
    if (heapUsageReturns == nullptr) {
        LOGE("arkdb: get heap usage returns error");
        return;
    }
    variableManager.SetHeapUsageInfo(std::move(heapUsageReturns));
    variableManager.ShowHeapUsageInfo();
}
} // OHOS::ArkCompiler::Toolchain