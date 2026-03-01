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

#ifndef ARK_RUNTIME_GC_HUNG_H
#define ARK_RUNTIME_GC_HUNG_H

#include <sys/syscall.h>
#include <unistd.h>
#include <string>
#include "libarkbase/utils/time.h"
#include "libarkbase/os/library_loader.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/mtmanaged_thread.h"

namespace ark::mem {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
const char LIB_IMONITOR[] = "libimonitor.so";
const unsigned int ZRHUNG_WP_GC = 8;
const uint64_t INTERVAL_LIMIT_MS_INIT = 50;
const uint64_t OVER_TIME_LIMIT_INIT_MS = 2000;
const uint64_t WATER_MARK_LIMIT = 20;

// zrhung functions type define
using ZrhungSendEvent = int (*)(int16_t, const char *, const char *);
using ZrhungGetConfig = int (*)(int16_t, char *, uint32_t);

enum GcPara {
    GC_PARA_ENABLE,
    GC_PARA_INTERVAL,  // ms
    GC_PARA_WATERMARK,
    GC_PARA_OVERTIME,  // ms
    GC_PARA_COUNT
};

// Functions of the Hung Check Function in the Ark VM,record gc's anomalous behavior
class GcHung {
public:
    GcHung();
    ~GcHung();
    GcHung(const GcHung &) = delete;
    GcHung &operator=(GcHung const &) = delete;
    GcHung(GcHung &&) = delete;
    GcHung &operator=(GcHung &&) = delete;
    void SendZerohungEvent(const PandaString &error, int pid, PandaString msg);
    bool IsConfigReady() const
    {
        return configReady_;
    }
    bool IsEnabled() const
    {
        return enabled_;
    }
    void SetEnabled(bool enabled)
    {
        enabled_ = enabled;
    }
    bool IsReady() const
    {
        return ready_;
    }
    static void InitPreFork(bool enabled);
    static void InitPostFork(bool isSystemserver);
    static GcHung *Current();
    static void Start();
    static void Check(const GCTask &task);
    // NOLINTNEXTLINE(google-runtime-references)
    static void Check(const PandaList<MTManagedThread *> &threads, uint64_t startTime);
    static bool UpdateConfig();

private:
    static GcHung *instance_;
    pid_t pid_ {-1};
    bool enabled_ {false};
    bool ready_ {false};
    uint64_t intervalLimitMs_ {0};
    uint64_t overTimeLimitMs_ {0};
    uint64_t lastGcTimeNs_ {0};
    uint64_t congestionDurationNs_ {0};
    int waterMarkLimit_ {0};
    int waterMark_ {0};
    int reportCount_ {0};
    bool configReady_ {false};
    bool isSystemserver_ {false};
    uint64_t startTimeNs_ {0};
    os::library_loader::LibraryHandle libimonitorDlHandler_ {nullptr};
    ZrhungSendEvent zrhungSendEvent_ {nullptr};
    ZrhungGetConfig zrhungGetConfig_ {nullptr};

    int GetConfig();
    int LoadLibimonitor();
    void ReportGcCongestion();
    void ReportSuspendTimedout();
    void InitInternal(bool isSystemserver);
    void CheckSuspend(const PandaList<MTManagedThread *> &threads, uint64_t startTime);
    void CheckFrequency();
    void CheckOvertime(const GCTask &task);
    void UpdateStartTime();
};

class ScopedGcHung {
public:
    explicit ScopedGcHung(const GCTask *taskStart)
    {
        GcHung::Start();
        task_ = taskStart;
    }

    ~ScopedGcHung()
    {
        GcHung::Check(*task_);
    }

private:
    const GCTask *task_;

    NO_COPY_SEMANTIC(ScopedGcHung);
    NO_MOVE_SEMANTIC(ScopedGcHung);
};

}  // namespace ark::mem
#endif  // ARK_RUNTIME_GC_HUNG_H
