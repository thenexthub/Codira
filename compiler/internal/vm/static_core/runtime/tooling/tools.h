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

#ifndef PANDA_RUNTIME_TOOLING_TOOLS_H
#define PANDA_RUNTIME_TOOLING_TOOLS_H

#include <memory>
#include "libarkbase/macros.h"
#include "sampler/sample_writer.h"

namespace ark::tooling {
class CoverageListener;

namespace sampler {
class Sampler;
}  // namespace sampler

class Tools {
public:
    Tools() = default;
    ~Tools() = default;

    void CreateSamplingProfiler();
    sampler::Sampler *GetSamplingProfiler();
    PANDA_PUBLIC_API bool StartSamplingProfiler(std::unique_ptr<sampler::StreamWriter> streamWriter, uint32_t interval);
    PANDA_PUBLIC_API void StopSamplingProfiler();
    void DestroySamplingProfiler();
    bool IsSamplingProfilerCreate();

    void CreateCoverageListener(const std::string &filePath);
    CoverageListener *GetCoverageListener();
    void DestroyCoverageListener();

private:
    NO_COPY_SEMANTIC(Tools);
    NO_MOVE_SEMANTIC(Tools);

    sampler::Sampler *sampler_ {nullptr};
    CoverageListener *coverageListener_ {nullptr};
};

}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_TOOLS_H
