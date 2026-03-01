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

#ifndef COMPILER_OPTIMIZER_CODEGEN_REGFILE_H
#define COMPILER_OPTIMIZER_CODEGEN_REGFILE_H

#include "operands.h"
#include "libarkbase/utils/arch.h"

/*
Register file wrapper  used for get major data and for Regalloc
*/
namespace ark::compiler {
#ifdef PANDA_COMPILER_TARGET_X86_64
namespace amd64 {
static constexpr size_t RENAMING_MASK_3_5_OR_9_11 {0xE38};
static constexpr size_t RENAMING_CONST {14U};

// There is a problem with callee/caller register numbers with amd64.
// For example, take a look at
// caller reg mask: 0000111111000111 and
// callee reg mask: 1111000000001000
// Stack walker requires this mask to be densed, so the decision is to
// rename regs number 3, 4, 5 to 11, 10, 9 (and vice versa).
// Resulting
// caller mask is 0000000111111111 and
// callee mask is 1111100000000000.
static inline constexpr size_t ConvertRegNumber(size_t regId)
{
    ASSERT(regId < MAX_NUM_REGS);
    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    if ((RENAMING_MASK_3_5_OR_9_11 & (size_t(1) << regId)) != 0) {
        return RENAMING_CONST - regId;
    }
    return regId;
}
}  // namespace amd64
#endif  // PANDA_COMPILER_TARGET_X86_64

class RegistersDescription {
public:
    explicit RegistersDescription(ArenaAllocator *aa, Arch arch) : arenaAllocator_(aa), arch_(arch) {}
    virtual ~RegistersDescription() = default;

    virtual ArenaVector<Reg> GetCalleeSaved() = 0;
    virtual void SetCalleeSaved(const ArenaVector<Reg> &) = 0;
    // Set used regs - change GetCallee
    virtual void SetUsedRegs(const ArenaVector<Reg> &) = 0;
    // Return zero register. If target architecture doesn't support zero register, it should return INVALID_REGISTER.
    virtual Reg GetZeroReg() const = 0;
    virtual bool IsZeroReg(Reg reg) const = 0;
    virtual Reg::RegIDType GetTempReg() = 0;
    virtual Reg::RegIDType GetTempVReg() = 0;
    // Return RegMapping bitset
    virtual bool SupportMapping(uint32_t) = 0;

    virtual bool IsValid() const
    {
        return false;
    };

    virtual bool IsCalleeRegister(Reg reg) = 0;

    ArenaAllocator *GetAllocator() const
    {
        return arenaAllocator_;
    };

    // May be re-define to ignore some cases
    virtual bool IsRegUsed(ArenaVector<Reg> vecReg, Reg reg)
    {
        // size ignored in arm64
        auto equality = [reg](Reg in) {
            return ((reg.GetId() == in.GetId()) && (reg.GetType() == in.GetType()) &&
                    (reg.GetSize() == in.GetSize())) ||
                   (!reg.IsValid() && !in.IsValid());
        };
        return (std::find_if(vecReg.begin(), vecReg.end(), equality) != vecReg.end());
    }

    static RegistersDescription *Create(ArenaAllocator *arenaAllocator, Arch arch);

    RegMask GetRegMask() const
    {
        return regMask_.None() ? GetDefaultRegMask() : regMask_;
    }

    void SetRegMask(const RegMask &mask)
    {
        regMask_ = mask;
    }

    // Get registers mask which used in codegen, runtime e.t.c
    // 0 means - available, 1 - unavailable to use
    // Note that it is a default architecture-specific registers mask.
    virtual RegMask GetDefaultRegMask() const = 0;

    // Get vector registers mask which used in codegen, runtime e.t.c
    virtual VRegMask GetVRegMask() = 0;

    virtual RegMask GetCallerSavedRegMask() const = 0;
    virtual RegMask GetCallerSavedVRegMask() const = 0;

    void FillUsedCalleeSavedRegisters(RegMask *calleeRegs, VRegMask *calleeVregs, bool setAllCalleeRegisters,
                                      bool irtocOptimized = false)
    {
        if (setAllCalleeRegisters) {
            *calleeRegs = RegMask(ark::GetCalleeRegsMask(arch_, false, irtocOptimized));
            *calleeVregs = VRegMask(ark::GetCalleeRegsMask(arch_, true, irtocOptimized));
        } else {
            ASSERT(!irtocOptimized);
            *calleeRegs = GetUsedRegsMask<RegMask, false>(GetCalleeSaved());
            *calleeVregs = GetUsedRegsMask<VRegMask, true>(GetCalleeSaved());
        }
    }

    NO_COPY_SEMANTIC(RegistersDescription);
    NO_MOVE_SEMANTIC(RegistersDescription);

private:
    ArenaAllocator *arenaAllocator_ {nullptr};
    Arch arch_;
    RegMask regMask_ {0};

    template <typename M, bool IS_FP>
    M GetUsedRegsMask(const ArenaVector<Reg> &regs)
    {
        M mask;
        for (auto reg : regs) {
            // NOLINTNEXTLINE(bugprone-branch-clone,-warnings-as-errors)
            if (reg.IsFloat() && IS_FP) {
                mask.set(reg.GetId());
            } else if (reg.IsScalar() && !IS_FP) {
                mask.set(reg.GetId());
            }
        }
        return mask;
    }
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_REGFILE_H
