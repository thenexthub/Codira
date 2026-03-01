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

#ifndef PANDA_STS_VM_INTERFACE_H
#define PANDA_STS_VM_INTERFACE_H

#include "hybrid/vm_interface.h"
#include <cstdint>
#include <cstddef>
#include <functional>

namespace platform {

class STSVMInterface : public VMInterface {
public:
    static constexpr size_t DEFAULT_XGC_EXECUTORS_COUNT = 2U;
    using NoWorkPred = std::function<bool()>;

    STSVMInterface() = default;

    STSVMInterface(const STSVMInterface &) = delete;
    void operator=(const STSVMInterface &) = delete;

    STSVMInterface(STSVMInterface &&) = delete;
    STSVMInterface &operator=(STSVMInterface &&) = delete;

    ~STSVMInterface() override = default;

    VMInterfaceType GetVMType() const override
    {
        return STSVMInterface::VMInterfaceType::ETS_VM_IFACE;
    }

    /**
     * @brief executes marking operation of STS VM GC that will be started from gotten object.
     * @param obj: pointer to object from which marking will be started.
     */
    virtual void MarkFromObject(void *obj) = 0;
    /**
     * @brief Increment count of threads that will execute XGC. If you use this method in the scope of XGC execution,
     * new count of threads will be ignored by currect XGC, but will be used in next one.
     */
    virtual void OnVMAttach() = 0;
    /**
     * @brief Decrement count of threads that will execute XGC. If you use this method in the scope of XGC execution,
     * new count of threads will be ignored by currect XGC, but will be used in next one.
     */
    virtual void OnVMDetach() = 0;
    /**
     * @brief Method waits for all threads to start XGC. The count of threads can be changed using methods OnVMAttach
     * and OnVMDetach. After this method XGC is considered to be running.
     * @param func: predicate that checks if XGC was interrupted or not
     * @returns value based on NoWorkPred, can be not only interruption in the common case
     */
    virtual bool StartXGCBarrier(const NoWorkPred &func) = 0;
    /**
     * @brief Method waits for all threads would call WaitForConcurrentMark.
     * @param func: predicate that checks if we need to leave barrier to continue marking
     * @returns true if all threads called this methods. It can return false if VM should try continue marking from new
     * objects.
     */
    virtual bool WaitForConcurrentMark(const NoWorkPred &func) = 0;
    /// @brief Method waits for all threads would call RemarkStartBarrier
    virtual void RemarkStartBarrier() = 0;
    /**
     * @brief Method waits for all threads would call WaitForRemark.
     * @param func: predicate that checks if we need to leave barrier to continue marking
     * @returns true if all threads called this methods. It can return false if VM should try continue marking from new
     * objects.
     */
    virtual bool WaitForRemark(const NoWorkPred &func) = 0;
    /// @brief Method waits for all threads would call FinishXGCBarrier
    virtual void FinishXGCBarrier() = 0;
};

}  // namespace platform

#endif  // PANDA_STS_VM_INTERFACE_H
