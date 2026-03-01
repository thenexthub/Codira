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

#ifndef LSPSERVER_LOGGER_H
#define LSPSERVER_LOGGER_H

#include <fstream>
#include <sys/stat.h>
#include <sstream>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <queue>
#include <thread>
#include <regex>
#include "../../json-rpc/Transport.h"

namespace ark {
#ifdef _WIN32
const std::string FILE_SEPARATOR = "\\";
#else
const std::string FILE_SEPARATOR = "/";
#endif
const int LOG_FILE_MAX = 10 * 1024 * 1024; // 10M
const int LOG_PATH_MAX = 1024;
const int LOG_FILE_MAX_COUNT = 5;

enum class MessageType { MSG_ERROR = 1, MSG_WARNING = 2, MSG_INFO = 3, MSG_LOG = 4 };

struct KernelLog {
    std::string date;
    std::string func;
    std::string state;
};

class Logger {
public:
    ~Logger() noexcept
    {
        try {
            if (outToFile.is_open()) {
                outToFile.close();
            }
        } catch (std::exception &e) {
            // do nothing and prevent destructor from throwing exceptions.
        } catch (...) {
            // do nothing and prevent destructor from throwing exceptions.
        }
    }

    Logger(const Logger &) = delete;

    Logger &operator=(const Logger &) = delete;

    static Logger &Instance()
    {
        static Logger instance = Logger();
        return instance;
    }

    std::string LogInfo(MessageType type, const std::string &message);

    void LogMessage(MessageType type, const std::string &message);

    void GetLocalTime(const time_t *time, struct tm &localTime) const
    {
#ifdef _WIN32
        localtime_s(&localTime, time);
#else
        (void)localtime_r(time, &localTime);
#endif
    }

    static void SetPath(const std::string &logPath);

    static std::string GetLogPath() { return pathBuf; }

    static std::queue<std::string> messageQueue;

    static const int messageQueueSize;

    void CollectMessageInfo(const std::string &message);

    static std::unordered_map<std::thread::id, std::vector<ark::KernelLog>> kernelLog;

    void CollectKernelLog(const std::thread::id &threadId, const std::string &funcName,
                                 const std::string &state);

    void CleanKernelLog(const std::thread::id &threadId);

    static void SetLogEnable(bool enable);

    static std::mutex logMtx;

private:
    Logger()
    {
        if (!enableLog) {
            return;
        }
        CheckRemoveAndOpen();
        InitLogQueue();
    }

    void CheckRemoveAndOpen()
    {
        path << Logger::pathBuf << "log.txt";
        HandleNewLog();
        if (!outToFile.is_open()) {
            outToFile.open(path.str(), std::ios_base::app | std::ios_base::binary);
        }
    }

    void HandleNewLog(size_t infoSize = 0);

    void InitLogQueue();

    void RemoveRedundantLogFile();

    static bool enableLog;
    std::stringstream path;
    static std::string pathBuf;
    std::ofstream outToFile{}; // lock by fileWriter
    std::mutex fileWriter{};
    std::regex logFileRegex{(R"(^cangjie_lsp_(\d{4})\.(\d{2})\.(\d{2})-(\d{2})-(\d{2})-(\d{2})_log\.txt$)")};
    std::priority_queue<std::string, std::vector<std::string>, std::greater<>> logQueue;
};

void CleanAndLog(std::stringstream &log, const std::string &str);
} // namespace ark

namespace Trace {
template <typename T>
std::string ToStringCustom(const T &t)
{
    std::ostringstream ss;
    ss << t;
    return ss.str();
}

template <>
inline std::string ToStringCustom(const std::string &s)
{
    return s;
}

template <typename... Args>
void Log(const Args &...args)
{
    ark::Logger &logger = ark::Logger::Instance();
    std::string threadId = ToStringCustom(std::this_thread::get_id());
    std::stringstream message;
    message << "[Thread " << threadId << "] ";
    ((message << ToStringCustom(args) << " "), ...);
    message << std::endl;
    logger.LogMessage(ark::MessageType::MSG_INFO, message.str());
}

void Wlog(const std::string &message);
void Elog(const std::string &message);
} // namespace Trace

#endif // LSPSERVER_LOGGER_H
