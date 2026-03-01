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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_INS_H
#define PANDA_ASSEMBLER_ASSEMBLY_INS_H

#include <unordered_map>
#include "assembly-debug.h"
#include "libarkfile/bytecode_emitter.h"
#include "libarkfile/file_items.h"
#include "isa.h"

namespace ark::pandasm {

enum class Opcode : std::uint16_t {
// CC-OFFNXT(G.PRE.02) opcode is class member
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) opcode,
    PANDA_INSTRUCTION_LIST(OPLIST)
#undef OPLIST
        INVALID,
    NUM_OPCODES = INVALID
};

enum InstFlags {
    NONE = 0,
    JUMP = (1U << 0U),
    COND = (1U << 1U),
    CALL = (1U << 2U),
    RETURN = (1U << 3U),
    ACC_READ = (1U << 4U),
    ACC_WRITE = (1U << 5U),
    PSEUDO = (1U << 6U),
    THROWING = (1U << 7U),
    METHOD_ID = (1U << 8U),
    FIELD_ID = (1U << 9U),
    TYPE_ID = (1U << 10U),
    STRING_ID = (1U << 11U),
    LITERALARRAY_ID = (1U << 12U),
    CALL_RANGE = (1U << 13U),
    STATIC_FIELD_ID = (1U << 14U),
    STATIC_METHOD_ID = (1U << 15U),
    ANY_CALL = (1U << 16U)
};

constexpr int INVALID_REG_IDX = -1;

