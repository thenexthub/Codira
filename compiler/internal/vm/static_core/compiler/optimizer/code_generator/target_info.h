/*
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

#ifndef PANDA_TARGET_H
#define PANDA_TARGET_H

#include "operands.h"
#include "arch_info_gen.h"
#include <string>

namespace ark::compiler {

// There is a problem with callee/caller register numbers with amd64.
// For example, take a look at
// caller reg mask: 0000111111000111 and
// callee reg mask: 1111000000001000
// Stack walker requires this mask to be densed, so the decision is to
// rename regs number 3, 4, 5 to 11, 10, 9 (and vice versa).
// Resulting
// caller mask is 0000000111111111 and
// callee mask is 1111100000000000.

constexpr size_t ConvertRegNumberX86(size_t regId)
{
    constexpr size_t RENAMING_MASK_3_5_OR_9_11 {0xE38};
    constexpr size_t RENAMING_CONST {14U};

    ASSERT(regId < MAX_NUM_REGS);
    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    if ((RENAMING_MASK_3_5_OR_9_11 & (size_t(1) << regId)) != 0) {
        return RENAMING_CONST - regId;
    }
    return regId;
}

// CC-OFFNXT(G.PRE.06) list generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_NUMERIC_REGISTERS(REG) \
    REG(0)                            \
    REG(1)                            \
    REG(2)                            \
    REG(3)                            \
    REG(4)                            \
    REG(5)                            \
    REG(6)                            \
    REG(7)                            \
    REG(8)                            \
    REG(9)                            \
    REG(10)                           \
    REG(11)                           \
    REG(12)                           \
    REG(13)                           \
    REG(14)                           \
    REG(15)                           \
    REG(16)                           \
    REG(17)                           \
    REG(18)                           \
    REG(19)                           \
    REG(20)                           \
    REG(21)                           \
    REG(22)                           \
    REG(23)                           \
    REG(24)                           \
    REG(25)                           \
    REG(26)                           \
    REG(27)                           \
    REG(28)                           \
    REG(29)                           \
    REG(30)                           \
    REG(31)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_REG(name, reg) static constexpr uint8_t name = (reg)

struct ArchCallingConventionX8664 {
    // Following registers are swapped (see comment above for ConvertRegNumberX86):
    // BX, SP, BP <==> R9, R10, R11
// CC-OFFNXT(G.PRE.06) solid logic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define X86_64_REGISTER_LIST(REG) \
    REG(rax) /* 0 */              \
    REG(rcx) /* 1 */              \
    REG(rdx) /* 2 */              \
    REG(r11) /* 3 */              \
    REG(r10) /* 4 */              \
    REG(r9)  /* 5 */              \
    REG(rsi) /* 6 */              \
    REG(rdi) /* 7 */              \
    REG(r8)  /* 8 */              \
    REG(rbp) /* 9 */              \
    REG(rsp) /* 10 */             \
    REG(rbx) /* 11 */             \
    REG(r12) /* 12 */             \
    REG(r13) /* 13 */             \
    REG(r14) /* 14 */             \
    REG(r15) /* 15 */

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_ENUM(r) id_##r,
    enum Id : uint8_t { X86_64_REGISTER_LIST(DEF_ENUM) ID_COUNT };
#undef DEF_ENUM

    DEFINE_REG(SP_REG, id_rsp);
    DEFINE_REG(FP_REG, id_rbp);
    DEFINE_REG(LR_REG, INVALID_REG_ID);
    DEFINE_REG(RETURN_REG, id_rax);
    DEFINE_REG(FP_RETURN_REG, 0);
    DEFINE_REG(ZERO_REG, INVALID_REG_ID);
    static constexpr std::array<size_t, 6> CALL_PARAMS_REGS = {id_rdi, id_rsi, id_rdx, id_rcx, id_r8, id_r9};
    static constexpr uint32_t TEMP_REGS_MASK = arch_info::x86_64::TEMP_REGS.to_ulong();
    static constexpr uint32_t TEMP_VREGS_MASK = arch_info::x86_64::TEMP_FP_REGS.to_ulong();
    static constexpr uint32_t GENERAL_REGS_MASK = MakeMaskByExcluding(ID_COUNT, SP_REG, FP_REG);
    static constexpr uint32_t SP_ALIGNMENT = 16;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_REG_NAME(r) #r,
    static constexpr std::array<const char *, ID_COUNT> REG_NAMES = {X86_64_REGISTER_LIST(DEF_REG_NAME)};
