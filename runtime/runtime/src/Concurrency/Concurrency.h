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


#ifndef MRT_CONCURRENCY_H
#define MRT_CONCURRENCY_H

#include <list>
#include <mutex>
#include <sys/types.h>
#include <unistd.h>

#include "ConcurrencyModel.h"
#include "CODEThreadModel/CODEThreadModel.h"
#include "Mutator/Mutator.h"
#include "RuntimeConfig.h"

namespace MapleRuntime {
class Concurrency : public ConcurrencyModel {
public:
    Concurrency() = default;
    ~Concurrency() override
    {
        if (concurrencyModel != nullptr) {
            delete concurrencyModel;
            concurrencyModel = nullptr;
        }
    }

    void Init(const ConcurrencyParam param, ScheduleType type = SCHEDULE_DEFAULT) override;

    void VisitGCRoots(RootVisitor* visitorHandle) override { concurrencyModel->VisitGCRoots(visitorHandle); }

    void* GetThreadScheduler() const override { return concurrencyModel->GetThreadScheduler(); }
    size_t GetReservedStackSize() const override { return concurrencyModel->GetReservedStackSize(); }
    bool GetStackGuardCheckFlag() const override { return concurrencyModel->GetStackGuardCheckFlag(); }

    uint32_t GetProcessorNum() const override { return processorNum; }

private:
    ConcurrencyModel* concurrencyModel = nullptr;
    uint32_t processorNum = 1;
};
} // namespace MapleRuntime
#endif // MRT_CONCURRENCY_H
