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

#include "tooling/dynamic/client/manager/watch_manager.h"

#include "tooling/dynamic/client/session/session.h"

using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
namespace OHOS::ArkCompiler::Toolchain {
WatchManager::WatchManager(uint32_t sessionId)
    : sessionId_(sessionId), runtimeClient_(sessionId)
{
}

void WatchManager::SendRequestWatch(const int32_t &watchInfoIndex, const std::string &callFrameId)
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    uint32_t id = session->GetMessageId();
    watchInfoMap_.emplace(id, watchInfoIndex);

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.evaluateOnCallFrame");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("callFrameId", callFrameId.c_str());
    params->Add("expression", watchInfoList_[watchInfoIndex].c_str());
    params->Add("objectGroup", "watch-group");
    params->Add("includeCommandLineAPI", false);
    params->Add("silent", true);
    params->Add("returnByValue", false);
    params->Add("generatePreview", false);
    params->Add("throwOnSideEffect", false);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        inputRowFlag_++;
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return;
}

void WatchManager::GetPropertiesCommand(const int32_t &watchInfoIndex, const std::string &objectId)
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    uint32_t id = session->GetMessageId();
    watchInfoMap_.emplace(id, watchInfoIndex);

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Runtime.getProperties");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("accessorPropertiesOnly", false);
    params->Add("generatePreview", true);
    params->Add("objectId", objectId.c_str());
    params->Add("ownProperties", true);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return;
}

void WatchManager::RequestWatchInfo(const std::unique_ptr<PtJson> &json)
{
    Result ret = json->GetString("callFrameId", &callFrameId_);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find callFrameId error");
        return;
    }
    for (uint i = 0; i < watchInfoList_.size(); i++) {
        SendRequestWatch(i, callFrameId_);
    }
}

std::string WatchManager::GetCallFrameId()
{
    return callFrameId_;
}

int WatchManager::GetWatchInfoSize()
{
    return watchInfoList_.size();
}

void WatchManager::AddWatchInfo(const std::string& watchInfo)
{
    watchInfoList_.emplace_back(watchInfo);
}

bool WatchManager::GetDebugState()
{
    return IsDebug_;
}
void WatchManager::DebugFalseState()
{
    IsDebug_ = false;
}
void WatchManager::DebugTrueState()
{
    IsDebug_ = true;
}

void WatchManager::SetWatchInfoMap(const int &id, const int &index)
{
    watchInfoMap_.emplace(id, index);
}

bool WatchManager::HandleWatchResult(const std::unique_ptr<PtJson> &json, int32_t id)
{
    Result ret;
    std::unique_ptr<PtJson> result;
    ret = json->GetObject("result", &result);
    if (ret != Result::SUCCESS) {
        LOGE("json parse result error");
        return false;
    }
    std::unique_ptr<PtJson> watchResult;
    ret = result->GetObject("result", &watchResult);
    if (ret != Result::SUCCESS) {
        ShowWatchResult2(id, std::move(json));
        LOGE("json parse result error");
        return false;
    }
    if (!ShowWatchResult(std::move(watchResult), id)) {
        return false;
    }
    inputRowFlag_--;
    if (inputRowFlag_ == 0) {
        std::cout << ">>> ";
        fflush(stdout);
        isShowWatchInfo_ = true;
    }
    return true;
}
bool WatchManager::ShowWatchResult(const std::unique_ptr<PtJson> &result, int32_t id)
{
    std::string type;
    Result ret = result->GetString("type", &type);
    if (ret != Result::SUCCESS) {
        LOGE("json parse type error");
        return false;
    }
    if (inputRowFlag_ == GetWatchInfoSize() && isShowWatchInfo_) {
        std::cout << "watch info :" << std::endl;
        isShowWatchInfo_ = false;
    }
    if (type == "undefined") {
        auto it = watchInfoMap_.find(id);
        if (it == watchInfoMap_.end()) {
            return false;
        }
        std::cout << "    " << watchInfoList_[it->second] << " = undefined" << std::endl;
    } else if (type == "object") {
        std::string objectId;
        ret = result->GetString("objectId", &objectId);
        if (ret != Result::SUCCESS) {
            LOGE("json parse object error");
            return false;
        }
        auto it = watchInfoMap_.find(id);
        if (it == watchInfoMap_.end()) {
            return false;
        }
        GetPropertiesCommand(it->second, objectId);
        inputRowFlag_++;
    } else  {
        auto it = watchInfoMap_.find(id);
        if (it == watchInfoMap_.end()) {
            return false;
        }
        std::string description;
        ret = result->GetString("description", &description);
        if (ret != Result::SUCCESS) {
            LOGE("json parse description error");
            return false;
        }
        std::cout << "    " << watchInfoList_[it->second] << " = " << description << std::endl;
    }
    watchInfoMap_.erase(id);
    DebugTrueState();
    return true;
}

void WatchManager::OutputWatchResult(const std::unique_ptr<PtJson> &watchResult)
{
    for (int32_t i = 0; i < watchResult->GetSize(); i++) {
        std::string name;
        Result ret = watchResult->Get(i)->GetString("name", &name);
        if (ret != Result::SUCCESS) {
            LOGE("json parse name error");
            continue;
        }
        std::unique_ptr<PtJson>  value;
        ret = watchResult->Get(i)->GetObject("value", &value);
        if (ret != Result::SUCCESS) {
            LOGE("json parse value error");
            continue;
        }
        std::string description;
        ret = value->GetString("description", &description);
        if (ret != Result::SUCCESS) {
            LOGE("json parse description error");
            continue;
        }
        std::cout << name << " = " << description;
        if (i < watchResult->GetSize() - 1) {
            std::cout << ", ";
        }
    }
    return;
}

bool WatchManager::ShowWatchResult2(const int &id, const std::unique_ptr<PtJson> &result)
{
    std::unique_ptr<PtJson> resultWatch;
    Result ret = result->GetObject("result", &resultWatch);
    if (ret != Result::SUCCESS) {
        LOGE("json parse result error");
        return false;
    }
    std::unique_ptr<PtJson> watchResult;
    ret = resultWatch->GetArray("result", &watchResult);
    if (ret != Result::SUCCESS) {
        LOGE("json parse result error");
        return false;
    }
    auto it = watchInfoMap_.find(id);
    if (it == watchInfoMap_.end()) {
        return false;
    }
    std::cout << "    " << watchInfoList_[it->second] << " = { ";
    OutputWatchResult(std::move(watchResult));
    std::cout << " }" << std::endl;
    inputRowFlag_--;
    if (inputRowFlag_ == 0) {
        std::cout << ">>> ";
        fflush(stdout);
        isShowWatchInfo_ = true;
    }
    DebugTrueState();
    watchInfoMap_.erase(id);
    return true;
}
}