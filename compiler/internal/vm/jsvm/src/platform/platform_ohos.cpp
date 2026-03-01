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

#include "platform.h"

// v8 header
#include "v8.h"

// OHOS API header
#include "hilog/log.h"
#include "hitrace_meter.h"
#include "init_param.h"
#include "jsvm_log.h"
#include "res_sched_client.h"
#include "unistd.h"
#ifdef ENABLE_HISYSEVENT
#include "hisysevent.h"
#endif
#include <dlfcn.h>
#include <string>
#include <sys/prctl.h>
#include <unordered_set>

#define USE_C_API
namespace platform {
void OS::Abort()
{
    std::abort();
}

uint64_t OS::GetUid()
{
    return static_cast<uint64_t>(getuid());
}

uint64_t OS::GetPid()
{
    return static_cast<uint64_t>(getprocpid());
}

uint64_t OS::GetTid()
{
    return static_cast<uint64_t>(getproctid());
}

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#endif
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_DOMAIN 0xD003900
#define LOG_TAG "JSVM"

void OS::PrintString(LogLevel level, const char* string)
{
    // convert platform defined LogLevel to hilog LogLevel
    static constexpr ::LogLevel convertArray[] = { ::LogLevel::LOG_DEBUG, ::LogLevel::LOG_INFO, ::LogLevel::LOG_WARN,
                                                   ::LogLevel::LOG_ERROR, ::LogLevel::LOG_FATAL };
    static_assert(sizeof(convertArray) / sizeof(::LogLevel) == static_cast<uint64_t>(OS::LogLevel::LOG_FATAL) + 1);

    HiLogPrint(LOG_APP, convertArray[static_cast<uint64_t>(level)], LOG_DOMAIN, LOG_TAG, "%{public}s", string);
}

void OS::Print(LogLevel level, const char* format, ...)
{
    constexpr size_t maxStringSize = 1024;
    char string[maxStringSize];
    va_list arguments;
    va_start(arguments, format);
    int len = vsnprintf_s(string, sizeof(string), sizeof(string) - 1, format, arguments);
    va_end(arguments);

    if (len < 0) {
        PrintString(LogLevel::LOG_ERROR, "vsnprintf_s failed");
        return;
    }
    PrintString(level, string);
}

#define JSVM_HITRACE_TAG HITRACE_TAG_OHOS

RunJsTrace::RunJsTrace(bool runJs) : runJs(runJs)
{
    if (runJs) {
        StartTrace(JSVM_HITRACE_TAG, "PureJS");
    } else {
        FinishTrace(JSVM_HITRACE_TAG);
    }
}

RunJsTrace::RunJsTrace(const char* name) : runJs(true)
{
    StartTrace(JSVM_HITRACE_TAG, name);
}

RunJsTrace::~RunJsTrace()
{
    if (runJs) {
        FinishTrace(JSVM_HITRACE_TAG);
    } else {
        StartTrace(JSVM_HITRACE_TAG, "PureJS");
    }
}

#ifdef USE_C_API
namespace ResourceSchedule {
namespace ResType {
extern "C" void ReportData(uint32_t resType,
                           int64_t value,
                           const std::unordered_map<std::string, std::string>& mapPayLoad);

enum : uint32_t { RES_TYPE_REPORT_KEY_THREAD = 39 };

enum ReportChangeStatus : int64_t { CREATE = 0, REMOVE = 1 };

enum ThreadRole : int64_t {
    USER_INTERACT = 0,
    NORMAL_DISPLAY = 1,
    IMPORTANT_DISPLAY = 2,
    NORMAL_AUDIO = 3,
    IMPORTANT_AUDIO = 4,
    IMAGE_DECODE = 5
};
} // namespace ResType
} // namespace ResourceSchedule
#endif

static bool isJitMode = true;
namespace ohos {
#define JITFORT_QUERY_ENCAPS 'E'
#define HM_PR_SET_JITFORT 0x6a6974

bool ProcessBundleNameParam(std::string& bundleName);
 
bool InJitMode()
{
    return isJitMode;
}

inline bool HasJitfortACL()
{
    return (prctl(HM_PR_SET_JITFORT, JITFORT_QUERY_ENCAPS, 0) == 0);
}

#ifdef USE_C_API
void ReportKeyThread(ThreadRole role)
{
    uint64_t uid = OS::GetUid();
    uint64_t tid = OS::GetTid();
    uint64_t pid = OS::GetPid();
    std::unordered_map<std::string, std::string> payLoad = { { "uid", std::to_string(uid) },
                                                             { "pid", std::to_string(pid) },
                                                             { "tid", std::to_string(tid) },
                                                             { "role", std::to_string(role) } };
    ReportData(ResourceSchedule::ResType::RES_TYPE_REPORT_KEY_THREAD,
               ResourceSchedule::ResType::ReportChangeStatus::CREATE, payLoad);
}
#else
void ReportKeyThread(ThreadRole role)
{
    static_assert(static_cast<int64_t>(ThreadRole::IMPORTANT_DISPLAY) ==
                  static_cast<int64_t>(OHOS::ResourceSchedule::ResType::IMPORTANT_DISPLAY));
    static_assert(static_cast<int64_t>(ThreadRole::USER_INTERACT) ==
                  static_cast<int64_t>(OHOS::ResourceSchedule::ResType::USER_INTERACT));

    uint64_t uid = OS::GetUid();
    uint64_t tid = OS::GetTid();
    uint64_t pid = OS::GetPid();
    std::unordered_map<std::string, std::string> payLoad = { { "uid", std::to_string(uid) },
                                                             { "pid", std::to_string(pid) },
                                                             { "tid", std::to_string(tid) },
                                                             { "role", std::to_string(role) } };
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        OHOS::ResourceSchedule::ResType::RES_TYPE_REPORT_KEY_THREAD,
        OHOS::ResourceSchedule::ResType::ReportChangeStatus::CREATE, payLoad);
}
#endif

bool ReadAdvancedSecurityMode()
{
    void *hdl = dlopen("/system/lib64/platformsdk/libdsmm_innersdk.z.so", RTLD_LAZY);
    if (!hdl) {
        LOG(Error) << "[AdvancedSecurityMode]: dlopen failed";
        return false;
    }

    using AdvSecModeGetPtr = int32_t (*)(const char *feature, uint32_t featureLen,
                                         const char *param, uint32_t paramLen, uint32_t *state);
    AdvSecModeGetPtr func = reinterpret_cast<AdvSecModeGetPtr>(dlsym(hdl, "AdvancedSecurityModeGetStateByFeature"));
    if (!func) {
        LOG(Error) << "[AdvancedSecurityMode]: dlsym failed";
        dlclose(hdl);
        return false;
    }

    const char* featureName = "RESTRICTED_JSVM_FEATURES";
    const char* emptyParam = "{}";
    uint32_t state = 0;
    int32_t ret = func(featureName, strlen(featureName), emptyParam, strlen(emptyParam), &state);
    dlclose(hdl);

    if (ret != 0) {
        LOG(Error) << "[AdvancedSecurityMode]: AdvSecModeGetPtr failed";
        return false;
    }
    return static_cast<bool>(state);
}

void SetSecurityMode()
{
    constexpr size_t secArgCnt = 2;
    if (ReadAdvancedSecurityMode() || !HasJitfortACL()) {
        isJitMode = false;
        int secArgc = secArgCnt;
        constexpr bool removeFlag = false;
        const char* secArgv[secArgCnt] = { "jsvm", "--jitless" };
        v8::V8::SetFlagsFromCommandLine(&secArgc, const_cast<char**>(reinterpret_cast<const char**>(secArgv)),
                                        removeFlag);
    }
}

constexpr int MAX_FILE_LENGTH = 32 * 1024 * 1024;

bool LoadStringFromFile(const std::string& filePath, std::string& content)
{
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        return false;
    }

