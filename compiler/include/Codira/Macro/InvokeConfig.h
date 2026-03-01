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

/**
 * @file
 *
 * This file configures parameters for C to call codira function.
 */

#ifndef CODIRA_INVOKECONFIG_H
#define CODIRA_INVOKECONFIG_H
#include "Codira/Lex/Token.h"
namespace Codira {
    namespace InvokeRuntime {
        enum RTLogLevel {
            RTLOG_VERBOSE,
            RTLOG_DEBUGY,
            RTLOG_INFO,
            RTLOG_WARNING,
            RTLOG_ERROR,
            RTLOG_FATAL_WITHOUT_ABORT,
            RTLOG_FATAL,
            RTLOG_OFF
        };

        struct HeapParam {
        public:
            HeapParam(const size_t& rSize, const size_t& hSize, const double& lThreshold,
                const double& hUtilization, const double& hGrowth, const double& aRate,
                const size_t& aWaitTime)
                : regionSize(rSize),
                  heapSize(hSize),
                  exemptionThreshold(lThreshold),
                  heapUtilization(hUtilization),
                  heapGrowth(hGrowth),
                  allocationRate(aRate),
                  allocationWaitTime(aWaitTime)
            {
            }
            ~HeapParam() = default;

        private:
            [[maybe_unused]] size_t regionSize; // The initial size of heap (must be in range (0, heapSize]).
            [[maybe_unused]] size_t heapSize;        // The maximum size of heap (must be > 0).
            [[maybe_unused]] double exemptionThreshold;     // The maximum size of heap need to free (must be > 0).
            [[maybe_unused]] double heapUtilization;
            [[maybe_unused]] double heapGrowth;
            [[maybe_unused]] double allocationRate;  // The utilization of of heap must be in range (0.0, 1.0).
            [[maybe_unused]] size_t allocationWaitTime;
        };

        struct GCParam {
        public:
            GCParam(const size_t& hThreshold, const double& gRatic, const uint64_t& gcInter,
                const uint64_t& backInterval, const int32_t& gThreads)
                : gcThreshold(hThreshold),
                  garbageThreshold(gRatic),
                  gcInterval(gcInter),
                  backupGCInterval(backInterval),
                  gcThreads(gThreads)
            {
            }
        private:
            [[maybe_unused]] size_t gcThreshold;
            [[maybe_unused]] double garbageThreshold;
            [[maybe_unused]] uint64_t gcInterval;
            [[maybe_unused]] uint64_t backupGCInterval;
            [[maybe_unused]] int32_t gcThreads;
        };

        struct LogParam {
        public:
            explicit LogParam(RTLogLevel rLevel) : logLevel(rLevel)
            {
            }
            ~LogParam() = default;
        private:
            [[maybe_unused]] enum RTLogLevel logLevel;
        };

        struct ConcurrencyParam {
        public:
            ConcurrencyParam(const size_t& tStackSize, const size_t& cStackSize, const uint32_t& pNum)
                : thStackSize(tStackSize),
                  coStackSize(cStackSize),
                  processorNum(pNum)
            {
            }
            ~ConcurrencyParam() = default;

        private:
            [[maybe_unused]] size_t thStackSize;    // The thread stack size (must be > 0).
            [[maybe_unused]] size_t coStackSize;    // The task stack size (must be in range (0, 2GB]).
            [[maybe_unused]] uint32_t processorNum; // The number of processors (must be > 0).
        };

        struct ConfigParam {
        public:
            ConfigParam(const HeapParam& hParam, const GCParam& gParam, const LogParam& lParam,
                const ConcurrencyParam& cParam)
                : heapParam(hParam), gcParam(gParam), logParam(lParam), coroutineParam(cParam)
            {
            }
            ~ConfigParam() = default;

        private:
            [[maybe_unused]] struct HeapParam heapParam;
            [[maybe_unused]] struct GCParam gcParam;
            [[maybe_unused]] struct LogParam logParam;
            [[maybe_unused]] struct ConcurrencyParam coroutineParam;
        };

        const int G_COROUTINE_NAME_SIZE = 32;
        struct CoroutineAttr {
            char name[G_COROUTINE_NAME_SIZE];
            size_t stackSize; // The stack size (must be in range (0, 2GB]).
        };
    } // namespace Invoke
} // namespace Codira
#endif // CODIRA_INVOKECONFIG_H
