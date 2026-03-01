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

#include "tooling/dynamic/test/client_utils/test_util.h"

#include "tooling/dynamic/client/domain/debugger_client.h"
#include "tooling/dynamic/client/domain/runtime_client.h"
#include "tooling/dynamic/client/utils/cli_command.h"
#include "tooling/dynamic/client/session/session.h"
#include "websocket/client/websocket_client.h"

#include <cstring>

namespace panda::ecmascript::tooling::test {
TestMap TestUtil::testMap_;

MatchFunc MatchRule::replySuccess = [] (auto recv, auto, auto) -> bool {
    std::unique_ptr<PtJson> json = PtJson::Parse(recv);
    Result ret;
    int32_t id = 0;
    ret = json->GetInt("id", &id);
    if (ret != Result::SUCCESS) {
        return false;
    }

    std::unique_ptr<PtJson> result = nullptr;
    ret = json->GetObject("result", &result);
    if (ret != Result::SUCCESS) {
        return false;
    }

    int32_t code = 0;
    ret = result->GetInt("code", &code);
    if (ret != Result::NOT_EXIST) {
        return false;
    }
    return true;
};

std::ostream &operator<<(std::ostream &out, ActionRule value)
{
    const char *s = nullptr;

#define ADD_CASE(entry) \
    case (entry):       \
        s = #entry;     \
        break

    switch (value) {
        ADD_CASE(ActionRule::STRING_EQUAL);
        ADD_CASE(ActionRule::STRING_CONTAIN);
        ADD_CASE(ActionRule::CUSTOM_RULE);
        default: {
            ASSERT(false && "Unknown ActionRule");
        }
    }
#undef ADD_CASE

    return out << s;
}

void TestUtil::NotifySuccess()
{
    std::vector<std::string> cliCmdStr = { "success" };
    CliCommand cmd(cliCmdStr, 0);
    if (cmd.ExecCommand() == ErrCode::ERR_FAIL) {
        LOG_DEBUGGER(ERROR) << "ExecCommand Test.success fail";
    }
}

void TestUtil::NotifyFail()
{
    std::vector<std::string> cliCmdStr = { "fail" };
    CliCommand cmd(cliCmdStr, 0);
    if (cmd.ExecCommand() == ErrCode::ERR_FAIL) {
        LOG_DEBUGGER(ERROR) << "ExecCommand Test.fail fail";
    }
}

void TestUtil::ForkSocketClient([[maybe_unused]] int port, const std::string &name)
{
#ifdef OHOS_PLATFORM
    auto correntPid = getpid();
#endif
    pid_t pid = fork();
    if (pid < 0) {
        LOG_DEBUGGER(FATAL) << "fork error";
        UNREACHABLE();
    } else if (pid == 0) {
        LOG_DEBUGGER(INFO) << "fork son pid: " << getpid();
        std::this_thread::sleep_for(std::chrono::microseconds(500000));  // 500000: 500ms for wait debugger
#ifdef OHOS_PLATFORM
        std::string pidStr = std::to_string(correntPid);
        std::string sockInfo = pidStr + "PandaDebugger";
#else
        std::string sockInfo = std::to_string(port);
#endif
        int ret = SessionManager::getInstance().CreateTestSession(sockInfo);
        LOG_ECMA_IF(ret, FATAL) << "CreateTestSession fail";

        WebSocketClient &client = SessionManager::getInstance().GetCurrentSession()->GetWebSocketClient();
        auto &testAction = TestUtil::GetTest(name)->testAction;
        for (const auto &action: testAction) {
            LOG_DEBUGGER(INFO) << "message: " << action.message;
            bool success = true;
            if (action.action == SocketAction::SEND) {
                std::vector<std::string> cliCmdStr = Utils::SplitString(action.message, " ");
                CliCommand cmd(cliCmdStr, 0);
                success = (cmd.ExecCommand() == ErrCode::ERR_OK);
            } else {
                ASSERT(action.action == SocketAction::RECV);
                std::string recv = client.Decode();
                HandleAcceptanceMessages(action, client, recv, success);
                SendMessage(action, recv);
                LOG_DEBUGGER(INFO) << "recv: " << recv;
                LOG_DEBUGGER(INFO) << "rule: " << action.rule;
            }
            if (!success) {
                LOG_DEBUGGER(ERROR) << "Notify fail";
                NotifyFail();
                SessionManager::getInstance().DelSessionById(0);
                exit(-1);
            }
        }

        SessionManager::getInstance().DelSessionById(0);
        exit(0);
    }
    LOG_DEBUGGER(INFO) << "ForkSocketClient end";
}

void TestUtil::HandleAcceptanceMessages(ActionInfo action, WebSocketClient &client, std::string &recv, bool &success)
{
    if (recv.empty()) {
        LOG_DEBUGGER(ERROR) << "Notify fail";
        NotifyFail();
        SessionManager::getInstance().DelSessionById(0);
        exit(-1);
    }
    int times = 0;
    while ((!strcmp(recv.c_str(), "try again")) && (times <= 5)) { // 5: five times
        std::this_thread::sleep_for(std::chrono::microseconds(500000)); // 500000: 500ms
        recv = client.Decode();
        times++;
    }
    switch (action.rule) {
        case ActionRule::STRING_EQUAL: {
            success = (recv == action.message);
            break;
        }
        case ActionRule::STRING_CONTAIN: {
            success = (recv.find(action.message) != std::string::npos);
            break;
        }
        case ActionRule::CUSTOM_RULE: {
            bool needMoreMsg = false;
            success = action.matchFunc(recv, action.message, needMoreMsg);
            while (needMoreMsg) {
                recv = client.Decode();
                success = action.matchFunc(recv, action.message, needMoreMsg);
            }
            break;
        }
    }
    return;
}

void TestUtil::SendMessage(ActionInfo action, std::string recv)
{
    switch (action.event) {
        case TestCase::SOURCE: {
            std::unique_ptr<PtJson> json = PtJson::Parse(recv);
            std::unique_ptr<PtJson> params = nullptr;
            Result ret = json->GetObject("params", &params);
            std::string scriptId;
            ret = params->GetString("scriptId", &scriptId);
            if (ret != Result::SUCCESS) {
                return;
            }
            SessionManager::getInstance().GetCurrentSession()->
                GetSourceManager().SendRequeSource(std::atoi(scriptId.c_str()));
            break;
        }
        case TestCase::WATCH: {
            if (!SessionManager::getInstance().GetCurrentSession()->GetWatchManager().GetWatchInfoSize()) {
                SessionManager::getInstance().GetCurrentSession()->GetWatchManager().AddWatchInfo("a");
                SessionManager::getInstance().GetCurrentSession()->GetWatchManager().AddWatchInfo("this");
            }
            const uint watchInfoSize = 2;
            for (uint i = 0; i < watchInfoSize; i++) {
                SessionManager::getInstance().GetCurrentSession()->GetWatchManager().SendRequestWatch(i, "0");
            }
            break;
        }
        case TestCase::WATCH_OBJECT: {
            std::unique_ptr<PtJson> json = PtJson::Parse(recv);
            std::unique_ptr<PtJson> result = nullptr;
            Result ret = json->GetObject("result", &result);
            std::unique_ptr<PtJson> watchResult = nullptr;
            ret = result->GetObject("result", &watchResult);
            if (ret != Result::SUCCESS) {
                return;
            }
            std::string objectId;
            ret = watchResult->GetString("objectId", &objectId);
            if (ret != Result::SUCCESS) {
                return;
            }
            SessionManager::getInstance().GetCurrentSession()->GetWatchManager().GetPropertiesCommand(0, objectId);
            break;
        }
        default: {
            return;
        }
    }
    return;
}
}  // namespace panda::ecmascript::tooling::test