constexpr InstFlags operator|(InstFlags a, InstFlags b)
{
    using Utype = std::underlying_type_t<InstFlags>;
    return static_cast<InstFlags>(static_cast<Utype>(a) | static_cast<Utype>(b));
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (flags),
constexpr std::array<unsigned, static_cast<size_t>(Opcode::NUM_OPCODES)> INST_FLAGS_TABLE = {
    PANDA_INSTRUCTION_LIST(OPLIST)};
#undef OPLIST

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (width),
constexpr std::array<size_t, static_cast<size_t>(Opcode::NUM_OPCODES)> INST_WIDTH_TABLE = {
    PANDA_INSTRUCTION_LIST(OPLIST)};
#undef OPLIST

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (def_idx),
constexpr std::array<int, static_cast<size_t>(Opcode::NUM_OPCODES)> DEF_IDX_TABLE = {PANDA_INSTRUCTION_LIST(OPLIST)};
#undef OPLIST

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (use_idxs),
constexpr std::array<std::array<int, MAX_NUMBER_OF_SRC_REGS>, static_cast<size_t>(Opcode::NUM_OPCODES)> USE_IDXS_TABLE =
    {PANDA_INSTRUCTION_LIST(OPLIST)};  // CC-OFF(G.FMT.03) any style changes will worsen readability
#undef OPLIST

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (prof_size),
// clang-format off
constexpr std::array<unsigned, static_cast<size_t>(Opcode::NUM_OPCODES) + 1> INST_PROFILE_SIZES =
    {PANDA_INSTRUCTION_LIST(OPLIST) 0};  // CC-OFF(G.FMT.03) any style changes will worsen readability
// clang-format on
#undef OPLIST

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class Ins {
public:
    constexpr static uint16_t ACCUMULATOR = -1;
    constexpr static size_t MAX_CALL_SHORT_ARGS = 2;
    constexpr static size_t MAX_CALL_ARGS = 4;
    constexpr static uint16_t MAX_NON_RANGE_CALL_REG = 15;
    constexpr static uint16_t MAX_RANGE_CALL_START_REG = 255;

    using IType = std::variant<int64_t, double>;
#if defined(NOT_OPTIMIZE_PERFORMANCE)
    using ImmArray = std::vector<ark::pandasm::Ins::IType>;
    using IDArray = std::vector<std::string>;
    using RegArray = std::vector<std::uint16_t>;
#else
    using ImmArray = std::array<ark::pandasm::Ins::IType, MAX_NUMBER_OF_IMMS>;
    using IDArray = std::array<std::unique_ptr<std::string>, MAX_NUMBER_OF_IDS>;
    using RegArray = std::array<std::uint16_t, MAX_NUMBER_OF_REGS>;
#endif

    Ins() = default;
    ~Ins() = default;
#if defined(NOT_OPTIMIZE_PERFORMANCE)
    DEFAULT_COPY_SEMANTIC(Ins);
#else
    NO_COPY_SEMANTIC(Ins);
#endif
    DEFAULT_MOVE_SEMANTIC(Ins);

    debuginfo::Ins insDebug {};

#if !defined(NOT_OPTIMIZE_PERFORMANCE)
private:
#endif
    // NOLINTBEGIN(readability-identifier-naming)
    ImmArray imms {}; /* list of arguments - immediates */
    IDArray ids {};   /* list of arguments - identifiers */
    RegArray regs {}; /* list of arguments - registers */
    // NOLINTEND(readability-identifier-naming)

#if defined(NOT_OPTIMIZE_PERFORMANCE)
public:
    std::string label;     /* label at the beginning of a line */
    bool setLabel = false; /* whether this label is defined */
#else
private:
    std::unique_ptr<std::string> label_ = nullptr; /* label at the beginning of a line */
#endif

public:
    Opcode opcode = Opcode::INVALID; /* operation type */
    uint16_t profileId {0};          /* Index in the profile vector */

private:
#if !defined(NOT_OPTIMIZE_PERFORMANCE)
    std::uint8_t idSize_ = 0U;
    std::uint8_t immSize_ = 0U;
    std::uint8_t regSize_ = 0U;
#endif

public:
    PANDA_PUBLIC_API std::string ToString(const std::string &endline = "", bool printArgs = false,
                                          size_t firstArgIdx = 0) const;

    [[nodiscard]] std::size_t RegSize() const noexcept
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        return regs.size();
#else
        return static_cast<std::size_t>(regSize_);
#endif
    }

    [[nodiscard]] bool RegEmpty() const noexcept
    {
        return RegSize() == 0U;
    }

    [[nodiscard]] std::uint16_t GetReg(std::size_t pos) const
    {
        ASSERT(pos < RegSize());
        return regs[pos];
    }

    void SetReg(std::size_t const pos, std::uint16_t const value)
    {
        ASSERT(pos < RegSize());
        regs[pos] = value;
    }

    void EmplaceReg(std::uint16_t value)
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        regs.emplace_back(value);
#else
        ASSERT(RegSize() < MAX_NUMBER_OF_REGS);
        regs[static_cast<std::size_t>(regSize_++)] = value;
#endif
    }

    void PopRegBack()
    {
        ASSERT(RegSize() > 0U);
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        regs.pop_back();
#else
        --regSize_;
#endif
    }

    void PopRegFront()
    {
        ASSERT(RegSize() > 0U);
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        regs.erase(regs.begin());
#else
        --regSize_;
        for (std::size_t i = 0U; i < regSize_; ++i) {
            regs[i] = regs[i + 1];
        }
#endif
    }

    void ClearReg() noexcept
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        regs.clear();
#else
        regSize_ = 0U;
#endif
    }

    [[nodiscard]] std::size_t IDSize() const noexcept
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        return ids.size();
#else
        return static_cast<std::size_t>(idSize_);
#endif
    }

    [[nodiscard]] bool IDEmpty() const noexcept
    {
        return IDSize() == 0U;
    }

    [[nodiscard]] std::string const &GetID(std::size_t pos) const
    {
        ASSERT(pos < IDSize());
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        return ids[pos];
#else
        return *ids[pos];
#endif
    }

    [[nodiscard]] std::string &BackID()
    {
        [[maybe_unused]] auto pos = IDSize();
        ASSERT(pos > 0U);
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        return ids.back();
#else
        return *ids[--pos];
#endif
    }

    template <typename... Args>
    void SetID(std::size_t pos, Args &&...args)
    {
        static_assert(std::is_constructible_v<std::string, Args...>, "Invalid string initialization value.");
        //  Only "existing" data can set again.
        ASSERT(pos < IDSize());
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        ids[pos] = std::string {std::forward<Args>(args)...};
#else
        ids[pos] = std::make_unique<std::string>(std::forward<Args>(args)...);
#endif
    }

    template <typename... Args>
    void EmplaceID(Args &&...args)
    {
        static_assert(std::is_constructible_v<std::string, Args...>, "Invalid string initialization value.");
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        ids.emplace_back(std::forward<Args>(args)...);
#else
        ASSERT(IDSize() < MAX_NUMBER_OF_IDS);
        ids[static_cast<std::size_t>(idSize_++)] = std::make_unique<std::string>(std::forward<Args>(args)...);
#endif
    }

    [[nodiscard]] std::size_t ImmSize() const noexcept
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        return imms.size();
#else
        return static_cast<std::size_t>(immSize_);
