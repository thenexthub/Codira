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

#ifndef PANDA_RUNTIME_TOOLING_THREAD_SAMPLING_INFO_H
#define PANDA_RUNTIME_TOOLING_THREAD_SAMPLING_INFO_H

#include <csetjmp>
#include "libarkbase/macros.h"

namespace ark::tooling::sampler {

class ThreadSamplingInfo {
public:
    ThreadSamplingInfo() = default;
    ~ThreadSamplingInfo() = default;

    NO_COPY_SEMANTIC(ThreadSamplingInfo);
    NO_MOVE_SEMANTIC(ThreadSamplingInfo);

    bool IsThreadSampling() const
    {
        return isThreadSampling_;
    }

    void SetThreadSampling(bool threadSampling)
    {
        isThreadSampling_ = threadSampling;
    }

    jmp_buf &GetSigSegvJmpEnv()
    {
        return sigsegvJmpEnv_;
    }

private:
    bool isThreadSampling_ {false};

    // Environment that saved by setjmp in SIGPROF handler in Sampler
    // Used for longjmp in case of SIGSEGV during thread sampling
    jmp_buf sigsegvJmpEnv_ {};
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_THREAD_SAMPLING_INFO_H
