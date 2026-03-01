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

#include "gc_hung.h"

#include <unistd.h>
#include <cstdlib>
#include <dlfcn.h>
#include <ostream>
#include <sstream>
#include <sys/types.h>
#include <ctime>
#include <cinttypes>
#include <cstdio>
#include <csignal>
#include <sys/time.h>
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/type_converter.h"

namespace ark::mem {

/*
 * If the macro HUNG_SYSTEM_SERVER_ONLY was defined, the module only
 * supervise system_server. Otherwise, all the processes running in ARK
 */
using std::string;

GcHung *GcHung::instance_ = nullptr;

// NOLINT(google-runtime-references)
static void Split(const PandaString &str, char delim, PandaVector<PandaString> *elems, bool skipEmpty = true)
{
    PandaIStringStream iss(str);
    for (PandaString item; getline(iss, item, delim);) {
        if (!(skipEmpty && item.empty())) {
            elems->push_back(item);
        }
    }
}

GcHung::GcHung()
    : intervalLimitMs_(INTERVAL_LIMIT_MS_INIT),
      overTimeLimitMs_(OVER_TIME_LIMIT_INIT_MS),
      waterMarkLimit_(WATER_MARK_LIMIT)
{
    LOG(DEBUG, GC) << "GcHung: Instance created";
}

GcHung::~GcHung()
{
    if (libimonitorDlHandler_.GetNativeHandle() != nullptr) {
        dlclose(libimonitorDlHandler_.GetNativeHandle());
    }
    LOG(DEBUG, GC) << "GcHung: Instance deleted: water_mark: " << waterMark_;
}

int GcHung::GetConfig()
{
    const unsigned int maxParaLen = 100;
    // get parameter from zrhung
    if (zrhungGetConfig_ == nullptr) {
        return -1;
    }

    char paraBuf[maxParaLen] = {0};  // NOLINT(modernize-avoid-c-arrays)
    if (zrhungGetConfig_(ZRHUNG_WP_GC, paraBuf, maxParaLen) != 0) {
        LOG(DEBUG, GC) << "GcHung: failed to get config";
        return -1;
    }

    PandaString paraStr(paraBuf);
    PandaVector<PandaString> paraVec;
    Split(paraStr, ',', &paraVec);

    if (paraVec.size() != GC_PARA_COUNT) {
        LOG(ERROR, GC) << "GcHung: parse parameters failed";
        return -1;
    }

#ifdef HUNG_SYSTEM_SERVER_ONLY
    enabled_ = (PandaStringToLL(paraVec[GC_PARA_ENABLE]) == 1) && is_systemserver_;
#else
    enabled_ = (PandaStringToLL(paraVec[GC_PARA_ENABLE]) == 1);
#endif  // HUNG_SYSTEM_SERVER_ONLY
    LOG(INFO, GC) << "GcHung: module enable:" << enabled_;

    intervalLimitMs_ = PandaStringToULL(paraVec[GC_PARA_INTERVAL]);
    waterMarkLimit_ = PandaStringToLL(paraVec[GC_PARA_WATERMARK]);
    overTimeLimitMs_ = PandaStringToULL(paraVec[GC_PARA_OVERTIME]);
    LOG(DEBUG, GC) << "GcHung: set parameter: interval_limit_ms_ = " << intervalLimitMs_ << "ms";
    LOG(DEBUG, GC) << "GcHung: set parameter: water_mark_limit_ = " << waterMarkLimit_;
    LOG(DEBUG, GC) << "GcHung: set parameter: over_time_limit_ms_ = " << overTimeLimitMs_ << "ms";
    configReady_ = true;

    return 0;
}

int GcHung::LoadLibimonitor()
{
    if ((zrhungSendEvent_ != nullptr) && (zrhungGetConfig_ != nullptr)) {
        return 1;
    }
    LOG(DEBUG, GC) << "GcHung: load libimonitor";
    auto resLoad = os::library_loader::Load(LIB_IMONITOR);
    if (!resLoad) {
        LOG(ERROR, RUNTIME) << "failed to load " << LIB_IMONITOR << " Error: " << resLoad.Error().ToString();
        return -1;
    }
    libimonitorDlHandler_ = std::move(resLoad.Value());

    auto zrhungSendEventDlsym = os::library_loader::ResolveSymbol(libimonitorDlHandler_, "zrhung_send_event");
    if (!zrhungSendEventDlsym) {
        LOG(ERROR, RUNTIME) << "failed to dlsym symbol: zrhung_send_event";
        dlclose(libimonitorDlHandler_.GetNativeHandle());
        return -1;
    }
    zrhungSendEvent_ = reinterpret_cast<ZrhungSendEvent>(zrhungSendEventDlsym.Value());

    auto zrhungGetConfigDlsym = os::library_loader::ResolveSymbol(libimonitorDlHandler_, "zrhung_get_config");
    if (!zrhungGetConfigDlsym) {
        LOG(ERROR, RUNTIME) << "failed to dlsym symbol: zrhung_get_config";
        dlclose(libimonitorDlHandler_.GetNativeHandle());
        return -1;
    }
    zrhungGetConfig_ = reinterpret_cast<ZrhungGetConfig>(zrhungGetConfigDlsym.Value());
    return 0;
}

void GcHung::InitInternal(bool isSystemserver)
{
    pid_ = getpid();
    lastGcTimeNs_ = 0;
    congestionDurationNs_ = 0;
    waterMark_ = 0;
    reportCount_ = 0;
    startTimeNs_ = 0;
    isSystemserver_ = isSystemserver;
#ifdef HUNG_SYSTEM_SERVER_ONLY
    ready_ = is_systemserver;  // if is_systemserver == false, hung will be close and no way to open again
#else
    ready_ = true;
#endif  // HUNG_SYSTEM_SERVER_ONLY

    LOG(DEBUG, GC) << "GcHung InitInternal: pid=" << pid_ << " enabled_=" << enabled_;
}

void GcHung::SendZerohungEvent(const PandaString &error, int pid, PandaString msg)
{
    msg = ">>>*******************" + error + "******************\n" + msg;
    if ((zrhungSendEvent_ == nullptr) || (zrhungGetConfig_ == nullptr)) {
        LOG(ERROR, GC) << "GcHung: zrhung functions not defined";
        return;
    }
    if (pid > 0) {
        PandaString command = "P=" + ToPandaString(pid);
        zrhungSendEvent_(ZRHUNG_WP_GC, command.c_str(), msg.c_str());
    } else {
        zrhungSendEvent_(ZRHUNG_WP_GC, nullptr, msg.c_str());
    }
}

// check threads suspend while get "Locks::mutator_lock->WriteLock()", and report to hung
void GcHung::CheckSuspend(const PandaList<MTManagedThread *> &threads, uint64_t startTime)
{
    LOG(DEBUG, GC) << "GcHung: check suspend timeout";
    PandaOStringStream oss;
    for (const auto &thread : threads) {
        if (thread == nullptr) {
            continue;
        }
        if (!thread->IsSuspended()) {
            auto tid = thread->GetId();

            oss << "GcHung: Timed out waiting for thread " << tid << " to suspend, waited for "
                << helpers::TimeConverter(time::GetCurrentTimeInNanos() - startTime) << std::endl;
        }
    }
    LOG(ERROR, GC) << oss.str();

    if (configReady_ && enabled_) {
        SendZerohungEvent("SuspendAll timed out", getpid(), oss.str());
    }
}

void GcHung::CheckFrequency()
{
    LOG(DEBUG, GC) << "GcHung: gc frequency check: PID = " << pid_
                   << " last_gc_time_ns_=" << helpers::TimeConverter(lastGcTimeNs_)
                   << " current_time=" << helpers::TimeConverter(time::GetCurrentTimeInNanos());

    if (lastGcTimeNs_ == 0) {
        lastGcTimeNs_ = time::GetCurrentTimeInNanos();
        return;
    }

    using ResultDuration = std::chrono::duration<uint64_t, std::deca>;
    std::chrono::microseconds msec(intervalLimitMs_);
    if ((startTimeNs_ - lastGcTimeNs_) < std::chrono::duration_cast<ResultDuration>(msec).count()) {
        waterMark_++;
        congestionDurationNs_ += (time::GetCurrentTimeInNanos() - lastGcTimeNs_);
        LOG(DEBUG, GC) << "GcHung: proc " << pid_ << " water_mark_:" << waterMark_
                       << " duration:" << helpers::TimeConverter(time::GetCurrentTimeInNanos() - lastGcTimeNs_);
    } else {
        waterMark_ = 0;
        congestionDurationNs_ = 0;
    }

    if (waterMark_ > waterMarkLimit_) {
        PandaOStringStream oss;
        oss << "GcHung: GC congestion PID:" << pid_ << " Freq:" << waterMark_ << "/"
            << helpers::TimeConverter(congestionDurationNs_);

        LOG(ERROR, GC) << oss.str();
        if (configReady_ && enabled_) {
            SendZerohungEvent("GC congestion", -1, oss.str());  // -1: invalid pid
        }
        waterMark_ = 0;
        congestionDurationNs_ = 0;
    }
    lastGcTimeNs_ = time::GetCurrentTimeInNanos();
}

void GcHung::CheckOvertime(const GCTask &task)
{
    uint64_t gcTime = time::GetCurrentTimeInNanos() - startTimeNs_;
    LOG(DEBUG, GC) << "GcHung: gc overtime check: start_time_ns_=" << helpers::TimeConverter(startTimeNs_)
                   << " current_time=" << helpers::TimeConverter(time::GetCurrentTimeInNanos())
                   << " total_time=" << helpers::TimeConverter(gcTime);
    using ResultDuration = std::chrono::duration<uint64_t, std::deca>;
    std::chrono::microseconds msec(overTimeLimitMs_);
    if (gcTime > std::chrono::duration_cast<ResultDuration>(msec).count()) {
        PandaOStringStream oss;
        oss << "GcHung: GC overtime: total:" << helpers::TimeConverter(gcTime) << " cause: " << task.reason;
        LOG(ERROR, GC) << oss.str();
        if (configReady_ && enabled_) {
            SendZerohungEvent("GC overtime", -1, oss.str());  // -1: invalid pid
        }
    }
}

void GcHung::UpdateStartTime()
{
    startTimeNs_ = time::GetCurrentTimeInNanos();
}

void GcHung::Start()
{
    if (instance_ == nullptr) {
        return;
    }
    if (!instance_->IsEnabled() || !instance_->IsReady()) {
        return;
    }
    instance_->UpdateStartTime();
}

void GcHung::Check(const GCTask &task)
{
    if (instance_ == nullptr) {
        LOG(WARNING, GC) << "GcHung not initiated yet";
        return;
    }
    if (!instance_->IsEnabled() || !instance_->IsReady()) {
        return;
    }
    if (task.reason != GCTaskCause::NATIVE_ALLOC_CAUSE) {
        instance_->CheckFrequency();
    }
    instance_->CheckOvertime(task);
}

// NOLINTNEXTLINE(google-runtime-references)
void GcHung::Check(const PandaList<MTManagedThread *> &threads, uint64_t startTime)
{
    if (instance_ != nullptr) {
        instance_->CheckSuspend(threads, startTime);
    } else {
        LOG(INFO, GC) << "GcHung not initiated yet, skip checking";
    }
}

bool GcHung::UpdateConfig()
{
    if (instance_ == nullptr) {
        LOG(ERROR, GC) << "GcHung Update Config failed, GcHung not initiated yet";
        return false;
    }
    if (!instance_->IsReady()) {
        LOG(ERROR, GC) << "GcHung Update Config failed, hung not ready";
        return false;
    }
    if (instance_->GetConfig() != 0) {
        LOG(ERROR, GC) << "GcHung Update Config failed, GetConfig again failed";
        return false;
    }
    LOG(ERROR, GC) << "GcHung Update Config success";
    return true;
}

void GcHung::InitPreFork(bool enabled)
{
    LOG(DEBUG, GC) << "GcHung: InitPreFork";
    if (instance_ == nullptr) {
        instance_ = new GcHung();
    }
    instance_->SetEnabled(enabled);
    if (enabled) {
        instance_->LoadLibimonitor();
        instance_->GetConfig();
    }
}

void GcHung::InitPostFork(bool isSystemserver)
{
    LOG(DEBUG, GC) << "GcHung: InitPostFork";
    if (instance_ != nullptr) {
        instance_->InitInternal(isSystemserver);
    }
}

GcHung *GcHung::Current()
{
    return instance_;
}

}  // namespace ark::mem