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

#include "debugger_arkapi.h"
#include <fstream>
#include <string_view>
#include "libarkbase/os/mutex.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/tooling/sampler/samples_record.h"
#include "runtime/tooling/sampler/sampling_profiler.h"
#include "types/profile_result.h"
#include "libarkbase/utils/json_builder.h"

namespace ark {
constexpr std::string_view ARK_DEBUGGER_LIB_PATH = "libark_inspector.z.so";
void *ArkDebugNativeAPI::gHybridDebuggerHandle_ = nullptr;
#ifdef PANDA_TARGET_WINDOWS
static void *Load(std::string_view libraryName)
{
    HMODULE module = LoadLibrary(libraryName.data());
    void *handle = reinterpret_cast<void *>(module);
    return handle;
}

static void *ResolveSymbol(void *handle, std::string_view symbol)
{
    HMODULE module = reinterpret_cast<HMODULE>(handle);
    void *addr = reinterpret_cast<void *>(GetProcAddress(module, symbol.data()));
    return addr;
}
#else  // UNIX_PLATFORM
static void *Load(std::string_view libraryName)
{
    void *handle = dlopen(libraryName.data(), RTLD_LAZY);
    return handle;
}

static void *ResolveSymbol(void *handle, std::string_view symbol)
{
    void *addr = dlsym(handle, symbol.data());
    return addr;
}
#endif

bool ArkDebugNativeAPI::NotifyDebugMode([[maybe_unused]] int tid, [[maybe_unused]] int32_t instanceId,
                                        [[maybe_unused]] bool debugApp, [[maybe_unused]] void *vm,
                                        [[maybe_unused]] DebuggerPostTask &debuggerPostTask)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, tid = " << tid << ", debugApp = " << debugApp
                        << ", instanceId = " << instanceId;
    if ((!debugApp) || (!Runtime::GetOptions().IsDebuggerEnable())) {
        LOG(ERROR, DEBUGGER) << "Runtime::GetOptions().IsDebuggerEnable()" << Runtime::GetOptions().IsDebuggerEnable();
        return true;
    }
    if (!debuggerPostTask) {
        LOG(ERROR, DEBUGGER) << "ArkDebugNativeAPI::NotifyDebugMode, debuggerPostTask is nullptr";
        return false;
    }

    gHybridDebuggerHandle_ = Load(ARK_DEBUGGER_LIB_PATH);
    if (gHybridDebuggerHandle_ == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] gHybridDebuggerHandle_ load fail";
        return false;
    }
    Runtime::GetCurrent()->SetDebugMode(true);

    // store debugger postTask in inspector.
    using StoreDebuggerInfo = void (*)(int, void *, const DebuggerPostTask &);
    auto symOfStoreDebuggerInfo =
        reinterpret_cast<StoreDebuggerInfo>(ResolveSymbol(gHybridDebuggerHandle_, "StoreDebuggerInfo"));
    if (symOfStoreDebuggerInfo == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] Resolve StoreDebuggerInfo symbol fail: " << dlerror();
        return false;
    }
    symOfStoreDebuggerInfo(tid, vm, debuggerPostTask);

#ifndef PANDA_TARGET_ARM32
    // Initialize debugger
    using InitializeDebuggerForSocketpair = bool (*)(void *, bool);
    auto sym = reinterpret_cast<InitializeDebuggerForSocketpair>(
        ResolveSymbol(gHybridDebuggerHandle_, "InitializeDebuggerForSocketpair"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] InitializeDebuggerForSocketpair symbol fail: " << dlerror();
        return false;
    }
    if (!sym(vm, true)) {
        LOG(ERROR, DEBUGGER) << "[NotifyDebugMode] InitializeDebuggerForSocketpair fail";
        return false;
    }
#endif

    using WaitForDebugger = void (*)(void *);
    auto symOfWaitForDebugger =
        reinterpret_cast<WaitForDebugger>(ResolveSymbol(gHybridDebuggerHandle_, "WaitForDebugger"));
    if (symOfWaitForDebugger == nullptr) {
        LOG(ERROR, DEBUGGER) << "Resolve symbol WaitForDebugger fail: " << dlerror();
        return false;
    }
    symOfWaitForDebugger(vm);

    return true;
}

bool ArkDebugNativeAPI::StopDebugger([[maybe_unused]] void *vm)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StopDebugger";

    using StopDebug = void (*)(void *);
    auto sym = reinterpret_cast<StopDebug>(ResolveSymbol(gHybridDebuggerHandle_, "StopDebug"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "g_initializeInspectorForStatic load error" << dlerror();
        return false;
    }

    sym(vm);
    ark::Runtime::GetCurrent()->SetDebugMode(false);
    return true;
}

