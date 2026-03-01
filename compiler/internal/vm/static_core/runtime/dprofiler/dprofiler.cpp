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

#include "runtime/dprofiler/dprofiler.h"

#include <chrono>

#include "dprof/profiling_data.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/serializer/serializer.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/class.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"

namespace ark {

class DProfilerListener : public RuntimeListener {
public:
    explicit DProfilerListener(DProfiler *dprofiler) : dprofiler_(dprofiler) {}

    void VmDeath() override
    {
        Runtime::GetCurrent()->GetClassLinker()->EnumerateClasses([this](Class *klass) -> bool {
            dprofiler_->AddClass(klass);
            return true;
        });
        dprofiler_->Dump();
    }

    ~DProfilerListener() override = default;

    DEFAULT_COPY_SEMANTIC(DProfilerListener);
    DEFAULT_MOVE_SEMANTIC(DProfilerListener);

private:
    DProfiler *dprofiler_;
};

static PandaString GetFullName(const Method *method)
{
    return reinterpret_cast<const char *>(method->GetClassName().data) + PandaString(".") +
           reinterpret_cast<const char *>(method->GetName().data);
}

static uint64_t GetHash()
{
    auto t = std::chrono::steady_clock::now();
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(t).time_since_epoch().count();
}

DProfiler::DProfiler(std::string_view appName, Runtime *runtime)
    : runtime_(runtime),
      profilingData_(
          MakePandaUnique<dprof::ProfilingData>(appName.data(), GetHash(), os::thread::GetCurrentThreadId())),
      listener_(MakePandaUnique<DProfilerListener>(this))
{
    runtime_->GetNotificationManager()->AddListener(listener_.get(), RuntimeNotificationManager::Event::VM_EVENTS);
}

void DProfiler::AddClass(const Class *klass)
{
    for (const auto &method : klass->GetMethods()) {
        if (method.GetHotnessCounter() != 0) {
            if (!hotMethods_.insert(&method).second) {
                LOG(ERROR, DPROF) << "Method already exsists: " << GetFullName(&method);
            }
        }
    }
}

void DProfiler::Dump()
{
    PandaUnorderedMap<PandaString, uint32_t> methodInfoMap;
    for (const Method *method : hotMethods_) {
        auto ret = methodInfoMap.emplace(std::make_pair(GetFullName(method), method->GetHotnessCounter()));
        if (!ret.second) {
            LOG(ERROR, DPROF) << "Method already exists: " << ret.first->first;
        }
    }

    std::vector<uint8_t> buffer;
    auto ret = serializer::TypeToBuffer(methodInfoMap, buffer);
    if (!ret) {
        LOG(ERROR, DPROF) << "Cannot serialize method_info_map. Error: " << ret.Error();
        return;
    }

    profilingData_->SetFeatureDate("hotness_counters.v1", std::move(buffer));
    profilingData_->DumpAndResetFeatures();
}

}  // namespace ark
