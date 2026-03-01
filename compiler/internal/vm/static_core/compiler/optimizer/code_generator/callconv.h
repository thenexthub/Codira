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

#ifndef COMPILER_OPTIMIZER_CODEGEN_CALLCONV_H
#define COMPILER_OPTIMIZER_CODEGEN_CALLCONV_H
/*
    Codegen Hi-Level calling-convention interface
    Also contains branches targets(labels)

    Responsible for
        Branches and jump-encoding
        Labels (points for jump)
        Conditional instructions
 */

#include "encode.h"
#include "compiler/optimizer/ir/datatype.h"
#include "compiler/optimizer/ir/locations.h"
#include "compiler/optimizer/code_generator/frame_info.h"
#include <functional>

namespace ark::compiler {
class ParameterInfo {
public:
    using SlotID = uint8_t;
    ParameterInfo() = default;
    virtual ~ParameterInfo() = default;
    // Get next native parameter, on condition, what previous list - in vector
    // Push data in Reg
    // Return register or stack_slot
    virtual std::variant<Reg, SlotID> GetNativeParam(const TypeInfo &) = 0;

    virtual Location GetNextLocation([[maybe_unused]] DataType::Type type) = 0;

    void Reset()
    {
        currentScalarNumber_ = 0;
        currentVectorNumber_ = 0;
        currentStackOffset_ = 0;
    }

    NO_COPY_SEMANTIC(ParameterInfo);
    NO_MOVE_SEMANTIC(ParameterInfo);

protected:
    uint32_t currentScalarNumber_ {0};  // NOLINT(misc-non-private-member-variables-in-classes)
    uint32_t currentVectorNumber_ {0};  // NOLINT(misc-non-private-member-variables-in-classes)
    uint8_t currentStackOffset_ {0};    // NOLINT(misc-non-private-member-variables-in-classes)
};

#ifdef PANDA_COMPILER_DEBUG_INFO
struct CfiOffsets {
    size_t pushFplr {0};
    size_t setFp {0};
    size_t pushCallees {0};
    size_t popCallees {0};
    size_t popFplr {0};
};

struct CfiInfo {
    CfiOffsets offsets;
    RegMask calleeRegs;
    VRegMask calleeVregs;
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_CFI_OFFSET(field, value) GetCfiInfo().offsets.field = value
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_CFI_CALLEE_REGS(value) GetCfiInfo().calleeRegs = value
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_CFI_CALLEE_VREGS(value) GetCfiInfo().calleeVregs = value
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_CFI_OFFSET(field, value)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_CFI_CALLEE_REGS(value)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_CFI_CALLEE_VREGS(value)
#endif

/// Specifies CallingConvention mode.
class CallConvMode final {
public:
    explicit CallConvMode(uint32_t value) : value_(value) {}

    DEFAULT_COPY_SEMANTIC(CallConvMode);
    DEFAULT_MOVE_SEMANTIC(CallConvMode);

