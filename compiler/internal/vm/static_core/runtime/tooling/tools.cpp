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

#include "runtime/tooling/coverage_listener.h"
#include "runtime/tooling/tools.h"
#include "runtime/tooling/sampler/sampling_profiler.h"

namespace ark::tooling {

extern "C" PANDA_PUBLIC_API int StartSamplingProfiler(const char *asptFilename, int interva)
{
    auto *runtime = Runtime::GetCurrent();
    if (runtime == nullptr) {
        LOG(DEBUG, PROFILER) << "That runtime is not created.";
        return 1;
    }
    return static_cast<int>(
        !runtime->GetTools().StartSamplingProfiler(std::make_unique<sampler::FileStreamWriter>(asptFilename), interva));
}

extern "C" PANDA_PUBLIC_API void StopSamplingProfiler()
{
    auto *runtime = Runtime::GetCurrent();
    if (runtime == nullptr) {
        LOG(DEBUG, PROFILER) << "That runtime is not created.";
        return;
    }
    runtime->GetTools().StopSamplingProfiler();
}

sampler::Sampler *Tools::GetSamplingProfiler()
{
    // Singleton instance
    return sampler_;
}

void Tools::CreateSamplingProfiler()
{
    ASSERT(sampler_ == nullptr);
    sampler_ = sampler::Sampler::Create();

    const char *samplerSegvOption = std::getenv("ARK_SAMPLER_DISABLE_SEGV_HANDLER");
    if (samplerSegvOption != nullptr) {
        std::string_view option = samplerSegvOption;
        if (option == "1" || option == "true" || option == "ON") {
            // SEGV handler for sampler is enable by default
            sampler_->SetSegvHandlerStatus(false);
        }
    }
}

bool Tools::StartSamplingProfiler(std::unique_ptr<sampler::StreamWriter> streamWriter, uint32_t interval)
{
    ASSERT(sampler_ != nullptr);
    sampler_->SetSampleInterval(interval);
    return sampler_->Start(std::move(streamWriter));
}

void Tools::StopSamplingProfiler()
{
    ASSERT(sampler_ != nullptr);
    sampler_->Stop();
}
void Tools::DestroySamplingProfiler()
{
    ASSERT(sampler_ != nullptr);
    sampler::Sampler::Destroy(sampler_);
    sampler_ = nullptr;
}

bool Tools::IsSamplingProfilerCreate()
{
    return sampler_ != nullptr;
}

void Tools::CreateCoverageListener(const std::string &filePath)
{
    coverageListener_ = new tooling::CoverageListener(filePath);
}

CoverageListener *Tools::GetCoverageListener()
{
    return coverageListener_;
}

void Tools::DestroyCoverageListener()
{
    if (coverageListener_ == nullptr) {
        return;
    }
    delete coverageListener_;
    coverageListener_ = nullptr;
}
}  // namespace ark::tooling
