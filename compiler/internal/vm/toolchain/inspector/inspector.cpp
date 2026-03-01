/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "inspector.h"
#include "init_static.h"

#include <shared_mutex>
#if defined(OHOS_PLATFORM)
#include <syscall.h>
#endif
#include <thread>
#if defined(OHOS_PLATFORM)
#include <unistd.h>
#endif

#include "common/log_wrapper.h"
#include "library_loader.h"

#if defined(IOS_PLATFORM)
#include "tooling/dynamic/debugger_service.h"
#endif

#if defined(ENABLE_FFRT_INTERFACES)
#include "ffrt.h"
#endif

#include <string>
#include <regex>

namespace OHOS::ArkCompiler::Toolchain {
namespace {
enum DispatchStatus : int32_t {
    UNKNOWN = 0,
    DISPATCHING,
    DISPATCHED
};

using SetDebugApp = void(*)(void*);
using InitializeDebugger = void(*)(void*, const std::function<void(const void*, const std::string&)>&);
using UninitializeDebugger = void(*)(void*);
using WaitForDebugger = void(*)(void*);
using OnMessage = void(*)(void*, std::string&&);
using ProcessMessage = void(*)(void*);
using GetDispatchStatus = int32_t(*)(void*);
using GetCallFrames = DebugResponse(*)(void*);
using OperateDebugMessage = DebugResponse(*)(void*, const char*);

SetDebugApp g_setDebugApp = nullptr;
OnMessage g_onMessage = nullptr;
InitializeDebugger g_initializeDebugger = nullptr;
UninitializeDebugger g_uninitializeDebugger = nullptr;
WaitForDebugger g_waitForDebugger = nullptr;
ProcessMessage g_processMessage = nullptr;
GetDispatchStatus g_getDispatchStatus = nullptr;
GetCallFrames g_getCallFrames = nullptr;
OperateDebugMessage g_operateDebugMessage = nullptr;

std::atomic<bool> g_hasArkFuncsInited = false;
std::unordered_map<const void*, Inspector*> g_inspectors;
std::unordered_map<int, std::pair<void*, const DebuggerPostTask>> g_debuggerInfo;
std::shared_mutex g_mutex;

#if !defined(IOS_PLATFORM)
thread_local void* g_handle = nullptr;
#endif
thread_local void* g_vm = nullptr;

#if !defined(IOS_PLATFORM)
#if defined(WINDOWS_PLATFORM)
constexpr char ARK_DEBUGGER_SHARED_LIB[] = "libark_tooling.dll";
#elif defined(MAC_PLATFORM)
constexpr char ARK_DEBUGGER_SHARED_LIB[] = "libark_tooling.dylib";
#else
constexpr char ARK_DEBUGGER_SHARED_LIB[] = "libark_tooling.so";
#endif
#endif

struct ClientArgs {
    void* server;
    bool isHybrid;
};

void* HandleHybridClient(void* arg)
{
    ClientArgs* args = static_cast<ClientArgs*>(arg);
    auto server = args->server;
    LOGI("HandleClient");
    if (server == nullptr) {
        LOGE("HandleClient server nullptr");
        return nullptr;
    }

#if defined(IOS_PLATFORM) || defined(MAC_PLATFORM)
    pthread_setname_np("OS_DebugThread");
#else
    pthread_setname_np(pthread_self(), "OS_DebugThread");
#endif

    static_cast<WsServer*>(server)->RunServer(args->isHybrid);
    delete args;
    return nullptr;
}

void* HandleNormalClient(void* const server)
{
    LOGI("HandleClient");
    if (server == nullptr) {
        LOGE("HandleClient server nullptr");
        return nullptr;
    }

#if defined(IOS_PLATFORM) || defined(MAC_PLATFORM)
    pthread_setname_np("OS_DebugThread");
#else
    pthread_setname_np(pthread_self(), "OS_DebugThread");
#endif

    static_cast<WsServer*>(server)->RunServer();
    return nullptr;
}

#if !defined(IOS_PLATFORM)
bool LoadArkDebuggerLibrary()
{
    if (g_handle != nullptr) {
        LOGI("LoadArkDebuggerLibrary, handle has already loaded");
        return true;
    }
    g_handle = Load(ARK_DEBUGGER_SHARED_LIB);
    if (g_handle == nullptr) {
        LOGE("LoadArkDebuggerLibrary, handle load failed");
        return false;
    }
    return true;
}

void* GetArkDynFunction(const char* symbol)
{
    return ResolveSymbol(g_handle, symbol);
}
#endif

void SendReply(const void* vm, const std::string& message)
{
    std::shared_lock<std::shared_mutex> lock(g_mutex);
    auto iter = g_inspectors.find(vm);
    if (iter != g_inspectors.end() && iter->second != nullptr &&
        iter->second->websocketServer_ != nullptr) {
        iter->second->websocketServer_->SendReply(message);
    }
}

void ResetServiceLocked(void *vm, bool isCloseHandle)
{
    auto iter = g_inspectors.find(vm);
    if (iter != g_inspectors.end() && iter->second != nullptr &&
        iter->second->websocketServer_ != nullptr) {
        iter->second->websocketServer_->StopServer();
        delete iter->second;
        iter->second = nullptr;
        g_inspectors.erase(iter);
    }
#if !defined(IOS_PLATFORM)
    if (g_handle != nullptr && isCloseHandle) {
        CloseHandle(g_handle);
        g_handle = nullptr;
    }
#endif
}

bool InitializeInspector(
    void* vm, const DebuggerPostTask& debuggerPostTask,
    const DebugInfo& debugInfo,
    int tidForSocketPair = 0, bool isHybrid = false)
{
    std::unique_lock<std::shared_mutex> lock(g_mutex);
    auto iter = g_inspectors.find(vm);
    if (iter != g_inspectors.end()) {
        LOGW("Inspector already exist!");
        return true;
    }

    Inspector *newInspector = new Inspector();
    g_inspectors.emplace(vm, newInspector);

    newInspector->tidForSocketPair_ = tidForSocketPair;
    newInspector->tid_ = pthread_self();
    newInspector->vm_ = vm;
    newInspector->debuggerPostTask_ = debuggerPostTask;
    newInspector->websocketServer_ = std::make_unique<WsServer>(debugInfo,
        std::bind(&Inspector::OnMessage, newInspector, std::placeholders::_1, isHybrid));

    pthread_t tid;
    
    auto server = static_cast<void *>(newInspector->websocketServer_.get());
    if (isHybrid) {
        ClientArgs* args = new ClientArgs{server, isHybrid};
        if (pthread_create(&tid, nullptr, &HandleHybridClient, args)) {
            LOGE("Create inspector thread failed");
            return false;
        };
    } else {
        if (pthread_create(&tid, nullptr, &HandleNormalClient, server)) {
            LOGE("Create inspector thread failed");
            return false;
        };
    }

    newInspector->websocketServer_->tid_ = tid;

    return true;
}

#if !defined(IOS_PLATFORM)
bool InitializeArkFunctionsOthers()
{
    g_initializeDebugger = reinterpret_cast<InitializeDebugger>(
        GetArkDynFunction("InitializeDebugger"));
    if (g_initializeDebugger == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    g_uninitializeDebugger = reinterpret_cast<UninitializeDebugger>(
        GetArkDynFunction("UninitializeDebugger"));
    if (g_uninitializeDebugger == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    g_waitForDebugger = reinterpret_cast<WaitForDebugger>(
        GetArkDynFunction("WaitForDebugger"));
    if (g_waitForDebugger == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    g_onMessage = reinterpret_cast<OnMessage>(
        GetArkDynFunction("OnMessage"));
    if (g_onMessage == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    g_getDispatchStatus = reinterpret_cast<GetDispatchStatus>(
        GetArkDynFunction("GetDispatchStatus"));
    if (g_getDispatchStatus == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    g_processMessage = reinterpret_cast<ProcessMessage>(
        GetArkDynFunction("ProcessMessage"));
    if (g_processMessage == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    g_getCallFrames = reinterpret_cast<GetCallFrames>(
        GetArkDynFunction("GetCallFrames"));
    if (g_getCallFrames == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    g_operateDebugMessage = reinterpret_cast<OperateDebugMessage>(
        GetArkDynFunction("OperateDebugMessage"));
    if (g_operateDebugMessage == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    g_setDebugApp = reinterpret_cast<SetDebugApp>(
        GetArkDynFunction("SetDebugApp"));
    if (g_setDebugApp == nullptr) {
        ResetServiceLocked(g_vm, true);
        return false;
    }
    return true;
}
#else
bool InitializeArkFunctionsIOS()
{
    using namespace panda::ecmascript;
    g_setDebugApp = reinterpret_cast<SetDebugApp>(&tooling::SetDebugApp);
    g_initializeDebugger = reinterpret_cast<InitializeDebugger>(&tooling::InitializeDebugger);
    g_uninitializeDebugger = reinterpret_cast<UninitializeDebugger>(&tooling::UninitializeDebugger);
    g_waitForDebugger = reinterpret_cast<WaitForDebugger>(&tooling::WaitForDebugger);
    g_onMessage = reinterpret_cast<OnMessage>(&tooling::OnMessage);
    g_getDispatchStatus = reinterpret_cast<GetDispatchStatus>(&tooling::GetDispatchStatus);
    g_processMessage = reinterpret_cast<ProcessMessage>(&tooling::ProcessMessage);
    g_getCallFrames = reinterpret_cast<GetCallFrames>(&tooling::GetCallFrames);
    g_operateDebugMessage = reinterpret_cast<OperateDebugMessage>(&tooling::OperateDebugMessage);
    return true;
}
#endif

bool InitializeArkFunctions()
{
    // no need to initialize again in case of multi-instance
    if (g_hasArkFuncsInited) {
        return true;
    }

    std::unique_lock<std::shared_mutex> lock(g_mutex);
    if (g_hasArkFuncsInited) {
        return true;
    }
#if !defined(IOS_PLATFORM)
    if (!InitializeArkFunctionsOthers()) {
        return false;
    }
#else
    InitializeArkFunctionsIOS()
#endif

    g_hasArkFuncsInited = true;
    return true;
}

} // namespace

void Inspector::OnMessage(std::string&& msg, bool isHybrid)
{
    if (isHybrid) {
        static const std::regex hasKey("\"sessionId\"\\s*:\\s*\"");
        auto pos = std::regex_search(msg, hasKey);
        if (!pos) {
            g_onMessage(vm_, std::move(msg)); //没有sessionId走1.1
        } else {
            static const std::regex valPattern("\"sessionId\"\\s*:\\s*\"(\\d*)\"");
            std::smatch m;
            if (std::regex_search(msg, m, valPattern) &&
                (m[1].str().empty() || std::all_of(m[1].first, m[1].second, ::isdigit))) {
                OnMessageStatic(std::move(msg));
                return;
            } else {
                LOGE("sessionId value must be empty or numeric string");
                return;
            }
        }
    } else {
        g_onMessage(vm_, std::move(msg));
    }
    // message will be processed soon if the debugger thread is in running or waiting status
    if (g_getDispatchStatus(vm_) != DispatchStatus::UNKNOWN) {
        return;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(DELAY_CHECK_DISPATCH_STATUS));
    if (g_getDispatchStatus(vm_) != DispatchStatus::UNKNOWN) {
        return;
    }
    // the debugger thread maybe in idle status, so try to post a task to wake it up
    if (debuggerPostTask_ != nullptr) {
        if (tidForSocketPair_ == 0) {
            debuggerPostTask_([tid = tid_, vm = vm_] {
                if (tid != pthread_self()) {
                    LOGE("Task not in debugger thread");
                    return;
                }
                g_processMessage(vm);
            });
        } else {
#if defined(OHOS_PLATFORM)
            debuggerPostTask_([tid = tidForSocketPair_, vm = vm_] {
                uint64_t threadOrTaskId = GetThreadOrTaskId();
                if (tid != static_cast<pid_t>(threadOrTaskId)) {
                    LOGE("Task not in debugger thread for socketpair");
                    return;
                }
                g_processMessage(vm);
            });
#endif // defined(OHOS_PLATFORM)
        }
    } else {
        LOGW("No debuggerPostTask provided");
    }
}

#if defined(OHOS_PLATFORM)
uint64_t Inspector::GetThreadOrTaskId()
{
#if defined(ENABLE_FFRT_INTERFACES)
    uint64_t threadOrTaskId = ffrt_this_task_get_id();
    if (threadOrTaskId != 0) {
        return threadOrTaskId;
    } else {
        return static_cast<uint64_t>(getproctid());
    }
#else
    return static_cast<uint64_t>(getproctid());
#endif // defined(ENABLE_FFRT_INTERFACES)
}
#endif // defined(OHOS_PLATFORM)

const DebuggerPostTask &GetDebuggerPostTask(int tid)
{
    std::shared_lock<std::shared_mutex> lock(g_mutex);
    if (g_debuggerInfo.find(tid) == g_debuggerInfo.end()) {
        static DebuggerPostTask tempTask;
        return tempTask;
    }
    return g_debuggerInfo[tid].second;
}

void *GetEcmaVM(int tid)
{
    std::shared_lock<std::shared_mutex> lock(g_mutex);
    if (g_debuggerInfo.find(tid) == g_debuggerInfo.end()) {
        return nullptr;
    }
    return g_debuggerInfo[tid].first;
}

bool InitializeDebuggerForSocketpair(void* vm, bool isHybrid)
{
#if !defined(IOS_PLATFORM)
    if (!LoadArkDebuggerLibrary()) {
        return false;
    }
#endif
    if (!InitializeArkFunctions()) {
        LOGE("Initialize ark functions failed");
        return false;
    }
    if (isHybrid) {
        if (!InitializeArkFunctionsForStatic()) {
            LOGE("Initialize ark functions failed");
            return false;
        };
        g_setDebugApp(vm);
    }
    g_initializeDebugger(vm, std::bind(&SendReply, vm, std::placeholders::_2));
    return true;
}

// for ohos platform.
bool StartDebugForSocketpair(int tid, int socketfd, bool isHybrid)
{
    LOGI("StartDebugForSocketpair, tid = %{private}d, socketfd = %{private}d", tid, socketfd);
    void* vm = GetEcmaVM(tid);
    if (vm == nullptr) {
        LOGD("VM has already been destroyed");
        return false;
    }
    g_vm = vm;
    if (!InitializeDebuggerForSocketpair(vm)) {
        return false;
    }
    const DebuggerPostTask &debuggerPostTask = GetDebuggerPostTask(tid);
    DebugInfo debugInfo = {socketfd};
    if (!InitializeInspector(vm, debuggerPostTask, debugInfo, tid, isHybrid)) {
        LOGE("Initialize inspector failed");
        return false;
    }

    return true;
}

// for cross-platform, previewer and old process of StartDebugger.
bool StartDebug(const std::string& componentName, void* vm, bool isDebugMode,
    int32_t instanceId, const DebuggerPostTask& debuggerPostTask, int port)
{
    LOGI("StartDebug, componentName = %{private}s, isDebugMode = %{private}d, instanceId = %{private}d",
        componentName.c_str(), isDebugMode, instanceId);
    g_vm = vm;
#if !defined(IOS_PLATFORM)
    if (!LoadArkDebuggerLibrary()) {
        return false;
    }
#endif
    if (!InitializeArkFunctions()) {
        LOGE("Initialize ark functions failed");
        return false;
    }

    g_initializeDebugger(vm, std::bind(&SendReply, vm, std::placeholders::_2));

    int startDebugInOldProcess = -2; // start debug in old process.
    DebugInfo debugInfo = {startDebugInOldProcess, componentName, instanceId, port};
    if (!InitializeInspector(vm, debuggerPostTask, debugInfo)) {
        LOGE("Initialize inspector failed");
        return false;
    }

    if (isDebugMode && port > 0) {
        LOGI("Wait for debugger for previewer");
        g_waitForDebugger(vm);
    }
    return true;
}

void WaitForDebugger(void* vm)
{
    LOGI("WaitForDebugger");
    g_waitForDebugger(vm);
}


int StartDebugger(uint32_t port, bool breakOnStart)
{
    return StartDebuggerInitForStatic(port, breakOnStart);
}
 
int StopDebugger()
{
    return StopDebuggerInitForStatic();
}

void StopDebug(void* vm, bool isHybrid)
{
    LOGI("StopDebug start, vm is %{private}p", vm);
    std::unique_lock<std::shared_mutex> lock(g_mutex);
    auto iter = g_inspectors.find(vm);
    if (iter == g_inspectors.end() || iter->second == nullptr) {
        return;
    }
#ifdef PANDA_TARGET_MACOS
    uint32_t tid = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(g_inspectors[vm]->tid_));
#else
    uint32_t tid = g_inspectors[vm]->tid_;
#endif
#if defined(OHOS_PLATFORM)
    tid = static_cast<uint32_t>(g_inspectors[vm]->tidForSocketPair_);
#endif // defined(OHOS_PLATFORM)
    auto debuggerInfo = g_debuggerInfo.find(tid);
    if (debuggerInfo != g_debuggerInfo.end()) {
        g_debuggerInfo.erase(debuggerInfo);
    }
    ResetServiceLocked(vm, false);
    g_uninitializeDebugger(vm);
#if !defined(IOS_PLATFORM)
    if (g_handle != nullptr) {
        CloseHandle(g_handle);
        g_handle = nullptr;
    }
#endif
    if (isHybrid) {
        StopDebuggerForStatic();
    }
    LOGI("StopDebug end");
}

void StopOldDebug(int tid, const std::string& componentName)
{
    LOGI("StopDebug start, componentName = %{private}s, tid = %{private}d", componentName.c_str(), tid);
    void* vm = GetEcmaVM(tid);
    if (vm == nullptr) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(g_mutex);
    auto iter = g_inspectors.find(vm);
    if (iter == g_inspectors.end() || iter->second == nullptr) {
        return;
    }

    ResetServiceLocked(vm, false);
    LOGI("StopDebug end");
}

// for socketpair process.
void StoreDebuggerInfo(int tid, void* vm, const DebuggerPostTask& debuggerPostTask)
{
    std::unique_lock<std::shared_mutex> lock(g_mutex);
    g_debuggerInfo.erase(tid);
    g_debuggerInfo.emplace(tid, std::make_pair(vm, debuggerPostTask));
}

// The returned pointer must be released using free() after it is no longer needed.
// Failure to release the memory will result in memory leaks.
const char* GetJsBacktrace()
{
#if defined(OHOS_PLATFORM)
    const char* result = GetJsBacktraceV1().response;
    if (result != nullptr) {
        return result;
    } else {
        return "";
    }
#else
    return "";
#endif
}

const char* OperateJsDebugMessage([[maybe_unused]] const char* message)
{
#if defined(OHOS_PLATFORM)
    const char* result = OperateJsDebugMessageV1(message).response;
    if (result != nullptr) {
        return result;
    } else {
        return "";
    }
#else
    return "";
#endif
}

// The returned pointer must be released using free() after it is no longer needed.
// Failure to release the memory will result in memory leaks.
DebugResponse GetJsBacktraceV1()
{
#if defined(OHOS_PLATFORM)
    void* vm = GetEcmaVM(Inspector::GetThreadOrTaskId());
    if (g_getCallFrames == nullptr) {
        LOGE("GetCallFrames symbol resolve failed");
        return {0, nullptr};
    }
    return g_getCallFrames(vm);
#else
    return {0, nullptr};
#endif
}

DebugResponse OperateJsDebugMessageV1([[maybe_unused]] const char* message)
{
#if defined(OHOS_PLATFORM)
    if (message == nullptr) {
        LOGE("OperateDebugMessage message is nullptr");
        return {0, nullptr};
    }
    void* vm = GetEcmaVM(Inspector::GetThreadOrTaskId());
    if (g_operateDebugMessage == nullptr) {
        LOGE("OperateDebugMessage symbol resolve failed");
        return {0, nullptr};
    }
    return g_operateDebugMessage(vm, message);
#else
    return {0, nullptr};
#endif
}
} // namespace OHOS::ArkCompiler::Toolchain