#endif
    }

    [[nodiscard]] IType GetImm(std::size_t pos) const
    {
        ASSERT(pos < ImmSize());
        return imms[pos];
    }

    void SetImm(std::size_t pos, IType value)
    {
        ASSERT(pos < ImmSize());
        imms[pos] = value;
    }

    void ClearImm() noexcept
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        imms.clear();
#else
        immSize_ = 0U;
#endif
    }

    template <typename T>
    [[nodiscard]] T GetImm(std::size_t pos) const
    {
        ASSERT(pos < ImmSize() && std::holds_alternative<T>(imms[pos]));
        return std::get<T>(imms[pos]);
    }

    template <typename T>
    void EmplaceImm(T value)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<std::decay_t<T>, IType>, "Invalid data type.");
        ASSERT(ImmSize() < MAX_NUMBER_OF_IMMS);
        if constexpr (std::is_same_v<std::decay_t<T>, IType>) {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
            imms.emplace_back(value);
#else
            imms[static_cast<std::size_t>(immSize_++)] = value;
#endif
        } else if constexpr (std::is_integral_v<T>) {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
            imms.emplace_back(static_cast<int64_t>(value));
#else
            imms[static_cast<std::size_t>(immSize_++)] = static_cast<int64_t>(value);
#endif
        } else {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
            imms.emplace_back(static_cast<double>(value));
#else
            imms[static_cast<std::size_t>(immSize_++)] = static_cast<double>(value);
#endif
        }
    }

    [[nodiscard]] bool HasLabel() const noexcept
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        return setLabel;
#else
        return label_ != nullptr;
#endif
    }

    [[nodiscard]] std::string const &Label() const
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        return label;
#else
        ASSERT(label_ != nullptr);
        return *label_;
