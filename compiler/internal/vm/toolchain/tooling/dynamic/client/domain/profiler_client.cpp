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

#include "tooling/dynamic/client/domain/profiler_client.h"
#include "tooling/dynamic/client/session/session.h"

using Result = panda::ecmascript::tooling::Result;
using Profile = panda::ecmascript::tooling::Profile;
namespace OHOS::ArkCompiler::Toolchain {
bool ProfilerClient::DispatcherCmd(const std::string &cmd)
{
    std::map<std::string, std::function<int()>> dispatcherTable {
        { "cpuprofile", std::bind(&ProfilerClient::CpuprofileCommand, this)},
        { "cpuprofile-stop", std::bind(&ProfilerClient::CpuprofileStopCommand, this)},
        { "cpuprofile-setSamplingInterval", std::bind(&ProfilerClient::SetSamplingIntervalCommand, this)},
        { "cpuprofile-enable", std::bind(&ProfilerClient::CpuprofileEnableCommand, this)},
        { "cpuprofile-disable", std::bind(&ProfilerClient::CpuprofileDisableCommand, this)},
    };

    auto entry = dispatcherTable.find(cmd);
    if (entry == dispatcherTable.end()) {
        LOGI("Unknown commond: %{public}s", cmd.c_str());
        return false;
    }
    entry->second();
    LOGI("DispatcherCmd cmd: %{public}s", cmd.c_str());
    return true;
}

int ProfilerClient::CpuprofileEnableCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, "cpuprofileenable");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Profiler.enable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Profiler");
    }
    return 0;
}

int ProfilerClient::CpuprofileDisableCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, "cpuprofiledisable");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Profiler.disable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Profiler");
    }
    return 0;
}

int ProfilerClient::CpuprofileCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, "cpuprofile");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Profiler.start");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Profiler");
    }
    return 0;
}

int ProfilerClient::CpuprofileStopCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, "cpuprofilestop");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Profiler.stop");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Profiler");
    }
    return 0;
}

int ProfilerClient::SetSamplingIntervalCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, "setsamplinginterval");
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Profiler.setSamplingInterval");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("interval", interval_);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Profiler");
    }
    return 0;
}

void ProfilerClient::RecvProfilerResult(std::unique_ptr<PtJson> json)
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

    std::unique_ptr<PtJson> profile;
    ret = result->GetObject("profile", &profile);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: the cmd is not cp-stop!");
        return;
    }

    char date[16];
    char time[16];
    bool res = Utils::GetCurrentTime(date, time, sizeof(date));
    if (!res) {
        LOGE("arkdb: get time failed");
        return;
    }

    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    ProfilerSingleton &pro = session->GetProfilerSingleton();
    std::string fileName = "CPU-" + std::to_string(sessionId_) + "-" + std::string(date) + "T" +
                           std::string(time) + ".cpuprofile";
    std::string cpufile = pro.GetAddress() + fileName;
    std::cout << "session " << sessionId_ << " cpuprofile file name is " << cpufile << std::endl;
    std::cout << ">>> ";
    fflush(stdout);
    WriteCpuProfileForFile(cpufile, profile->Stringify());
    pro.AddCpuName(fileName);
}

bool ProfilerClient::WriteCpuProfileForFile(const std::string &fileName, const std::string &data)
{
    std::ofstream ofs;
    std::string realPath;
    bool res = Utils::RealPath(fileName, realPath, false);
    if (!res) {
        LOGE("arkdb: path is not realpath!");
        return false;
    }
    ofs.open(fileName.c_str(), std::ios::out);
    if (!ofs.is_open()) {
        LOGE("arkdb: file open error!");
        return false;
    }
    size_t strSize = data.size();
    ofs.write(data.c_str(), strSize);
    ofs.close();
    ofs.clear();
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return false;
    }
    ProfilerSingleton &pro = session->GetProfilerSingleton();
    pro.SetAddress("/data/");
    return true;
}

void ProfilerClient::SetSamplingInterval(int interval)
{
    this->interval_ = interval;
}
} // OHOS::ArkCompiler::Toolchain
