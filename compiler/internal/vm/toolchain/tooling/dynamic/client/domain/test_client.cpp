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

#include "tooling/dynamic/client/domain/test_client.h"

#include "tooling/dynamic/client/session/session.h"

using PtJson = panda::ecmascript::tooling::PtJson;
namespace OHOS::ArkCompiler::Toolchain {
bool TestClient::DispatcherCmd(const std::string &cmd)
{
    std::map<std::string, std::function<int()>> dispatcherTable {
        { "success", std::bind(&TestClient::SuccessCommand, this)},
        { "fail", std::bind(&TestClient::FailCommand, this)},
    };

    auto entry = dispatcherTable.find(cmd);
    if (entry != dispatcherTable.end()) {
        entry->second();
        LOGI("TestClient DispatcherCmd cmd: %{public}s", cmd.c_str());
        return true;
    } else {
        LOGI("Unknown commond: %{public}s", cmd.c_str());
        return false;
    }
}

int TestClient::SuccessCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Test.success");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Test");
    }
    return 0;
}

int TestClient::FailCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Test.fail");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Test");
    }
    return 0;
}
} // OHOS::ArkCompiler::Toolchain