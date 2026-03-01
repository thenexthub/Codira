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

#ifndef PANDA_LIBPANDABASE_OS_CPU_AFFINITY_H
#define PANDA_LIBPANDABASE_OS_CPU_AFFINITY_H

#include "libarkbase/concepts.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/thread.h"

#include <array>
#include <limits>

#ifdef PANDA_TARGET_UNIX
#include "platforms/unix/libarkbase/cpu_affinity.h"
#elif defined PANDA_TARGET_WINDOWS
#include "platforms/windows/libarkbase/cpu_affinity.h"
#else
#error "Unsupported platform"
#endif

namespace ark::os {

class CpuSet {
public:
    PANDA_PUBLIC_API CpuSet();

    NO_COPY_SEMANTIC(CpuSet);
    NO_MOVE_SEMANTIC(CpuSet);

    /**
     * Add cpu into the cpu-set
     * @param cpu number of cpu
     */
    void Set(int cpu);

    /// Full clear the cpu-set
    PANDA_PUBLIC_API void Clear();

    /**
     * Remove cpu from the cpu-set
     * @param cpu number of cpu
     */
    void Remove(int cpu);

    /// @return CPUs count in the cpu-set
    [[nodiscard]] PANDA_PUBLIC_API size_t Count() const;

    /**
     * @param cpu number of cpu
     * @return true if cpu-set has the cpu number, false - otherwise
     */
    [[nodiscard]] bool IsSet(int cpu) const;

    /// @return true if cpu-set has not any cpu
    [[nodiscard]] PANDA_PUBLIC_API bool IsEmpty() const;

    ~CpuSet() = default;

    [[nodiscard]] bool operator==(const CpuSet &other) const;
    [[nodiscard]] bool operator!=(const CpuSet &other) const;

    CpuSet &operator&=(const CpuSet &other);
    CpuSet &operator|=(const CpuSet &other);
    CpuSet &operator^=(const CpuSet &other);

    static void And(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs);
    static void Or(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs);
    static void Xor(CpuSet &result, const CpuSet &lhs, const CpuSet &rhs);
    static void Copy(CpuSet &destination, const CpuSet &source);

private:
    [[nodiscard]] CpuSetType *GetData();
    [[nodiscard]] const CpuSetType *GetData() const;

    friend class CpuAffinityManager;

    CpuSetType cpuset_;
};

// NOLINTNEXTLINE(hicpp-signed-bitwise)
enum CpuPower : uint8_t { BEST = 1U, MIDDLE = 1U << 1U, WEAK = 1U << 2U, ANY = 0U, ALL = BEST | MIDDLE | WEAK };

class CpuAffinityManager {
public:
    struct CpuInfo {
        size_t number;
        uint64_t freq;
    };

    CpuAffinityManager() = delete;
    NO_COPY_SEMANTIC(CpuAffinityManager);
    NO_MOVE_SEMANTIC(CpuAffinityManager);
    ~CpuAffinityManager() = default;

    /// Initialize Cpu Affinity Manager: load CPUs info, indicate CPUs power
    PANDA_PUBLIC_API static void Initialize();

    /// @return true if Cpu Affinity Manager was enabled correctly
    [[nodiscard]] static bool IsCpuAffinityEnabled();

    /**
     * Set the thread on specific CPUs
     * @param tid thread id for setting
     * @param cpuset cpu-set to indicate used CPU numbers
     * @return true if setting was successfully, false - otherwise
     */
    PANDA_PUBLIC_API static bool SetAffinityForThread(int tid, const CpuSet &cpuset);

    /**
     * Set the thread on CPUs with specific flags
     * @param tid thread id for setting
     * @param power_flags Flags to indicate CPUs power
     * @return true if setting was successfully, false - otherwise
     */
    PANDA_PUBLIC_API static bool SetAffinityForThread(int tid, uint8_t powerFlags);

    /**
     * Set current thread on CPUs with specific flags
     * @param power_flags Flags to indicate CPUs power
     * @see CpuPower
     * @return true if setting was successfully, false - otherwise
     */
    PANDA_PUBLIC_API static bool SetAffinityForCurrentThread(uint8_t powerFlags);

    /**
     * Set current thread on specific CPUs
     * @param cpuset cpu-set to indicate used CPU numbers
     * @see CpuSet
     * @return true if setting was successfully, false - otherwise
     */
    PANDA_PUBLIC_API static bool SetAffinityForCurrentThread(const CpuSet &cpuset);

    /// @return const reference on best + middle cpu-set in system
    PANDA_PUBLIC_API static const CpuSet &GetBestAndMiddleCpuSet()
    {
        return bestAndMiddleCpuSet_;
    }

    /**
     * Get cpu affinity for current thread
     * @param tid thread id for getting
     * @param cpuset reference for affinity saving
     * @return true if getting was successfully, false - otherwise
     */
    static bool GetThreadAffinity(int tid, CpuSet &cpuset);

    /**
     * Get cpu affinity for current thread
     * @param cpuset reference for affinity saving
     * @return true if getting was successfully, false - otherwise
     */
    PANDA_PUBLIC_API static bool GetCurrentThreadAffinity(CpuSet &cpuset);

    /**
     * Get string representation for cpu-set
     * @tparam StringType type of result string
     * @param cpuset cpu-set for string representation
     * @return string with 0 (unused cpu) and 1 (used cpu) for cpu-set representation
     */
    template <class StringType = std::string>
    static std::enable_if_t<is_stringable_v<StringType>, StringType> CpuSetToString(const CpuSet &cpuset)
    {
        StringType mask(cpuCount_, '0');
        for (size_t i = 0U; i < cpuCount_; ++i) {
            if (cpuset.IsSet(i)) {
                mask[cpuCount_ - 1 - i] = '1';
            }
        }
        return mask;
    }

    /// Finalize Cpu Affinity Manager
    PANDA_PUBLIC_API static void Finalize();

private:
    /// Load cpu frequency info from system
    static void LoadCpuFreq();

    static size_t cpuCount_;

    static CpuSet bestCpuSet_;
    static CpuSet middleCpuSet_;
    PANDA_PUBLIC_API static CpuSet bestAndMiddleCpuSet_;
    static CpuSet weakCpuSet_;
};

}  // namespace ark::os

#endif  // PANDA_LIBPANDABASE_OS_CPU_AFFINITY_H
