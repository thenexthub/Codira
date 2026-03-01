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

#include "utils/timers.h"
#include "os/file.h"

#include <algorithm>
#include <cstring>

#ifndef IS_ARKUI_X_TARGET
#include <nlohmann/json.hpp>
#endif

namespace panda {
TimeStartFunc Timer::timerStart = Timer::TimerStartDoNothing;
TimeEndFunc Timer::timerEnd = Timer::TimerEndDoNothing;
std::unordered_map<std::string_view, TimeRecord> Timer::timers_;
std::vector<std::string_view> Timer::events_;
std::mutex Timer::mutex_;
std::string Timer::perfFile_;
#ifndef IS_ARKUI_X_TARGET
static const int INDENT_LEVEL = 4;
#endif

void Timer::InitializeTimer(std::string &perfFile)
{
    if (!perfFile.empty()) {
        Timer::timerStart = Timer::TimerStartImpl;
        Timer::timerEnd = Timer::TimerEndImpl;
        perfFile_ = perfFile;
    }
}

void WriteFile(std::stringstream &ss, std::string &perfFile)
{
    std::ofstream fs;
    fs.open(panda::os::file::File::GetExtendedFilePath(perfFile));
    if (!fs.is_open()) {
        std::cerr << "Failed to open perf file: " << perfFile << ". Errro: " << std::strerror(errno) << std::endl;
        return;
    }
    fs << ss.str();
    fs.close();
}

bool DescentComparator(const std::pair<std::string, double> p1, const std::pair<std::string, double> p2)
{
    return p1.second > p2.second;
}

void ProcessTimePointRecord(
    const std::string& file,
    const TimePointRecord& timePointRecord,
    double& eventTime,
    std::vector<std::pair<std::string, double>>& eachFileTime,
    std::vector<std::string>& incompleteRecords)
{
    if (timePointRecord.startTime != std::chrono::steady_clock::time_point() &&
        timePointRecord.endTime != std::chrono::steady_clock::time_point()) {
        auto duration = timePointRecord.endTime - timePointRecord.startTime;
        if (duration.count() >= 0) {
            auto t = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            eventTime += t;
            if (!file.empty()) {
                eachFileTime.emplace_back(file, t);
            }
        } else {
            incompleteRecords.push_back(file + ": The end time is earlier than the start time");
        }
    } else {
        incompleteRecords.push_back(file + ": " +
            (timePointRecord.startTime == std::chrono::steady_clock::time_point() ?
            "Lack of start time" : "Lack of end time"));
    }
}

void Timer::PrintTimers()
{
#ifndef IS_ARKUI_X_TARGET
    const std::string suffix = ".json";
    if (perfFile_.length() >= suffix.length() &&
        perfFile_.compare(perfFile_.length() - suffix.length(), suffix.length(), suffix) == 0) {
        PrintJson();
        return;
    }
#endif
    PrintTxt();
}

#ifndef IS_ARKUI_X_TARGET
void to_json(nlohmann::json& j, const TimeRecord& t)
{
    auto early = std::chrono::steady_clock::time_point::max();
    auto late = std::chrono::steady_clock::time_point::min();

    for (auto& [fileName, timePointRecord] : t.timePoints) {
        if (timePointRecord.startTime < early) {
            early = timePointRecord.startTime;
        }
        if (timePointRecord.endTime > late) {
            late = timePointRecord.endTime;
        }
    }
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(early.time_since_epoch()).count();
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(late.time_since_epoch()).count();
    auto duration = end - start;

    std::string parentEvent;
    auto eventIter = eventParent.find(t.event);
    if (eventIter != eventParent.end()) {
        parentEvent = eventIter->second;
    }
    j = nlohmann::json{
        {"name", t.event},
        {"parentEvent", parentEvent},
        {"startTime", start},
        {"endTime", end},
        {"duration", duration}
    };
}

void Timer::PrintJson()
{
    std::stringstream ss;
    nlohmann::json jsonArray = nlohmann::json::array();

    for (auto &event: events_) {
        auto &timeRecord = timers_.at(event);
        nlohmann::json j = timeRecord;
        jsonArray.push_back(j);
    }

    ss << jsonArray.dump(INDENT_LEVEL) << std::endl;

    WriteFile(ss, perfFile_);
}
#endif

void Timer::PrintTxt()
{
    std::stringstream ss;
    ss << "------------- Compilation time consumption in milliseconds: -------------" << std::endl;
    ss << "Note: When compiling multiple files in parallel, " <<
          "we will track the time consumption of each file individually. The output will aggregate these times, " <<
          "potentially resulting in a total time consumption less than the sum of individual file times." <<
          std::endl << std::endl;

    std::vector<std::string> summedUpTimeString;
    ss << "------------- Compilation time consumption of each file: ----------------" << std::endl;

    for (auto &event: events_) {
        auto &timeRecord = timers_.at(event);
        auto formattedEvent =
            std::string(timeRecord.level, ' ') + std::string(timeRecord.level, '#') + std::string(event);

        double eventTime = 0.0;
        std::vector<std::pair<std::string, double>> eachFileTime;
        std::vector<std::string> incompleteRecords;

        for (const auto &[file, timePointRecord] : timeRecord.timePoints) {
            ProcessTimePointRecord(file, timePointRecord, eventTime, eachFileTime, incompleteRecords);
        }

        // print each file time consumption in descending order
        if (!eachFileTime.empty()) {
            std::sort(eachFileTime.begin(), eachFileTime.end(), DescentComparator);
            ss << formattedEvent << ", time consumption of each file:" << std::endl;
        }
        for (auto &pair : eachFileTime) {
            if (!pair.first.empty()) {
                ss << pair.first << ": " << pair.second << " ms" <<std::endl;
            }
        }

        // Print incomplete records
        if (!incompleteRecords.empty()) {
            ss << formattedEvent << ", Incomplete records:" << std::endl;
            for (const auto& record : incompleteRecords) {
                ss << "  " << record << std::endl;
            }
        }

        // collect the sum of each file's time consumption
        std::stringstream eventSummedUpTimeStream;
        eventSummedUpTimeStream << formattedEvent << ": " << eventTime << " ms";
        summedUpTimeString.push_back(eventSummedUpTimeStream.str());
    }
    ss << "------------- Compilation time consumption summed up: -------------------" << std::endl;
    for (auto &str: summedUpTimeString) {
        ss << str << std::endl;
    }
    ss << "-------------------------------------------------------------------------" << std::endl;
    WriteFile(ss, perfFile_);
}

void Timer::TimerStartImpl(const std::string_view event, std::string fileName)
{
    TimePointRecord tpr;
    tpr.startTime = std::chrono::steady_clock::now();
    int level = 0;
    auto eventIter = eventMap.find(event);
    if (eventIter != eventMap.end()) {
        level = eventIter->second;
    } else {
        std::cerr << "Undefined event: " << event << ". Please check!" << std::endl;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = timers_.find(event);
    if (iter != timers_.end()) {
        iter->second.timePoints.emplace(fileName, tpr);
    } else {
        TimeRecord tr;
        tr.timePoints.emplace(fileName, tpr);
        tr.event = event;
        tr.level = level;

        timers_.emplace(event, tr);
        events_.push_back(event);
    }
}

void Timer::TimerEndImpl(const std::string_view event, std::string fileName)
{
    auto endTime = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lock(mutex_);
    auto time_record_iter = timers_.find(event);
    if (time_record_iter == timers_.end()) {
        std::cerr << "Event " << event << " not found in records, skip record end time!" << std::endl;
        return;
    }
    auto& timePoints = time_record_iter->second.timePoints;
    auto time_point_iter = timePoints.find(fileName);
    if (time_point_iter == timePoints.end()) {
        std::cerr << "Event " << event << " and file " << fileName <<
            " start timer not found in records, skip record end time!" << std::endl;
        return;
    }
    time_point_iter->second.endTime = endTime;
}

}  // namespace panda