#undef DEF_REG_NAME

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_FP_REG_NAME_X86(r) "xmm" #r,
    static constexpr std::array<const char *, 32> FP_REG_NAMES = {DEFINE_NUMERIC_REGISTERS(DEF_FP_REG_NAME_X86)};
#undef DEF_FP_REG_NAME_X86
    static constexpr const char *GetRegName(size_t reg, bool isFp)
    {
        ASSERT(reg < REG_NAMES.size() || isFp);
        ASSERT(reg < FP_REG_NAMES.size() || !isFp);
        return isFp ? FP_REG_NAMES[reg] : REG_NAMES[reg];
    }
};

struct ArchCallingConventionAarch64 {
    DEFINE_REG(SP_REG, 63);
    DEFINE_REG(FP_REG, 29);
    DEFINE_REG(LR_REG, 30);
    DEFINE_REG(RETURN_REG, 0);
    DEFINE_REG(FP_RETURN_REG, 0);
    DEFINE_REG(ZERO_REG, 31);
    static constexpr std::array CALL_PARAMS_REGS = {0, 1, 2, 3, 4, 5, 6, 7};
    static constexpr uint32_t TEMP_REGS_MASK = arch_info::arm64::TEMP_REGS.to_ulong();
    static constexpr uint32_t TEMP_VREGS_MASK = arch_info::arm64::TEMP_FP_REGS.to_ulong();
    static constexpr uint32_t GENERAL_REGS_MASK = MakeMaskByExcluding(32, SP_REG, FP_REG, ZERO_REG, LR_REG);
    static constexpr uint32_t SP_ALIGNMENT = 16;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_FP_REG_NAME_AARCH_64(r) "r" #r,
    static constexpr std::array<const char *, 32> REG_NAMES = {DEFINE_NUMERIC_REGISTERS(DEF_FP_REG_NAME_AARCH_64)};
#undef DEF_FP_REG_NAME_AARCH_64

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_FP_REG_NAME_AARCH_64(r) "d" #r,
    static constexpr std::array<const char *, 32> FP_REG_NAMES = {DEFINE_NUMERIC_REGISTERS(DEF_FP_REG_NAME_AARCH_64)};
#undef DEF_FP_REG_NAME_AARCH_64

    static constexpr const char *GetRegName(size_t reg, bool isFp)
    {
        ASSERT(reg < REG_NAMES.size() || isFp);
        ASSERT(reg < FP_REG_NAMES.size() || !isFp);
        return isFp ? FP_REG_NAMES[reg] : REG_NAMES[reg];
    }
};

struct ArchCallingConventionAarch32 {
    DEFINE_REG(SP_REG, 13);
    DEFINE_REG(FP_REG, 11);
    DEFINE_REG(LR_REG, 14);
    DEFINE_REG(RETURN_REG, 0);
    DEFINE_REG(FP_RETURN_REG, 0);
    DEFINE_REG(ZERO_REG, INVALID_REG_ID);
    static constexpr std::array CALL_PARAMS_REGS = {0, 1, 2, 3};
    static constexpr uint32_t TEMP_REGS_MASK = arch_info::arm32::TEMP_REGS.to_ulong();
    static constexpr uint32_t TEMP_VREGS_MASK = arch_info::arm32::TEMP_FP_REGS.to_ulong();
    static constexpr uint32_t GENERAL_REGS_MASK = MakeMaskByExcluding(16, SP_REG, FP_REG, LR_REG);
    static constexpr uint32_t SP_ALIGNMENT = 8;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_FP_REG_NAME_AARCH_32(r) "r" #r,
    static constexpr std::array<const char *, 32> REG_NAMES = {DEFINE_NUMERIC_REGISTERS(DEF_FP_REG_NAME_AARCH_32)};
#undef DEF_FP_REG_NAME_AARCH_32

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_FP_REG_NAME_AARCH_32(r) "d" #r,
    static constexpr std::array<const char *, 32> FP_REG_NAMES = {DEFINE_NUMERIC_REGISTERS(DEF_FP_REG_NAME_AARCH_32)};
#undef DEF_FP_REG_NAME_AARCH_32

