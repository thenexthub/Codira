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

#include "CrashReporter.h"
#include <cangjie/Utils/FileUtil.h>
#include <fstream>
#include <sstream>
#include "Logger.h"
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#endif

namespace {

#ifdef __linux__
constexpr size_t STACK_SIZE = 50;
constexpr size_t SIGNAL_NUM = 3;
constexpr int SIGNALS[SIGNAL_NUM] = {SIGABRT, SIGSEGV, SIGPIPE};
#endif

void MessageInfoHandler()
{
    std::string baseDir = ark::Logger::GetLogPath();
    std::string dotLogDir = Codira::FileUtil::JoinPath(baseDir, ".log");
    std::ofstream ofs{dotLogDir + ark::FILE_SEPARATOR + "messageInfo.txt", std::ios::app};
    std::stringstream log;
    while (!ark::Logger::messageQueue.empty()) {
        log << ark::Logger::messageQueue.front() << std::endl;
        ark::Logger::messageQueue.pop();
    }
    ofs << log.str();
    ofs.close();
}

void KernelLogHandler(const std::thread::id &threadId)
{
    std::string baseDir = ark::Logger::GetLogPath();
    std::string dotLogDir = Codira::FileUtil::JoinPath(baseDir, ".log");
    std::ofstream ofs{dotLogDir + ark::FILE_SEPARATOR + "kernelLog.txt", std::ios::app};
    std::stringstream log;

    if (ark::Logger::kernelLog.find(threadId) != ark::Logger::kernelLog.end()) {
        log << "threadId: " << threadId << std::endl;
        for (const auto& message : ark::Logger::kernelLog[threadId]) {
            log << message.date << ": " << "function: " << message.func << " state: " << message.state << std::endl;
        }
    }
    ofs << log.str();
    ofs.close();
}

#ifdef __linux__
void PrintStackTraceOnSignal(std::ostream &os)
{
    ark::Logger &logger = ark::Logger::Instance();
    void *array[STACK_SIZE];
    int size = backtrace(array, STACK_SIZE);
    auto backtraces = backtrace_symbols(array, size);
    std::stringstream log;
    for (size_t i = 0; i < static_cast<size_t>(size); i++) {
        std::stringstream ss;
        ss << "stack[" << i << "]:" << std::hex << backtraces[static_cast<size_t>(i)] << std::endl;
        log << logger.LogInfo(ark::MessageType::MSG_ERROR, ss.str());
    }
    os << log.str();
    free(backtraces);
    _Exit(EXIT_FAILURE);
}
#elif defined(_WIN32)
std::string ReportException(const LPEXCEPTION_POINTERS &lpExceptionInfo)
{
    std::string exceptionReason = "Exception(" + std::to_string(lpExceptionInfo->ExceptionRecord->ExceptionCode) + "):";
    switch (lpExceptionInfo->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            exceptionReason = "Access violation";
            break;
        case EXCEPTION_BREAKPOINT:
            exceptionReason = "Exception: Breakpoint";
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            exceptionReason = "Exception: Array index out of bounds";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            exceptionReason = "Exception: Stack overflow";
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            exceptionReason = "Exception: General Protection Fault";
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            exceptionReason = "Exception: Illegal instruction in program";
            break;
        case EXCEPTION_INT_OVERFLOW:
            exceptionReason = "Exception: Integer overflow";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            exceptionReason = "Exception: Integer division by zero";
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            exceptionReason = "Exception: Floating point value underflow";
            break;
        default:
            exceptionReason = "Unknown exception";
    }
    return exceptionReason;
}

void InitStack(DWORD &dwImageType, STACKFRAME64 &frame, const CONTEXT &context)
{
#ifdef _M_IX86
    frame.AddrPC.Offset = context.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
#elif _M_X64
    dwImageType = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset = context.Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Rsp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
    dwImageType = IMAGE_FILE_MACHINE_IA64;
    frame.AddrPC.Offset = context.StIIP;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.IntSp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrBStore.Offset = context.RsBSP;
    frame.AddrBStore.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.IntSp;
    frame.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif
}
#endif
} // namespace

