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

#ifndef RUNTIME_TRACE_H
#define RUNTIME_TRACE_H

#include "libarkbase/macros.h"

#ifdef PANDA_USE_HITRACE
#include "hitrace_meter.h"
constexpr uint64_t TRACER_TAG = HITRACE_TAG_ARK;
#else
#include <cstdint>
#include <string>
constexpr uint64_t TRACER_TAG = 0ULL;
constexpr uint64_t HITRACE_LEVEL_INFO = 0ULL;

extern "C" {
inline void StartTrace([[maybe_unused]] uint64_t tag, [[maybe_unused]] const std::string &name,
                       [[maybe_unused]] float limit = -1)
{
}
inline void FinishTrace([[maybe_unused]] uint64_t tag) {}

inline void StartAsyncTrace([[maybe_unused]] uint64_t tag, [[maybe_unused]] const std::string &name,
                            [[maybe_unused]] int32_t taskId, [[maybe_unused]] float limit = -1)
{
}

#ifdef PANDA_USE_HITRACE
inline void StartAsyncTraceEx([[maybe_unused]] HiTraceOutputLevel level, [[maybe_unused]] uint64_t tag,
                              [[maybe_unused]] const char *name, [[maybe_unused]] int32_t taskId,
                              [[maybe_unused]] const char *customCategory, [[maybe_unused]] const char *customArgs)
{
}
#else
inline void StartAsyncTraceEx([[maybe_unused]] uint64_t level, [[maybe_unused]] uint64_t tag,
                              [[maybe_unused]] const char *name, [[maybe_unused]] int32_t taskId,
                              [[maybe_unused]] const char *customCategory, [[maybe_unused]] const char *customArgs)
{
}
#endif

inline void FinishAsyncTrace([[maybe_unused]] uint64_t tag, [[maybe_unused]] const std::string &name,
                             [[maybe_unused]] int32_t taskId)
{
}

#ifdef PANDA_USE_HITRACE
inline void FinishAsyncTraceEx([[maybe_unused]] HiTraceOutputLevel level, [[maybe_unused]] uint64_t tag,
                               [[maybe_unused]] const char *name, [[maybe_unused]] int32_t taskId)
{
}
#else
inline void FinishAsyncTraceEx([[maybe_unused]] uint64_t level, [[maybe_unused]] uint64_t tag,
                               [[maybe_unused]] const char *name, [[maybe_unused]] int32_t taskId)
{
}
#endif

inline void CountTrace([[maybe_unused]] uint64_t tag, [[maybe_unused]] const std::string &name,
                       [[maybe_unused]] int64_t count)
{
}
}
#endif

namespace ark {

class Tracer {
public:
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    inline static constexpr char LAUNCH[] = "Launch";
    inline static constexpr char COROUTINE_EXECUTION[] = "CoroutineExecution";
    inline static constexpr char WORKERS_EXPANSION[] = "WorkersExpansion";
    inline static constexpr char WORKERS_CONTRACTION[] = "WorkersContraction";
    // NOLINTEND(modernize-avoid-c-arrays)

    static void Start(const std::string &name, float limit = -1)
    {
        StartTrace(TRACER_TAG, name, limit);
    }

    static void Finish()
    {
        FinishTrace(TRACER_TAG);
    }

    static void StartAsync(const std::string &name, int32_t taskId, const char *taskName = "")
    {
        StartAsyncTraceEx(HITRACE_LEVEL_INFO, TRACER_TAG, name.c_str(), taskId, taskName, "");
    }

    static void FinishAsync(const std::string &name, int32_t taskId)
    {
        FinishAsyncTraceEx(HITRACE_LEVEL_INFO, TRACER_TAG, name.c_str(), taskId);
    }

    static void Count(const std::string &name, int64_t count)
    {
        CountTrace(TRACER_TAG, name, count);
    }
};

class ScopedTrace {
public:
    explicit ScopedTrace(const char *str, bool enabled) : enabled_(enabled)
    {
        if (UNLIKELY(enabled_)) {
            Tracer::Start(str);
        }
    }
    explicit ScopedTrace(const std::string &str, bool enabled) : ScopedTrace(str.c_str(), enabled) {}

    ~ScopedTrace()
    {
        if (UNLIKELY(enabled_)) {
            Tracer::Finish();
        }
    }

    NO_COPY_SEMANTIC(ScopedTrace);
    NO_MOVE_SEMANTIC(ScopedTrace);

private:
    bool enabled_;
};

}  // namespace ark

#endif  // RUNTIME_TRACE_H