    static constexpr const char *GetRegName(size_t reg, bool isFp)
    {
        ASSERT(reg < REG_NAMES.size() || isFp);
        ASSERT(reg < FP_REG_NAMES.size() || !isFp);
        return isFp ? FP_REG_NAMES[reg] : REG_NAMES[reg];
    }
};

struct ArchCallingConventionX86 {
    DEFINE_REG(SP_REG, 0);
    DEFINE_REG(FP_REG, 0);
    DEFINE_REG(LR_REG, 0);
    DEFINE_REG(RETURN_REG, 0);
    DEFINE_REG(FP_RETURN_REG, 0);
    DEFINE_REG(ZERO_REG, 0);
    static constexpr std::array CALL_PARAMS_REGS = {0, 1, 2, 3, 4, 5, 6, 7};
    static constexpr uint32_t TEMP_REGS_MASK = MakeMask(0);
    static constexpr uint32_t TEMP_VREGS_MASK = MakeMask(0);
    static constexpr uint32_t GENERAL_REGS_MASK = MakeMaskByExcluding(16, 0);
    static constexpr uint32_t SP_ALIGNMENT = 8;

    static constexpr const char *GetRegName([[maybe_unused]] size_t reg, [[maybe_unused]] bool isFp)
    {
        return "not supported";
    }
};

template <Arch ARCH>
struct ArchCallingConvention {
    using Target = std::conditional_t<
        ARCH == Arch::X86_64, ArchCallingConventionX8664,
        std::conditional_t<
            ARCH == Arch::X86, ArchCallingConventionX86,
            std::conditional_t<ARCH == Arch::AARCH64, ArchCallingConventionAarch64,
                               std::conditional_t<ARCH == Arch::AARCH32, ArchCallingConventionAarch32, void>>>>;

    static constexpr size_t WordSize()
    {
        return ArchTraits<ARCH>::IS_64_BITS ? sizeof(uint64_t) : sizeof(uint32_t);
    }
    static constexpr size_t GetReturnRegId()
    {
        return Target::RETURN_REG;
    }
    static constexpr size_t GetReturnFpRegId()
    {
        return Target::FP_RETURN_REG;
    }
    static constexpr Reg GetReturnReg(TypeInfo type)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
        if constexpr (ARCH == Arch::AARCH32) {
            if (type.IsFloat()) {
#if PANDA_TARGET_ARM32_ABI_HARD
                return Reg(GetReturnFpRegId(), (type == FLOAT64_TYPE) ? FLOAT64_TYPE : FLOAT32_TYPE);
#else
                return Reg(GetReturnRegId(), (type == FLOAT64_TYPE) ? INT64_TYPE : INT32_TYPE);
#endif
            }
            return Reg(GetReturnRegId(), type);
        }
        return type.IsFloat() ? Reg(GetReturnFpRegId(), type) : Reg(GetReturnRegId(), type);
    }
    static constexpr Reg GetStackReg()
    {
        return Reg(Target::SP_REG, ArchTraits<ARCH>::IS_64_BITS ? INT64_TYPE : INT32_TYPE);
    }
    static constexpr Reg GetFrameReg()
    {
        return Reg(Target::FP_REG, ArchTraits<ARCH>::IS_64_BITS ? INT64_TYPE : INT32_TYPE);
    }
    static constexpr Reg GetLinkReg()
    {
        return Reg(Target::LR_REG, ArchTraits<ARCH>::IS_64_BITS ? INT64_TYPE : INT32_TYPE);
    }
    static constexpr Reg GetZeroReg()
    {
        return Reg(Target::ZERO_REG, ArchTraits<ARCH>::IS_64_BITS ? INT64_TYPE : INT32_TYPE);
    }
    static constexpr bool SupportLinkReg()
    {
        return Target::LR_REG != INVALID_REG_ID;
    }
    static constexpr bool SupportZeroReg()
    {
        return Target::ZERO_REG != INVALID_REG_ID;
    }
    static constexpr size_t GetParamRegsCount()
    {
        return Target::CALL_PARAMS_REGS.size();
    }
    static constexpr size_t GetParamRegId(size_t index)
    {
        ASSERT(index < GetParamRegsCount());
        return Target::CALL_PARAMS_REGS[index];
    }
    static constexpr size_t GetSpAlignment()
    {
        return Target::SP_ALIGNMENT;
    }
    static constexpr RegMask GetParamRegsMask()
    {
        return MakeMask(Target::CALL_PARAMS_REGS);
    }

    static constexpr RegMask GetTempRegsMask()
    {
        return RegMask(Target::TEMP_REGS_MASK);
    }
    static constexpr RegMask GetTempVRegsMask()
    {
        return Target::TEMP_VREGS_MASK;
    }
    static constexpr RegMask GetGeneralRegsMask()
    {
        return Target::GENERAL_REGS_MASK;
    }
    static constexpr RegMask GetGeneralVRegsMask()
    {
        return Target::GENERAL_REGS_MASK;
    }
    static constexpr RegMask GetAvailableRegsMask()
    {
        return GetGeneralRegsMask() & (~GetTempRegsMask());
    }
    static constexpr RegMask GetAvailableVRegsMask()
    {
        return RegMask(ArchTraits<ARCH>::CALLEE_FP_REG_MASK | ArchTraits<ARCH>::CALLER_FP_REG_MASK) &
               ~GetTempVRegsMask();
    }
};

