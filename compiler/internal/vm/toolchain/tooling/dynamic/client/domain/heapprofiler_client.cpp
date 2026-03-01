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

#include "tooling/dynamic/client/domain/heapprofiler_client.h"
#include "tooling/dynamic/client/session/session.h"

using Result = panda::ecmascript::tooling::Result;
namespace OHOS::ArkCompiler::Toolchain {
static constexpr int32_t SAMPLING_INTERVAL = 16384;
bool HeapProfilerClient::DispatcherCmd(const std::string &cmd, const std::string &arg)
{
    path_ = arg;

    std::map<std::string, std::function<int()>> dispatcherTable {
        { "allocationtrack", std::bind(&HeapProfilerClient::AllocationTrackCommand, this)},
        { "allocationtrack-stop", std::bind(&HeapProfilerClient::AllocationTrackStopCommand, this)},
        { "heapdump", std::bind(&HeapProfilerClient::HeapDumpCommand, this)},
        { "heapprofiler-enable", std::bind(&HeapProfilerClient::Enable, this)},
        { "heapprofiler-disable", std::bind(&HeapProfilerClient::Disable, this)},
        { "sampling", std::bind(&HeapProfilerClient::Samping, this)},
        { "sampling-stop", std::bind(&HeapProfilerClient::SampingStop, this)},
        { "collectgarbage", std::bind(&HeapProfilerClient::CollectGarbage, this)}
    };

    auto entry = dispatcherTable.find(cmd);
    if (entry != dispatcherTable.end() && entry->second != nullptr) {
        entry->second();
        LOGI("DispatcherCmd reqStr1: %{public}s", cmd.c_str());
        return true;
    }

    LOGI("unknown command: %{public}s", cmd.c_str());
    return false;
}

int HeapProfilerClient::HeapDumpCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, HEAPDUMP);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.takeHeapSnapshot");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("reportProgress", true);
    params->Add("captureNumericValue", true);
    params->Add("exposeInternals", false);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "HeapProfiler");
    }
    return 0;
}

int HeapProfilerClient::AllocationTrackCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, ALLOCATION);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.startTrackingHeapObjects");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("trackAllocations", true);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "HeapProfiler");
    }
    return 0;
}

int HeapProfilerClient::AllocationTrackStopCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, ALLOCATION_STOP);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.stopTrackingHeapObjects");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("reportProgress", true);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "HeapProfiler");
    }
    return 0;
}

int HeapProfilerClient::Enable()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, ENABLE);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.enable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "HeapProfiler");
    }
    return 0;
}

int HeapProfilerClient::Disable()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, DISABLE);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.disable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "HeapProfiler");
    }
    return 0;
}

int HeapProfilerClient::Samping()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, SAMPLING);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.startSampling");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("samplingInterval", SAMPLING_INTERVAL);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "HeapProfiler");
    }
    return 0;
}

int HeapProfilerClient::SampingStop()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, SAMPLING_STOP);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.stopSampling");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "HeapProfiler");
    }
    return 0;
}

int HeapProfilerClient::CollectGarbage()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    idEventMap_.emplace(id, COLLECT_GARBAGE);
    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "HeapProfiler.collectGarbage");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "HeapProfiler");
    }
    return 0;
}