    ~CallConvMode() = default;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CALLCONV_MODE_MODIFIERS(name)  \
    void Set##name(bool v)                     \
    {                                          \
        Flag##name ::Set(v, &value_);          \
    }                                          \
    bool Is##name() const                      \
    {                                          \
        /* CC-OFFNXT(G.PRE.05) function gen */ \
        return Flag##name ::Get(value_);       \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CALLCONV_MODE(name)                    \
    static CallConvMode name(bool set = true)          \
    {                                                  \
        /* CC-OFFNXT(G.PRE.05) function gen */         \
        return CallConvMode(Flag##name ::Encode(set)); \
    }                                                  \
    DECLARE_CALLCONV_MODE_MODIFIERS(name)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CALLCONV_MODE_MODIFIERS(name)  \
    void Set##name(bool v)                     \
    {                                          \
        Flag##name ::Set(v, &value_);          \
    }                                          \
    bool Is##name() const                      \
    {                                          \
        /* CC-OFFNXT(G.PRE.05) function gen */ \
        return Flag##name ::Get(value_);       \
    }

    // Panda ABI convention (native - otherwise)
    DECLARE_CALLCONV_MODE(Panda);
    // Compile for osr (jit - otherwise)
    DECLARE_CALLCONV_MODE(Osr);
    // The method from dynamic language
    DECLARE_CALLCONV_MODE(DynCall);
    // Use optimized regset for Irtoc code
    DECLARE_CALLCONV_MODE(OptIrtoc);

#undef DECLARE_CALLCONV_MODE
#undef DECLARE_CALLCONV_MODIFIERS

private:
    using FlagPanda = BitField<bool, 0, 1>;
    using FlagOsr = FlagPanda::NextFlag;
    using FlagDynCall = FlagOsr::NextFlag;
    using FlagOptIrtoc = FlagDynCall::NextFlag;

    uint32_t value_ {0};

    friend CallConvMode operator|(CallConvMode a, CallConvMode b);
};

inline CallConvMode operator|(CallConvMode a, CallConvMode b)
{
    return CallConvMode(a.value_ | b.value_);
}

/// Holds specific information about dynamic call mode
class CallConvDynInfo {
public:
    // Fixed parameter regs
    enum : uint8_t {
        REG_METHOD = 0,
        REG_NUM_ARGS,
        REG_COUNT,
    };

    // Call frame slots
    enum : uint8_t {
        FIXED_SLOT_COUNT = 0,
        SLOT_CALLEE = FIXED_SLOT_COUNT,
    };

    explicit CallConvDynInfo() = default;

    explicit CallConvDynInfo(uint32_t numExpectedArgs, uintptr_t expandEntrypointsTableTlsOffset,
                             uintptr_t expandEntrypointOffset)
        : expandEntrypointsTableTlsOffset_(expandEntrypointsTableTlsOffset),
          expandEntrypointOffset_(expandEntrypointOffset),
          numExpectedArgs_(numExpectedArgs),
          checkRequired_(true)
    {
    }

    auto GetExpandEntrypointsTableTlsOffset()
    {
        return expandEntrypointsTableTlsOffset_;
    }

    auto GetExpandEntrypointOffset()
    {
        return expandEntrypointOffset_;
    }

    auto GetNumExpectedArgs()
    {
        return numExpectedArgs_;
    }

    auto IsCheckRequired()
    {
        return checkRequired_;
    }

private:
    uintptr_t expandEntrypointsTableTlsOffset_ {0};
    uintptr_t expandEntrypointOffset_ {0};
    uint32_t numExpectedArgs_ {0};
    bool checkRequired_ {false};
};

/// CallConv - just holds information about calling convention in current architecture.
class CallingConvention {
public:
    virtual ~CallingConvention() = default;

    // All possible reasons for call and return
    enum Reason {
        // Reason for save/restore registers
        FUNCTION,  // Function inside programm
        NATIVE,    // native function
        PROGRAMM   // Enter/exit from programm (UNSUPPORTED)
    };

    // Implemented in target.cpp
    static CallingConvention *Create(ArenaAllocator *arenaAllocator, Encoder *enc, RegistersDescription *descr,
                                     Arch arch, bool isPandaAbi = false, bool isOsr = false, bool isDyn = false,
                                     bool printAsm = false, bool isOptIrtoc = false);

public:
    CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr, CallConvMode mode)
        : allocator_(allocator), encoder_(enc), regfile_(descr), mode_(mode)
    {
    }

    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }

    Encoder *GetEncoder() const
    {
        return encoder_;
    }

    void SetEncoder(Encoder *enc)
    {
        encoder_ = enc;
    }

    RegistersDescription *GetRegfile() const
    {
        return regfile_;
    }

    virtual bool IsValid() const
    {
        return false;
    }

    void SetDynInfo(CallConvDynInfo dynInfo)
    {
        dynInfo_ = dynInfo;
    }

    CallConvDynInfo &GetDynInfo()
    {
        return dynInfo_;
    }

    CallConvMode GetMode() const
    {
        return mode_;
    }

    bool IsPandaMode() const
    {
        return mode_.IsPanda();
    }

    bool IsOsrMode() const
    {
        return mode_.IsOsr();
    }

    bool IsDynCallMode() const
    {
        return mode_.IsDynCall();
    }

#ifdef PANDA_COMPILER_DEBUG_INFO
    CfiInfo &GetCfiInfo()
    {
        return cfiInfo_;
    }

    const CfiInfo &GetCfiInfo() const
    {
        return cfiInfo_;
    }
    static constexpr bool ProvideCFI()
    {
        return true;
    }
#else
    static constexpr bool ProvideCFI()
    {
        return false;
    }
#endif

    // Prologue/Epilogue interfaces
    virtual void GeneratePrologue(const FrameInfo &frameInfo) = 0;
    virtual void GenerateEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) = 0;

    virtual void GenerateNativePrologue(const FrameInfo &frameInfo) = 0;
    virtual void GenerateNativeEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) = 0;
    virtual void GenerateEpilogueHead(const FrameInfo &frameInfo, std::function<void()> postJob) = 0;

    // Code generation completion interfaces
    virtual void *GetCodeEntry() = 0;
    virtual uint32_t GetCodeSize() = 0;

    // Calculating information about parameters and save regs_offset registers for special needs
    virtual ParameterInfo *GetParameterInfo(uint8_t regsOffset) = 0;

    NO_COPY_SEMANTIC(CallingConvention);
    NO_MOVE_SEMANTIC(CallingConvention);

private:
    // Must not use ExecModel!
    ArenaAllocator *allocator_ {nullptr};
    Encoder *encoder_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
#ifdef PANDA_COMPILER_DEBUG_INFO
    CfiInfo cfiInfo_;
#endif
    CallConvDynInfo dynInfo_ {};
    CallConvMode mode_ {0};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_CALLCONV_H
