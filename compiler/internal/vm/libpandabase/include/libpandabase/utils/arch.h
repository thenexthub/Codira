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

#ifndef LIBPANDABASE_UTILS_ARCH_H
#define LIBPANDABASE_UTILS_ARCH_H

#include "macros.h"
#include "utils/math_helpers.h"
#include "utils/regmask.h"
#include "concepts.h"

namespace panda {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARCH_LIST(D) \
    D(NONE)          \
    D(AARCH32)       \
    D(AARCH64)       \
    D(X86)           \
    D(X86_64)

enum class Arch {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF(v) v,
    ARCH_LIST(DEF)
#undef DEF
};

template <Arch arch>
struct ArchTraits;

template <>
struct ArchTraits<Arch::AARCH32> {
    static constexpr size_t CODE_ALIGNMENT = 8;
    static constexpr size_t INSTRUCTION_ALIGNMENT = 4;
    static constexpr size_t INSTRUCTION_MAX_SIZE_BITS = 32;
    static constexpr size_t POINTER_SIZE = 4;
    static constexpr bool IS_64_BITS = false;
    static constexpr size_t THREAD_REG = 10;
    static constexpr size_t CALLER_REG_MASK = 0x0000000f;
    static constexpr size_t CALLER_FP_REG_MASK = 0x0000ffff;  // s0-s15 or d0-d7
    static constexpr size_t CALLEE_REG_MASK = 0x000007f0;
    static constexpr size_t CALLEE_FP_REG_MASK = 0xffff0000;  // s16-s31 or d8-d15
    static constexpr bool SUPPORT_OSR = false;
    static constexpr bool SUPPORT_DEOPTIMIZATION = true;
    static constexpr const char *ISA_NAME = "arm";
    static constexpr size_t DWARF_SP = 13;
    static constexpr size_t DWARF_RIP = 15;
    static constexpr size_t DWARF_FP = 11;
    static constexpr size_t DWARF_LR = 14;
    using WordType = uint32_t;
};

template <>
struct ArchTraits<Arch::AARCH64> {
    static constexpr size_t CODE_ALIGNMENT = 16;
    static constexpr size_t INSTRUCTION_ALIGNMENT = 4;
    static constexpr size_t INSTRUCTION_MAX_SIZE_BITS = 32;
    static constexpr size_t POINTER_SIZE = 8;
    static constexpr bool IS_64_BITS = true;
    static constexpr size_t THREAD_REG = 28;
    static constexpr size_t CALLER_REG_MASK = 0x0007ffff;
    static constexpr size_t CALLER_FP_REG_MASK = 0xffff00ff;
    static constexpr size_t CALLEE_REG_MASK = 0x1ff80000;
    static constexpr size_t CALLEE_FP_REG_MASK = 0x0000ff00;
    static constexpr bool SUPPORT_OSR = true;
    static constexpr bool SUPPORT_DEOPTIMIZATION = true;
    static constexpr const char *ISA_NAME = "arm64";
    static constexpr size_t DWARF_SP = 31;
    static constexpr size_t DWARF_RIP = 32;
    static constexpr size_t DWARF_FP = 29;
    static constexpr size_t DWARF_LR = 30;
    using WordType = uint64_t;
};

template <>
struct ArchTraits<Arch::X86> {
    static constexpr size_t CODE_ALIGNMENT = 16;
    static constexpr size_t INSTRUCTION_ALIGNMENT = 1;
    static constexpr size_t INSTRUCTION_MAX_SIZE_BITS = 8;
    static constexpr size_t POINTER_SIZE = 4;
    static constexpr bool IS_64_BITS = false;
    static constexpr size_t THREAD_REG = 0;
    static constexpr size_t CALLER_REG_MASK = 0x00000000;
    static constexpr size_t CALLER_FP_REG_MASK = 0x00000000;
    static constexpr size_t CALLEE_REG_MASK = 0x00000001;
    static constexpr size_t CALLEE_FP_REG_MASK =
        0x00000001;  // TODO(msherstennikov): fill once x86 codegen is supported
    static constexpr bool SUPPORT_OSR = false;
    static constexpr bool SUPPORT_DEOPTIMIZATION = false;
    static constexpr const char *ISA_NAME = "x86";
    static constexpr size_t DWARF_SP = 0;
    static constexpr size_t DWARF_RIP = 0;
    static constexpr size_t DWARF_FP = 0;
    static constexpr size_t DWARF_LR = 0;
    using WordType = uint32_t;
};

template <>
struct ArchTraits<Arch::X86_64> {
    static constexpr size_t CODE_ALIGNMENT = 16;
    static constexpr size_t INSTRUCTION_ALIGNMENT = 1;
    static constexpr size_t INSTRUCTION_MAX_SIZE_BITS = 8;
    static constexpr size_t POINTER_SIZE = 8;
    static constexpr bool IS_64_BITS = true;
    static constexpr size_t THREAD_REG = 15;               // %r15
    static constexpr size_t CALLER_REG_MASK = 0x000001FF;  // %rax, %rcx, %rdx, %rsi, %rdi, %r8, %r9, %r10, %r11
    static constexpr size_t CALLER_FP_REG_MASK = 0x0000FFFF;
    static constexpr size_t CALLEE_REG_MASK = 0x0000F800;  // %rbx, %r12, %r13, %r14, %r15
    static constexpr size_t CALLEE_FP_REG_MASK = 0x00000000;
    static constexpr bool SUPPORT_OSR = false;
    static constexpr bool SUPPORT_DEOPTIMIZATION = true;
    static constexpr const char *ISA_NAME = "x86_64";
    static constexpr size_t DWARF_SP = 7;
    static constexpr size_t DWARF_RIP = 16;
    static constexpr size_t DWARF_FP = 6;
    static constexpr size_t DWARF_LR = 0;
    using WordType = uint64_t;
};

template <>
struct ArchTraits<Arch::NONE> {
    static constexpr size_t CODE_ALIGNMENT = 0;
    static constexpr size_t INSTRUCTION_ALIGNMENT = 0;
    static constexpr size_t INSTRUCTION_MAX_SIZE_BITS = 1;
    static constexpr size_t POINTER_SIZE = 0;
    static constexpr bool IS_64_BITS = false;
    static constexpr size_t CALLEE_REG_MASK = 0x00000000;
    static constexpr size_t CALLEE_FP_REG_MASK = 0x00000000;
    static constexpr const char *ISA_NAME = "";
    static constexpr size_t DWARF_SP = 0;
    static constexpr size_t DWARF_RIP = 0;
    static constexpr size_t DWARF_FP = 0;
    static constexpr size_t DWARF_LR = 0;
    using WordType = void;
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage,-warnings-as-errors)
#define DEF_ARCH_PROPERTY_GETTER(func_name, property)                                                 \
    constexpr std::remove_const_t<decltype(ArchTraits<Arch::AARCH64>::property)> func_name(Arch arch) \
    {                                                                                                 \
        ASSERT(arch != Arch::NONE);                                                                   \
        if (arch == Arch::X86) {                                                                      \
            return ArchTraits<Arch::X86>::property;                                                   \
        }                                                                                             \
        if (arch == Arch::X86_64) {                                                                   \
            return ArchTraits<Arch::X86_64>::property;                                                \
        }                                                                                             \
        if (arch == Arch::AARCH32) {                                                                  \
            return ArchTraits<Arch::AARCH32>::property;                                               \
        }                                                                                             \
        if (arch == Arch::AARCH64) {                                                                  \
            return ArchTraits<Arch::AARCH64>::property;                                               \
        }                                                                                             \
        UNREACHABLE();                                                                                \
    }

DEF_ARCH_PROPERTY_GETTER(DoesArchSupportDeoptimization, SUPPORT_DEOPTIMIZATION)
DEF_ARCH_PROPERTY_GETTER(GetCodeAlignment, CODE_ALIGNMENT)
DEF_ARCH_PROPERTY_GETTER(GetInstructionAlignment, INSTRUCTION_ALIGNMENT)
DEF_ARCH_PROPERTY_GETTER(GetInstructionSizeBits, INSTRUCTION_MAX_SIZE_BITS)
DEF_ARCH_PROPERTY_GETTER(Is64BitsArch, IS_64_BITS)
DEF_ARCH_PROPERTY_GETTER(PointerSize, POINTER_SIZE)
DEF_ARCH_PROPERTY_GETTER(GetThreadReg, THREAD_REG)
// constant is needed for correct call libdwarf-library
DEF_ARCH_PROPERTY_GETTER(GetIsaName, ISA_NAME)
DEF_ARCH_PROPERTY_GETTER(GetDwarfSP, DWARF_SP)
DEF_ARCH_PROPERTY_GETTER(GetDwarfRIP, DWARF_RIP)
DEF_ARCH_PROPERTY_GETTER(GetDwarfFP, DWARF_FP)
DEF_ARCH_PROPERTY_GETTER(GetDwarfLR, DWARF_LR)

constexpr const char *GetArchString(Arch arch)
{
    switch (arch) {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF(v)    \
    case Arch::v: \
        return #v;
        ARCH_LIST(DEF)
#undef DEF
        default:
            UNREACHABLE();
    }
}

inline constexpr RegMask GetCallerRegsMask(Arch arch, bool is_fp)
{
    switch (arch) {
        case Arch::AARCH32:
            return is_fp ? ArchTraits<Arch::AARCH32>::CALLER_FP_REG_MASK : ArchTraits<Arch::AARCH32>::CALLER_REG_MASK;
        case Arch::AARCH64:
            return is_fp ? ArchTraits<Arch::AARCH64>::CALLER_FP_REG_MASK : ArchTraits<Arch::AARCH64>::CALLER_REG_MASK;
        case Arch::X86:
            return is_fp ? ArchTraits<Arch::X86>::CALLER_FP_REG_MASK : ArchTraits<Arch::X86>::CALLER_REG_MASK;
        case Arch::X86_64:
            return is_fp ? ArchTraits<Arch::X86_64>::CALLER_FP_REG_MASK : ArchTraits<Arch::X86_64>::CALLER_REG_MASK;
        default:
            UNREACHABLE();
    }
}

inline constexpr RegMask GetCalleeRegsMask(Arch arch, bool is_fp)
{
    switch (arch) {
        case Arch::AARCH32:
            return is_fp ? ArchTraits<Arch::AARCH32>::CALLEE_FP_REG_MASK : ArchTraits<Arch::AARCH32>::CALLEE_REG_MASK;
        case Arch::AARCH64:
            return is_fp ? ArchTraits<Arch::AARCH64>::CALLEE_FP_REG_MASK : ArchTraits<Arch::AARCH64>::CALLEE_REG_MASK;
        case Arch::X86:
            return is_fp ? ArchTraits<Arch::X86>::CALLEE_FP_REG_MASK : ArchTraits<Arch::X86>::CALLEE_REG_MASK;
        case Arch::X86_64:
            return is_fp ? ArchTraits<Arch::X86_64>::CALLEE_FP_REG_MASK : ArchTraits<Arch::X86_64>::CALLEE_REG_MASK;
        default:
            UNREACHABLE();
    }
}

inline constexpr size_t GetFirstCalleeReg(Arch arch, bool is_fp)
{
    if (arch == Arch::X86_64 && is_fp) {
        // in amd64 xmm regs are volatile, so we return first reg (1) > last reg(0) to imitate empty list;
        // also number of registers = last reg (0) - first reg (1) + 1 == 0
        return 1;
    }

    return GetCalleeRegsMask(arch, is_fp).GetMinRegister();
}

inline constexpr size_t GetLastCalleeReg(Arch arch, bool is_fp)
{
    if (arch == Arch::X86_64 && is_fp) {
        return 0;
    }

    return GetCalleeRegsMask(arch, is_fp).GetMaxRegister();
}

inline constexpr size_t GetCalleeRegsCount(Arch arch, bool is_fp)
{
    return GetCalleeRegsMask(arch, is_fp).Count();
}

inline constexpr size_t GetFirstCallerReg(Arch arch, bool is_fp)
{
    return GetCallerRegsMask(arch, is_fp).GetMinRegister();
}

inline constexpr size_t GetLastCallerReg(Arch arch, bool is_fp)
{
    return GetCallerRegsMask(arch, is_fp).GetMaxRegister();
}

inline constexpr size_t GetCallerRegsCount(Arch arch, bool is_fp)
{
    return GetCallerRegsMask(arch, is_fp).Count();
}

inline constexpr size_t GetRegsCount(Arch arch)
{
    return GetCalleeRegsCount(arch, false) + GetCalleeRegsCount(arch, true) + GetCallerRegsCount(arch, false) +
           GetCallerRegsCount(arch, true);
}

#ifdef PANDA_TARGET_ARM32
static constexpr Arch RUNTIME_ARCH = Arch::AARCH32;
#elif defined(PANDA_TARGET_ARM64)
static constexpr Arch RUNTIME_ARCH = Arch::AARCH64;
#elif defined(PANDA_TARGET_X86)
static constexpr Arch RUNTIME_ARCH = Arch::X86;
#elif defined(PANDA_TARGET_AMD64)
static constexpr Arch RUNTIME_ARCH = Arch::X86_64;
#else
static constexpr Arch RUNTIME_ARCH = Arch::NONE;
#endif

template <class String = std::string>
std::enable_if_t<is_stringable_v<String>, Arch> GetArchFromString(const String &str)
{
    // TODO(msherstennikov): implement using macro if "aarch64", "aarch32" and so on would be a proper choice
    if (str == "arm64") {
        return Arch::AARCH64;
    }
    if (str == "arm" || str == "arm32") {
        return Arch::AARCH32;
    }
    if (str == "x86") {
        return Arch::X86;
    }
    if (str == "x86_64" || str == "x64") {
        return Arch::X86_64;
    }
    return Arch::NONE;
}

template <class String = std::string>
std::enable_if_t<is_stringable_v<String>, String> GetStringFromArch(const Arch &arch)
{
    if (arch == Arch::AARCH64) {
        return "arm64";
    }
    if (arch == Arch::AARCH32) {
        return "arm";
    }
    if (arch == Arch::X86) {
        return "x86";
    }
    if (arch == Arch::X86_64) {
        return "x86_64";
    }
    return "none";
}

}  // namespace panda

#endif  // LIBPANDABASE_UTILS_ARCH_H