    file.seekg(0, std::ios::end);
    const long fileLength = file.tellg();
    if (fileLength > MAX_FILE_LENGTH) {
        return false;
    }

    content.clear();
    file.seekg(0, std::ios::beg);
    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(content));
    return true;
}

std::unique_ptr<char[]> ProcessBundleNameParam()
{
    int pid = getprocpid();
    static std::string bundleName;
    if (bundleName == "") {
        std::string filePath = "/proc/" + std::to_string(pid) + "/cmdline";
        if (LoadStringFromFile(filePath, bundleName) && !bundleName.empty()) {
            auto pos = bundleName.find(":");
            if (pos != std::string::npos) {
                bundleName = bundleName.substr(0, pos);
            }
        } else {
            bundleName = "INVALID_BUNDLE_NAME";
        }
    }
    std::unique_ptr<char[]> name = std::make_unique<char[]>(bundleName.size() + 1);
    errno_t ret = strcpy_s(name.get(), bundleName.size() + 1, bundleName.c_str());
    if (ret != EOK) {
        name.get()[0] = '\0';
    }
    return std::move(name);
}

void WriteHisysevent(HiSysEventParam* params, int size)
{
    OH_HiSysEvent_Write(OHOS::HiviewDFX::HiSysEvent::Domain::JSVM_RUNTIME, "APP_STATS",
                        HiSysEventEventType::HISYSEVENT_STATISTIC,
                        params, size);
}

void WriteJSVMInitToHisysevent()
{
#ifdef ENABLE_HISYSEVENT
    std::unique_ptr<char[]> name = ProcessBundleNameParam();
    HiSysEventParam param = {
        .name = "BUNDLE_NAME",
        .t = HISYSEVENT_STRING,
        .v = { .s = name.get() },
        .arraySize = 0,
    };
    HiSysEventParam params[] = { param };
    WriteHisysevent(params, sizeof(params) / sizeof(params[0]));
#endif
}

void WriteJitBlockedToHisysevent()
{
#ifdef ENABLE_HISYSEVENT
    static bool blockHasWritten = false;
    if (blockHasWritten) { return; }
    blockHasWritten = true;
    std::unique_ptr<char[]> name = ProcessBundleNameParam();
    HiSysEventParam param = {
        .name = "BUNDLE_NAME",
        .t = HISYSEVENT_STRING,
        .v = { .s = name.get() },
        .arraySize = 0,
    };
    HiSysEventParam runJitParam = {
        .name = "IF_JIT_BLOCKED",
        .t = HISYSEVENT_BOOL,
        .v = { .b = true },
        .arraySize = 0,
    };
    HiSysEventParam params[] = { param, runJitParam };
    WriteHisysevent(params, sizeof(params) / sizeof(params[0]));
#endif
}
} // namespace ohos

} // namespace platform