class Target {
public:
    constexpr explicit Target(Arch arch) : arch_(arch) {}
    ~Target() = default;
    DEFAULT_MOVE_CTOR(Target);
    DEFAULT_COPY_CTOR(Target);
    NO_MOVE_OPERATOR(Target);
    NO_COPY_OPERATOR(Target);

    constexpr Arch GetArch() const
    {
        return arch_;
    }

    static constexpr Target Current()
    {
        return Target(RUNTIME_ARCH);
    }

#undef TARGET_DEFINE_GETTER
// CC-OFFNXT(G.PRE.06) solid logic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TARGET_DEFINE_GETTER(name)                                   \
    constexpr auto name() const                                      \
    {                                                                \
        switch (arch_) {                                             \
            case Arch::X86_64:                                       \
                /* CC-OFFNXT(G.PRE.05) function gen */               \
                return ArchCallingConvention<Arch::X86_64>::name();  \
            case Arch::X86:                                          \
                /* CC-OFFNXT(G.PRE.05) function gen */               \
                return ArchCallingConvention<Arch::X86>::name();     \
            case Arch::AARCH64:                                      \
                /* CC-OFFNXT(G.PRE.05) function gen */               \
                return ArchCallingConvention<Arch::AARCH64>::name(); \
            case Arch::AARCH32:                                      \
                /* CC-OFFNXT(G.PRE.05) function gen */               \
                return ArchCallingConvention<Arch::AARCH32>::name(); \
            default:                                                 \
                UNREACHABLE();                                       \
        }                                                            \
    }

    TARGET_DEFINE_GETTER(WordSize);
    TARGET_DEFINE_GETTER(GetReturnRegId);
    TARGET_DEFINE_GETTER(GetReturnFpRegId);
    TARGET_DEFINE_GETTER(GetStackReg);
    TARGET_DEFINE_GETTER(GetFrameReg);
    TARGET_DEFINE_GETTER(GetLinkReg);
    TARGET_DEFINE_GETTER(SupportLinkReg);
    TARGET_DEFINE_GETTER(GetZeroReg);
    TARGET_DEFINE_GETTER(SupportZeroReg);
    TARGET_DEFINE_GETTER(GetTempRegsMask);
    TARGET_DEFINE_GETTER(GetTempVRegsMask);
    TARGET_DEFINE_GETTER(GetGeneralRegsMask);
    TARGET_DEFINE_GETTER(GetAvailableRegsMask);
    TARGET_DEFINE_GETTER(GetAvailableVRegsMask);
    TARGET_DEFINE_GETTER(GetParamRegsCount);
    TARGET_DEFINE_GETTER(GetParamRegsMask);
    TARGET_DEFINE_GETTER(GetSpAlignment);

#undef TARGET_DEFINE_GETTER_1_ARG
// CC-OFFNXT(G.PRE.06) solid logic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TARGET_DEFINE_GETTER_1_ARG(name, arg_type)                      \
    constexpr auto name(arg_type arg) const                             \
    {                                                                   \
        switch (arch_) {                                                \
            case Arch::X86_64:                                          \
                /* CC-OFFNXT(G.PRE.05) function gen */                  \
                return ArchCallingConvention<Arch::X86_64>::name(arg);  \
            case Arch::X86:                                             \
                /* CC-OFFNXT(G.PRE.05) function gen */                  \
                return ArchCallingConvention<Arch::X86>::name(arg);     \
            case Arch::AARCH64:                                         \
                /* CC-OFFNXT(G.PRE.05) function gen */                  \
                return ArchCallingConvention<Arch::AARCH64>::name(arg); \
            case Arch::AARCH32:                                         \
                /* CC-OFFNXT(G.PRE.05) function gen */                  \
                return ArchCallingConvention<Arch::AARCH32>::name(arg); \
            default:                                                    \
                UNREACHABLE();                                          \
        }                                                               \
    }

