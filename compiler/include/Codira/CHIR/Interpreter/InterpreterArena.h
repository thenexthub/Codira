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
 * This file declares an arena for allocating interpreter values.
 */

#ifndef CODIRA_CHIR_INTERRETER_INTERPREVERARENA_H
#define CODIRA_CHIR_INTERRETER_INTERPREVERARENA_H

#include "Codira/CHIR/Interpreter/InterpreterValueUtils.h"

namespace Codira::CHIR::Interpreter {

class Arena {
public:
    /* list of objects that needs to run finalizer on them */
    std::vector<IVal*> finalizingObjects;

    Arena()
    {
        buckets.reserve(BUCKETS);
        buckets.emplace_back(std::make_unique<std::vector<IVal>>());
        buckets.back()->reserve(BUCKET_SIZE);
        finalizingObjects.reserve(BUCKET_SIZE);
    }
    IVal* Allocate(IVal&& value)
    {
        if (buckets.back()->size() == BUCKET_SIZE) {
            buckets.emplace_back(std::make_unique<std::vector<IVal>>());
            buckets.back()->reserve(BUCKET_SIZE);
        }
        auto& lastBucket = buckets.back();
        lastBucket->emplace_back(std::move(value));
        auto ptr = &lastBucket->back();
        return ptr;
    }

    void PrintStats()
    {
        std::cout << "Number of buckets: " << buckets.size() << std::endl;
    }

    int64_t GetAllocatedSize()
    {
        CODEC_ASSERT(buckets.size() >= 1);
        size_t r = ((buckets.size() - 1) * BUCKET_SIZE + buckets.back()->size()) * sizeof(IVal);
        return static_cast<int64_t>(r);
    }

private:
    static const size_t BUCKETS = 2048;
    static const size_t BUCKET_SIZE = 2048;

    // Why unique_ptr? Because in C++ vector reallocation may either copy or move its contents.
    // It sohuld move if possible -- and it should be possible in this case.
    // HOWEVER, unique_ptr will FORCE the move. IE -- if something goes wrong and moving is not
    // possible for some reason, this will NOT COMPILE.
    // Therefore, I think to start of it's worth keeping the indirection on, and remove it only
    // after profiling. T0D0!!
    using Bucket = std::unique_ptr<std::vector<IVal>>;
    std::vector<Bucket> buckets;
};

} // namespace Codira::CHIR::Interpreter

#endif // CODIRA_CHIR_INTERRETER_INTERPREVERARENA_H
