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

#include "Codira/Macro/InvokeUtil.h"
#include "Codira/Basic/Print.h"
#include "Codira/Macro/InvokeConfig.h"
#include "Codira/Utils/Utils.h"
#include "Codira/Utils/StdUtils.h"

namespace Codira {
using namespace Utils;
using namespace InvokeRuntime;

namespace {
const static std::string G_CODE_RUNTIME_INIT = "InitCODERuntime";
const static std::string G_CODE_RUNTIME_FINI = "FiniCODERuntime";
const static std::string G_CODE_NEW_TASK_FROM_C = "RunCODETask";
const static std::string G_RELEASE_HANDLE_FROM_C = "ReleaseHandle";
using CodiraInitFromC = int64_t (*)(ConfigParam*);
using CodiraFiniFromC = int64_t (*)();
std::vector<HANDLE> g_openedLibHandles;
std::mutex g_openedLibMutex;

#ifdef _WIN32
// environment variable names in windows is case-insensitive, so they all uppercase in frontend global option
const static std::string G_CODEHEAPSIZE = "CODEHEAPSIZE";
const static std::string G_CODESTACKSIZE = "CODESTACKSIZE";
#else
const static std::string G_CODEHEAPSIZE = "codeHeapSize";
const static std::string G_CODESTACKSIZE = "codeStackSize";
#endif

const static size_t UNIT_LEN = 2; // supports "kb", "mb", "gb".
const static size_t KB = 1024;
const static size_t MB = KB * KB;
const static size_t G_MIN_HEAP_SIZE = 4UL * KB;  // min: 4MB.
const static size_t G_MIN_STACK_SIZE = 64;       // min: 64KB.
const static size_t G_MAX_STACK_SIZE = 1UL * MB; // max: 1GB.

size_t GetSizeFromEnv(std::string& str)
{
    (void)str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    size_t len = str.length();
    // The last two characters are units, such as "kb".
    if (len <= UNIT_LEN) {
        return SIZE_MAX;
    }
    // Split size and unit.
    auto unit = str.substr(len - UNIT_LEN, UNIT_LEN);
    (void)std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
    // unit must be kb mb or gb
    if (unit != "kb" && unit != "mb" && unit != "gb") {
        return SIZE_MAX;
    }
    int32_t iRes = 0;
    auto num = str.substr(0, len - UNIT_LEN);
    if (auto r = Stoi(num)) {
        // str must be a number
        iRes = *r;
        // number must > 0
        if (iRes <= 0) {
            return 0;
        }
    } else {
        return SIZE_MAX;
    }

    size_t tempSize = static_cast<size_t>(iRes);
    if (unit == "kb") {
        return tempSize;
    } else if (unit == "mb") {
        // unit: 1024 * 1KB = 1MB
        return tempSize * KB;
    } else if (unit == "gb") {
        // unit: 1024 * 1024 * 1KB = 1GB
        return tempSize * MB;
    }
    return SIZE_MAX;
}

/**
 * Get heap size from environment variable.
 * The unit must be added when configuring "codeHeapSize", it supports "kb", "mb", "gb".
 * Valid heap size must >= 4MB.
 * for example:
 *     export codeHeapSize = 16GB
 */
size_t GetHeapSizeFromEnv(std::unordered_map<std::string, std::string>& envs)
{
    if (envs.find(G_CODEHEAPSIZE) == envs.end()) {
        return HEAP_SIZE;
    }
    auto heapSize = GetSizeFromEnv(envs.at(G_CODEHEAPSIZE));
    if (heapSize == SIZE_MAX) {
        Warningln("unsupported codeHeapSize for macro, using 1GB as default size");
        return HEAP_SIZE;
    }
    if (heapSize < G_MIN_HEAP_SIZE) {
        Warningln("unsupported codeHeapSize for macro, must >= 4MB, using 1GB as default size");
        return HEAP_SIZE;
    }
    return heapSize;
}

/**
 * Get stack size from environment variable.
 * The unit must be added when configuring "codeStackSize", it supports "kb", "mb", "gb".
 * Valid stack size range is [64KB, 1GB].
 * for example:
 *     export codeStackSize = 128kb
 */
size_t GetStackSizeFromEnv(std::unordered_map<std::string, std::string>& envs)
{
    if (envs.find(G_CODESTACKSIZE) == envs.end()) {
        return CO_STACK_SIZE;
    }
    auto stackSize = GetSizeFromEnv(envs.at(G_CODESTACKSIZE));
    if (stackSize == SIZE_MAX) {
        Warningln("unsupported codeStackSize for macro, using 4MB as default size");
        return CO_STACK_SIZE;
    }
    if (stackSize < G_MIN_STACK_SIZE || stackSize > G_MAX_STACK_SIZE) {
        Warningln("unsupported codeStackSize for macro, the valid range is [64KB, 1GB], using 4MB as default size");
        return CO_STACK_SIZE;
    }
    return stackSize;
}
} // namespace

void InvokeRuntime::SetOpenedLibHandles(HANDLE handle)
{
    std::unique_lock<std::mutex> lock(g_openedLibMutex);
    (void)g_openedLibHandles.emplace_back(handle);
}

std::vector<HANDLE> InvokeRuntime::GetOpenedLibHandles()
{
    std::unique_lock<std::mutex> lock(g_openedLibMutex);
    return g_openedLibHandles;
}

void InvokeRuntime::ClearOpenedLibHandles()
{
    std::unique_lock<std::mutex> lock(g_openedLibMutex);
    g_openedLibHandles.clear();
}

int64_t InvokeRuntime::CallRuntime(
    const HANDLE handle, const std::string& method, std::unordered_map<std::string, std::string>& envs)
{
    auto runtimeFunc = reinterpret_cast<CodiraInitFromC>(GetMethod(handle, method.c_str()));
    if (runtimeFunc == nullptr) {
        Errorln("could not find runtime method: ", method);
        return 1;
    }
    auto heapSize = GetHeapSizeFromEnv(envs);
    HeapParam hParam = HeapParam(REGION_SIZE, heapSize, EXEMPTION_THRESHOLD, HEAP_UTILIZATION,
        HEAP_GROWTH, ALLOCATION_RATE, ALLOCATION_WAIT_TIME);
    GCParam gcParam = GCParam(GC_THRESHOLD, GARBAGE_THRESHOLD, GC_INTERVAL, BACKUP_GC_INTERNAL, GC_THREADS);
    auto logParam = LogParam(RTLogLevel::RTLOG_FATAL);
    ConcurrencyParam cParam =
        ConcurrencyParam(STACK_SIZE, GetStackSizeFromEnv(envs), PROCESSOR_NUM);

    ConfigParam param = {hParam, gcParam, logParam, cParam};
    return runtimeFunc(&param);
}

bool InvokeRuntime::PrepareRuntime(const HANDLE handle, std::unordered_map<std::string, std::string>& initArgs)
{
    auto callInit = CallRuntime(handle, G_CODE_RUNTIME_INIT, initArgs);
    if (callInit != 0) {
        Errorln("macro expansion failed because of runtime initiate failed. ");
        return false;
    }
    return true;
}

void InvokeRuntime::FinishRuntime(const HANDLE handle)
{
    auto method = GetMethod(handle, G_CODE_RUNTIME_FINI.c_str());
    auto runtimeFunc = reinterpret_cast<CodiraFiniFromC>(method);
    auto finiInit = runtimeFunc();
    if (finiInit != 0) {
        Errorln("runtime finish failed: ");
        return;
    }
}

bool RuntimeInit::InitRuntimeMethod()
{
    runtimeMethodFunc = InvokeRuntime::GetMethod(handle, std::string(G_CODE_NEW_TASK_FROM_C).c_str());
    runtimeReleaseFunc = InvokeRuntime::GetMethod(handle, std::string(G_RELEASE_HANDLE_FROM_C).c_str());
    if (runtimeMethodFunc == nullptr || runtimeReleaseFunc == nullptr) {
        auto errorInfo = runtimeMethodFunc == nullptr ? G_CODE_NEW_TASK_FROM_C : G_RELEASE_HANDLE_FROM_C;
        Errorln("could not find the create task method: ", errorInfo);
        return false;
    }
    return true;
}

void RuntimeInit::CloseMacroDynamicLibrary()
{
    int retCode = -1;
    for (auto& openedLib : InvokeRuntime::GetOpenedLibHandles()) {
        retCode = InvokeRuntime::CloseSymbolTable(openedLib);
        if (retCode != 0) {
            Errorln("close macro related dynamic library failed: ");
            return;
        }
    }
    InvokeRuntime::ClearOpenedLibHandles();
}
} // namespace Codira
