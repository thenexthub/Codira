/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_LIBPANDABASE_OS_UNIX_TIME_H
#define PANDA_LIBPANDABASE_OS_UNIX_TIME_H

#include <chrono>
#include <fstream>
#include <string>
#include <unistd.h>

namespace ark::os::time {

template <class T>
static uint64_t GetClockTime(clockid_t clock)
{
    struct timespec time = {0, 0};
    if (clock_gettime(clock, &time) != -1) {
        auto duration = std::chrono::seconds {time.tv_sec} + std::chrono::nanoseconds {time.tv_nsec};
        return std::chrono::duration_cast<T>(duration).count();
    }
    return 0;
}

template <class T>
static uint64_t GetStartRealTime()
{
    std::string fileName = "/proc/" + std::to_string(getpid()) + "/stat";
    std::ifstream fileStream(fileName);
    if (fileStream.fail()) {
        return 0;
    }

    std::string timeStr;
    // The number 22 is the position of the start real time entry in stat file, all entries in the file are separated by
    // space, the stat file is read by word until the required entry is reached
    constexpr int START_TIME_POSITION = 22;
    for (int i = 0; i < START_TIME_POSITION; ++i) {
        if (!std::getline(fileStream, timeStr, ' ')) {
            return 0;
        }
    }

    if (int clockTicksPerSecond = sysconf(_SC_CLK_TCK); clockTicksPerSecond != -1) {
        auto time = std::chrono::duration<double>(stoull(timeStr) / static_cast<double>(clockTicksPerSecond));
        return std::chrono::duration_cast<T>(time).count();
    }
    return 0;
}

}  // namespace ark::os::time

#endif  // PANDA_LIBPANDABASE_OS_UNIX_TIME_H
