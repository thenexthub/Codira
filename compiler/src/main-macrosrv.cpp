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
#include "Codira/Macro/MacroEvaluation.h"

#include <algorithm>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef __WIN32
#include <windows.h>
#else
#include <csignal>
#include <sys/signal.h>
#include <sys/stat.h>
#endif

using namespace Codira;

namespace {

#if defined(__linux__) || defined(__APPLE__)
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
const size_t ARGS_NUM = 6;
#endif
#else
const size_t ARGS_NUM = 5;
#endif

const size_t IDX_OF_READ_HANDLE = 1;
const size_t IDX_OF_WRITE_HANDLE = 2;
const size_t IDX_OF_ENABLE_PARA = 3;
const size_t IDX_OF_CODEC_FOLDER = 4;
const size_t IDX_OF_PPID = 5;
#if defined(__linux__) || defined(__APPLE__)
const unsigned int CHECK_INTERVAL = 2;
static void MonitoringParentProcess(pid_t pid)
{
    while (true) {
        sleep(CHECK_INTERVAL);
        if (kill(pid, 0) != 0) {
            perror("parent process not exits");
            [[maybe_unused]] std::lock_guard lg(MacroProcMsger::GetInstance().mutex);
            MacroProcMsger::GetInstance().CloseClientResource();
            RuntimeInit::GetInstance().CloseRuntime();
            exit(1);
        }
    }
}
#endif

bool IsNumber(const std::string& str)
{
    for (char c : str) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

bool IsArgsValid(const std::vector<std::string>& args)
{
    if (args.size() != ARGS_NUM) {
        Errorln(
            "Macro srv: Incorrect number of args, " + std::to_string(args.size()) + " : " + std::to_string(ARGS_NUM));
        return false;
    }
    if (!IsNumber(args[IDX_OF_READ_HANDLE])) {
        Errorln("Macro srv: Arg of read handle is not number");
        return false;
    }
    if (!IsNumber(args[IDX_OF_WRITE_HANDLE])) {
        Errorln("Macro srv: Arg of write handle is not number");
        return false;
    }
    if (args[IDX_OF_CODEC_FOLDER].empty()) {
        Errorln("Macro srv: Arg of codec folder is empty");
        return false;
    }
    return true;
}
#ifdef __WIN32
bool CheckPipe(HANDLE read, HANDLE write)
{
    if (GetNamedPipeInfo(read, nullptr, nullptr, nullptr, nullptr) == FALSE) {
        Errorln("Macro srv: Read handle is not available");
        return false;
    }
    if (GetNamedPipeInfo(write, nullptr, nullptr, nullptr, nullptr) == FALSE) {
        CloseHandle(read);
        Errorln("Macro srv: Write handle is not available");
        return false;
    }
    return true;
}
#else
bool IsPipe(int fd)
{
    struct stat status;
    fstat(fd, &status);
    return S_ISFIFO(status.st_mode);
}
bool CheckPipe(int read, int write)
{
    if (!IsPipe(read)) {
        Errorln("Macro srv: Read pipe is not available");
        return false;
    }
    if (!IsPipe(write)) {
        close(read);
        Errorln("Macro srv: Write pipe is not available");
        return false;
    }
    return true;
}

#endif
} // namespace

int main(int argc, const char* argv[], [[maybe_unused]] const char** envp)
{
    auto args = Utils::StringifyArgumentVector(argc, argv);
    if (!IsArgsValid(args)) {
        return -1;
    }
#ifdef _WIN32
    HANDLE hRead = reinterpret_cast<HANDLE>(atoi(args[IDX_OF_READ_HANDLE].c_str()));
    HANDLE hWrite = reinterpret_cast<HANDLE>(atoi(args[IDX_OF_WRITE_HANDLE].c_str()));
#else
    int hRead = stoi(args[IDX_OF_READ_HANDLE]);
    int hWrite = stoi(args[IDX_OF_WRITE_HANDLE]);
    int ppid = stoi(args[IDX_OF_PPID]);
    std::thread w(MonitoringParentProcess, ppid);
    w.detach();
#endif
    if (!CheckPipe(hRead, hWrite)) {
        return -1;
    }
    GlobalOptions gpt;
    // codec folder to find runtime for lsp not in sdk
    std::string codecFolder = args[IDX_OF_CODEC_FOLDER];
    if (codecFolder.back() == '"') {
        codecFolder.back() = '\\';
    }
    gpt.executablePath = codecFolder;
    gpt.enableParallelMacro = args[IDX_OF_ENABLE_PARA] == "1" ? true : false;
    DiagnosticEngine diag;
    CompilerInvocation compilerInvocation;
    compilerInvocation.globalOptions = gpt;
    CompilerInstance ci(compilerInvocation, diag);
    MacroCollector macroCollector;
    MacroEvaluation evaluator(&ci, &macroCollector, false);
    {
        [[maybe_unused]] std::lock_guard lg(MacroProcMsger::GetInstance().mutex);
        MacroProcMsger::GetInstance().SetSrvPipeHandle(hRead, hWrite);
    }
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    RuntimeInit::GetInstance().InitRuntime(
        ci.invocation.GetRuntimeLibPath(), ci.invocation.globalOptions.environment.allVariables);
#endif
    evaluator.ExecuteEvalSrvTask();
    RuntimeInit::GetInstance().CloseRuntime();
#ifdef _WIN32
    CloseHandle(hRead);
    CloseHandle(hWrite);
#else
    [[maybe_unused]] std::lock_guard lg(MacroProcMsger::GetInstance().mutex);
    MacroProcMsger::GetInstance().CloseClientResource();
#endif
    return 0;
}