namespace ark {
#ifdef __linux__
void CrashReporter::SignalHandler(int sig)
{
    std::string baseDir = Logger::GetLogPath();
    std::string dotLogDir = Codira::FileUtil::JoinPath(baseDir, ".log");
    (void)Codira::FileUtil::CreateDirs(dotLogDir + FILE_SEPARATOR);
    std::ofstream ofs{dotLogDir + FILE_SEPARATOR + "crash.dump", std::ios::app};
    if (sig == SIGSEGV) {
        ofs << "LSPServer has received a SIGSEGV signal. The crash stack is as follows:" << std::endl;
    }
    MessageInfoHandler();
    KernelLogHandler(std::this_thread::get_id());
    ::PrintStackTraceOnSignal(ofs);
    ofs.close();
}

void SigpipeHandler(int unused)
{
    ark::Logger::Instance().LogMessage(ark::MessageType::MSG_ERROR,
                                       "Macro expand throw Sigpipe Exception!");
}

void CrashReporter::RegisterHandlers()
{
    struct sigaction act;
    act.sa_handler = SignalHandler;
    (void)sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESETHAND;
    for (auto &sig : SIGNALS) {
        if (sig == SIGPIPE) {
            struct sigaction actSigpipe;
            actSigpipe.sa_handler = SigpipeHandler;
            (void)sigemptyset(&actSigpipe.sa_mask);
            (void)sigaction(sig, &actSigpipe, nullptr);
            continue;
        }
        if (sigaction(sig, &act, nullptr) == -1) {
            continue;
        }
    }
}
#elif defined(_WIN32)
LONG WINAPI VectoredExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    std::string baseDir = ark::Logger::GetLogPath();
    std::string dotLogDir = Codira::FileUtil::JoinPath(baseDir, ".log");
    (void)Codira::FileUtil::CreateDirs(dotLogDir + ark::FILE_SEPARATOR);
    MessageInfoHandler();
    KernelLogHandler(std::this_thread::get_id());
    std::ofstream ofs{dotLogDir + ark::FILE_SEPARATOR + "crash.dump", std::ios::app};
    std::stringstream log;
    const int maxNameLength = 500;
    auto reason = ReportException(pExceptionInfo);
    log << "LSPServer has received " + reason + " Exception. The crash stack is as follows:" << std::endl;
    HANDLE hProcess = GetCurrentProcess();
    CONTEXT context = *(pExceptionInfo->ContextRecord);
    if (!SymInitialize(hProcess, NULL, TRUE)) {
        return EXCEPTION_EXECUTE_HANDLER;
    }
    DWORD dwImageType = IMAGE_FILE_MACHINE_I386;
    STACKFRAME64 frame;
    InitStack(dwImageType, frame, context);
    int index = 0;
    while (true) {
        if (StackWalk64(dwImageType, hProcess, GetCurrentThread(), &frame, &context, NULL, SymFunctionTableAccess64,
                        SymGetModuleBase64, nullptr)) {
            std::stringstream ss;
            ss << "stack[" << index << "]:0x" << std::hex << frame.AddrPC.Offset << "  0x" << std::hex
            << frame.AddrFrame.Offset << std::endl;
            log << Logger::Instance().LogInfo(ark::MessageType::MSG_ERROR, ss.str());
            index++;
        } else {
            break;
        }
        if (frame.AddrFrame.Offset == 0) {
            break;
        }
        BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + maxNameLength];
        IMAGEHLP_SYMBOL64 *pSymbol = reinterpret_cast<IMAGEHLP_SYMBOL64 *>(symbolBuffer);
        pSymbol->SizeOfStruct = sizeof(symbolBuffer);
        pSymbol->MaxNameLength = maxNameLength;
        DWORD64 dwDisplament = 0;
        if (SymGetSymFromAddr(hProcess, frame.AddrPC.Offset, &dwDisplament, pSymbol)) {
            std::stringstream ss;
            ss << "symbol:" << pSymbol->Name << std::endl;
            log << Logger::Instance().LogInfo(ark::MessageType::MSG_ERROR, ss.str());
        }
    }
    ofs << log.str();
    ofs.close();
    SymCleanup(hProcess);
    return EXCEPTION_EXECUTE_HANDLER;
}

void CrashReporter::RegisterHandlers()
{
    AddVectoredExceptionHandler(0, VectoredExceptionHandler);
}
#elif defined(__APPLE__)
void CrashReporter::RegisterHandlers()
{}
#endif
} // namespace ark
