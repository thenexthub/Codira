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


#ifndef MRT_CODE_RUNTIME_H
#define MRT_CODE_RUNTIME_H

// Codira's Runtime
#include "Common/Runtime.h"
#include "RuntimeConfig.h"
#include "Base/Globals.h"
#include "Base/Log.h"
#include "schedule.h"

#include <unordered_map>
namespace MapleRuntime {
using ConcurrencyModelMap = std::unordered_map<void*, ConcurrencyModel*>;
// other compiler-oriented runtime functions are declared in compilerCalls.h
enum class StackGrowConfig {
    UNDEF = 0,
    STACK_GROW_OFF = 1,
    STACK_GROW_ON = 2,
};

class CodiraRuntime : private Runtime {
public:
    static void CreateAndInit(const RuntimeParam& runtimeParam);
    static void FiniAndDelete();
    void* CreateSubSchedulerAndInit(ScheduleType type = SCHEDULE_UI_THREAD);
    void* CreateSingleThreadScheduler();
    bool CheckSubSchedulerValid(void* scheduler);
    bool FiniSubScheduler(void* scheduler);
    static HeapParam GetHeapParam() { return Runtime::Current().GetRuntimeParam().heapParam; }
    void SetGCThreshold(uint64_t threshold) override
    {
        if (threshold == 0) {
            LOG(RTLOG_ERROR, "The threshold must be greater than 0.\n");
        } else {
            param.gcParam.gcThreshold = threshold * KB;
            LOG(RTLOG_INFO, "gcThreshold is set to %d KB.", threshold);
        }
    }
    static ConcurrencyParam GetConcurrencyParam() { return Runtime::Current().GetRuntimeParam().coParam; }
    static GCParam GetGCParam() { return Runtime::Current().GetRuntimeParam().gcParam; }
    static LogParam GetLogParam() { return Runtime::Current().GetRuntimeParam().logParam; }
    RuntimeParam GetRuntimeParam() const override { return param; }
    void SetCommandLinesArgs(int argc, const char* argv[])
    {
        commandLineArgs = new const char* [argc + 1]();
        size_t cstrLen = 0;
        for (int i = 0; i < argc; ++i) {
            cstrLen = strlen(argv[i]) + 1;
            commandLineArgs[i] = new const char[cstrLen]();
            CHECK_E(memcpy_s(const_cast<char*>(commandLineArgs[i]), cstrLen, argv[i], cstrLen) != EOK, "memcpy_s fail");
        }
    }

    const char** GetCommandLineArgs() { return commandLineArgs; }
    static StackGrowConfig stackGrowConfig;

protected:
    explicit CodiraRuntime(const RuntimeParam& runtimeParam);
    ~CodiraRuntime() override
    {
        if (commandLineArgs) {
            for (int i = 0; commandLineArgs[i]; ++i) {
                delete[] commandLineArgs[i];
            }
            delete[] commandLineArgs;
        }
    }

private:
    void Init();
    void Fini();

    RuntimeParam param;
    const char** commandLineArgs = nullptr;
    ConcurrencyModelMap subModelMap;
    std::mutex mtx;
};
} // namespace MapleRuntime
#endif // MRT_CODE_RUNTIME_H
