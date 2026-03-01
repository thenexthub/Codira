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

#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <mutex>
#include <regex>
#include "../common/Utils.h"

namespace ark {
std::queue<std::string> Logger::messageQueue;
const int Logger::messageQueueSize = 50;
std::unordered_map<std::thread::id, std::vector<ark::KernelLog>> Logger::kernelLog;
std::mutex Logger::logMtx;
std::string Logger::pathBuf;
bool Logger::enableLog = false;

std::string Logger::LogInfo(MessageType type, const std::string &message)
{
    std::stringstream time;
    auto newTime = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(newTime);
    struct tm localTime {};
    GetLocalTime(&t, localTime);
    time << std::put_time(&localTime, "%Y-%m-%d %X ");
    auto durationInMS =
        std::chrono::duration_cast<std::chrono::milliseconds>(newTime.time_since_epoch());
    time << durationInMS.count();

    std::string typeString;
    switch (type) {
        case MessageType::MSG_ERROR:
            typeString = "Error";
            break;
        case MessageType::MSG_WARNING:
            typeString = "Warning";
            break;
        case MessageType::MSG_INFO:
            typeString = "Info";
            break;
        case MessageType::MSG_LOG:
            typeString = "Log";
            break;
        default:
            typeString = "missing";
    }

    std::stringstream log;
    log << time.str() << ":[" << typeString << "]" << message;
    return log.str();
}

void Logger::LogMessage(MessageType type, const std::string &message)
{
    if (!enableLog) {
        return;
    }

    std::string logInfo = LogInfo(type, message);
    // out to file
    std::lock_guard<std::mutex> lock(fileWriter);
    HandleNewLog(logInfo.size());
    if (!outToFile.is_open()) {
        return;
    }
    outToFile << logInfo << std::endl;
    (void)outToFile.flush();
}

void Logger::SetPath(const std::string &logPath)
{
    if (logPath.empty()) {
        char tempBuf[LOG_PATH_MAX] = {0};
        // get the log path, in the same dir of src
        if (getcwd(tempBuf, sizeof(tempBuf) - 1) == nullptr) {
            Logger::pathBuf = "";
            return;
        }
        Logger::pathBuf = std::string(tempBuf) + FILE_SEPARATOR;
        return;
    }
    Logger::pathBuf = logPath + FILE_SEPARATOR;
}

void CleanAndLog(std::stringstream &log, const std::string &str)
{
    log.clear();
    log.str("");
    log << str;
}

void Logger::CollectMessageInfo(const std::string &message)
{
    if (messageQueue.size() >= Logger::messageQueueSize) {
        messageQueue.pop();
    }
    messageQueue.push(message);
}

void Logger::CollectKernelLog(const std::thread::id &threadId, const std::string &funcName,
                              const std::string &state)
{
    std::unique_lock<std::mutex> lock(logMtx);
    std::stringstream time;
    auto newTime = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(newTime);
    struct tm localTime {};
    GetLocalTime(&t, localTime);
    time << std::put_time(&localTime, "%Y-%m-%d %X ");
    auto durationInMS =
        std::chrono::duration_cast<std::chrono::milliseconds>(newTime.time_since_epoch());
    time << durationInMS.count();

    KernelLog logInfo = {time.str(), funcName, state};
    if (kernelLog.find(threadId) == kernelLog.end()) {
        kernelLog[threadId] = std::vector<ark::KernelLog>();
    }
    kernelLog[threadId].push_back(logInfo);
}

void Logger::CleanKernelLog(const std::thread::id &threadId)
{
    std::unique_lock<std::mutex> lock(logMtx);
    if (kernelLog.find(threadId) == kernelLog.end()) {
        return;
    }
    (void)kernelLog.erase(threadId);
}

void Logger::InitLogQueue()
{
    auto txtFiles = Codira::FileUtil::GetAllFilesUnderCurrentPath(pathBuf, "txt");
    for (auto& txtFile: txtFiles) {
        if (std::regex_match(txtFile, logFileRegex)) {
            logQueue.push(txtFile);
        }
    }
    RemoveRedundantLogFile();
}

void Logger::RemoveRedundantLogFile()
{
    while (logQueue.size() > LOG_FILE_MAX_COUNT) {
        auto logFile = logQueue.top();
        FileUtil::Remove(LSPJoinPath(pathBuf, logFile));
        logQueue.pop();
    }
}

void Logger::HandleNewLog(size_t infoSize)
{
    struct stat statBuf {};
    std::string logPath = FileUtil::Normalize(path.str());
    // rename log.txt to cangjie_lsp_($time)_log.txt
    if (stat(logPath.c_str(), &statBuf) == 0 &&
        ((static_cast<size_t>(statBuf.st_size) + infoSize) > LOG_FILE_MAX)) {
        std::stringstream logName;
        logName << Logger::pathBuf;
        time_t nowTime;
        char name[LOG_PATH_MAX] = {0};
        (void)time(&nowTime);
        struct tm localTime {};
        GetLocalTime(&nowTime, localTime);
        (void)strftime(name, sizeof(name), "cangjie_lsp_%Y.%m.%d-%H-%M-%S_log.txt", &localTime);
        logName << name;
        std::string oldName = path.str();
        std::string newName = logName.str();
        if (outToFile.is_open()) {
            outToFile.close();
            outToFile.clear();
        }
        if (rename(oldName.c_str(), newName.c_str()) == 0) {
            logQueue.emplace(name);
            RemoveRedundantLogFile();
        }
        // open file
        outToFile.open(logPath, std::ios_base::app | std::ios_base::binary);
    }
}

void Logger::SetLogEnable(bool enable)
{
    enableLog = enable;
}
} // namespace ark


void Trace::Wlog(const std::string &message)
{
    ark::Logger &logger = ark::Logger::Instance();
    logger.LogMessage(ark::MessageType::MSG_WARNING, message);
}

void Trace::Elog(const std::string &message)
{
    ark::Logger &logger = ark::Logger::Instance();
    logger.LogMessage(ark::MessageType::MSG_ERROR, message);
}