#endif
    }

    template <typename T>
    void SetLabel(T &&data)
    {
        static_assert(std::is_constructible_v<std::string, T>, "Invalid string initialization value.");
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        setLabel = true;
        label = std::string(std::forward<T>(data));
#else
        ASSERT(label_ == nullptr);
        label_ = std::make_unique<std::string>(std::forward<T>(data));
#endif
    }

    void RemoveLabel() noexcept
    {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        setLabel = false;
        label.clear();
#else
        if (label_) {
            label_.reset();
        }
#endif
    }

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    bool Emit(BytecodeEmitter &emitter, panda_file::MethodItem *method,
              const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods,
              const std::unordered_map<std::string, panda_file::BaseMethodItem *> &staticMethods,
              const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields,
              const std::unordered_map<std::string, panda_file::BaseFieldItem *> &staticFields,
              const std::unordered_map<std::string, panda_file::BaseClassItem *> &classes,
              const std::unordered_map<std::string_view, panda_file::StringItem *> &strings,
              const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays,
              const std::unordered_map<std::string_view, ark::Label> &labels) const;

    size_t OperandListLength() const
    {
        return RegSize() + IDSize() + ImmSize();
    }

    bool HasFlag(InstFlags flag) const
    {
        if (opcode == Opcode::INVALID) {  // NOTE(mbolshov): introduce 'label' opcode for labels
            return false;
        }
        return (INST_FLAGS_TABLE[static_cast<size_t>(opcode)] & flag) != 0;
    }

    bool CanThrow() const
    {
        return HasFlag(InstFlags::THROWING) || HasFlag(InstFlags::METHOD_ID) || HasFlag(InstFlags::STATIC_METHOD_ID) ||
               HasFlag(InstFlags::FIELD_ID) || HasFlag(InstFlags::STATIC_FIELD_ID) || HasFlag(InstFlags::TYPE_ID) ||
               HasFlag(InstFlags::STRING_ID);
    }

    bool IsJump() const
    {
        return HasFlag(InstFlags::JUMP);
    }

    bool IsConditionalJump() const
    {
        return IsJump() && HasFlag(InstFlags::COND);
    }

    bool IsCall() const
    {  // Non-range call
        return HasFlag(InstFlags::CALL);
    }

    bool IsCallRange() const
    {  // Range call
        return HasFlag(InstFlags::CALL_RANGE);
    }

    bool IsPseudoCall() const
    {
        return HasFlag(InstFlags::PSEUDO) && HasFlag(InstFlags::CALL);
    }

    bool IsAnyCall() const
    {
        return HasFlag(InstFlags::ANY_CALL);
    }

    bool IsReturn() const
    {
        return HasFlag(InstFlags::RETURN);
    }

    size_t MaxRegEncodingWidth() const
    {
        if (opcode == Opcode::INVALID) {
            return 0;
        }
        return INST_WIDTH_TABLE[static_cast<size_t>(opcode)];
    }

    // Use this method if an identical copy of object is required.
    ark::pandasm::Ins Clone() const
    {
        ark::pandasm::Ins clone {};
        clone.insDebug = insDebug.Clone();

        for (std::size_t i = 0U; i < ImmSize(); ++i) {
            clone.EmplaceImm(GetImm(i));
        }

        for (std::size_t i = 0U; i < IDSize(); ++i) {
            clone.EmplaceID(GetID(i));
        }

        for (std::size_t i = 0U; i < RegSize(); ++i) {
            clone.EmplaceReg(GetReg(i));
        }

        if (HasLabel()) {
            clone.SetLabel(Label());
        }

        clone.opcode = opcode;
        clone.profileId = profileId;

#if !defined(NOT_OPTIMIZE_PERFORMANCE)
        clone.idSize_ = idSize_;
        clone.immSize_ = immSize_;
        clone.regSize_ = regSize_;
#endif

        return clone;
    }

    std::vector<uint16_t> Uses() const
    {
        if (IsPseudoCall()) {
            std::vector<uint16_t> res(RegSize());
            for (std::size_t i = 0U; i < RegSize(); ++i) {
                res.emplace_back(GetReg(i));
            }
            return res;
        }

        if (opcode == Opcode::INVALID) {
            return {};
        }

        auto useIdxs = USE_IDXS_TABLE[static_cast<size_t>(opcode)];
        std::vector<uint16_t> res(MAX_NUMBER_OF_SRC_REGS + 1);
        if (HasFlag(InstFlags::ACC_READ)) {
            res.push_back(Ins::ACCUMULATOR);
        }
        for (auto idx : useIdxs) {
            if (idx == INVALID_REG_IDX) {
                break;
            }
            ASSERT(static_cast<size_t>(idx) < RegSize());
            res.emplace_back(GetReg(idx));
        }
        return res;
    }

    std::optional<size_t> Def() const
    {
        if (opcode != Opcode::INVALID) {
            auto defIdx = DEF_IDX_TABLE[static_cast<size_t>(opcode)];
            if (defIdx != INVALID_REG_IDX) {
                return std::make_optional(static_cast<size_t>(GetReg(static_cast<size_t>(defIdx))));
            }
            if (HasFlag(InstFlags::ACC_WRITE)) {
                return std::make_optional(static_cast<size_t>(Ins::ACCUMULATOR));
            }
        }
        return std::nullopt;
    }

    bool IsValidToEmit() const
    {
        const auto invalidRegNum = 1U << MaxRegEncodingWidth();
        for (std::size_t i = 0U; i < RegSize(); ++i) {
            if (GetReg(i) >= invalidRegNum) {
                return false;
            }
        }
        return true;
    }

    bool HasDebugInfo() const
    {
        return insDebug.LineNumber() != 0U;
    }

    bool IsDevirtual() const
    {
        return isDevirtual_;
    }

    void SetIsDevirtual()
    {
        isDevirtual_ = true;
    }

private:
    std::string OperandsToString(bool printArgs = false, size_t firstArgIdx = 0) const;
    std::string RegsToString(bool &first, bool printArgs = false, size_t firstArgIdx = 0) const;
    std::string ImmsToString(bool &first) const;
    std::string IdsToString(bool &first) const;

    std::string IdToString(size_t idx, bool isFirst) const;
    std::string ImmToString(size_t idx, bool isFirst) const;
    std::string RegToString(size_t idx, bool isFirst, bool printArgs = false, size_t firstArgIdx = 0) const;
    bool isDevirtual_ = false;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_INS_H
