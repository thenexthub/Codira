/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_RUNTIME_COMPILER_QUEUE_AGED_COUNTER_PRIORITY_H_
#define PANDA_RUNTIME_COMPILER_QUEUE_AGED_COUNTER_PRIORITY_H_

#include "runtime/compiler_queue_counter_priority.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/method-inl.h"

namespace ark {

/**
 * The aged counter priority queue works mostly as counter priority queue (see description),
 * but it sorts the methods by its aged hotness counters.
 * If the aged counter is less then some death value, it is considered as expired and is removed.
 * Epoch duration and death counter is configured.
 * This queue is thread unsafe (should be used under lock).
 */
class CompilerPriorityAgedCounterQueue : public CompilerPriorityCounterQueue {
public:
    explicit CompilerPriorityAgedCounterQueue(mem::InternalAllocatorPtr allocator, uint64_t maxLength,
                                              uint64_t deathCounterValue, uint64_t epochDuration)
        : CompilerPriorityCounterQueue(allocator, maxLength, 0 /* unused */)
    {
        deathCounterValue_ = deathCounterValue;
        epochDuration_ = epochDuration;
        if (epochDuration_ <= 0) {
            LOG(FATAL, COMPILATION_QUEUE) << "Incorrect value of epoch duration: " << epochDuration_;
        }
        SetQueueName("priority aged counter compilation queue");
    }

protected:
    bool UpdateCounterAndCheck(CompilationQueueElement *element) override
    {
        // Rounding
        uint64_t currentTime = time::GetCurrentTimeInMillis();
        ASSERT(currentTime >= element->GetTimestamp());
        uint64_t duration = currentTime - element->GetTimestamp();
        uint64_t epochs = duration / epochDuration_;
        int64_t agedCounter = element->GetContext().GetMethod()->GetHotnessCounter() / std::pow(2, epochs);
        element->UpdateCounter(agedCounter);
        // Hotness counter is 0 when method was put into queue and was decremented while waiting in queue.
        // Thus it is negative and let's compare its absolute value with death_counter_value_
        return (static_cast<uint64_t>(std::abs(agedCounter)) < deathCounterValue_);
    }

private:
    uint64_t deathCounterValue_;
    uint64_t epochDuration_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COMPILER_QUEUE_AGED_COUNTER_PRIORITY_H_
