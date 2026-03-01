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
#ifndef PANDA_RUNTIME_SAFEPOINT_TIMER_H_
#define PANDA_RUNTIME_SAFEPOINT_TIMER_H_

#ifdef SAFEPOINT_TIME_CHECKER_ENABLED
// CC-OFFNXT(G.PRE.02-CPP) code generation
#define SAFEPOINT_TIME_CHECKER(expr) expr
#else
// CC-OFFNXT(G.PRE.02-CPP) code generation
#define SAFEPOINT_TIME_CHECKER(expr)
#endif  // SAFEPOINT_TIME_CHECKER_ENABLED

#ifdef SAFEPOINT_TIME_CHECKER_ENABLED
#include "libarkbase/macros.h"
#include "runtime/include/timer_table.h"

namespace ark {

class SafepointTimerTable {
public:
    static void RegisterTimer(TimerTable::ThreadId id, std::string_view name)
    {
        safepointTimers_->RegisterTimer(id, name);
    }

    static void DisarmTimer(TimerTable::ThreadId id, std::string_view name)
    {
        safepointTimers_->DisarmTimer(id, name);
    }

    static void ResetTimers(TimerTable::ThreadId id, bool needRecord)
    {
        safepointTimers_->ResetTimers(id, needRecord);
    }

    static void Report(std::stringstream &ss)
    {
        safepointTimers_->Report(ss);
    }

    static bool IsInitialized()
    {
        return safepointTimers_ != nullptr;
    }

    static void Initialize(TimerTable::TimeDiff recordingTime, std::string reportFilepath);
    static void Destroy();

public:
    static inline std::string reportFilepath_;
    static TimerTable *safepointTimers_;
};

class SafepointTimer {
public:
    explicit SafepointTimer(std::string_view name);
    ~SafepointTimer();
    NO_COPY_SEMANTIC(SafepointTimer);
    NO_MOVE_SEMANTIC(SafepointTimer);

private:
    std::string_view name_;
};

}  // namespace ark
#endif  // SAFEPOINT_TIME_CHECKER_ENABLED

#endif  // PANDA_RUNTIME_SAFEPOINT_TIMER_H_
