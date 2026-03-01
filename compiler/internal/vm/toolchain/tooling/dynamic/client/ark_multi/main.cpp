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

#include <list>
#include <uv.h>

#include "ecmascript/js_runtime_options.h"
#include "ecmascript/napi/include/jsnapi_expo.h"
#include "ecmascript/platform/file.h"
#include "ecmascript/platform/mutex.h"
#include "tooling/dynamic/utils/utils.h"
#ifdef PANDA_TARGET_MACOS
#include <unistd.h>
#include <sys/syscall.h>
#endif
static panda::ecmascript::Mutex g_mutex;
static std::list<std::string> g_files;
static std::list<std::string>::iterator g_iter;
static panda::ecmascript::JSRuntimeOptions g_runtimeOptions;
static uv_async_t *g_exitSignal = nullptr;
static int g_threadCount = 0;
static int g_runningCount = 0;

static constexpr int MAX_THREAD = 1024;

namespace OHOS::ArkCompiler::Toolchain {
bool ExecutePandaFile(panda::ecmascript::EcmaVM *vm,
                      const panda::ecmascript::JSRuntimeOptions &runtimeOptions,
                      const std::string &file, const std::string &entry)
{
    panda::LocalScope scope(vm);

    if (runtimeOptions.WasAOTOutputFileSet()) {
        panda::JSNApi::LoadAotFile(vm, "");
    }

    bool ret = panda::JSNApi::Execute(vm, file, entry);

    return ret;
}

std::pair<std::string, std::string> GetNextPara()
{
    std::string fileName = *g_iter;
    std::string fileAbc = fileName.substr(fileName.find_last_of('/') + 1);
    std::string entry = fileAbc.substr(0, fileAbc.size() - 4);
    g_iter++;
    g_runningCount++;
    return {fileName, entry};
}

std::string GetMsg(int ret, std::string& msg, std::string& fileName)
{
    if (!ret) {
#ifdef PANDA_TARGET_MACOS
        msg = "[FAILED] [" + std::to_string(syscall(SYS_thread_selfid)) + "] Run " +
            fileName + " failed!";
#else
        msg = "[FAILED] [" + std::to_string(gettid()) + "] Run " + fileName + " failed!";
#endif
    } else {
#ifdef PANDA_TARGET_MACOS
        msg = "[PASS] [" + std::to_string(syscall(SYS_thread_selfid)) + "] Run " +
            fileName + " success!";
#else
        msg = "[PASS] [" + std::to_string(gettid()) + "] Run " + fileName + " success!";
#endif
    }
    return msg;
}

bool StartThread(uv_loop_t *loop)
{
    uv_thread_t tid = 0;
    int ret = uv_thread_create(&tid, [] (void* arg) -> void {
        while (true) {
            g_mutex.Lock();
            if (g_iter == g_files.end()) {
                g_threadCount--;
                if (g_threadCount == 0) {
                    uv_async_send(g_exitSignal);
                }
                g_mutex.Unlock();
                break;
            }
            auto [fileName, entry] = GetNextPara();
            g_mutex.Unlock();

            panda::ecmascript::EcmaVM *vm = panda::JSNApi::CreateEcmaVM(g_runtimeOptions);
            if (vm == nullptr) {
                std::cerr << "Cannot create vm." << std::endl;
                return;
            }
            panda::JSNApi::SetBundle(vm, !g_runtimeOptions.GetMergeAbc());
            bool ret = ExecutePandaFile(vm, g_runtimeOptions, fileName, entry);
            panda::JSNApi::DestroyJSVM(vm);

            auto loop = static_cast<uv_loop_t *>(arg);
            auto work = new uv_work_t;
            std::string msg = GetMsg(ret, msg, fileName);
            work->data = new char[msg.size() + 1];
            if (strncpy_s(static_cast<char*>(work->data), msg.size() + 1, msg.data(), msg.size()) != EOK) {
                std::cerr << "strncpy_s fail." << std::endl;
                delete[] static_cast<char*>(work->data);
                delete work;
                return;
            }
            uv_queue_work(loop, work, [] (uv_work_t*) {}, [] (uv_work_t* work, int) {
                std::cerr << static_cast<char*>(work->data) << std::endl;
                delete[] static_cast<char*>(work->data);
                delete work;
            });
        }
    }, loop);
    return ret != 0;
}

int Main(const int argc, const char **argv)
{
    auto startTime =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    std::cerr << "Run begin [" << getpid() << "]" << std::endl;
    if (argc < 3) { // 3: at least have three arguments
        std::cerr << "At least have three arguments." << std::endl;
        return -1;
    }

    std::string countStr = argv[1];
    int32_t count;
    if (!Utils::StrToInt32(countStr, count)) {
        std::cerr << "The argument about the number of threads is incorrect." << std::endl;
        return -1;
    }
    g_threadCount = std::min(count, MAX_THREAD);

    std::string filePath = argv[2];
    std::string realPath;
    if (!panda::ecmascript::RealPath(filePath, realPath, true)) {
        std::cerr << "RealPath return fail";
        return -1;
    }

    g_mutex.Lock();
    std::string line;
    std::ifstream in(realPath);
    while (std::getline(in, line)) {
        if (line.find_last_of(".abc") == std::string::npos) {  // endwith
            std::cerr << "Not endwith .abc" << line << std::endl;
            return -1;
        }
        g_files.emplace_back(line);
    }
    g_iter = g_files.begin();
    g_mutex.Unlock();

    bool retOpt = g_runtimeOptions.ParseCommand(argc - 2, argv + 2);
    if (!retOpt) {
        std::cerr << "ParseCommand failed." << std::endl;
        return -1;
    }
    panda::ecmascript::EcmaVM *vm = panda::JSNApi::CreateEcmaVM(g_runtimeOptions);
    if (vm == nullptr) {
        std::cerr << "Cannot create vm." << std::endl;
        return -1;
    }
    panda::JSNApi::SetBundle(vm, !g_runtimeOptions.GetMergeAbc());

    uv_loop_t* loop = uv_default_loop();
    g_exitSignal = new uv_async_t;
    g_exitSignal->data = loop;
    uv_async_init(loop, g_exitSignal, []([[maybe_unused]] uv_async_t* handle) {
        g_mutex.Lock();
        assert (g_threadCount == 0);
        g_mutex.Unlock();
        auto loop = static_cast<uv_loop_t*>(handle->data);
        uv_stop(loop);
    });

    int threadCountLocal = g_threadCount;
    for (int i = 0; i < threadCountLocal; i++) {
        StartThread(loop);
    }

    uv_run(loop, UV_RUN_DEFAULT);

    uv_close(reinterpret_cast<uv_handle_t*>(g_exitSignal), [] (uv_handle_t* handle) {
        if (handle != nullptr) {
            delete reinterpret_cast<uv_handle_t*>(handle);
            handle = nullptr;
        }
    });
    panda::JSNApi::DestroyJSVM(vm);

    auto endTime =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    g_mutex.Lock();
    const long long timeUnit = 1000'000'000;
    std::cerr << "Run end, total file count: " << g_runningCount << ", used: "
	      << ((endTime - startTime) / timeUnit) << "s." << std::endl;
    g_mutex.Unlock();
    return 0;
}
} // OHOS::ArkCompiler::Toolchain

int main(int argc, const char **argv)
{
    return OHOS::ArkCompiler::Toolchain::Main(argc, argv);
}
