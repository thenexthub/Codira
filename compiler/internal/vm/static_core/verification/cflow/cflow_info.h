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

#ifndef PANDA_VERIFICATION_CFLOW_INFO_HPP_
#define PANDA_VERIFICATION_CFLOW_INFO_HPP_

#include "libarkbase/macros.h"
#include "verification/verification_status.h"

#include "runtime/include/class.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"

#include <cstdint>
#include <optional>

namespace ark::verifier {
class Job;

enum class InstructionType { NORMAL, JUMP, COND_JUMP, RETURN, THROW };

class CflowMethodInfo {
public:
    enum Flag : uint8_t { INSTRUCTION = 1, EXCEPTION_SOURCE = 2, EXCEPTION_HANDLER = 4, JUMP_TARGET = 8 };

    CflowMethodInfo() = delete;
    CflowMethodInfo(uint8_t const *addrStart, size_t codeSize)
        : addrStart_ {addrStart},
          addrEnd_ {addrStart + codeSize}  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    {
        flags_.resize(codeSize);
    }
    ~CflowMethodInfo() = default;
    NO_COPY_SEMANTIC(CflowMethodInfo);
    NO_MOVE_SEMANTIC(CflowMethodInfo);

    uint8_t const *GetAddrStart() const
    {
        return addrStart_;
    }

    uint8_t const *GetAddrEnd() const
    {
        return addrEnd_;
    }

    bool IsAddrValid(uint8_t const *addr) const
    {
        return addrStart_ <= addr && addr < addrEnd_;
    }

    bool IsFlagSet(uint8_t const *addr, Flag flag) const
    {
        ASSERT(addr >= addrStart_);
        ASSERT(addr < addrEnd_);
        return ((flags_[addr - addrStart_] & flag) != 0);
    }

    void SetFlag(uint8_t const *addr, Flag flag)
    {
        ASSERT(addr >= addrStart_);
        ASSERT(addr < addrEnd_);
        flags_[addr - addrStart_] |= flag;
    }

    void ClearFlag(uint8_t const *addr, Flag flag)
    {
        ASSERT(addr >= addrStart_);
        ASSERT(addr < addrEnd_);
        flags_[addr - addrStart_] &= static_cast<uint8_t>(~flag);
    }

    PandaVector<uint8_t const *> const *GetHandlerStartAddresses() const
    {
        return &handlerStartAddresses_;
    }

private:
    uint8_t const *addrStart_;
    uint8_t const *addrEnd_;
    PandaVector<uint8_t> flags_;
    PandaVector<uint8_t const *> handlerStartAddresses_;

    VerificationStatus FillCodeMaps(Method const *method);
    VerificationStatus ProcessCatchBlocks(Method const *method);

    friend PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(Method const *method);
};

PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(Method const *method);
}  // namespace ark::verifier

#endif  // !PANDA_VERIFICATION_CFLOW_INFO_HPP_
