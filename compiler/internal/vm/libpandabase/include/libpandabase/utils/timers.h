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

#ifndef LIBPANDABASE_UTILS_TIMERS_H
#define LIBPANDABASE_UTILS_TIMERS_H

#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "panda_visibility.h"

namespace panda {
// Event list
constexpr std::string_view EVENT_GENERATE_ABC = "generate merged abc by es2abc (async)";
constexpr std::string_view EVENT_TOTAL = "Total";
constexpr std::string_view EVENT_COMPILE = "Compile";
constexpr std::string_view EVENT_RESOLVE_DEPS = "Resolve file dep relations";
constexpr std::string_view EVENT_EMIT_ABC = "Emit abc";
constexpr std::string_view EVENT_READ_INPUT_AND_CACHE = "Read input and cache (async)";
constexpr std::string_view EVENT_COMPILE_FILE = "Compile file (async)";
constexpr std::string_view EVENT_COMPILE_ABC_FILE = "Compile abc file (async)";
constexpr std::string_view EVENT_COMPILE_ABC_FILE_RECORD = "Compile abc file record (async)";
constexpr std::string_view EVENT_REPLACE_ABC_FILE_RECORD = "Replace abc file records (async)";
constexpr std::string_view EVENT_UPDATE_ABC_PKG_VERSION = "Update abc package version (async)";
constexpr std::string_view EVENT_UPDATE_ABC_PROGRAM_STRING = "Update abc program string (async)";
constexpr std::string_view EVENT_UPDATE_ABC_PROG_CACHE = "Update abc program cache (async)";
constexpr std::string_view EVENT_OPTIMIZE_PROGRAM = "Optimize program (async)";
constexpr std::string_view EVENT_PARSE = "Parse (async)";
constexpr std::string_view EVENT_COMPILE_TO_PROGRAM = "Compile to program (async)";
constexpr std::string_view EVENT_EMIT_SINGLE_PROGRAM = "Emit single program";
constexpr std::string_view EVENT_EMIT_MERGED_PROGRAM = "Emit merged program";
constexpr std::string_view EVENT_EMIT_CACHE_FILE = "Emit cache file";

// Event level, indicating the level of an event, where each event is a sub-event of the nearest preceding level event.
// The top-level event's level is 0
enum EventLevel { ZEROTH = 0, FIRST, SECOND, THIRD, FORTH, FIFTH, SIXTH };

// Holds pairs of {event, level} according to the order during compilation process.
// When adding new events later, please add them in the compilation order for ease of reading and maintenance.
const std::unordered_map<std::string_view, EventLevel> eventMap = {
    {EVENT_TOTAL, EventLevel::ZEROTH},
    {EVENT_COMPILE, EventLevel::FIRST},
    {EVENT_READ_INPUT_AND_CACHE, EventLevel::SECOND},
    {EVENT_COMPILE_ABC_FILE, EventLevel::SECOND},
    {EVENT_COMPILE_ABC_FILE_RECORD, EventLevel::FORTH},
    {EVENT_UPDATE_ABC_PKG_VERSION, EventLevel::THIRD},
    {EVENT_UPDATE_ABC_PROGRAM_STRING, EventLevel::THIRD},
    {EVENT_UPDATE_ABC_PROG_CACHE, EventLevel::THIRD},
    {EVENT_COMPILE_FILE, EventLevel::SECOND},
    {EVENT_PARSE, EventLevel::THIRD},
    {EVENT_COMPILE_TO_PROGRAM, EventLevel::THIRD},
    {EVENT_OPTIMIZE_PROGRAM, EventLevel::SECOND},
    {EVENT_RESOLVE_DEPS, EventLevel::FIRST},
    {EVENT_EMIT_ABC, EventLevel::FIRST},
    {EVENT_EMIT_SINGLE_PROGRAM, EventLevel::SECOND},
    {EVENT_EMIT_MERGED_PROGRAM, EventLevel::SECOND},
    {EVENT_EMIT_CACHE_FILE, EventLevel::SECOND},
    {EVENT_REPLACE_ABC_FILE_RECORD, EventLevel::THIRD},
};

const std::unordered_map<std::string_view, std::string_view> eventParent = {
    { EVENT_TOTAL, EVENT_GENERATE_ABC },
    { EVENT_COMPILE, EVENT_TOTAL },
    { EVENT_READ_INPUT_AND_CACHE, EVENT_COMPILE },
    { EVENT_COMPILE_ABC_FILE, EVENT_COMPILE },
    { EVENT_COMPILE_ABC_FILE_RECORD, EVENT_COMPILE_ABC_FILE },
    { EVENT_REPLACE_ABC_FILE_RECORD, EVENT_COMPILE_ABC_FILE },
    { EVENT_UPDATE_ABC_PKG_VERSION, EVENT_COMPILE_ABC_FILE },
    { EVENT_UPDATE_ABC_PROGRAM_STRING, EVENT_COMPILE_ABC_FILE },
    { EVENT_UPDATE_ABC_PROG_CACHE, EVENT_COMPILE_ABC_FILE },
    { EVENT_COMPILE_FILE, EVENT_COMPILE },
    { EVENT_PARSE, EVENT_COMPILE_FILE },
    { EVENT_COMPILE_TO_PROGRAM, EVENT_COMPILE_FILE },
    { EVENT_OPTIMIZE_PROGRAM, EVENT_COMPILE },
    { EVENT_RESOLVE_DEPS, EVENT_TOTAL },
    { EVENT_EMIT_ABC, EVENT_TOTAL },
    { EVENT_EMIT_SINGLE_PROGRAM, EVENT_EMIT_ABC },
    { EVENT_EMIT_MERGED_PROGRAM, EVENT_EMIT_ABC },
    { EVENT_EMIT_CACHE_FILE, EVENT_EMIT_ABC },
};

typedef void (*TimeStartFunc)(const std::string_view event, std::string fileName);
typedef void (*TimeEndFunc)(const std::string_view event, std::string fileName);
typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

struct TimePointRecord {
    TimePoint startTime;
    TimePoint endTime;
};

struct TimeRecord {
    std::unordered_map<std::string, TimePointRecord> timePoints;  // pair of {fileName, TimePointRecord}
    std::string event;
    int level;
};

class Timer {
public:
    static void InitializeTimer(std::string &perfFile);
    static void PrintTimers();

    PANDA_PUBLIC_API static TimeStartFunc timerStart;
    PANDA_PUBLIC_API static TimeEndFunc timerEnd;

private:
    static void TimerStartImpl(const std::string_view event, std::string fileName = "");
    static void TimerEndImpl(const std::string_view event, std::string fileName = "");
    static void TimerStartDoNothing(const std::string_view event, std::string fileName = "") {}
    static void TimerEndDoNothing(const std::string_view event, std::string fileName = "") {}
    static void PrintJson();
    static void PrintTxt();

    static std::unordered_map<std::string_view, TimeRecord> timers_;  // pair of {event, TimeRecord}
    static std::vector<std::string_view> events_;
    static std::mutex mutex_;
    static std::string perfFile_;
};

}  // namespace panda

#endif  // LIBPANDABASE_UTILS_TIMERS_H
