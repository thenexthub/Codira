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

#include "libarkbase/os/cpu_affinity.h"
#include "libarkbase/os/thread.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <thread>

namespace ark::os {

CpuSet::CpuSet() : cpuset_(0) {}

void CpuSet::Set(int cpu)
{
    cpuset_ = 1 << cpu;
}

void CpuSet::Clear()
{
    cpuset_ = 0;
}

void CpuSet::Remove(int cpu)
{
    cpuset_ &= ~(1 << cpu);
}

size_t CpuSet::Count() const
{
    unsigned long mask = cpuset_;
    size_t count = 0;
    while (mask) {
        if (mask & 1) {
            count++;
        }
        mask >>= 1;
    }
    return count;
}

bool CpuSet::IsSet(int cpu) const
{
    return (cpuset_ & (1 << cpu)) != 0;
}

bool CpuSet::IsEmpty() const
{
    return Count() == 0U;
}

CpuSetType *CpuSet::GetData()
{
    return &cpuset_;
}

const CpuSetType *CpuSet::GetData() const
{
    return &cpuset_;
}

bool CpuSet::operator==(const CpuSet &other) const
{
    return cpuset_ == other.cpuset_;
}

bool CpuSet::operator!=(const CpuSet &other) const
{
    return cpuset_ != other.cpuset_;
}

/* static */
void CpuSet::And(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs)
{
    result.cpuset_ = lhs.cpuset_ & rhs.cpuset_;
}

/* static */
void CpuSet::Or(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs)
{
    result.cpuset_ = lhs.cpuset_ | rhs.cpuset_;
}

/* static */
void CpuSet::Xor(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs)
{
    result.cpuset_ = lhs.cpuset_ | rhs.cpuset_;
}

/* static */
void CpuSet::Copy(CpuSet &destination, const CpuSet &source)
{
    destination.cpuset_ = source.cpuset_;
}

CpuSet &CpuSet::operator&=(const CpuSet &other)
{
    cpuset_ &= other.cpuset_;
    return *this;
}

CpuSet &CpuSet::operator|=(const CpuSet &other)
{
    cpuset_ |= other.cpuset_;
    return *this;
}

CpuSet &CpuSet::operator^=(const CpuSet &other)
{
    cpuset_ ^= other.cpuset_;
    return *this;
}

size_t CpuAffinityManager::cpuCount_ = 0U;
CpuSet CpuAffinityManager::bestCpuSet_;           // NOLINT(fuchsia-statically-constructed-objects)
CpuSet CpuAffinityManager::middleCpuSet_;         // NOLINT(fuchsia-statically-constructed-objects)
CpuSet CpuAffinityManager::bestAndMiddleCpuSet_;  // NOLINT(fuchsia-statically-constructed-objects)
CpuSet CpuAffinityManager::weakCpuSet_;           // NOLINT(fuchsia-statically-constructed-objects)

/* static */
void CpuAffinityManager::Initialize()
{
    ASSERT(!IsCpuAffinityEnabled());
    ASSERT(bestCpuSet_.IsEmpty());
    ASSERT(middleCpuSet_.IsEmpty());
    ASSERT(bestAndMiddleCpuSet_.IsEmpty());
    ASSERT(weakCpuSet_.IsEmpty());
    cpuCount_ = std::thread::hardware_concurrency();
    LoadCpuFreq();
}

/* static */
bool CpuAffinityManager::IsCpuAffinityEnabled()
{
    return cpuCount_ != 0U;
}

/* static */
void CpuAffinityManager::Finalize()
{
    cpuCount_ = 0U;
    bestCpuSet_.Clear();
    middleCpuSet_.Clear();
    bestAndMiddleCpuSet_.Clear();
    weakCpuSet_.Clear();
}

/* static */
void CpuAffinityManager::LoadCpuFreq()
{
    // No implement for windows now
    cpuCount_ = 0U;
}

/* static */
bool CpuAffinityManager::GetThreadAffinity([[maybe_unused]] int tid, [[maybe_unused]] CpuSet &cpuset)
{
    return false;
}

/* static */
bool CpuAffinityManager::GetCurrentThreadAffinity(CpuSet &cpuset)
{
    return GetThreadAffinity(0, cpuset);
}

/* static */
bool CpuAffinityManager::SetAffinityForThread([[maybe_unused]] int tid, [[maybe_unused]] uint8_t power_flags)
{
    return false;
}

/* static */
bool CpuAffinityManager::SetAffinityForThread([[maybe_unused]] int tid, [[maybe_unused]] const CpuSet &cpuset)
{
    return false;
}

/* static */
bool CpuAffinityManager::SetAffinityForCurrentThread(uint8_t power_flags)
{
    return SetAffinityForThread(0, power_flags);
}

/* static */
bool CpuAffinityManager::SetAffinityForCurrentThread(const CpuSet &cpuset)
{
    return SetAffinityForThread(0, cpuset);
}

}  // namespace ark::os