bool ArkDebugNativeAPI::StartDebuggerForSocketPair([[maybe_unused]] int tid, [[maybe_unused]] int socketfd)
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::StartDebugForSocketPair, tid = " << tid << " socketfd is " << socketfd;
    if (!Runtime::GetOptions().IsDebuggerEnable()) {
        LOG(ERROR, DEBUGGER) << "Runtime::GetOptions().IsDebuggerEnable() " << Runtime::GetOptions().IsDebuggerEnable();
        return false;
    }
    using StartDebuggerForSocketpair = bool (*)(int, int, bool);
    auto sym =
        reinterpret_cast<StartDebuggerForSocketpair>(ResolveSymbol(gHybridDebuggerHandle_, "StartDebugForSocketpair"));
    if (sym == nullptr) {
        LOG(ERROR, DEBUGGER) << "g_initializeInspectorForStatic load error:%{public}s" << dlerror();
        return false;
    }
    bool ret = sym(tid, socketfd, true);
    if (!ret) {
        // Reset the config
        ark::Runtime::GetCurrent()->SetDebugMode(false);
    }
    return ret;
}

bool ArkDebugNativeAPI::IsDebugModeEnabled()
{
    LOG(INFO, DEBUGGER) << "ArkDebugNativeAPI::IsDebugModeEnabled is " << ark::Runtime::GetCurrent()->IsDebugMode();

    return ark::Runtime::GetCurrent()->IsDebugMode();
}

// NOLINTBEGIN(fuchsia-statically-constructed-objects)
static std::shared_ptr<tooling::sampler::SamplesRecord> g_profileInfoBuffer = nullptr;
static os::memory::Mutex g_profileMutex;
static std::string g_filePath;
// NOLINTEND(fuchsia-statically-constructed-objects)

bool ArkDebugNativeAPI::StartProfiling(const std::string &filePath, uint32_t interval)
{
    if (filePath.empty()) {
        LOG(DEBUG, PROFILER) << "File path is empty";
        return false;
    }
    os::memory::LockHolder lock(g_profileMutex);
    if (ark::Runtime::GetCurrent() == nullptr) {
        LOG(DEBUG, PROFILER) << "That runtime is not created.";
        return false;
    }
    if (g_profileInfoBuffer) {
        LOG(WARNING, PROFILER) << "Repeatedly start cpu profiler, please call StopProfiling first.";
        return false;
    }
    g_profileInfoBuffer = std::make_shared<tooling::sampler::SamplesRecord>();
    g_profileInfoBuffer->SetThreadStartTime(tooling::sampler::Sampler::GetMicrosecondsTimeStamp());
    if (!Runtime::GetCurrent()->GetTools().IsSamplingProfilerCreate()) {
        Runtime::GetCurrent()->GetTools().CreateSamplingProfiler();
    }
    auto result = Runtime::GetCurrent()->GetTools().StartSamplingProfiler(
        std::make_unique<tooling::sampler::InspectorStreamWriter>(g_profileInfoBuffer), interval);
    if (!result) {
        g_profileInfoBuffer.reset();
        LOG(DEBUG, PROFILER) << "Fatal, profiler start failed";
        return false;
    }
    g_filePath = filePath;
    return true;
}

bool ArkDebugNativeAPI::StopProfiling()
{
    os::memory::LockHolder lock(g_profileMutex);
    if (Runtime::GetCurrent() == nullptr) {
        LOG(DEBUG, PROFILER) << "That runtime is not created.";
        g_profileInfoBuffer.reset();
        return false;
    }

    if (!g_profileInfoBuffer) {
        LOG(DEBUG, PROFILER) << "Fatal, profiler inactive";
        g_profileInfoBuffer.reset();
        return false;
    }

    Runtime::GetCurrent()->GetTools().StopSamplingProfiler();
    auto profileInfoPtr = g_profileInfoBuffer->GetAllThreadsProfileInfos();
    if (!profileInfoPtr) {
        g_profileInfoBuffer.reset();
        LOG(WARNING, PROFILER) << "The CPU profiler did not collect any data.";
        return true;
    }
    g_profileInfoBuffer.reset();
    tooling::inspector::Profile profile(std::move(profileInfoPtr));
    JsonObjectBuilder builder;
    profile.Serialize(builder);
    std::string jsonData = std::move(builder).Build();
    std::ofstream outputFile(g_filePath);
    if (!outputFile.is_open()) {
        LOG(ERROR, PROFILER) << "Failed to open output file: " << g_filePath;
        return false;
    }
    outputFile << jsonData;
    if (outputFile.fail()) {
        LOG(ERROR, PROFILER) << "Failed to write profiling data to file: " << g_filePath;
        return false;
    }
    outputFile.close();
    g_filePath.clear();
    return true;
}
}  // namespace ark