void HeapProfilerClient::RecvReply(std::unique_ptr<PtJson> json)
{
    if (json == nullptr) {
        LOGE("json parse error");
        return;
    }
    if (!json->IsObject()) {
        LOGE("json parse format error");
        json->ReleaseRoot();
        return;
    }
    std::unique_ptr<PtJson> result;
    Result ret = json->GetObject("result", &result);
    if (ret == Result::SUCCESS) {
        SaveHeapsamplingData(std::move(result));
        return;
    }
    std::string wholeMethod;
    std::string method;
    ret = json->GetString("method", &wholeMethod);
    if (ret != Result::SUCCESS || wholeMethod.empty()) {
        LOGE("find method error");
        return;
    }
    std::string::size_type length = wholeMethod.length();
    std::string::size_type indexPoint = 0;
    indexPoint = wholeMethod.find_first_of('.', 0);
    if (indexPoint == std::string::npos || indexPoint == 0 || indexPoint == length - 1) {
        return;
    }
    method = wholeMethod.substr(indexPoint + 1, length);
    if (method == "lastSeenObjectId") {
        isAllocationMsg_ = true;
    }
    std::unique_ptr<PtJson> params;
    ret = json->GetObject("params", &params);
    if (ret != Result::SUCCESS) {
        LOGE("find params error");
        return;
    }
    std::string chunk;
    ret = params->GetString("chunk", &chunk);
    if (ret == Result::SUCCESS) {
        SaveHeapSnapshotAndAllocationTrackData(chunk);
    }
    return;
}

void HeapProfilerClient::SaveHeapSnapshotAndAllocationTrackData(const std::string &chunk)
{
    std::string head = "{\"snapshot\":\n";
    std::string heapFile;
    if (!strncmp(chunk.c_str(), head.c_str(), head.length())) {
        char date[16];
        char time[16];
        bool res = Utils::GetCurrentTime(date, time, sizeof(date));
        if (!res) {
            LOGE("arkdb: get time failed");
            return;
        }
        if (isAllocationMsg_) {
            heapFile = "Heap-" + std::to_string(sessionId_) + "-" + std::string(date) + "T" + std::string(time) +
                        ".heaptimeline";
            fileName_ = path_ + heapFile;
            std::cout << "heaptimeline file name is " << fileName_ << std::endl;
        } else {
            heapFile = "Heap-"+ std::to_string(sessionId_) + "-"  + std::string(date) + "T" + std::string(time) +
                        ".heapsnapshot";
            fileName_ = path_ + heapFile;
            std::cout << "heapsnapshot file name is " << fileName_ << std::endl;
        }
        std::cout << ">>> ";
        fflush(stdout);
    }

    std::string tail = "]\n}\n";
    std::string subStr = chunk.substr(chunk.length() - tail.length(), chunk.length());
    if (!strncmp(subStr.c_str(), tail.c_str(), tail.length())) {
        isAllocationMsg_ = false;
    }
    WriteHeapProfilerForFile(fileName_, chunk);
    return;
}

void HeapProfilerClient::SaveHeapsamplingData(std::unique_ptr<PtJson> result)
{
    std::unique_ptr<PtJson> profile;
    std::string heapFile;
    Result ret = result->GetObject("profile", &profile);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: get profile failed");
        return;
    }
    char date[16];
    char time[16];
    bool res = Utils::GetCurrentTime(date, time, sizeof(date));
    if (!res) {
        LOGE("arkdb: get time failed");
        return;
    }
    heapFile = "Heap-" + std::to_string(sessionId_) + "-" + std::string(date) + "T" + std::string(time) +
                ".heapprofile";
    fileName_ = path_ + heapFile;
    std::cout << "heapprofile file name is " << fileName_ << std::endl;
    std::cout << ">>> ";
    fflush(stdout);

    WriteHeapProfilerForFile(fileName_, profile->Stringify());
    return;
}

bool HeapProfilerClient::WriteHeapProfilerForFile(const std::string &fileName, const std::string &data)
{
    std::ofstream ofs;
    std::string realPath;
    bool res = Utils::RealPath(fileName, realPath, false);
    if (!res) {
        LOGE("arkdb: path is not realpath!");
        return false;
    }
    ofs.open(fileName.c_str(), std::ios::app);
    if (!ofs.is_open()) {
        LOGE("arkdb: file open error!");
        return false;
    }
    size_t strSize = data.size();
    ofs.write(data.c_str(), strSize);
    ofs.close();
    ofs.clear();
    return true;
}
} // OHOS::ArkCompiler::Toolchain