    TARGET_DEFINE_GETTER_1_ARG(GetReturnReg, TypeInfo);
    TARGET_DEFINE_GETTER_1_ARG(GetParamRegId, size_t);

    constexpr size_t GetFpParamRegId(size_t index) const
    {
        switch (arch_) {
            case Arch::X86_64:
            case Arch::AARCH64:
            case Arch::AARCH32:
                return index;
            default:
                UNREACHABLE();
        }
    }

    constexpr Reg GetReturnReg() const
    {
        return GetReturnReg(GetPtrRegType());
    }

    constexpr Reg GetReturnFpReg() const
    {
        return GetReturnReg(Is64BitsArch(GetArch()) ? FLOAT64_TYPE : FLOAT32_TYPE);
    }

    constexpr TypeInfo GetPtrRegType() const
    {
        return Is64BitsArch(GetArch()) ? INT64_TYPE : INT32_TYPE;
    }

    constexpr Reg GetParamReg(size_t index, TypeInfo type) const
    {
        auto regId {type.IsScalar() ? GetParamRegId(index) : GetFpParamRegId(index)};
        return Reg(regId, type);
    }

    constexpr Reg GetParamReg(size_t index) const
    {
        return Reg(GetParamRegId(index), GetPtrRegType());
    }

    constexpr RegMask GetParamRegsMask(size_t limit) const
    {
        size_t mask = 0;
        for (size_t i = 0; i < limit; i++) {
            mask |= 1UL << GetParamRegId(i);
        }
        return RegMask(mask);
    }

    // NOTE(msherstennikov): Take into account register size
    std::string GetRegName(size_t reg, bool isFp) const
    {
        switch (arch_) {
            case Arch::X86_64:
                return ArchCallingConventionX8664::GetRegName(reg, isFp);
            case Arch::X86:
                return ArchCallingConventionX86::GetRegName(reg, isFp);
            case Arch::AARCH64:
                return ArchCallingConventionAarch64::GetRegName(reg, isFp);
            case Arch::AARCH32:
                return ArchCallingConventionAarch32::GetRegName(reg, isFp);
            case Arch::NONE:
                return "r" + std::to_string(reg);
            default:
                UNREACHABLE();
        }
    }

private:
    const Arch arch_;
};

// Check that all getters can be evaluated in compile time.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define VERIFY_GETTER(name)                                                    \
    static_assert(Target(Arch::X86_64).name() == Target(Arch::X86_64).name()); \
    static_assert(Target(Arch::AARCH64).name() == Target(Arch::AARCH64).name())
VERIFY_GETTER(WordSize);
VERIFY_GETTER(GetReturnRegId);
VERIFY_GETTER(GetReturnFpRegId);
VERIFY_GETTER(GetStackReg);
VERIFY_GETTER(GetFrameReg);
VERIFY_GETTER(GetLinkReg);
VERIFY_GETTER(SupportLinkReg);
VERIFY_GETTER(GetZeroReg);
VERIFY_GETTER(SupportZeroReg);
VERIFY_GETTER(GetTempRegsMask);
VERIFY_GETTER(GetTempVRegsMask);
VERIFY_GETTER(GetGeneralRegsMask);
VERIFY_GETTER(GetAvailableRegsMask);
VERIFY_GETTER(GetAvailableVRegsMask);
VERIFY_GETTER(GetParamRegsCount);
VERIFY_GETTER(GetParamRegsMask);
VERIFY_GETTER(GetSpAlignment);
static_assert(Target(Arch::X86_64).GetParamReg(0) == Target(Arch::X86_64).GetParamReg(0));
static_assert(Target(Arch::AARCH64).GetParamReg(0) == Target(Arch::AARCH64).GetParamReg(0));

}  // namespace ark::compiler

#endif  // PANDA_TARGET_H
