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

#ifndef COMPILER_OPTIMIZER_IR_INST_H
#define COMPILER_OPTIMIZER_IR_INST_H

#include <vector>
#include "constants.h"
#include "datatype.h"

#ifdef PANDA_COMPILER_DEBUG_INFO
#include "debug_info.h"
#endif

#include "ir-dyn-base-types.h"
#include "marker.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/bit_field.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/bit_vector.h"
#include "libarkbase/macros.h"
#include "libarkbase/mem/arena_allocator.h"
#include "opcodes.h"
#include "optimizer/select_transform_type.h"
#include "compiler_options.h"
#include "runtime_interface.h"
#include "spill_fill_data.h"
#include "compiler/code_info/vreg_info.h"
namespace ark::compiler {
class Inst;
class BasicBlock;
class Graph;
class GraphVisitor;
class VnObject;
class SaveStateItem;
class LocationsInfo;
using InstVector = ArenaVector<Inst *>;

template <size_t N>
class FixedInputsInst;

/*
 * Condition code, used in Compare, If[Imm] and Select[Imm] instructions.
 *
 * N.B. BranchElimination and Peephole rely on the order of these codes. Change carefully.
 */
enum ConditionCode {
    // All types.
    CC_EQ = 0,  // ==
    CC_NE,      // !=
    // Signed integers and floating-point numbers.
    CC_LT,  // <
    CC_LE,  // <=
    CC_GT,  // >
    CC_GE,  // >=
    // Unsigned integers.
    CC_B,   // <
    CC_BE,  // <=
    CC_A,   // >
    CC_AE,  // >=
    // Compare result of bitwise AND with zero
    CC_TST_EQ,  // (lhs AND rhs) == 0
    CC_TST_NE,  // (lhs AND rhs) != 0
    // First and last aliases.
    CC_FIRST = CC_EQ,
    CC_LAST = CC_TST_NE,
};

const char *GetConditionCodeString(ConditionCode code);
ConditionCode GetInverseConditionCode(ConditionCode code);
ConditionCode InverseSignednessConditionCode(ConditionCode code);
bool IsSignedConditionCode(ConditionCode code);
PANDA_PUBLIC_API ConditionCode SwapOperandsConditionCode(ConditionCode code);

template <typename T>
bool Compare(ConditionCode cc, T lhs, T rhs)
{
    using SignedT = std::make_signed_t<T>;
    using UnsignedT = std::make_unsigned_t<T>;
    auto lhsU = bit_cast<UnsignedT>(lhs);
    auto rhsU = bit_cast<UnsignedT>(rhs);
    auto lhsS = bit_cast<SignedT>(lhs);
    auto rhsS = bit_cast<SignedT>(rhs);

    switch (cc) {
        case ConditionCode::CC_EQ:
            return lhsU == rhsU;
        case ConditionCode::CC_NE:
            return lhsU != rhsU;
        case ConditionCode::CC_LT:
            return lhsS < rhsS;
        case ConditionCode::CC_LE:
            return lhsS <= rhsS;
        case ConditionCode::CC_GT:
            return lhsS > rhsS;
        case ConditionCode::CC_GE:
            return lhsS >= rhsS;
        case ConditionCode::CC_B:
            return lhsU < rhsU;
        case ConditionCode::CC_BE:
            return lhsU <= rhsU;
        case ConditionCode::CC_A:
            return lhsU > rhsU;
        case ConditionCode::CC_AE:
            return lhsU >= rhsU;
        case ConditionCode::CC_TST_EQ:
            return (lhsU & rhsU) == 0;
        case ConditionCode::CC_TST_NE:
            return (lhsU & rhsU) != 0;
        default:
            UNREACHABLE();
            return false;
    }
}

enum class Opcode : int16_t {
    INVALID = -1,
// NOLINTBEGIN(readability-identifier-naming)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, ...) opcode,
    OPCODE_LIST(INST_DEF)

#undef INST_DEF
    // NOLINTEND(readability-identifier-naming)
    NUM_OPCODES
};

/// Convert opcode to its string representation
constexpr std::array<const char *const, static_cast<size_t>(Opcode::NUM_OPCODES)> OPCODE_NAMES = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(opcode, ...) #opcode,
    OPCODE_LIST(INST_DEF)
#undef INST_DEF
};

constexpr const char *GetOpcodeString(Opcode opc)
{
    ASSERT(static_cast<int>(opc) < static_cast<int>(Opcode::NUM_OPCODES));
    return OPCODE_NAMES[static_cast<int>(opc)];
}

/// Instruction flags. See `instrutions.yaml` section `flags` for more information.
namespace inst_flags {
namespace internal {
enum FlagsIndex {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FLAG_DEF(flag) flag##_INDEX,
    FLAGS_LIST(FLAG_DEF)
#undef FLAG_DEF
        FLAGS_COUNT
};
}  // namespace internal

enum Flags : uint64_t {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FLAG_DEF(flag) flag = (1ULL << internal::flag##_INDEX),
    FLAGS_LIST(FLAG_DEF)
#undef FLAG_DEF
        FLAGS_COUNT = internal::FLAGS_COUNT,
    NONE = 0
};

inline constexpr uint64_t GetFlagsMask(Opcode opcode)
{
#define INST_DEF(OPCODE, BASE, FLAGS) (FLAGS),  // NOLINT(cppcoreguidelines-macro-usage)
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    constexpr std::array<uint64_t, static_cast<int>(Opcode::NUM_OPCODES)> INST_FLAGS_TABLE = {OPCODE_LIST(INST_DEF)};
#undef INST_DEF
    return INST_FLAGS_TABLE[static_cast<size_t>(opcode)];
}

inline constexpr bool HasFlag(Opcode opcode, Flags flag)
{
    return (GetFlagsMask(opcode) & flag) != 0;
}
}  // namespace inst_flags

#ifndef NDEBUG
namespace inst_modes {
namespace internal {
enum ModeIndex {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MODE_DEF(mode) mode##_INDEX,
    MODES_LIST(MODE_DEF)
#undef MODE_DEF
        MODES_COUNT
};
}  // namespace internal

enum Mode : uint8_t {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MODE_DEF(mode) mode = (1U << internal::mode##_INDEX),
    MODES_LIST(MODE_DEF)
#undef MODE_DEF
        MODES_COUNT = internal::MODES_COUNT,
};

inline constexpr uint8_t GetModesMask(Opcode opcode)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    constexpr std::array<uint8_t, static_cast<int>(Opcode::NUM_OPCODES)> INST_MODES_TABLE = {INST_MODES_LIST};
    return INST_MODES_TABLE[static_cast<size_t>(opcode)];
}
}  // namespace inst_modes
#endif

namespace internal {
inline constexpr std::array<const char *, ShiftType::INVALID_SHIFT + 1> SHIFT_TYPE_NAMES = {"LSL", "LSR", "ASR", "ROR",
                                                                                            "INVALID"};
}  // namespace internal

inline const char *GetShiftTypeStr(ShiftType type)
{
    ASSERT(type <= INVALID_SHIFT);
    return internal::SHIFT_TYPE_NAMES[type];
}

/// Describes type of the object produced by an instruction.
class ObjectTypeInfo {
public:
    using ClassType = RuntimeInterface::ClassPtr;

    constexpr ObjectTypeInfo() = default;
    ObjectTypeInfo(ClassType klass, bool isExact)
        : class_(reinterpret_cast<uintptr_t>(klass) | static_cast<uintptr_t>(isExact))
    {
        ASSERT((reinterpret_cast<uintptr_t>(klass) & EXACT_MASK) == 0);
    }

    bool operator==(const ObjectTypeInfo &other) const
    {
        return class_ == other.class_;
    }

    bool operator!=(const ObjectTypeInfo &other) const
    {
        return class_ != other.class_;
    }

    // NOLINTNEXTLINE(*-explicit-constructor)
    operator bool() const
    {
        return IsValid();
    }

    ClassType GetClass() const
    {
        return reinterpret_cast<ClassType>(class_ & ~EXACT_MASK);
    }

    bool IsExact() const
    {
        return (class_ & EXACT_MASK) != 0;
    }

    bool IsValid() const
    {
        return class_ > 1;
    }

    static const ObjectTypeInfo INVALID;
    static const ObjectTypeInfo UNKNOWN;

private:
    explicit constexpr ObjectTypeInfo(uintptr_t klass) : class_(klass) {}

private:
    static constexpr uintptr_t EXACT_MASK = 1;
    // Lowest bit in ClassPtr is always zero due to alignment, we set it to 1 if `klass` is the exact class
    // of the object and to 0 if it is some superclass of that class
    uintptr_t class_ = 0;
};

using VRegType = VRegInfo::VRegType;

/// Class for storing panda bytecode's virtual register
class VirtualRegister final {
public:
    using ValueType = uint16_t;

    static constexpr unsigned BITS_FOR_VREG_TYPE = MinimumBitsToStore(VRegType::COUNT);
    static constexpr unsigned BITS_FOR_VREG = (sizeof(ValueType) * BITS_PER_BYTE) - BITS_FOR_VREG_TYPE;

    static constexpr ValueType INVALID = std::numeric_limits<ValueType>::max() >> BITS_FOR_VREG_TYPE;
    // This value we marked the virtual registers, that create in bridge for SS
    static constexpr ValueType BRIDGE = INVALID - 1U;
    static constexpr ValueType MAX_NUM_VIRT_REGS = BRIDGE - 2U;

    VirtualRegister() = default;
    explicit VirtualRegister(ValueType v, VRegType type) : value_(v)
    {
        ASSERT(ValidNumVirtualReg(value_));
        VRegTypeField::Set(type, &value_);
    }

    explicit operator uint16_t() const
    {
        return value_;
    }

    ValueType Value() const
    {
        return ValueField::Get(value_);
    }

    bool IsAccumulator() const
    {
        return VRegTypeField::Get(value_) == VRegType::ACC;
    }

    bool IsEnv() const
    {
        return IsSpecialReg() && !IsAccumulator();
    }

    bool IsSpecialReg() const
    {
        return VRegTypeField::Get(value_) != VRegType::VREG;
    }

    VRegType GetVRegType() const
    {
        return static_cast<VRegType>(VRegTypeField::Get(value_));
    }

    bool IsBridge() const
    {
        return ValueField::Get(value_) == BRIDGE;
    }

    static bool ValidNumVirtualReg(uint16_t num)
    {
        return num <= INVALID;
    }

private:
    ValueType value_ {INVALID};

    using ValueField = BitField<unsigned, 0, BITS_FOR_VREG>;
    using VRegTypeField = ValueField::NextField<unsigned, BITS_FOR_VREG_TYPE>;
};

// How many bits will be used in Inst's bit fields for number of inputs.
constexpr size_t BITS_PER_INPUTS_NUM = 3;
// Maximum number of static inputs
constexpr size_t MAX_STATIC_INPUTS = (1U << BITS_PER_INPUTS_NUM) - 1;

/// Currently Input class is just a wrapper for the Inst class.
class Input final {
public:
    Input() = default;
    explicit Input(Inst *inst) : inst_(inst) {}

    Inst *GetInst()
    {
        return inst_;
    }
    const Inst *GetInst() const
    {
        return inst_;
    }

    static inline uint8_t GetPadding(Arch arch, uint32_t inputsCount)
    {
        return static_cast<uint8_t>(!Is64BitsArch(arch) && inputsCount % 2U == 1U);
    }

private:
    Inst *inst_ {nullptr};
};

inline bool operator==(const Inst *lhs, const Input &rhs)
{
    return lhs == rhs.GetInst();
}

inline bool operator==(const Input &lhs, const Inst *rhs)
{
    return lhs.GetInst() == rhs;
}

inline bool operator==(const Input &lhs, const Input &rhs)
{
    return lhs.GetInst() == rhs.GetInst();
}

inline bool operator!=(const Inst *lhs, const Input &rhs)
{
    return lhs != rhs.GetInst();
}

inline bool operator!=(const Input &lhs, const Inst *rhs)
{
    return lhs.GetInst() != rhs;
}

inline bool operator!=(const Input &lhs, const Input &rhs)
{
    return lhs.GetInst() != rhs.GetInst();
}

/**
 * User is a intrusive list node, thus it stores pointers to next and previous users.
 * Also user has properties value to determine owner instruction and corresponding index of the input.
 */
class User final {
public:
    User() = default;
    User(bool isStatic, unsigned index, unsigned size)
        : properties_(IsStaticFlag::Encode(isStatic) | IndexField::Encode(index) | SizeField::Encode(size) |
                      BbNumField::Encode(BbNumField::MaxValue()))
    {
        ASSERT(index < 1U << (BITS_FOR_INDEX - 1U));
        ASSERT(size < 1U << (BITS_FOR_SIZE - 1U));
    }
    ~User() = default;

    // Copy/move semantic is disabled because we use tricky pointer arithmetic based on 'this' value
    NO_COPY_SEMANTIC(User);
    NO_MOVE_SEMANTIC(User);

    PANDA_PUBLIC_API Inst *GetInst();
    const Inst *GetInst() const
    {
        return const_cast<User *>(this)->GetInst();
    }

    Inst *GetInput();
    const Inst *GetInput() const;

    bool IsDynamic() const
    {
        return !IsStaticFlag::Decode(properties_);
    }
    unsigned GetIndex() const
    {
        return IndexField::Decode(properties_);
    }
    unsigned GetSize() const
    {
        return SizeField::Decode(properties_);
    }

    VirtualRegister GetVirtualRegister() const
    {
        ASSERT(IsDynamic());
        return VirtualRegister(VRegField::Decode(properties_),
                               static_cast<VRegType>(VRegTypeField::Decode(properties_)));
    }

    void SetVirtualRegister(VirtualRegister reg)
    {
        static_assert(sizeof(reg) <= sizeof(uintptr_t), "Consider passing the register by reference");
        ASSERT(IsDynamic());
        VRegField::Set(reg.Value(), &properties_);
        VRegTypeField::Set(reg.GetVRegType(), &properties_);
    }

    uint32_t GetBbNum() const
    {
        ASSERT(IsDynamic());
        return BbNumField::Decode(properties_);
    }

    void SetBbNum(uint32_t bbNum)
    {
        ASSERT(IsDynamic());
        BbNumField::Set(bbNum, &properties_);
    }

    auto GetNext() const
    {
        return next_;
    }

    auto GetPrev() const
    {
        return prev_;
    }

    void SetNext(User *next)
    {
        next_ = next;
    }

    void SetPrev(User *prev)
    {
        prev_ = prev;
    }

    void Remove()
    {
        if (prev_ != nullptr) {
            prev_->next_ = next_;
        }
        if (next_ != nullptr) {
            next_->prev_ = prev_;
        }
    }

private:
    static constexpr unsigned BITS_FOR_INDEX = 21;
    static constexpr unsigned BITS_FOR_SIZE = BITS_FOR_INDEX;
    static constexpr unsigned BITS_FOR_BB_NUM = 20;
    using IndexField = BitField<unsigned, 0, BITS_FOR_INDEX>;
    using SizeField = IndexField::NextField<unsigned, BITS_FOR_SIZE>;
    using IsStaticFlag = SizeField::NextFlag;

    using BbNumField = IsStaticFlag::NextField<uint32_t, BITS_FOR_BB_NUM>;

    using VRegField = IsStaticFlag::NextField<unsigned, VirtualRegister::BITS_FOR_VREG>;
    using VRegTypeField = VRegField::NextField<unsigned, VirtualRegister::BITS_FOR_VREG_TYPE>;

    uint64_t properties_ {0};
    User *next_ {nullptr};
    User *prev_ {nullptr};
};

/**
 * List of users. Intended for range loop.
 * @tparam T should be User or const User
 */
template <typename T>
class UserList {
    template <typename U>
    struct UserIterator {
        // NOLINTBEGIN(readability-identifier-naming)
        using iterator_category = std::forward_iterator_tag;
        using value_type = U;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;
        // NOLINTEND(readability-identifier-naming)

        UserIterator() = default;
        explicit UserIterator(U *u) : user_(u) {}

        UserIterator &operator++()
        {
            user_ = user_->GetNext();
            return *this;
        }
        bool operator!=(const UserIterator &other)
        {
            return user_ != other.user_;
        }
        bool operator==(const UserIterator &other)
        {
            return user_ == other.user_;
        }
        U &operator*()
        {
            return *user_;
        }
        U *operator->()
        {
            return user_;
        }

    private:
        U *user_ {nullptr};
    };

public:
    using Iterator = UserIterator<T>;
    using ConstIterator = UserIterator<const T>;
    using PointerType = std::conditional_t<std::is_const_v<T>, T *const *, T **>;

    explicit UserList(PointerType head) : head_(head) {}

    // NOLINTNEXTLINE(readability-identifier-naming)
    Iterator begin()
    {
        return Iterator(*head_);
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    Iterator end()
    {
        return Iterator(nullptr);
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    ConstIterator begin() const
    {
        return ConstIterator(*head_);
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    ConstIterator end() const
    {
        return ConstIterator(nullptr);
    }
    bool Empty() const
    {
        return *head_ == nullptr;
    }
    T &Front()
    {
        return **head_;
    }
    const T &Front() const
    {
        return **head_;
    }

private:
    PointerType head_ {nullptr};
};

inline bool operator==(const User &lhs, const User &rhs)
{
    return lhs.GetInst() == rhs.GetInst();
}

/**
 * Operands class for instructions with fixed inputs count.
 * Actually, this class do absolutely nothing except that we can get sizeof of it when allocating memory.
 */
template <int N>
struct Operands {
    static_assert(N < MAX_STATIC_INPUTS, "Invalid inputs number");

    std::array<User, N> users;
    std::array<Input, N> inputs;
};

enum InputOrd { INP0 = 0, INP1 = 1, INP2 = 2, INP3 = 3 };

/**
 * Specialized version for instructions with variable inputs count.
 * Users and inputs are stored outside of this class.
 */
class DynamicOperands {
public:
    explicit DynamicOperands(ArenaAllocator *allocator) : allocator_(allocator) {}

    User *Users()
    {
        return users_;
    }

    NO_UB_SANITIZE Input *Inputs()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<Input *>(users_ + capacity_) + 1;
    }

    /// Append new input (and user accordingly)
    PANDA_PUBLIC_API unsigned Append(Inst *inst);

    /// Remove input and user with index `index`.
    PANDA_PUBLIC_API void Remove(unsigned index);

    /// Reallocate inputs/users storage to a new one with specified capacity.
    void Reallocate(size_t newCapacity = 0);

    /// Get instruction to which these operands belongs to.
    Inst *GetOwnerInst() const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<Inst *>(const_cast<DynamicOperands *>(this) + 1);
    }

    User *GetUser(unsigned index)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return &users_[capacity_ - index - 1];
    }

    Input *GetInput(unsigned index)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return &Inputs()[index];
    }

    void SetInput(unsigned index, Input input)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Inputs()[index] = input;
    }

    size_t Size() const
    {
        return size_;
    }

private:
    User *users_ {nullptr};
    size_t size_ {0};
    size_t capacity_ {0};
    ArenaAllocator *allocator_ {nullptr};
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)

/// Base class for all instructions, should not be instantiated directly
class InstBase {
    NO_COPY_SEMANTIC(InstBase);
    NO_MOVE_SEMANTIC(InstBase);

public:
    virtual ~InstBase() = default;

public:
    ALWAYS_INLINE void operator delete([[maybe_unused]] void *unused, [[maybe_unused]] size_t size)
    {
        UNREACHABLE();
    }
    ALWAYS_INLINE void *operator new([[maybe_unused]] size_t size, void *ptr) noexcept
    {
        return ptr;
    }
    ALWAYS_INLINE void operator delete([[maybe_unused]] void *unused1, [[maybe_unused]] void *unused2) noexcept {}

    void *operator new([[maybe_unused]] size_t size) = delete;

protected:
    InstBase() = default;
};

/// Base instruction class
class PANDA_PUBLIC_API Inst : public MarkerSet, public InstBase {
public:
    // Used for SFINAE for inputs deduction during CreateInst-calls
    template <typename... Ds>
    using CheckBase = std::enable_if_t<std::conjunction_v<std::is_convertible<Ds, Inst *>...>>;

public:
    /**
     * Create new instruction. All instructions must be created with this method.
     * It allocates additional space before Inst object for def-use structures.
     *
     * @tparam InstType - concrete type of instruction, shall be derived from Inst
     * @tparam Args - constructor arguments types
     * @param allocator - allocator for memory allocating
     * @param args - constructor arguments
     * @return - new instruction
     */
    template <typename InstType, typename... Args>
    [[nodiscard]] static InstType *New(ArenaAllocator *allocator, Args &&...args);

    INST_CAST_TO_DECL()

    // Methods for instruction chaining inside basic blocks.
    Inst *GetNext()
    {
        return next_;
    }
    const Inst *GetNext() const
    {
        return next_;
    }
    Inst *GetPrev()
    {
        return prev_;
    }
    const Inst *GetPrev() const
    {
        return prev_;
    }
    void SetNext(Inst *next)
    {
        next_ = next;
    }
    void SetPrev(Inst *prev)
    {
        prev_ = prev;
    }

    // Id accessors
    auto GetId() const
    {
        return id_;
    }
    void SetId(int id)
    {
        id_ = id;
    }

    auto GetLinearNumber() const
    {
        return linearNumber_;
    }
    void SetLinearNumber(LinearNumber number)
    {
        linearNumber_ = number;
    }

    auto GetCloneNumber() const
    {
        return cloneNumber_;
    }
    void SetCloneNumber(int32_t number)
    {
        cloneNumber_ = number;
    }

    // Opcode accessors
    Opcode GetOpcode() const
    {
        return opcode_;
    }
    void SetOpcode(Opcode opcode)
    {
        opcode_ = opcode;
        SetField<FieldFlags>(inst_flags::GetFlagsMask(opcode));
    }
    const char *GetOpcodeStr() const
    {
        return GetOpcodeString(GetOpcode());
    }

    // Bytecode PC accessors
    uint32_t GetPc() const
    {
        return pc_;
    }
    void SetPc(uint32_t pc)
    {
        pc_ = pc;
    }

    // Type accessors
    DataType::Type GetType() const
    {
        return FieldType::Get(bitFields_);
    }
    void SetType(DataType::Type type)
    {
        FieldType::Set(type, &bitFields_);
    }
    bool HasType() const
    {
        return GetType() != DataType::Type::NO_TYPE;
    }

    virtual AnyBaseType GetAnyType() const
    {
        return AnyBaseType::UNDEFINED_TYPE;
    }

    // Parent basic block accessors
    BasicBlock *GetBasicBlock()
    {
        return bb_;
    }
    const BasicBlock *GetBasicBlock() const
    {
        return bb_;
    }
    void SetBasicBlock(BasicBlock *bb)
    {
        bb_ = bb;
    }

    // Instruction properties getters
    bool IsControlFlow() const
    {
        return GetFlag(inst_flags::CF);
    }
    bool IsVirtualCall() const
    {
        return GetOpcode() == Opcode::CallVirtual || GetOpcode() == Opcode::CallResolvedVirtual;
    }
    bool IsStaticCall() const
    {
        return GetOpcode() == Opcode::CallStatic || GetOpcode() == Opcode::CallResolvedStatic;
    }
    bool IsNativeApiCall() const
    {
        return GetOpcode() == Opcode::CallNative;
    }
    bool IsReferenceForNativeApiCall() const
    {
        if (GetType() != DataType::REFERENCE) {
            return false;
        }

        for (const auto &user : GetUsers()) {
            if (user.GetInst()->GetOpcode() == Opcode::WrapObjectNative) {
                return true;
            }
        }
        return false;
    }
    bool IsMethodResolver() const
    {
        return opcode_ == Opcode::ResolveVirtual || opcode_ == Opcode::ResolveStatic ||
               opcode_ == Opcode::ResolveByName;
    }
    bool IsFieldResolver() const
    {
        return opcode_ == Opcode::ResolveObjectField || opcode_ == Opcode::ResolveObjectFieldStatic;
    }
    bool IsResolver() const
    {
        return IsFieldResolver() || IsMethodResolver();
    }
    bool IsInitObject() const
    {
        return GetOpcode() == Opcode::InitObject;
    }
    bool IsMultiArray() const
    {
        return GetOpcode() == Opcode::MultiArray;
    }
    bool IsDynamicCall() const
    {
        return GetOpcode() == Opcode::CallDynamic;
    }
    bool IsIndirectCall() const
    {
        return GetOpcode() == Opcode::CallIndirect;
    }
    bool IsIntrinsic() const
    {
        /* Opcode::Builtin is left for backward compatibility, the compiler
         * itself should never generate an instruction with such an opcode */
        return GetOpcode() == Opcode::Intrinsic || GetOpcode() == Opcode::Builtin;
    }

    /* IsBuiltin actual meaning would be "it MAY be inlined by the CG"
     * however, since we do not make guarantees about whether it will
     * actually be inlined nor the safety of the intrinsic itself, just
     * checking the instruction flags to see if it is suitable for any
     * particular optimization seems to be a better approach
     */
    static bool IsBuiltin()
    {
        return false;
    }

    bool IsCall() const
    {
        return GetFlag(inst_flags::CALL) && !IsIntrinsic();
    }

    bool IsCallOrIntrinsic() const
    {
        return GetFlag(inst_flags::CALL);
    }

    bool IsSpillFill() const
    {
        return GetOpcode() == Opcode::SpillFill;
    }

    bool IsNullCheck() const
    {
        return GetOpcode() == Opcode::NullCheck;
    }

    bool IsNullPtr() const
    {
        return GetOpcode() == Opcode::NullPtr;
    }

    bool IsLoadUniqueObject() const
    {
        return GetOpcode() == Opcode::LoadUniqueObject;
    }

    bool IsReturn() const
    {
        return GetOpcode() == Opcode::Return || GetOpcode() == Opcode::ReturnI || GetOpcode() == Opcode::ReturnVoid;
    }

    bool IsUnresolved() const
    {
        switch (GetOpcode()) {
            case Opcode::UnresolvedLoadAndInitClass:
            case Opcode::UnresolvedLoadType:
            case Opcode::UnresolvedStoreStatic:
                return true;
            default:
                return false;
        }
    }
    bool WithGluedInsts() const
    {
        return GetOpcode() == Opcode::LoadArrayPair || GetOpcode() == Opcode::LoadArrayPairI ||
               GetOpcode() == Opcode::LoadObjectPair;
    }
    bool IsLoad() const
    {
        return GetFlag(inst_flags::LOAD);
    }
    bool IsStore() const
    {
        return GetFlag(inst_flags::STORE);
    }
    bool IsAccRead() const;
    bool IsAccWrite() const;
    bool IsMemory() const
    {
        return IsLoad() || IsStore();
    }
    bool CanThrow() const
    {
        return GetFlag(inst_flags::CAN_THROW);
    }
    bool IsCheck() const
    {
        return GetFlag(inst_flags::IS_CHECK);
    }
    bool RequireState() const
    {
        return GetFlag(inst_flags::REQUIRE_STATE);
    }
    // Returns true if the instruction not removable in DCE
    bool IsNotRemovable() const
    {
        return GetFlag(inst_flags::NO_DCE);
    }

    // Returns true if the instruction doesn't have destination register
    bool NoDest() const
    {
        return GetFlag(inst_flags::PSEUDO_DST) || GetFlag(inst_flags::NO_DST) || GetType() == DataType::VOID;
    }

    bool HasSideEffectsFlags() const
    {
        // NOTE (#31054): When adding new instruction flags, update this method accordingly.
        const uintptr_t forbiddenFlags = inst_flags::CAN_THROW | inst_flags::TERMINATOR | inst_flags::ALLOC |
                                         inst_flags::CALL | inst_flags::RUNTIME_CALL |
                                         inst_flags::IMPLICIT_RUNTIME_CALL | inst_flags::STORE | inst_flags::BARRIER |
                                         inst_flags::REQUIRE_STATE | inst_flags::CAN_DEOPTIMIZE | inst_flags::NO_DCE;
        return (GetFlagsMask() & forbiddenFlags) != 0;
    }

    bool HasPseudoDestination() const
    {
        return GetFlag(inst_flags::PSEUDO_DST);
    }

    bool HasImplicitRuntimeCall() const
    {
        return GetFlag(inst_flags::IMPLICIT_RUNTIME_CALL);
    }

    bool CanDeoptimize() const
    {
        return GetFlag(inst_flags::CAN_DEOPTIMIZE);
    }

    bool RequireTmpReg() const
    {
        return GetFlag(inst_flags::REQUIRE_TMP);
    }

    // Returns true if the instruction is low-level
    bool IsLowLevel() const
    {
        return GetFlag(inst_flags::LOW_LEVEL);
    }

    // Returns true if the instruction not hoistable
    bool IsNotHoistable() const
    {
        return GetFlag(inst_flags::NO_HOIST);
    }

    // Returns true Cse can't be applied to the instruction
    bool IsNotCseApplicable() const
    {
        return GetFlag(inst_flags::NO_CSE);
    }

    // Returns true if the instruction is a barrier
    virtual bool IsBarrier() const
    {
        return GetFlag(inst_flags::BARRIER);
    }

    // Returns true if opcode can not be moved throught runtime calls (REFERENCE type only)
    bool IsRefSpecial() const
    {
        bool result = GetFlag(inst_flags::REF_SPECIAL);
        ASSERT(!result || IsReferenceOrAny());
        return result;
    }

    // Returns true if the instruction is a commutative
    bool IsCommutative() const
    {
        return GetFlag(inst_flags::COMMUTATIVE);
    }

    // Returns true if the instruction allocates a new object on the heap
    bool IsAllocation() const
    {
        return GetFlag(inst_flags::ALLOC);
    }

    // Returns true if the instruction can be used in if-conversion
    bool IsIfConvertable() const
    {
        return GetFlag(inst_flags::IFCVT);
    }

    virtual bool IsRuntimeCall() const
    {
        return GetFlag(inst_flags::RUNTIME_CALL);
    }

    virtual bool NoNullPtr() const
    {
        return GetFlag(inst_flags::NO_NULLPTR);
    }

    PANDA_PUBLIC_API virtual bool IsPropagateLiveness() const;

    // Returns true if the instruction doesn't have side effects(call runtime, throw e.t.c.)
    virtual bool IsSafeInst() const
    {
        return false;
    }

    virtual bool IsBinaryInst() const
    {
        return false;
    }

    virtual bool IsBinaryImmInst() const
    {
        return false;
    }

    bool RequireRegMap() const;

    ObjectTypeInfo GetObjectTypeInfo() const
    {
        return objectTypeInfo_;
    }

    bool HasObjectTypeInfo() const
    {
        return objectTypeInfo_.IsValid();
    }

    void SetObjectTypeInfo(ObjectTypeInfo o)
    {
        objectTypeInfo_ = o;
    }

    Inst *GetDataFlowInput(int index) const
    {
        return GetDataFlowInput(GetInput(index).GetInst());
    }
    static Inst *GetDataFlowInput(Inst *inputInst);

    bool IsPrecedingInSameBlock(const Inst *other) const;

    bool IsDominate(const Inst *other) const;

    bool InSameBlockOrDominate(const Inst *other) const;

    bool IsAdd() const
    {
        return GetOpcode() == Opcode::Add || GetOpcode() == Opcode::AddOverflowCheck;
    }

    bool IsSub() const
    {
        return GetOpcode() == Opcode::Sub || GetOpcode() == Opcode::SubOverflowCheck;
    }

    bool IsAddSub() const
    {
        return IsAdd() || IsSub();
    }

    bool IsShift() const
    {
        return GetOpcode() == Opcode::Shl || GetOpcode() == Opcode::Shr || GetOpcode() == Opcode::AShr;
    }

    const SaveStateInst *GetSaveState() const
    {
        return const_cast<Inst *>(this)->GetSaveState();
    }

    SaveStateInst *GetSaveState()
    {
        if (!RequireState()) {
            return nullptr;
        }
        if (GetInputsCount() == 0) {
            return nullptr;
        }
        auto ss = GetInput(GetInputsCount() - 1).GetInst();
        if (ss->GetOpcode() == Opcode::SaveStateDeoptimize) {
            return ss->CastToSaveStateDeoptimize();
        }
        if (ss->GetOpcode() != Opcode::SaveState) {
            return nullptr;
        }

        return ss->CastToSaveState();
    }

    void SetSaveState(Inst *inst)
    {
        ASSERT(RequireState());
        SetInput(GetInputsCount() - 1, inst);
    }

    PANDA_PUBLIC_API virtual uint32_t GetInliningDepth() const;

    bool IsZeroRegInst() const;

    bool IsReferenceOrAny() const;
    bool IsMovableObject();

    /// Return instruction clone
    PANDA_PUBLIC_API virtual Inst *Clone(const Graph *targetGraph) const;

    uint64_t GetFlagsMask() const
    {
        return GetField<FieldFlags>();
    }

    void SetFlagsMask(inst_flags::Flags flag)
    {
        SetField<FieldFlags>(flag);
    }

    bool GetFlag(inst_flags::Flags flag) const
    {
        return (GetFlagsMask() & flag) != 0;
    }

    void SetFlag(inst_flags::Flags flag)
    {
        SetField<FieldFlags>(GetFlagsMask() | flag);
    }

    void ClearFlag(inst_flags::Flags flag)
    {
        SetField<FieldFlags>(GetFlagsMask() & ~static_cast<uintptr_t>(flag));
    }

#ifndef NDEBUG
    uint8_t GetModesMask() const
    {
        return inst_modes::GetModesMask(opcode_);
    }

    bool SupportsMode(inst_modes::Mode mode) const
    {
        return (GetModesMask() & mode) != 0;
    }
#endif

    void SetTerminator()
    {
        SetFlag(inst_flags::Flags::TERMINATOR);
    }

    bool IsTerminator() const
    {
        return GetFlag(inst_flags::TERMINATOR);
    }

    PANDA_PUBLIC_API void InsertBefore(Inst *inst);
    void InsertAfter(Inst *inst);

    /// Return true if instruction has dynamic operands storage.
    bool IsOperandsDynamic() const
    {
        return GetField<InputsCount>() == MAX_STATIC_INPUTS;
    }

    /**
     * Add user to the instruction.
     * @param user - pointer to User object
     */
    void AddUser(User *user)
    {
        ASSERT(user && user->GetInst());
        user->SetNext(firstUser_);
        user->SetPrev(nullptr);
        if (firstUser_ != nullptr) {
            ASSERT(firstUser_->GetPrev() == nullptr);
            firstUser_->SetPrev(user);
        }
        firstUser_ = user;
    }

    /**
     * Remove instruction from users.
     * @param user - pointer to User object
     */
    void RemoveUser(User *user)
    {
        ASSERT(user);
        ASSERT(HasUsers());
        if (user == firstUser_) {
            firstUser_ = user->GetNext();
        }
        user->Remove();
    }

    /**
     * Set input instruction in specified index.
     * Old input will be removed.
     * @param index - index of input to be set
     * @param inst - new input instruction NOTE sherstennikov: currently it can be nullptr, is it correct?
     */
    void SetInput(unsigned index, Inst *inst)
    {
        CHECK_LT(index, GetInputsCount());
        auto &input = GetInputs()[index];
        auto user = GetUser(index);
        if (input.GetInst() != nullptr && input.GetInst()->HasUsers()) {
            input.GetInst()->RemoveUser(user);
        }
        if (inst != nullptr) {
            inst->AddUser(user);
        }
        input = Input(inst);
    }

    /**
     * Replace all inputs that points to specified instruction by new one.
     * @param old_input - instruction that should be replaced
     * @param new_input - new input instruction
     */
    void ReplaceInput(Inst *oldInput, Inst *newInput)
    {
        unsigned index = 0;
        for (auto input : GetInputs()) {
            if (input.GetInst() == oldInput) {
                SetInput(index, newInput);
            }
            index++;
        }
    }

    /**
     * Replace inputs that point to this instruction by given instruction.
     * @param inst - new input instruction
     */
    void ReplaceUsers(Inst *inst)
    {
        ASSERT(inst != this);
        ASSERT(inst != nullptr);
        for (auto it = GetUsers().begin(); it != GetUsers().end(); it = GetUsers().begin()) {
            it->GetInst()->SetInput(it->GetIndex(), inst);
        }
    }

    /**
     * Swap first 2 operands of the instruction.
     * NB! Don't swap inputs while iterating over instruction's users:
     * for (auto user : instruction.GetUsers()) {
     *     // Don't do this!
     *     user.GetInst()->SwapInputs();
     * }
     */
    void SwapInputs()
    {
        ASSERT(GetInputsCount() >= 2U);
        auto input0 = GetInput(0).GetInst();
        auto input1 = GetInput(1).GetInst();
        SetInput(0, input1);
        SetInput(1, input0);
    }

    /**
     * Append input instruction.
     * Available only for variadic inputs instructions, such as PHI.
     * @param input - input instruction
     * @return index in inputs container where new input is placed
     */
    unsigned AppendInput(Inst *input)
    {
        ASSERT(input != nullptr);
        ASSERT(IsOperandsDynamic());
        DynamicOperands *operands = GetDynamicOperands();
        return operands->Append(input);
    }

    unsigned AppendInput(Input input)
    {
        static_assert(sizeof(Input) <= sizeof(uintptr_t));  // Input become larger, so pass it by reference then
        return AppendInput(input.GetInst());
    }

    /**
     * Remove input from inputs container
     * Available only for variadic inputs instructions, such as PHI.
     * @param index - index of input in inputs container
     */
    virtual void RemoveInput(unsigned index)
    {
        ASSERT(IsOperandsDynamic());
        DynamicOperands *operands = GetDynamicOperands();
        ASSERT(index < operands->Size());
        operands->Remove(index);
    }

    /// Remove all inputs
    void RemoveInputs()
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            for (auto inputsCount = GetInputsCount(); inputsCount != 0; --inputsCount) {
                RemoveInput(inputsCount - 1);
            }
        } else {
            for (size_t i = 0; i < GetInputsCount(); ++i) {
                SetInput(i, nullptr);
            }
        }
    }

    /// Remove all users
    template <bool WITH_INPUTS = false>
    void RemoveUsers()
    {
        auto users = GetUsers();
        while (!users.Empty()) {
            // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
            if constexpr (WITH_INPUTS) {
                auto &user = users.Front();
                user.GetInst()->RemoveInput(user.GetIndex());
                // NOLINTNEXTLINE(readability-misleading-indentation)
            } else {
                RemoveUser(&users.Front());
            }
        }
    }

    /**
     * Get input by index
     * @param index - index of input
     * @return input instruction
     */
    Input GetInput(unsigned index)
    {
        ASSERT(index < GetInputsCount());
        return GetInputs()[index];
    }

    Input GetInput(unsigned index) const
    {
        ASSERT(index < GetInputsCount());
        return GetInputs()[index];
    }

    Span<Input> GetInputs()
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            DynamicOperands *operands = GetDynamicOperands();
            return Span<Input>(operands->Inputs(), operands->Size());
        }

        auto inputsCount {GetField<InputsCount>()};
        return Span<Input>(
            reinterpret_cast<Input *>(reinterpret_cast<uintptr_t>(this) -
                                      (inputsCount + Input::GetPadding(RUNTIME_ARCH, inputsCount)) * sizeof(Input)),
            inputsCount);
    }
    Span<const Input> GetInputs() const
    {
        return Span<const Input>(const_cast<Inst *>(this)->GetInputs());
    }

    virtual DataType::Type GetInputType([[maybe_unused]] size_t index) const
    {
        ASSERT(index < GetInputsCount());
        if (GetInput(index).GetInst()->IsSaveState()) {
            return DataType::NO_TYPE;
        }
        return GetType();
    }

    UserList<User> GetUsers()
    {
        return UserList<User>(&firstUser_);
    }
    UserList<const User> GetUsers() const
    {
        return UserList<const User>(&firstUser_);
    }

    size_t GetInputsCount() const
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            return GetDynamicOperands()->Size();
        }
        return GetInputs().Size();
    }

    bool HasUsers() const
    {
        return firstUser_ != nullptr;
    };

    bool HasSingleUser() const
    {
        return firstUser_ != nullptr && firstUser_->GetNext() == nullptr;
    }

    /// Reserve space in dataflow storage for specified inputs count
    void ReserveInputs(size_t capacity);

    virtual void SetLocation([[maybe_unused]] size_t index, [[maybe_unused]] Location location) {}

    virtual Location GetLocation([[maybe_unused]] size_t index) const
    {
        return Location::RequireRegister();
    }

    virtual Location GetDstLocation() const
    {
        return Location::MakeRegister(GetDstReg(), GetType());
    }

    virtual Location GetDstLocation([[maybe_unused]] unsigned index) const
    {
        ASSERT(index == 0);
        return GetDstLocation();
    }

    virtual void SetTmpLocation([[maybe_unused]] Location location) {}

    virtual Location GetTmpLocation() const
    {
        return Location::Invalid();
    }

    virtual bool CanBeNull() const
    {
        ASSERT_PRINT(GetType() == DataType::Type::REFERENCE, "CanBeNull only applies to reference types");
        return true;
    }

    virtual uint32_t Latency() const
    {
        return g_options.GetCompilerSchedLatency();
    }

    template <typename Accessor>
    void SetField(typename Accessor::ValueType value)
    {
        Accessor::Set(value, &bitFields_);
    }

    template <typename Accessor>
    typename Accessor::ValueType GetField() const
    {
        return Accessor::Get(bitFields_);
    }

    void SetAllFields(uint64_t bitFields)
    {
        bitFields_ = bitFields;
    }

    uint64_t GetAllFields() const
    {
        return bitFields_;
    }

    bool Is(Opcode opcode) const
    {
        return opcode_ == opcode;
    }

    bool IsNot(Opcode opcode) const
    {
        return opcode_ != opcode;
    }

    bool IsOneOf(Opcode opcode1, Opcode opcode2) const
    {
        return Is(opcode1) || Is(opcode2);
    }

    template <typename... Ts>
    bool IsOneOf(Opcode opcode1, Ts... opcodes) const
    {
        return Is(opcode1) || IsOneOf(opcodes...);
    }

    bool IsPhi() const
    {
        return opcode_ == Opcode::Phi;
    }

    bool IsCatchPhi() const
    {
        return opcode_ == Opcode::CatchPhi;
    }

    bool IsConst() const
    {
        return opcode_ == Opcode::Constant;
    }

    bool IsParameter() const
    {
        return opcode_ == Opcode::Parameter;
    }

    virtual bool IsBoolConst() const
    {
        return false;
    }

    bool IsSaveState() const
    {
        return opcode_ == Opcode::SaveState || opcode_ == Opcode::SafePoint || opcode_ == Opcode::SaveStateOsr ||
               opcode_ == Opcode::SaveStateDeoptimize;
    }

    bool IsClassInst() const
    {
        return opcode_ == Opcode::InitClass || opcode_ == Opcode::LoadClass || opcode_ == Opcode::LoadAndInitClass ||
               opcode_ == Opcode::UnresolvedLoadAndInitClass;
    }

    virtual size_t GetHashCode() const
    {
        // NOTE (Aleksandr Popov) calculate hash code
        return 0;
    }

    virtual void SetVnObject(VnObject *vnObj) const
    {
        AppendOpcodeAndTypeToVnObject(vnObj);
        AppendInputsToVnObject(vnObj);  // Default behavior for most insts
    }

    Register GetDstReg() const
    {
        return dstReg_;
    }

    void SetDstReg(Register reg)
    {
        dstReg_ = reg;
    }

    uint32_t GetVN() const
    {
        return vn_;
    }

    void SetVN(uint32_t vn)
    {
        vn_ = vn;
    }
    void Dump(std::ostream *out, bool newLine = true) const;
    PANDA_PUBLIC_API virtual bool DumpInputs(std::ostream *out) const;
    PANDA_PUBLIC_API virtual void DumpOpcode(std::ostream *out) const;
    void DumpBytecode(std::ostream *out) const;

#ifdef PANDA_COMPILER_DEBUG_INFO
    void DumpSourceLine(std::ostream *out) const;
#endif  // PANDA_COMPILER_DEBUG_INFO

    virtual void SetDstReg([[maybe_unused]] unsigned index, Register reg)
    {
        ASSERT(index == 0);
        SetDstReg(reg);
    }

    virtual Register GetDstReg([[maybe_unused]] unsigned index) const
    {
        ASSERT(index == 0);
        return GetDstReg();
    }

    virtual size_t GetDstCount() const
    {
        return 1;
    }

    virtual uint32_t GetSrcRegIndex() const
    {
        return 0;
    }

    virtual void SetSrcReg([[maybe_unused]] unsigned index, [[maybe_unused]] Register reg) {}

    virtual Register GetSrcReg([[maybe_unused]] unsigned index) const
    {
        return GetInvalidReg();
    }

    User *GetFirstUser() const
    {
        return firstUser_;
    }

#ifdef PANDA_COMPILER_DEBUG_INFO
    InstDebugInfo *GetDebugInfo() const
    {
        return debugInfo_;
    }

    void SetDebugInfo(InstDebugInfo *info)
    {
        debugInfo_ = info;
    }

    RuntimeInterface::MethodPtr GetCurrentMethod() const
    {
        return currentMethod_;
    }

    void SetCurrentMethod(RuntimeInterface::MethodPtr currentMethod)
    {
        currentMethod_ = currentMethod;
    }
#endif

    using Initializer = std::tuple<Opcode, DataType::Type, uint32_t>;

protected:
    using InstBase::InstBase;
    static constexpr int INPUT_COUNT = 0;

    Inst() = default;

    explicit Inst(Opcode opcode) : Inst(Initializer {opcode, DataType::Type::NO_TYPE, INVALID_PC}) {}

    explicit Inst(Initializer t) : pc_(std::get<uint32_t>(t)), opcode_(std::get<Opcode>(t))
    {
        bitFields_ = inst_flags::GetFlagsMask(opcode_);
        SetField<FieldType>(std::get<DataType::Type>(t));
    }

protected:
    using FieldFlags = BitField<uint64_t, 0, MinimumBitsToStore(1ULL << inst_flags::FLAGS_COUNT)>;
    using FieldType = FieldFlags::NextField<DataType::Type, MinimumBitsToStore(DataType::LAST)>;
    using InputsCount = FieldType::NextField<uint32_t, BITS_PER_INPUTS_NUM>;
    using LastField = InputsCount;

    DynamicOperands *GetDynamicOperands() const
    {
        return reinterpret_cast<DynamicOperands *>(reinterpret_cast<uintptr_t>(this) - sizeof(DynamicOperands));
    }

    void AppendOpcodeAndTypeToVnObject(VnObject *vnObj) const;
    void AppendInputsToVnObject(VnObject *vnObj) const;

private:
    template <typename InstType, typename... Args>
    static InstType *ConstructInst(InstType *ptr, ArenaAllocator *allocator, Args &&...args);

    User *GetUser(unsigned index)
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            return GetDynamicOperands()->GetUser(index);
        }
        auto inputsCount {GetField<InputsCount>()};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<User *>(reinterpret_cast<Input *>(this) -
                                        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                        (inputsCount + Input::GetPadding(RUNTIME_ARCH, inputsCount))) -
               // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
               index - 1;  // CC-OFF(G.FMT.02) project code style
    }

    size_t OperandsStorageSize() const
    {
        if (UNLIKELY(IsOperandsDynamic())) {
            return sizeof(DynamicOperands);
        }

        auto inputsCount {GetField<InputsCount>()};
        return inputsCount * (sizeof(Input) + sizeof(User)) +
               Input::GetPadding(RUNTIME_ARCH, inputsCount) * sizeof(Input);
    }

private:
    /// Basic block this instruction belongs to
    BasicBlock *bb_ {nullptr};

#ifdef PANDA_COMPILER_DEBUG_INFO
    InstDebugInfo *debugInfo_ {nullptr};
    RuntimeInterface::MethodPtr currentMethod_ {nullptr};
#endif

    /// Next instruction within basic block
    Inst *next_ {nullptr};

    /// Previous instruction within basic block
    Inst *prev_ {nullptr};

    /// First user in users chain
    User *firstUser_ {nullptr};

    /// This value hold properties of the instruction. It accessed via BitField types(f.e. FieldType).
    uint64_t bitFields_ {0};

    /// Unique id of instruction
    uint32_t id_ {INVALID_ID};

    /// Unique id of instruction
    uint32_t vn_ {INVALID_VN};

    /// Bytecode pc
    uint32_t pc_ {INVALID_PC};

    /// Number used in cloning
    uint32_t cloneNumber_ {INVALID_ID};

    /// Instruction number getting while visiting graph
    LinearNumber linearNumber_ {INVALID_LINEAR_NUM};

    ObjectTypeInfo objectTypeInfo_ {};

    /// Opcode, see opcodes.def
    Opcode opcode_ {Opcode::INVALID};

    // Destination register type - defined in FieldType
    Register dstReg_ {GetInvalidReg()};
};

/**
 * Proxy class that injects new field - type of the source operands - into property field of the instruction.
 * Should be used when instruction has sources of the same type and type of the instruction is not match to type of
 * sources. Examples: Cmp, Compare
 * @tparam T Base instruction class after which this mixin is injected
 */
template <typename T>
class InstWithOperandsType : public T {
public:
    using T::T;

    void SetOperandsType(DataType::Type type)
    {
        T::template SetField<FieldOperandsType>(type);
    }
    virtual DataType::Type GetOperandsType() const
    {
        return T::template GetField<FieldOperandsType>();
    }

protected:
    using FieldOperandsType =
        typename T::LastField::template NextField<DataType::Type, MinimumBitsToStore(DataType::LAST)>;
    using LastField = FieldOperandsType;
};

/**
 * Mixin for NeedBarrier flag.
 * @tparam T Base instruction class after which this mixin is injected
 */
template <typename T>
class NeedBarrierMixin : public T {
public:
    using T::T;

    void SetNeedBarrier(bool v)
    {
        T::template SetField<NeedBarrierFlag>(v);
    }
    bool GetNeedBarrier() const
    {
        return T::template GetField<NeedBarrierFlag>();
    }

protected:
    using NeedBarrierFlag = typename T::LastField::NextFlag;
    using LastField = NeedBarrierFlag;
};

enum class DynObjectAccessType {
    UNKNOWN = 0,  // corresponds by value semantic
    BY_NAME = 1,
    BY_INDEX = 2,
    LAST = BY_INDEX
};

enum class DynObjectAccessMode {
    UNKNOWN = 0,
    DICTIONARY = 1,
    ARRAY = 2,
    LAST = ARRAY,
};

/**
 * Mixin for dynamic object access properties.
 * @tparam T Base instruction class after which this mixin is injected
 */
template <typename T>
class DynObjectAccessMixin : public T {
public:
    using T::T;
    using Type = DynObjectAccessType;
    using Mode = DynObjectAccessMode;

    void SetAccessType(Type type)
    {
        T::template SetField<AccessType>(type);
    }

    Type GetAccessType() const
    {
        return T::template GetField<AccessType>();
    }

    void SetAccessMode(Mode mode)
    {
        T::template SetField<AccessMode>(mode);
    }

    Mode GetAccessMode() const
    {
        return T::template GetField<AccessMode>();
    }

protected:
    using AccessType = typename T::LastField::template NextField<Type, MinimumBitsToStore(Type::LAST)>;
    using AccessMode = typename AccessType::template NextField<Mode, MinimumBitsToStore(Mode::LAST)>;
    using LastField = AccessMode;
};

enum ObjectType {
    MEM_OBJECT = 0,
    MEM_STATIC,
    MEM_DYN_GLOBAL,
    MEM_DYN_INLINED,
    MEM_DYN_PROPS,
    MEM_DYN_ELEMENTS,
    MEM_DYN_CLASS,
    MEM_DYN_HCLASS,
    MEM_DYN_METHOD,
    MEM_DYN_PROTO_HOLDER,
    MEM_DYN_PROTO_CELL,
    MEM_DYN_CHANGE_FIELD,
    MEM_DYN_ARRAY_LENGTH,
    LAST = MEM_DYN_ARRAY_LENGTH
};

inline const char *ObjectTypeToString(ObjectType objType)
{
    static constexpr auto COUNT = static_cast<uint8_t>(ObjectType::LAST) + 1;
    static constexpr std::array<const char *, COUNT> OBJ_TYPE_NAMES = {
        "Object", "Static", "GlobalVar",        "Dynamic inlined", "Properties",     "Elements", "Class",
        "Hclass", "Method", "Prototype holder", "Prototype cell",  "IsChangeFieald", "Length"};
    auto idx = static_cast<uint8_t>(objType);
    ASSERT(idx <= LAST);
    return OBJ_TYPE_NAMES[idx];
}

/// This mixin aims to implement type id accessors.
class TypeIdMixin {
public:
    static constexpr uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();
    static constexpr uint32_t MEM_PROMISE_CLASS_ID = RuntimeInterface::MEM_PROMISE_CLASS_ID;
    static constexpr uint32_t MEM_DYN_CLASS_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_CLASS;
    static constexpr uint32_t MEM_DYN_HCLASS_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_HCLASS;
    static constexpr uint32_t MEM_DYN_METHOD_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_METHOD;
    static constexpr uint32_t MEM_DYN_PROTO_HOLDER_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_PROTO_HOLDER;
    static constexpr uint32_t MEM_DYN_PROTO_CELL_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_PROTO_CELL;
    static constexpr uint32_t MEM_DYN_CHANGE_FIELD_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_CHANGE_FIELD;
    static constexpr uint32_t MEM_DYN_GLOBAL_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_GLOBAL;
    static constexpr uint32_t MEM_DYN_PROPS_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_PROPS;
    static constexpr uint32_t MEM_DYN_ELEMENTS_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_ELEMENTS;
    static constexpr uint32_t MEM_DYN_ARRAY_LENGTH_ID = MEM_PROMISE_CLASS_ID - ObjectType::MEM_DYN_ARRAY_LENGTH;

    TypeIdMixin(uint32_t typeId, RuntimeInterface::MethodPtr method) : typeId_(typeId), method_(method) {}

    TypeIdMixin() = default;
    DEFAULT_COPY_SEMANTIC(TypeIdMixin);
    DEFAULT_MOVE_SEMANTIC(TypeIdMixin);
    virtual ~TypeIdMixin() = default;

    void SetTypeId(uint32_t id)
    {
        typeId_ = id;
    }

    auto GetTypeId() const
    {
        return typeId_;
    }

    void SetMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }
    auto GetMethod() const
    {
        return method_;
    }

protected:
    void AppendTypeIdAndMethodToVnObject(VnObject *vnObj) const;

private:
    uint32_t typeId_ {0};
    // The pointer to the method in which this instruction is executed(inlined method)
    RuntimeInterface::MethodPtr method_ {nullptr};
};

/// This mixin aims to implement two type id accessors.
class TypeIdMixin2 : public TypeIdMixin {
    using Base = TypeIdMixin;
    using Base::Base;

public:
    TypeIdMixin2(uint32_t typeId0, uint32_t typeId1, RuntimeInterface::MethodPtr method)
        : TypeIdMixin(typeId1, method), typeId0_(typeId0)
    {
    }

    TypeIdMixin2() = default;
    NO_COPY_SEMANTIC(TypeIdMixin2);
    NO_MOVE_SEMANTIC(TypeIdMixin2);
    ~TypeIdMixin2() override = default;

    void SetTypeId0(uint32_t id)
    {
        typeId0_ = id;
    }
    auto GetTypeId0() const
    {
        return typeId0_;
    }
    void SetTypeId1(uint32_t id)
    {
        SetTypeId(id);
    }
    auto GetTypeId1() const
    {
        return GetTypeId();
    }

private:
    uint32_t typeId0_ {0};
};

/// This mixin aims to implement type of klass.
template <typename T>
class ClassTypeMixin : public T {
public:
    using T::T;

    void SetClassType(ClassType classType)
    {
        T::template SetField<ClassTypeField>(classType);
    }

    ClassType GetClassType() const
    {
        return T::template GetField<ClassTypeField>();
    }

protected:
    using ClassTypeField = typename T::LastField::template NextField<ClassType, MinimumBitsToStore(ClassType::COUNT)>;
    using LastField = ClassTypeField;
};

/// Mixin to check if null check inside CheckCast and IsInstance can be omitted.
template <typename T>
class OmitNullCheckMixin : public T {
public:
    using T::T;

    void SetOmitNullCheck(bool omitNullCheck)
    {
        T::template SetField<OmitNullCheckFlag>(omitNullCheck);
    }

    bool GetOmitNullCheck() const
    {
        return T::template GetField<OmitNullCheckFlag>();
    }

protected:
    using OmitNullCheckFlag = typename T::LastField::NextFlag;
    using LastField = OmitNullCheckFlag;
};

template <typename T>
class ScaleMixin : public T {
public:
    using T::T;

    void SetScale(uint32_t scale)
    {
        T::template SetField<ScaleField>(scale);
    }

    uint32_t GetScale() const
    {
        return T::template GetField<ScaleField>();
    }

protected:
    using ScaleField = typename T::LastField::template NextField<uint32_t, MinimumBitsToStore(MAX_SCALE)>;
    using LastField = ScaleField;
};

template <typename T>
class ObjectTypeMixin : public T {
public:
    using T::T;

    void SetObjectType(ObjectType type)
    {
        T::template SetField<ObjectTypeField>(type);
    }

    ObjectType GetObjectType() const
    {
        return T::template GetField<ObjectTypeField>();
    }

protected:
    using ObjectTypeField = typename T::LastField::template NextField<ObjectType, MinimumBitsToStore(ObjectType::LAST)>;
    using LastField = ObjectTypeField;
};

/// This mixin aims to implement field accessors.
class FieldMixin {
public:
    explicit FieldMixin(RuntimeInterface::FieldPtr field) : field_(field) {}

    FieldMixin() = default;
    NO_COPY_SEMANTIC(FieldMixin);
    NO_MOVE_SEMANTIC(FieldMixin);
    virtual ~FieldMixin() = default;

    void SetObjField(RuntimeInterface::FieldPtr field)
    {
        field_ = field;
    }
    auto GetObjField() const
    {
        return field_;
    }

protected:
    void AppendObjFieldToVnObject(VnObject *vnObj) const;

private:
    RuntimeInterface::FieldPtr field_ {nullptr};
};

/// This mixin aims to implement 2 field accessors.
class FieldMixin2 {
public:
    explicit FieldMixin2(RuntimeInterface::FieldPtr field0, RuntimeInterface::FieldPtr field1)
        : field0_(field0), field1_(field1)
    {
    }

    FieldMixin2() = default;
    NO_COPY_SEMANTIC(FieldMixin2);
    NO_MOVE_SEMANTIC(FieldMixin2);
    virtual ~FieldMixin2() = default;

    void SetObjField0(RuntimeInterface::FieldPtr field)
    {
        field0_ = field;
    }
    auto GetObjField0() const
    {
        return field0_;
    }
    void SetObjField1(RuntimeInterface::FieldPtr field)
    {
        field1_ = field;
    }
    auto GetObjField1() const
    {
        return field1_;
    }

private:
    RuntimeInterface::FieldPtr field0_ {nullptr};
    RuntimeInterface::FieldPtr field1_ {nullptr};
};

/// This mixin aims to implement volatile accessors.
template <typename T>
class VolatileMixin : public T {
public:
    using T::T;

    void SetVolatile(bool isVolatile)
    {
        T::template SetField<IsVolatileFlag>(isVolatile);
    }
    bool GetVolatile() const
    {
        return T::template GetField<IsVolatileFlag>();
    }

protected:
    using IsVolatileFlag = typename T::LastField::NextFlag;
    using LastField = IsVolatileFlag;
};
/// Mixin for Inlined calls/returns.
template <typename T>
class InlinedInstMixin : public T {
public:
    using T::T;

    void SetInlined(bool v)
    {
        T::template SetField<IsInlinedFlag>(v);
    }
    bool IsInlined() const
    {
        return T::template GetField<IsInlinedFlag>();
    }

protected:
    using IsInlinedFlag = typename T::LastField::NextFlag;
    using LastField = IsInlinedFlag;
};

/// Mixin for Array/String instruction
template <typename T>
class ArrayInstMixin : public T {
public:
    using T::T;

    void SetIsArray(bool v)
    {
        T::template SetField<IsStringFlag>(!v);
    }

    void SetIsString(bool v)
    {
        T::template SetField<IsStringFlag>(v);
    }

    bool IsArray() const
    {
        return !(T::template GetField<IsStringFlag>());
    }

    bool IsString() const
    {
        return T::template GetField<IsStringFlag>();
    }

protected:
    using IsStringFlag = typename T::LastField::NextFlag;
    using LastField = IsStringFlag;
};

/// Mixin for instructions with immediate constant value
class ImmediateMixin {
public:
    explicit ImmediateMixin(uint64_t immediate) : immediate_(immediate) {}

    NO_COPY_SEMANTIC(ImmediateMixin);
    NO_MOVE_SEMANTIC(ImmediateMixin);
    virtual ~ImmediateMixin() = default;

    void SetImm(uint64_t immediate)
    {
        immediate_ = immediate;
    }
    auto GetImm() const
    {
        return immediate_;
    }

protected:
    ImmediateMixin() = default;

private:
    uint64_t immediate_ {0};
};

/// Mixin for instructions with ConditionCode
template <typename T>
class ConditionMixin : public T {
public:
    enum class Prediction { NONE = 0, LIKELY, UNLIKELY, SIZE = UNLIKELY };

    using T::T;
    explicit ConditionMixin(ConditionCode cc)
    {
        T::template SetField<CcFlag>(cc);
    }
    NO_COPY_SEMANTIC(ConditionMixin);
    NO_MOVE_SEMANTIC(ConditionMixin);
    ~ConditionMixin() override = default;

    auto GetCc() const
    {
        return T::template GetField<CcFlag>();
    }
    void SetCc(ConditionCode cc)
    {
        T::template SetField<CcFlag>(cc);
    }
    void InverseConditionCode()
    {
        SetCc(GetInverseConditionCode(GetCc()));
        if (IsLikely()) {
            SetUnlikely();
        } else if (IsUnlikely()) {
            SetLikely();
        }
    }

    bool IsLikely() const
    {
        return T::template GetField<PredictionFlag>() == Prediction::LIKELY;
    }
    bool IsUnlikely() const
    {
        return T::template GetField<PredictionFlag>() == Prediction::UNLIKELY;
    }
    void SetLikely()
    {
        T::template SetField<PredictionFlag>(Prediction::LIKELY);
    }
    void SetUnlikely()
    {
        T::template SetField<PredictionFlag>(Prediction::UNLIKELY);
    }

protected:
    ConditionMixin() = default;

    using CcFlag = typename T::LastField::template NextField<ConditionCode, MinimumBitsToStore(ConditionCode::CC_LAST)>;
    using PredictionFlag = typename CcFlag::template NextField<Prediction, MinimumBitsToStore(Prediction::SIZE)>;
    using LastField = PredictionFlag;
};

template <typename T>
class SelectTransformTypeMixin : public T {
public:
    using T::T;
    explicit SelectTransformTypeMixin(SelectTransformType type)
    {
        SetSelectTransformType(type);
    }
    NO_COPY_SEMANTIC(SelectTransformTypeMixin);
    NO_MOVE_SEMANTIC(SelectTransformTypeMixin);
    ~SelectTransformTypeMixin() override = default;

    SelectTransformType GetSelectTransformType() const
    {
        return T::template GetField<SelectTransformTypeFlag>();
    }

    void SetSelectTransformType(SelectTransformType type)
    {
        T::template SetField<SelectTransformTypeFlag>(type);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto *clone = T::Clone(targetGraph);
        static_cast<SelectTransformTypeMixin *>(clone)->SetSelectTransformType(this->GetSelectTransformType());
        return clone;
    }

protected:
    SelectTransformTypeMixin() = default;

    using SelectTransformTypeFlag =
        typename T::LastField::template NextField<SelectTransformType, MinimumBitsToStore(SelectTransformType::LAST)>;
    using LastField = SelectTransformType;
};

/// Mixin for instrucion with ShiftType
class ShiftTypeMixin {
public:
    explicit ShiftTypeMixin(ShiftType shiftType) : shiftType_(shiftType) {}
    NO_COPY_SEMANTIC(ShiftTypeMixin);
    NO_MOVE_SEMANTIC(ShiftTypeMixin);
    virtual ~ShiftTypeMixin() = default;

    void SetShiftType(ShiftType shiftType)
    {
        shiftType_ = shiftType;
    }

    ShiftType GetShiftType() const
    {
        return shiftType_;
    }

protected:
    ShiftTypeMixin() = default;

private:
    ShiftType shiftType_ {INVALID_SHIFT};
};

/// Mixin for instructions with multiple return values
template <typename T, size_t N>
class MultipleOutputMixin : public T {
public:
    using T::T;

    Register GetDstReg(unsigned index) const override
    {
        ASSERT(index < N);
        if (index == 0) {
            return T::GetDstReg();
        }
        return dstRegs_[index - 1];
    }

    Location GetDstLocation(unsigned index) const override
    {
        ASSERT(index < N);
        if (index == 0) {
            return Location::MakeRegister(T::GetDstReg());
        }
        return Location::MakeRegister(dstRegs_[index - 1]);
    }

    void SetDstReg(unsigned index, Register reg) override
    {
        ASSERT(index < N);
        if (index == 0) {
            T::SetDstReg(reg);
        } else {
            dstRegs_[index - 1] = reg;
        }
    }

    size_t GetDstCount() const override
    {
        return N;
    }

private:
    std::array<Register, N - 1> dstRegs_;
};

/**
 * Instruction with fixed number of inputs.
 * Shall not be instantiated directly, only through derived classes.
 */
template <size_t N>
class FixedInputsInst : public Inst {
public:
    using Inst::Inst;

    static constexpr int INPUT_COUNT = N;

    using InputsArray = std::array<Inst *, INPUT_COUNT>;
    struct Initializer {
        Inst::Initializer base;
        InputsArray inputs;
    };

    explicit FixedInputsInst(Initializer t) : Inst(t.base)
    {
        SetField<InputsCount>(INPUT_COUNT);
        for (size_t i = 0; i < t.inputs.size(); i++) {
            SetInput(i, t.inputs[i]);
        }
    }

    template <typename... Inputs, typename = Inst::CheckBase<Inputs...>>
    explicit FixedInputsInst(Inst::Initializer t, Inputs... inputs) : FixedInputsInst(Initializer {t, {inputs...}})
    {
    }

    explicit FixedInputsInst(Opcode opcode) : Inst(opcode)
    {
        SetField<InputsCount>(INPUT_COUNT);
    }
    FixedInputsInst(Opcode opcode, DataType::Type type) : FixedInputsInst({opcode, type, INVALID_PC}) {}

    void SetSrcReg(unsigned index, Register reg) override
    {
        ASSERT(index < N);
        srcRegs_[index] = reg;
    }

    Register GetSrcReg(unsigned index) const override
    {
        ASSERT(index < N);
        return srcRegs_[index];
    }

    Location GetLocation(size_t index) const override
    {
        return Location::MakeRegister(GetSrcReg(index), GetInputType(index));
    }

    void SetLocation(size_t index, Location location) override
    {
        SetSrcReg(index, location.GetValue());
    }

    void SetDstLocation(Location location)
    {
        SetDstReg(location.GetValue());
    }

    void SetTmpLocation(Location location) override
    {
        tmpLocation_ = location;
    }

    Location GetTmpLocation() const override
    {
        return tmpLocation_;
    }

    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override;

private:
    template <typename T, std::size_t... IS>
    constexpr auto CreateArray(T value, [[maybe_unused]] std::index_sequence<IS...> unused)
    {
        return std::array<T, sizeof...(IS)> {(static_cast<void>(IS), value)...};
    }

private:
    std::array<Register, N> srcRegs_ = CreateArray(GetInvalidReg(), std::make_index_sequence<INPUT_COUNT>());
    Location tmpLocation_ {};
};

using FixedInputsInst0 = FixedInputsInst<0>;
using FixedInputsInst1 = FixedInputsInst<1>;
using FixedInputsInst2 = FixedInputsInst<2U>;
using FixedInputsInst3 = FixedInputsInst<3U>;
using FixedInputsInst4 = FixedInputsInst<4U>;

/// Instruction with variable inputs count
class DynamicInputsInst : public Inst {
public:
    using Inst::Inst;

    static constexpr int INPUT_COUNT = MAX_STATIC_INPUTS;

    Location GetLocation(size_t index) const override
    {
        if (locations_ == nullptr) {
            return Location::Invalid();
        }
        return locations_->GetLocation(index);
    }

    Location GetDstLocation() const override
    {
        if (locations_ == nullptr) {
            return Location::Invalid();
        }
        return locations_->GetDstLocation();
    }

    Location GetTmpLocation() const override
    {
        if (locations_ == nullptr) {
            return Location::Invalid();
        }
        return locations_->GetTmpLocation();
    }

    void SetLocation(size_t index, Location location) override
    {
        ASSERT(locations_ != nullptr);
        locations_->SetLocation(index, location);
    }

    void SetDstLocation(Location location)
    {
        ASSERT(locations_ != nullptr);
        locations_->SetDstLocation(location);
    }

    void SetTmpLocation(Location location) override
    {
        ASSERT(locations_ != nullptr);
        locations_->SetTmpLocation(location);
    }

    void SetLocationsInfo(LocationsInfo *info)
    {
        locations_ = info;
    }

    Register GetSrcReg(unsigned index) const override
    {
        return GetLocation(index).GetValue();
    }

    void SetSrcReg(unsigned index, Register reg) override
    {
        SetLocation(index, Location::MakeRegister(reg, GetInputType(index)));
    }

private:
    LocationsInfo *locations_ {nullptr};
};

/// Unary operation instruction
class PANDA_PUBLIC_API UnaryOperation : public FixedInputsInst<1> {
public:
    using FixedInputsInst::FixedInputsInst;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (GetOpcode() == Opcode::Cast) {
            return GetInput(0).GetInst()->GetType();
        }
        return GetType();
    }

    bool IsSafeInst() const override
    {
        return true;
    }

    void SetVnObject(VnObject *vnObj) const override;

    Inst *Evaluate();
};

/// Binary operation instruction
class BinaryOperation : public FixedInputsInst<2U> {
public:
    using FixedInputsInst::FixedInputsInst;

    uint32_t Latency() const override
    {
        if (GetOpcode() == Opcode::Div) {
            return g_options.GetCompilerSchedLatencyLong();
        }
        return g_options.GetCompilerSchedLatency();
    }

    bool IsSafeInst() const override
    {
        return true;
    }

    bool IsBinaryInst() const override
    {
        return true;
    }

    Inst *Evaluate();

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        auto inputType = GetInput(index).GetInst()->GetType();
        auto type = GetType();
        if (GetOpcode() == Opcode::Sub && !DataType::IsTypeSigned(type)) {
            ASSERT(GetCommonType(inputType) == GetCommonType(type) || inputType == DataType::POINTER ||
                   DataType::GetCommonType(inputType) == DataType::INT64);
            return inputType;
        }
        if (GetOpcode() == Opcode::Add && type == DataType::POINTER) {
            ASSERT(GetCommonType(inputType) == GetCommonType(type) ||
                   DataType::GetCommonType(inputType) == DataType::INT64);
            return inputType;
        }
        return GetType();
    }
};

/// Binary operation instruction with c immidiate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API BinaryImmOperation : public FixedInputsInst<1>, public ImmediateMixin {
public:
    using FixedInputsInst::FixedInputsInst;

    BinaryImmOperation(Initializer t, uint64_t imm) : FixedInputsInst(std::move(t)), ImmediateMixin(imm) {}

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        auto inputType = GetInput(index).GetInst()->GetType();
        auto type = GetType();
        if (GetOpcode() == Opcode::SubI && !DataType::IsTypeSigned(type)) {
            ASSERT(GetCommonType(inputType) == GetCommonType(type) || inputType == DataType::POINTER);
            return inputType;
        }
        if (GetOpcode() == Opcode::AddI && type == DataType::POINTER) {
            ASSERT(DataType::GetCommonType(inputType) == DataType::GetCommonType(GetType()) ||
                   (inputType == DataType::REFERENCE && GetType() == DataType::POINTER));
            return inputType;
        }
        return GetType();
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    bool IsSafeInst() const override
    {
        return true;
    }

    bool IsBinaryImmInst() const override
    {
        return true;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<BinaryImmOperation *>(clone)->SetImm(GetImm());
        return clone;
    }
};

/// Unary operation that shifts its own operand prior the application.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API UnaryShiftedRegisterOperation : public FixedInputsInst<1>,
                                                       public ImmediateMixin,
                                                       public ShiftTypeMixin {
public:
    using FixedInputsInst::FixedInputsInst;

    UnaryShiftedRegisterOperation(Initializer t, uint64_t imm, ShiftType shiftType)
        : FixedInputsInst(std::move(t)), ImmediateMixin(imm), ShiftTypeMixin(shiftType)
    {
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;
    Inst *Clone(const Graph *targetGraph) const override;
};

/// Binary operation that shifts its second operand prior the application.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API BinaryShiftedRegisterOperation : public FixedInputsInst<2U>,
                                                        public ImmediateMixin,
                                                        public ShiftTypeMixin {
public:
    using FixedInputsInst::FixedInputsInst;

    BinaryShiftedRegisterOperation(Initializer t, uint64_t imm, ShiftType shiftType)
        : FixedInputsInst(std::move(t)), ImmediateMixin(imm), ShiftTypeMixin(shiftType)
    {
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;
    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override;
};

class PANDA_PUBLIC_API ExtractBitfieldInst : public FixedInputsInst<1> {
public:
    using FixedInputsInst::FixedInputsInst;

    ExtractBitfieldInst(Initializer t, unsigned sourceBit, unsigned width, bool signExt = false)
        : FixedInputsInst(std::move(t)), sourceBit_(sourceBit), width_(width), signExt_(signExt)
    {
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    void SetSourceBit(unsigned sourceBit)
    {
        sourceBit_ = sourceBit;
    }

    void SetWidth(unsigned width)
    {
        ASSERT(width > 0);
        width_ = width;
    }

    ExtractBitfieldInst &AsSignBitExtension()
    {
        signExt_ = true;
        return *this;
    }

    ExtractBitfieldInst &AsZeroExtension()
    {
        signExt_ = false;
        return *this;
    }

    unsigned GetSourceBit() const
    {
        return sourceBit_;
    }

    unsigned GetWidth() const
    {
        return width_;
    }

    bool IsSignBitExtension() const
    {
        return signExt_;
    }

    bool IsZeroExtension() const
    {
        return !signExt_;
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

private:
    unsigned sourceBit_ {0};
    unsigned width_ {0};
    bool signExt_ {false};
};

class SpillFillInst;

/// Mixin to hold location data
class LocationDataMixin {
public:
    void SetLocationData(SpillFillData locationData)
    {
        locationData_ = locationData;
    }

    auto GetLocationData() const
    {
        return locationData_;
    }

    auto &GetLocationData()
    {
        return locationData_;
    }

protected:
    LocationDataMixin() = default;
    NO_COPY_SEMANTIC(LocationDataMixin);
    NO_MOVE_SEMANTIC(LocationDataMixin);
    virtual ~LocationDataMixin() = default;

private:
    SpillFillData locationData_ {};
};

/// Mixin to hold input types of call instruction
template <typename T>
class InputTypesMixin : public T {
public:
    using Base = T;
    using Base::AppendInput;
    using Base::Base;

    void AllocateInputTypes(ArenaAllocator *allocator, size_t capacity)
    {
        ASSERT(allocator != nullptr);
        ASSERT(inputTypes_ == nullptr);
        inputTypes_ = allocator->New<ArenaVector<DataType::Type>>(allocator->Adapter());
        ASSERT(inputTypes_ != nullptr);
        inputTypes_->reserve(capacity);
        ASSERT(inputTypes_->capacity() >= capacity);
    }
    void AddInputType(DataType::Type type)
    {
        ASSERT(inputTypes_ != nullptr);
        inputTypes_->push_back(type);
    }
    void SetInputType(unsigned index, DataType::Type type)
    {
        ASSERT(inputTypes_ != nullptr);
        (*inputTypes_)[index] = type;
    }
    ArenaVector<DataType::Type> *GetInputTypes()
    {
        return inputTypes_;
    }
    void CloneTypes(ArenaAllocator *allocator, InputTypesMixin<T> *targetInst) const
    {
        if (UNLIKELY(inputTypes_ == nullptr)) {
            return;
        }
        targetInst->AllocateInputTypes(allocator, inputTypes_->size());
        for (auto inputType : *inputTypes_) {
            targetInst->AddInputType(inputType);
        }
    }
    void AppendInputs(const std::initializer_list<std::pair<Inst *, DataType::Type>> &inputs)
    {
        static_assert(std::is_base_of_v<DynamicInputsInst, T>);
        for (auto [input, type] : inputs) {
            static_cast<Inst *>(this)->AppendInput(input);
            AddInputType(type);
        }
    }

    void AppendInput(Inst *input, DataType::Type type)
    {
        static_assert(std::is_base_of_v<DynamicInputsInst, T>);
        static_cast<Inst *>(this)->AppendInput(input);
        AddInputType(type);
    }

    void SetInputs(ArenaAllocator *allocator, const std::initializer_list<std::pair<Inst *, DataType::Type>> &inputs)
    {
        static_assert(std::is_base_of_v<DynamicInputsInst, T>);
        static_cast<Inst *>(this)->ReserveInputs(inputs.size());
        AllocateInputTypes(allocator, inputs.size());
        AppendInputs(inputs);
    }

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ArenaVector<DataType::Type> *inputTypes_ {nullptr};
};

/// Mixin to hold method data
class MethodDataMixin {
public:
    static constexpr uint32_t INVALID_METHOD_ID = std::numeric_limits<uint32_t>::max();

    MethodDataMixin(uint32_t id, RuntimeInterface::MethodPtr method)
    {
        methodId_ = id;
        method_ = method;
    }

    void SetCallMethodId(uint32_t id)
    {
        methodId_ = id;
    }
    auto GetCallMethodId() const
    {
        return methodId_;
    }
    void SetCallMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }
    RuntimeInterface::MethodPtr GetCallMethod() const
    {
        return method_;
    }
    void SetFunctionObject(uintptr_t func)
    {
        function_ = func;
    }
    auto GetFunctionObject() const
    {
        return function_;
    }

protected:
    MethodDataMixin() = default;
    NO_COPY_SEMANTIC(MethodDataMixin);
    NO_MOVE_SEMANTIC(MethodDataMixin);
    virtual ~MethodDataMixin() = default;

    void AppendMethodIdToVnObject(VnObject *vnObj) const;

private:
    uint32_t methodId_ {INVALID_METHOD_ID};
    RuntimeInterface::MethodPtr method_ {nullptr};
    uintptr_t function_ {0};
};

/// ResolveStatic
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API ResolveStaticInst : public FixedInputsInst1, public MethodDataMixin {
public:
    using Base = FixedInputsInst1;
    using Base::Base;

    ResolveStaticInst(Inst::Initializer t, uint32_t methodId, RuntimeInterface::MethodPtr method)
        : Base(std::move(t)), MethodDataMixin(methodId, method)
    {
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index == 0);
        // This is SaveState input
        return DataType::NO_TYPE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        ASSERT(targetGraph != nullptr);
        auto instClone = FixedInputsInst1::Clone(targetGraph);
        auto rslvClone = static_cast<ResolveStaticInst *>(instClone);
        rslvClone->SetCallMethodId(GetCallMethodId());
        rslvClone->SetCallMethod(GetCallMethod());
        return instClone;
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// ResolveVirtual
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API ResolveVirtualInst : public FixedInputsInst2, public MethodDataMixin {
public:
    using Base = FixedInputsInst2;
    using Base::Base;

    ResolveVirtualInst(Inst::Initializer t, uint32_t methodId, RuntimeInterface::MethodPtr method)
        : Base(std::move(t)), MethodDataMixin(methodId, method)
    {
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(GetInputsCount() == 2U && index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                // This is SaveState input
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        ASSERT(targetGraph != nullptr);
        auto instClone = FixedInputsInst2::Clone(targetGraph);
        auto rslvClone = static_cast<ResolveVirtualInst *>(instClone);
        rslvClone->SetCallMethodId(GetCallMethodId());
        rslvClone->SetCallMethod(GetCallMethod());
        return instClone;
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

class PANDA_PUBLIC_API InitStringInst : public FixedInputsInst2 {
public:
    using Base = FixedInputsInst2;
    using Base::Base;

    InitStringInst(Initializer t, StringCtorType ctorType) : Base(std::move(t))
    {
        SetStringCtorType(ctorType);
    }

    bool IsFromString() const
    {
        return GetField<StringCtorTypeField>() == StringCtorType::STRING;
    }
    bool IsFromCharArray() const
    {
        return GetField<StringCtorTypeField>() == StringCtorType::CHAR_ARRAY;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    void SetStringCtorType(StringCtorType ctorType)
    {
        SetField<StringCtorTypeField>(ctorType);
    }

    StringCtorType GetStringCtorType() const
    {
        return GetField<StringCtorTypeField>();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst2::Clone(targetGraph);
        clone->CastToInitString()->SetStringCtorType(GetStringCtorType());
        return clone;
    }

private:
    using StringCtorTypeField = Base::LastField::NextField<StringCtorType, MinimumBitsToStore(StringCtorType::COUNT)>;
    using LastField = StringCtorTypeField;
};

/// Call instruction
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API CallInst : public InlinedInstMixin<InputTypesMixin<DynamicInputsInst>>, public MethodDataMixin {
public:
    using Base = InlinedInstMixin<InputTypesMixin<DynamicInputsInst>>;
    using Base::Base;

    CallInst(Initializer t, uint32_t methodId, RuntimeInterface::MethodPtr method = nullptr)
        : Base(std::move(t)), MethodDataMixin(methodId, method)
    {
    }

    int GetObjectIndex() const
    {
        return GetOpcode() == Opcode::CallResolvedVirtual ? 1 : 0;
    }

    Inst *GetObjectInst()
    {
        return GetInput(GetObjectIndex()).GetInst();
    }

    const Inst *GetObjectInst() const
    {
        return GetInput(GetObjectIndex()).GetInst();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(inputTypes_ != nullptr);
        ASSERT(index < inputTypes_->size());
        ASSERT(index < GetInputsCount());
        return (*inputTypes_)[index];
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    void SetIsNative(bool isNative)
    {
        SetField<IsNativeFlag>(isNative);
    }

    bool GetIsNative() const
    {
        return GetField<IsNativeFlag>();
    }

    void SetCanNativeException(bool isNative)
    {
        SetField<IsNativeExceptionFlag>(isNative);
    }

    bool GetCanNativeException() const
    {
        return GetField<IsNativeExceptionFlag>();
    }

    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override;

    bool IsRuntimeCall() const override
    {
        if (IsNativeApiCall()) {
            // checking runtime_call flag is enough
            return Base::IsRuntimeCall();
        }
        return !IsInlined();
    }

protected:
    using IsNativeFlag = LastField::NextFlag;
    using IsNativeExceptionFlag = IsNativeFlag::NextFlag;
    using LastField = IsNativeExceptionFlag;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API CallIndirectInst : public InputTypesMixin<DynamicInputsInst> {
public:
    using Base = InputTypesMixin<DynamicInputsInst>;
    using Base::Base;

    explicit CallIndirectInst(Initializer t) : Base(std::move(t)) {}

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(inputTypes_ != nullptr);
        ASSERT(index < inputTypes_->size());
        ASSERT(index < GetInputsCount());
        return (*inputTypes_)[index];
    }

    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override;
};

/// Length methods instruction
class LengthMethodInst : public ArrayInstMixin<FixedInputsInst1> {
public:
    using Base = ArrayInstMixin<FixedInputsInst1>;
    using Base::Base;

    explicit LengthMethodInst(Opcode opcode, bool isArray = true) : Base(opcode)
    {
        SetIsArray(isArray);
    }
    explicit LengthMethodInst(Initializer t, bool isArray = true) : Base(std::move(t))
    {
        SetIsArray(isArray);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::REFERENCE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(static_cast<LengthMethodInst *>(clone)->IsArray() == IsArray());
        return clone;
    }
};

/// Compare instruction
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API CompareInst : public InstWithOperandsType<ConditionMixin<FixedInputsInst2>> {
public:
    using BaseInst = InstWithOperandsType<ConditionMixin<FixedInputsInst2>>;
    using BaseInst::BaseInst;

    CompareInst(Initializer t, ConditionCode cc) : BaseInst(std::move(t))
    {
        SetCc(cc);
    }
    CompareInst(Initializer t, DataType::Type operType, ConditionCode cc) : BaseInst(std::move(t))
    {
        SetOperandsType(operType);
        SetCc(cc);
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    void SetVnObject(VnObject *vnObj) const override;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetOperandsType();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToCompare()->GetCc() == GetCc());
        ASSERT(clone->CastToCompare()->GetOperandsType() == GetOperandsType());
        return clone;
    }
};

/// Mixin for AnyTypeMixin instructions
template <typename T>
class AnyTypeMixin : public T {
public:
    using T::T;

    void SetAnyType(AnyBaseType anyType)
    {
        T::template SetField<AnyBaseTypeField>(anyType);
    }

    AnyBaseType GetAnyType() const override
    {
        return T::template GetField<AnyBaseTypeField>();
    }

    bool IsIntegerWasSeen() const
    {
        return T::template GetField<IntegerWasSeen>();
    }

    void SetIsIntegerWasSeen(bool value)
    {
        T::template SetField<IntegerWasSeen>(value);
    }

    bool IsSpecialWasSeen() const
    {
        return T::template GetField<SpecialWasSeen>();
    }

    profiling::AnyInputType GetAllowedInputType() const
    {
        return T::template GetField<AllowedInputType>();
    }

    void SetAllowedInputType(profiling::AnyInputType type)
    {
        T::template SetField<AllowedInputType>(type);
    }

    bool IsTypeWasProfiled() const
    {
        return T::template GetField<TypeWasProfiled>();
    }

    void SetIsTypeWasProfiled(bool value)
    {
        T::template SetField<TypeWasProfiled>(value);
    }

protected:
    using AnyBaseTypeField =
        typename T::LastField::template NextField<AnyBaseType, MinimumBitsToStore(AnyBaseType::COUNT)>;
    using IntegerWasSeen = typename AnyBaseTypeField::NextFlag;
    using SpecialWasSeen = typename IntegerWasSeen::NextFlag;
    using AllowedInputType =
        typename AnyBaseTypeField::template NextField<profiling::AnyInputType,
                                                      MinimumBitsToStore(profiling::AnyInputType::LAST)>;
    static_assert(SpecialWasSeen::END_BIT == AllowedInputType::END_BIT);
    using TypeWasProfiled = typename AllowedInputType::NextFlag;
    using LastField = TypeWasProfiled;
};

/// CompareAnyTypeInst instruction
class PANDA_PUBLIC_API CompareAnyTypeInst : public AnyTypeMixin<FixedInputsInst1> {
public:
    using BaseInst = AnyTypeMixin<FixedInputsInst1>;
    using BaseInst::BaseInst;

    CompareAnyTypeInst(Opcode opcode, uint32_t pc, Inst *input0, AnyBaseType anyType = AnyBaseType::UNDEFINED_TYPE)
        : BaseInst({opcode, DataType::Type::BOOL, pc}, input0)
    {
        SetAnyType(anyType);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetInput(index).GetInst()->GetType();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    void SetVnObject(VnObject *vnObj) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToCompareAnyType()->GetAnyType() == GetAnyType());
        return clone;
    }
};

/// GetAnyTypeName instruction
class PANDA_PUBLIC_API GetAnyTypeNameInst : public AnyTypeMixin<FixedInputsInst0> {
public:
    using BaseInst = AnyTypeMixin<FixedInputsInst0>;
    using BaseInst::BaseInst;

    GetAnyTypeNameInst(Opcode opcode, uint32_t pc, AnyBaseType anyType = AnyBaseType::UNDEFINED_TYPE)
        : BaseInst({opcode, DataType::Type::ANY, pc})
    {
        SetAnyType(anyType);
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    void SetVnObject(VnObject *vnObj) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToGetAnyTypeName()->SetAnyType(GetAnyType());
        return clone;
    }
};

/// CastAnyTypeValueInst instruction
class PANDA_PUBLIC_API CastAnyTypeValueInst : public AnyTypeMixin<FixedInputsInst1> {
public:
    using BaseInst = AnyTypeMixin<FixedInputsInst1>;
    using BaseInst::BaseInst;

    CastAnyTypeValueInst(Opcode opcode, uint32_t pc, Inst *input0, AnyBaseType anyType = AnyBaseType::UNDEFINED_TYPE)
        : BaseInst({opcode, AnyBaseTypeToDataType(anyType), pc}, input0)
    {
        SetAnyType(anyType);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetInput(index).GetInst()->GetType();
    }

    DataType::Type GetDeducedType() const
    {
        return AnyBaseTypeToDataType(GetAnyType());
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToCastAnyTypeValue();
        ASSERT(clone->GetAnyType() == GetAnyType());
        ASSERT(clone->GetType() == GetType());
        return clone;
    }
};

/// CastValueToAnyTypeInst instruction
class PANDA_PUBLIC_API CastValueToAnyTypeInst : public AnyTypeMixin<FixedInputsInst1> {
public:
    using BaseInst = AnyTypeMixin<FixedInputsInst1>;
    using BaseInst::BaseInst;

    CastValueToAnyTypeInst(Opcode opcode, uint32_t pc, AnyBaseType anyType, Inst *input0)
        : BaseInst({opcode, DataType::ANY, pc}, input0)
    {
        SetAnyType(anyType);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetInput(index).GetInst()->GetType();
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToCastValueToAnyType();
        ASSERT(clone->GetAnyType() == GetAnyType());
        ASSERT(clone->GetType() == GetType());
        return clone;
    }
};

/// AnyTypeCheckInst instruction
class PANDA_PUBLIC_API AnyTypeCheckInst : public AnyTypeMixin<FixedInputsInst2> {
public:
    using BaseInst = AnyTypeMixin<FixedInputsInst2>;
    using BaseInst::BaseInst;

    explicit AnyTypeCheckInst(Initializer t, AnyBaseType anyType = AnyBaseType::UNDEFINED_TYPE) : BaseInst(std::move(t))
    {
        SetAnyType(anyType);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return (index == 0) ? DataType::ANY : DataType::NO_TYPE;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToAnyTypeCheck()->GetAnyType() == GetAnyType());
        return clone;
    }

    DeoptimizeType GetDeoptimizeType() const;
};

/// HclassCheck instruction
enum class HclassChecks {
    ALL_CHECKS = 0,
    IS_FUNCTION,
    IS_NOT_CLASS_CONSTRUCTOR,
};
class PANDA_PUBLIC_API HclassCheckInst : public AnyTypeMixin<FixedInputsInst2> {
public:
    using BaseInst = AnyTypeMixin<FixedInputsInst2>;
    using BaseInst::BaseInst;

    explicit HclassCheckInst(Initializer t, AnyBaseType anyType = AnyBaseType::UNDEFINED_TYPE) : BaseInst(std::move(t))
    {
        SetAnyType(anyType);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return (index == 0) ? DataType::REFERENCE : DataType::NO_TYPE;
    }

    void SetCheckIsFunction(bool value)
    {
        SetField<CheckTypeIsFunction>(value);
    }

    bool GetCheckIsFunction() const
    {
        return GetField<CheckTypeIsFunction>();
    }

    void SetCheckFunctionIsNotClassConstructor(bool value = true)
    {
        SetField<CheckFunctionIsNotClassConstructor>(value);
    }

    bool GetCheckFunctionIsNotClassConstructor() const
    {
        return GetField<CheckFunctionIsNotClassConstructor>();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    void ExtendFlags(Inst *inst);

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<HclassCheckInst *>(FixedInputsInst2::Clone(targetGraph));
        clone->SetCheckFunctionIsNotClassConstructor(GetCheckFunctionIsNotClassConstructor());
        clone->SetCheckIsFunction(GetCheckIsFunction());
        return clone;
    }

protected:
    using CheckTypeIsFunction = LastField::NextFlag;
    using CheckFunctionIsNotClassConstructor = CheckTypeIsFunction::NextFlag;
    using LastField = CheckFunctionIsNotClassConstructor;
};

/**
 * ConstantInst represent constant value.
 *
 * Available types: INT64, FLOAT32, FLOAT64, ANY. All integer types are stored as INT64 value.
 * Once type of constant is set, it can't be changed anymore.
 */
class PANDA_PUBLIC_API ConstantInst : public FixedInputsInst<0> {
public:
    using FixedInputsInst::FixedInputsInst;

    template <typename T>
    explicit ConstantInst(Opcode /* unused */, T value, bool supportInt32 = false) : FixedInputsInst(Opcode::Constant)
    {
        ASSERT(GetTypeFromCType<T>() != DataType::NO_TYPE);
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-branch-clone)
        if constexpr (GetTypeFromCType<T>() == DataType::FLOAT64) {
            value_ = bit_cast<uint64_t, double>(value);
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (GetTypeFromCType<T>() == DataType::FLOAT32) {
            value_ = bit_cast<uint32_t, float>(value);
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (GetTypeFromCType<T>() == DataType::ANY) {
            value_ = value.Raw();
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if (GetTypeFromCType<T>(supportInt32) == DataType::INT32) {
            value_ = static_cast<int32_t>(value);
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else {
            value_ = value;
        }

        SetType(GetTypeFromCType<T>(supportInt32));
    }

    bool IsSafeInst() const override
    {
        return true;
    }

    uint64_t GetRawValue() const
    {
        return value_;
    }

    uint32_t GetInt32Value() const
    {
        ASSERT(GetType() == DataType::INT32);
        return static_cast<uint32_t>(value_);
    }

    uint64_t GetInt64Value() const
    {
        ASSERT(GetType() == DataType::INT64);
        return value_;
    }

    uint64_t GetIntValue() const
    {
        ASSERT(GetType() == DataType::INT64 || GetType() == DataType::INT32);
        return value_;
    }

    float GetFloatValue() const
    {
        ASSERT(GetType() == DataType::FLOAT32);
        return bit_cast<float, uint32_t>(static_cast<uint32_t>(value_));
    }

    double GetDoubleValue() const
    {
        ASSERT(GetType() == DataType::FLOAT64);
        return bit_cast<double, uint64_t>(value_);
    }

    ConstantInst *GetNextConst()
    {
        return nextConst_;
    }
    void SetNextConst(ConstantInst *nextConst)
    {
        nextConst_ = nextConst;
    }

    template <typename T>
    static constexpr DataType::Type GetTypeFromCType(bool supportInt32 = false)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-branch-clone)
        if constexpr (std::is_integral_v<T>) {
            if (supportInt32 && sizeof(T) == sizeof(uint32_t)) {
                return DataType::INT32;
            }
            return DataType::INT64;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (std::is_same_v<T, float>) {
            return DataType::FLOAT32;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (std::is_same_v<T, double>) {
            return DataType::FLOAT64;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (std::is_same_v<T, DataType::Any>) {
            return DataType::ANY;
        }
        return DataType::NO_TYPE;
    }

    inline bool IsEqualConst(double value, [[maybe_unused]] bool supportInt32 = false)
    {
        // Special compare for NaN
        if (GetType() == DataType::FLOAT64 && std::isnan(value) && std::isnan(bit_cast<double, uint64_t>(value_))) {
            return true;
        }
        return IsEqualConst(DataType::FLOAT64, bit_cast<uint64_t, double>(value));
    }
    inline bool IsEqualConst(float value, [[maybe_unused]] bool supportInt32 = false)
    {
        // Special compare for NaN
        if (GetType() == DataType::FLOAT32 && std::isnan(value) && std::isnan(bit_cast<float, uint32_t>(value_))) {
            return true;
        }
        return IsEqualConst(DataType::FLOAT32, bit_cast<uint32_t, float>(value));
    }
    inline bool IsEqualConst(DataType::Any value, [[maybe_unused]] bool supportInt32 = false)
    {
        return IsEqualConst(DataType::ANY, value.Raw());
    }
    inline bool IsEqualConst(DataType::Type type, uint64_t value)
    {
        return GetType() == type && value_ == value;
    }
    template <typename T>
    inline bool IsEqualConst(T value, bool supportInt32 = false)
    {
        static_assert(GetTypeFromCType<T>() == DataType::INT64);
        if (supportInt32 && sizeof(T) == sizeof(uint32_t)) {
            return (GetType() == DataType::INT32 && static_cast<int32_t>(value_) == static_cast<int32_t>(value));
        }
        return (GetType() == DataType::INT64 && value_ == static_cast<uint64_t>(value));
    }

    inline bool IsEqualConstAllTypes(int64_t value, bool supportInt32 = false)
    {
        return IsEqualConst(value, supportInt32) || IsEqualConst(static_cast<float>(value)) ||
               IsEqualConst(static_cast<double>(value));
    }

    bool IsBoolConst() const override
    {
        ASSERT(IsConst());
        return (GetType() == DataType::INT32 || GetType() == DataType::INT64) &&
               (GetIntValue() == 0 || GetIntValue() == 1);
    }

    bool IsNaNConst() const
    {
        ASSERT(DataType::IsFloatType(GetType()));
        if (GetType() == DataType::FLOAT32) {
            return std::isnan(GetFloatValue());
        }
        // DataType::FLOAT64
        return std::isnan(GetDoubleValue());
    }

    void SetImmTableSlot(ImmTableSlot immSlot)
    {
        immSlot_ = immSlot;
    }

    auto GetImmTableSlot() const
    {
        return immSlot_;
    }

    Location GetDstLocation() const override
    {
        if (GetImmTableSlot() != GetInvalidImmTableSlot()) {
            return Location::MakeConstant(GetImmTableSlot());
        }
        return Inst::GetDstLocation();
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override;

private:
    uint64_t value_ {0};
    ConstantInst *nextConst_ {nullptr};
    ImmTableSlot immSlot_ {GetInvalidImmTableSlot()};
};

// Type describing the purpose of the SpillFillInst.
// RegAlloc may use this information to preserve correct order of several SpillFillInst
// instructions placed along each other in the graph.
enum SpillFillType {
    UNKNOWN,
    INPUT_FILL,
    CONNECT_SPLIT_SIBLINGS,
    SPLIT_MOVE,
};

class PANDA_PUBLIC_API SpillFillInst : public FixedInputsInst0 {
public:
    explicit SpillFillInst(ArenaAllocator *allocator, Opcode opcode, SpillFillType type = UNKNOWN)
        : FixedInputsInst0(opcode), spillFills_(allocator->Adapter()), sfType_(type)
    {
    }

    void AddMove(Register src, Register dst, DataType::Type type)
    {
        AddSpillFill(Location::MakeRegister(src, type), Location::MakeRegister(dst, type), type);
    }

    void AddSpill(Register src, StackSlot dst, DataType::Type type)
    {
        AddSpillFill(Location::MakeRegister(src, type), Location::MakeStackSlot(dst), type);
    }

    void AddFill(StackSlot src, Register dst, DataType::Type type)
    {
        AddSpillFill(Location::MakeStackSlot(src), Location::MakeRegister(dst, type), type);
    }

    void AddMemCopy(StackSlot src, StackSlot dst, DataType::Type type)
    {
        AddSpillFill(Location::MakeStackSlot(src), Location::MakeStackSlot(dst), type);
    }

    void AddSpillFill(const SpillFillData &spillFill)
    {
        spillFills_.emplace_back(spillFill);
    }

    void AddSpillFill(const Location &src, const Location &dst, DataType::Type type)
    {
        spillFills_.emplace_back(SpillFillData {src.GetKind(), dst.GetKind(), src.GetValue(), dst.GetValue(), type});
    }

    const ArenaVector<SpillFillData> &GetSpillFills() const
    {
        return spillFills_;
    }

    ArenaVector<SpillFillData> &GetSpillFills()
    {
        return spillFills_;
    }

    const SpillFillData &GetSpillFill(size_t n) const
    {
        ASSERT(n < spillFills_.size());
        return spillFills_[n];
    }

    SpillFillData &GetSpillFill(size_t n)
    {
        ASSERT(n < spillFills_.size());
        return spillFills_[n];
    }

    void RemoveSpillFill(size_t n)
    {
        ASSERT(n < spillFills_.size());
        spillFills_.erase(spillFills_.begin() + n);
    }

    // Get register number, holded by n-th spill-fill
    Register GetInputReg(size_t n) const
    {
        ASSERT(n < spillFills_.size());
        ASSERT(spillFills_[n].SrcType() == LocationType::REGISTER);
        return spillFills_[n].SrcValue();
    }

    void ClearSpillFills()
    {
        spillFills_.clear();
    }

    SpillFillType GetSpillFillType() const
    {
        return sfType_;
    }

    void SetSpillFillType(SpillFillType type)
    {
        sfType_ = type;
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

#ifndef NDEBUG
    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToSpillFill();
        clone->SetSpillFillType(GetSpillFillType());
        for (auto spillFill : spillFills_) {
            clone->AddSpillFill(spillFill);
        }
        return clone;
    }
#endif

private:
    ArenaVector<SpillFillData> spillFills_;
    SpillFillType sfType_ {UNKNOWN};
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API ParameterInst : public FixedInputsInst<0>, public LocationDataMixin {
public:
    using FixedInputsInst::FixedInputsInst;
    static constexpr uint16_t DYNAMIC_NUM_ARGS = std::numeric_limits<uint16_t>::max();
    static constexpr uint16_t INVALID_ARG_REF_NUM = std::numeric_limits<uint16_t>::max();

    explicit ParameterInst(Opcode /* unused */, uint16_t argNumber, DataType::Type type = DataType::NO_TYPE)
        : FixedInputsInst({Opcode::Parameter, type, INVALID_PC}), argNumber_(argNumber)
    {
    }
    uint16_t GetArgNumber() const
    {
        return argNumber_;
    }

    void SetArgNumber(uint16_t argNumber)
    {
        argNumber_ = argNumber;
    }
    uint16_t GetArgRefNumber() const
    {
        return argRefNumber_;
    }

    void SetArgRefNumber(uint16_t argRefNumber)
    {
        argRefNumber_ = argRefNumber;
    }
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override;

private:
    uint16_t argNumber_ {0};
    uint16_t argRefNumber_ {INVALID_ARG_REF_NUM};
};

inline bool IsZeroConstant(const Inst *inst)
{
    return inst->IsConst() && inst->GetType() == DataType::INT64 && inst->CastToConstant()->GetIntValue() == 0;
}

inline bool IsZeroConstantOrNullPtr(const Inst *inst)
{
    return IsZeroConstant(inst) || inst->GetOpcode() == Opcode::NullPtr;
}

/// Phi instruction
class PANDA_PUBLIC_API PhiInst : public AnyTypeMixin<DynamicInputsInst> {
public:
    using BaseInst = AnyTypeMixin<DynamicInputsInst>;
    using BaseInst::BaseInst;

    explicit PhiInst(Initializer t, AnyBaseType anyType = AnyBaseType::UNDEFINED_TYPE) : BaseInst(std::move(t))
    {
        SetAnyType(anyType);
    }

    /// Get basic block corresponding to given input index. Returned pointer to basic block, can't be nullptr
    BasicBlock *GetPhiInputBb(unsigned index);
    const BasicBlock *GetPhiInputBb(unsigned index) const
    {
        return (const_cast<PhiInst *>(this))->GetPhiInputBb(index);
    }

    uint32_t GetPhiInputBbNum(unsigned index) const
    {
        ASSERT(index < GetInputsCount());
        return GetDynamicOperands()->GetUser(index)->GetBbNum();
    }

    void SetPhiInputBbNum(unsigned index, uint32_t bbNum)
    {
        ASSERT(index < GetInputsCount());
        GetDynamicOperands()->GetUser(index)->SetBbNum(bbNum);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = DynamicInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToPhi()->GetAnyType() == GetAnyType());
        return clone;
    }

    AnyBaseType GetAssumedAnyType() const
    {
        return GetAnyType();
    }

    void SetAssumedAnyType(AnyBaseType type)
    {
        SetAnyType(type);
    }

    /// Get input instruction corresponding to the given basic block, can't be null.
    Inst *GetPhiInput(BasicBlock *bb);
    Inst *GetPhiDataflowInput(BasicBlock *bb);
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    // Get index of the given block in phi inputs
    size_t GetPredBlockIndex(const BasicBlock *block) const;

protected:
    using FlagIsLive = LastField::NextFlag;
    using LastField = FlagIsLive;
};

/**
 * Immediate for SaveState:
 * value - constant value to be stored
 * vreg - virtual register number
 */
struct SaveStateImm {
    uint64_t value;
    uint16_t vreg;
    DataType::Type type;
    VRegType vregType;
};

/**
 * Frame state saving instruction
 * Aims to save pbc registers before calling something that can raise exception
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API SaveStateInst : public DynamicInputsInst {
public:
    using DynamicInputsInst::DynamicInputsInst;

    SaveStateInst(Opcode opcode, uint32_t pc, void *method, CallInst *inst, uint32_t inliningDepth)
        : DynamicInputsInst({opcode, DataType::NO_TYPE, pc}),
          method_(method),
          callerInst_(inst),
          inliningDepth_(inliningDepth)
    {
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    void AppendBridge(Inst *inst)
    {
        ASSERT(inst != nullptr);
        auto newInput = AppendInput(inst);
        SetVirtualRegister(newInput, VirtualRegister(VirtualRegister::BRIDGE, VRegType::VREG));
    }

    void SetVirtualRegister(size_t index, VirtualRegister reg)
    {
        static_assert(sizeof(reg) <= sizeof(uintptr_t), "Consider passing the register by reference");
        ASSERT(index < GetInputsCount());
        GetDynamicOperands()->GetUser(index)->SetVirtualRegister(reg);
    }

    VirtualRegister GetVirtualRegister(size_t index) const
    {
        ASSERT(index < GetInputsCount());
        return GetDynamicOperands()->GetUser(index)->GetVirtualRegister();
    }

    bool Verify() const
    {
        for (size_t i {0}; i < GetInputsCount(); ++i) {
            if (static_cast<uint16_t>(GetVirtualRegister(i)) == VirtualRegister::INVALID) {
                return false;
            }
        }
        return true;
    }

    bool RemoveNumericInputs()
    {
        size_t idx = 0;
        size_t inputsCount = GetInputsCount();
        bool removed = false;
        while (idx < inputsCount) {
            auto inputInst = GetInput(idx).GetInst();
            if (DataType::IsTypeNumeric(inputInst->GetType())) {
                RemoveInput(idx);
                inputsCount--;
                removed = true;
            } else {
                idx++;
            }
        }
        return removed;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetInput(index).GetInst()->GetType();
    }
    auto GetMethod() const
    {
        return method_;
    }
    auto SetMethod(void *method)
    {
        method_ = method;
    }

    auto GetCallerInst() const
    {
        return callerInst_;
    }
    auto SetCallerInst(CallInst *inst)
    {
        callerInst_ = inst;
    }

    uint32_t GetInliningDepth() const override
    {
        return inliningDepth_;
    }
    void SetInliningDepth(uint32_t inliningDepth)
    {
        inliningDepth_ = inliningDepth;
    }

    void AppendImmediate(uint64_t imm, uint16_t vreg, DataType::Type type, VRegType vregType);

    const ArenaVector<SaveStateImm> *GetImmediates() const
    {
        return immediates_;
    }

    const SaveStateImm &GetImmediate(size_t index) const
    {
        ASSERT(immediates_ != nullptr && index < immediates_->size());
        return (*immediates_)[index];
    }

    void AllocateImmediates(ArenaAllocator *allocator, size_t size = 0);

    size_t GetImmediatesCount() const
    {
        if (immediates_ == nullptr) {
            return 0;
        }
        return immediates_->size();
    }

    void SetRootsRegMaskBit(size_t reg)
    {
        ASSERT(reg < rootsRegsMask_.size());
        rootsRegsMask_.set(reg);
    }

    void SetRootsStackMaskBit(size_t slot)
    {
        if (rootsStackMask_ != nullptr) {
            rootsStackMask_->SetBit(slot);
        }
    }

    ArenaBitVector *GetRootsStackMask()
    {
        return rootsStackMask_;
    }
    void SetRootsStackMask(ArenaBitVector *rootsStackMask)
    {
        rootsStackMask_ = rootsStackMask;
    }

    auto &GetRootsRegsMask()
    {
        return rootsRegsMask_;
    }

    void SetRootsRegsMask(uint32_t rootsRegsMask)
    {
        rootsRegsMask_ = rootsRegsMask;
    }

    void CreateRootsStackMask(ArenaAllocator *allocator)
    {
        ASSERT(rootsStackMask_ == nullptr);
        rootsStackMask_ = allocator->New<ArenaBitVector>(allocator);
        ASSERT(rootsStackMask_ != nullptr);
        rootsStackMask_->Reset();
    }

    Inst *Clone(const Graph *targetGraph) const override;

    void SetInputsWereDeleted()
    {
        SetField<FlagInputsWereDeleted>(true);
    }

    bool GetInputsWereDeleted() const
    {
        return GetField<FlagInputsWereDeleted>();
    }

    bool GetInputsWereDeletedRec() const;

    static bool InstMayRequireRegMap(const Inst *inst)
    {
        // The call may be inlined later on and have Deoptimize or DeoptimizeIf
        // Or the block may become try after inlining
        return inst->RequireRegMap() || inst->IsCall() || inst->CanThrow();
    }

    template <typename PredT = bool (*)(const Inst *)>
    bool CanRemoveInputs(PredT &&mayRequireRegMap = SaveStateInst::InstMayRequireRegMap) const
    {
        for (auto &user : GetUsers()) {
            if (mayRequireRegMap(user.GetInst())) {
                return false;
            }
        }
        return true;
    }

#ifndef NDEBUG
    bool GetInputsWereDeletedSafely() const
    {
        return GetField<FlagInputsWereDeletedSafely>();
    }

    void SetInputsWereDeletedSafely()
    {
        SetField<FlagInputsWereDeletedSafely>(true);
    }
#endif

protected:
    using FlagInputsWereDeleted = LastField::NextFlag;
#ifndef NDEBUG
    using FlagInputsWereDeletedSafely = FlagInputsWereDeleted::NextFlag;
    using LastField = FlagInputsWereDeletedSafely;
#else
    using LastField = FlagInputsWereDeleted;
#endif

private:
    ArenaVector<SaveStateImm> *immediates_ {nullptr};
    void *method_ {nullptr};
    // If instruction is in the inlined graph, this variable points to the inliner's call instruction.
    CallInst *callerInst_ {nullptr};
    uint32_t inliningDepth_ {0};
    ArenaBitVector *rootsStackMask_ {nullptr};
    std::bitset<BITS_PER_UINT32> rootsRegsMask_ {0};
};

/// Load value from array or string
class PANDA_PUBLIC_API LoadInst : public ArrayInstMixin<NeedBarrierMixin<FixedInputsInst2>> {
public:
    using Base = ArrayInstMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;

    explicit LoadInst(Opcode opcode, bool isArray = true) : Base(opcode)
    {
        SetIsArray(isArray);
    }
    explicit LoadInst(Initializer t, bool needBarrier = false, bool isArray = true) : Base(std::move(t))
    {
        SetNeedBarrier(needBarrier);
        SetIsArray(isArray);
    }

    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0: {
                auto inputType = GetInput(0).GetInst()->GetType();
                ASSERT(inputType == DataType::NO_TYPE || inputType == DataType::REFERENCE ||
                       inputType == DataType::ANY);
                return inputType;
            }
            case 1:
                return DataType::INT32;
            default:
                return DataType::NO_TYPE;
        }
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = NeedBarrierMixin<FixedInputsInst2>::Clone(targetGraph);
        ASSERT(static_cast<LoadInst *>(clone)->IsArray() == IsArray());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }
};

class LoadCompressedStringCharInst : public FixedInputsInst3 {
public:
    using Base = FixedInputsInst3;
    using Base::Base;

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetLength() const
    {
        return GetInput(2U).GetInst();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            case 2U:
                return DataType::INT32;
            default:
                return DataType::NO_TYPE;
        }
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoadCompressedStringCharInstI : public FixedInputsInst2, public ImmediateMixin {
public:
    using Base = FixedInputsInst2;
    using Base::Base;

    LoadCompressedStringCharInstI(Initializer t, uint64_t imm) : Base(std::move(t)), ImmediateMixin(imm) {}

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            default:
                return DataType::NO_TYPE;
        }
    }
};
/// Store value into array element
class StoreInst : public NeedBarrierMixin<FixedInputsInst3> {
public:
    using Base = NeedBarrierMixin<FixedInputsInst3>;
    using Base::Base;

    explicit StoreInst(Initializer t, bool needBarrier = false) : Base(std::move(t))
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetStoredValue()
    {
        return GetInput(2U).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0: {
                auto input = GetInput(0).GetInst();
                auto inputType = input->GetType();
                ASSERT(inputType == DataType::ANY || inputType == DataType::REFERENCE);
                return inputType;
            }
            case 1:
                return DataType::INT32;
            case 2U:
                return GetType();
            default:
                return DataType::NO_TYPE;
        }
    }

    // StoreArray call barriers twice,so we need to save input register for second call
    PANDA_PUBLIC_API bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }
};

/// Load value from array, using array index as immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadInstI : public VolatileMixin<ArrayInstMixin<NeedBarrierMixin<FixedInputsInst1>>>,
                                   public ImmediateMixin {
public:
    using Base = VolatileMixin<ArrayInstMixin<NeedBarrierMixin<FixedInputsInst1>>>;
    using Base::Base;

    LoadInstI(Opcode opcode, uint64_t imm, bool isArray = true) : Base(opcode), ImmediateMixin(imm)
    {
        SetIsArray(isArray);
    }

    LoadInstI(Initializer t, uint64_t imm, bool isVolatile = false, bool needBarrier = false, bool isArray = true)
        : Base(std::move(t)), ImmediateMixin(imm)
    {
        SetIsArray(isArray);
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index == 0);
        auto inputType = GetInput(0).GetInst()->GetType();
        ASSERT(inputType == DataType::ANY || inputType == DataType::REFERENCE);
        return inputType;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<LoadInstI *>(FixedInputsInst::Clone(targetGraph));
        clone->SetImm(GetImm());
        ASSERT(clone->IsArray() == IsArray());
        ASSERT(clone->GetVolatile() == GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }
};

/// Load value from pointer with offset
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadMemInstI : public VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>, public ImmediateMixin {
public:
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>;
    using Base::Base;

    LoadMemInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}
    LoadMemInstI(Initializer t, uint64_t imm, bool isVolatile = false, bool needBarrier = false)
        : Base(std::move(t)), ImmediateMixin(imm)
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    Inst *GetPointer()
    {
        return GetInput(0).GetInst();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index == 0);
        auto input0Type = GetInput(0).GetInst()->GetType();
        ASSERT(input0Type == DataType::POINTER || input0Type == DataType::REFERENCE);
        return input0Type;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<LoadMemInstI *>(FixedInputsInst::Clone(targetGraph));
        clone->SetImm(GetImm());
        ASSERT(clone->GetVolatile() == GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }
};

/// Store value into array element, using array index as immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreInstI : public VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>, public ImmediateMixin {
public:
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;

    StoreInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *GetStoredValue()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0: {
                auto inputType = GetInput(0).GetInst()->GetType();
                ASSERT(inputType == DataType::ANY || inputType == DataType::REFERENCE);
                return inputType;
            }
            case 1:
                return GetType();
            default:
                UNREACHABLE();
        }
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<StoreInstI *>(FixedInputsInst::Clone(targetGraph));
        clone->SetImm(GetImm());
        ASSERT(clone->GetVolatile() == GetVolatile());
        return clone;
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    // StoreArrayI call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }
};

/// Store value into pointer by offset
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreMemInstI : public VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>, public ImmediateMixin {
public:
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;

    StoreMemInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *GetPointer()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetStoredValue()
    {
        return GetInput(1).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0: {
                auto input0Type = GetInput(0).GetInst()->GetType();
                ASSERT(input0Type == DataType::POINTER || input0Type == DataType::REFERENCE);
                return input0Type;
            }
            case 1:
                return GetType();
            default:
                UNREACHABLE();
        }
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<StoreMemInstI *>(FixedInputsInst::Clone(targetGraph));
        clone->SetImm(GetImm());
        ASSERT(clone->GetVolatile() == GetVolatile());
        return clone;
    }
};

/// Bounds check, using array index as immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API BoundsCheckInstI : public ArrayInstMixin<FixedInputsInst<2U>>, public ImmediateMixin {
public:
    using Base = ArrayInstMixin<FixedInputsInst<2U>>;
    using Base::Base;

    BoundsCheckInstI(Opcode opcode, uint64_t imm, bool isArray = true) : Base(opcode), ImmediateMixin(imm)
    {
        SetIsArray(isArray);
    }

    BoundsCheckInstI(Initializer t, uint64_t imm, bool isArray = true) : Base(std::move(t)), ImmediateMixin(imm)
    {
        SetIsArray(isArray);
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToBoundsCheckI()->SetImm(GetImm());
        ASSERT(clone->CastToBoundsCheckI()->IsArray() == IsArray());
        return clone;
    }
};

/// Bounds check instruction
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class BoundsCheckInst : public ArrayInstMixin<FixedInputsInst<3U>> {
public:
    using Base = ArrayInstMixin<FixedInputsInst<3U>>;
    using Base::Base;

    explicit BoundsCheckInst(Opcode opcode, bool isArray = true) : Base(opcode)
    {
        SetIsArray(isArray);
    }

    explicit BoundsCheckInst(Initializer t, bool isArray = true) : Base(std::move(t))
    {
        SetIsArray(isArray);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToBoundsCheck()->IsArray() == IsArray());
        return clone;
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        if (index == GetInputsCount() - 1) {
            return DataType::NO_TYPE;
        }
        return GetType();
    }
};

class NullCheckInst : public FixedInputsInst2 {
public:
    using Base = FixedInputsInst2;
    using Base::Base;

    bool IsImplicit() const
    {
        return GetField<IsImplicitFlag>();
    }

    void SetImplicit(bool isImplicit = true)
    {
        SetField<IsImplicitFlag>(isImplicit);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        if (index == GetInputsCount() - 1) {
            return DataType::NO_TYPE;
        }
        return DataType::REFERENCE;
    }

private:
    using IsImplicitFlag = LastField::NextFlag;
    using LastField = IsImplicitFlag;
};

/// Return immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API ReturnInstI : public FixedInputsInst<0>, public ImmediateMixin {
public:
    using FixedInputsInst::FixedInputsInst;

    ReturnInstI(Opcode opcode, uint64_t imm) : FixedInputsInst(opcode), ImmediateMixin(imm) {}
    ReturnInstI(Inst::Initializer t, uint64_t imm) : FixedInputsInst(std::move(t)), ImmediateMixin(imm) {}

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToReturnI()->SetImm(GetImm());
        return clone;
    }
};

class ReturnInlinedInst : public FixedInputsInst<1> {
public:
    using FixedInputsInst::FixedInputsInst;

    bool IsExtendedLiveness() const
    {
        return GetField<IsExtendedLivenessFlag>();
    }

    void SetExtendedLiveness(bool isExtenedLiveness = true)
    {
        SetField<IsExtendedLivenessFlag>(isExtenedLiveness);
    }

private:
    using IsExtendedLivenessFlag = LastField::NextFlag;
    using LastField = IsExtendedLivenessFlag;
};

/// Monitor instruction
class PANDA_PUBLIC_API MonitorInst : public FixedInputsInst2 {
public:
    using Base = FixedInputsInst2;
    using Base::Base;

    bool IsExit() const
    {
        return GetField<Exit>();
    }

    bool IsEntry() const
    {
        return !GetField<Exit>();
    }

    void SetExit()
    {
        SetField<Exit>(true);
    }

    void SetEntry()
    {
        SetField<Exit>(false);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 1) {
            return DataType::NO_TYPE;
        }
        return DataType::REFERENCE;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

protected:
    using Exit = LastField::NextFlag;
    using LastField = Exit;
};

#include "inst_flags.inl"
#include "intrinsics_flags.inl"

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API IntrinsicInst : public InlinedInstMixin<InputTypesMixin<DynamicInputsInst>>,
                                       public TypeIdMixin,
                                       public MethodDataMixin {
public:
    using Base = InlinedInstMixin<InputTypesMixin<DynamicInputsInst>>;
    using Base::Base;
    using IntrinsicId = RuntimeInterface::IntrinsicId;

    IntrinsicInst(Opcode opcode, IntrinsicId intrinsicId) : Base(opcode), intrinsicId_(intrinsicId)
    {
        AdjustFlags(intrinsicId, this);
    }

    IntrinsicInst(Initializer t, IntrinsicId intrinsicId) : Base(std::move(t)), intrinsicId_(intrinsicId)
    {
        AdjustFlags(intrinsicId, this);
    }

    IntrinsicId GetIntrinsicId() const
    {
        return intrinsicId_;
    }

    void SetIntrinsicId(IntrinsicId intrinsicId)
    {
        if (intrinsicId_ != RuntimeInterface::IntrinsicId::INVALID) {
            SetField<FieldFlags>(inst_flags::GetFlagsMask(GetOpcode()));
        }
        intrinsicId_ = intrinsicId;
        AdjustFlags(intrinsicId, this);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(inputTypes_ != nullptr);
        ASSERT(index < inputTypes_->size());
        ASSERT(index < GetInputsCount());
        return (*inputTypes_)[index];
    }

    uint32_t GetImm(size_t index) const
    {
        ASSERT(HasImms() && index < GetImms().size());
        return GetImms()[index];
    }

    void SetImm(size_t index, uint32_t imm)
    {
        ASSERT(HasImms() && index < GetImms().size());
        GetImms()[index] = imm;
    }

    ArenaVector<uint32_t> &GetImms()
    {
        return *imms_;
    }

    const ArenaVector<uint32_t> &GetImms() const
    {
        return *imms_;
    }

    bool HasImms() const
    {
        return imms_ != nullptr;
    }

    void AddImm(ArenaAllocator *allocator, uint32_t imm)
    {
        if (imms_ == nullptr) {
            imms_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
        }
        ASSERT(imms_ != nullptr);
        imms_->push_back(imm);
    }

    bool IsNativeCall() const;

    bool HasArgumentsOnStack() const
    {
        return GetField<ArgumentsOnStack>();
    }

    void SetArgumentsOnStack()
    {
        SetField<ArgumentsOnStack>(true);
    }

    bool IsReplaceOnDeoptimize() const
    {
        return GetField<ReplaceOnDeoptimize>();
    }

    void SetReplaceOnDeoptimize()
    {
        SetField<ReplaceOnDeoptimize>(true);
    }

    bool HasIdInput() const
    {
        return GetField<IdInput>();
    }

    void SetHasIdInput()
    {
        SetField<IdInput>(true);
    }

    bool IsMethodFirstInput() const
    {
        return GetField<MethodFirstInput>();
    }

    void SetMethodFirstInput()
    {
        SetField<MethodFirstInput>(true);
    }

    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override;

    bool CanBeInlined()
    {
        return IsInlined();
    }

    void SetRelocate()
    {
        SetField<Relocate>(true);
    }

    bool GetRelocate() const
    {
        return GetField<Relocate>();
    }

    uint32_t GetTypeId() = delete;  // only method field of TypeIdMixin is used

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    void DumpImms(std::ostream *out) const;

protected:
    using ArgumentsOnStack = LastField::NextFlag;
    using Relocate = ArgumentsOnStack::NextFlag;
    using ReplaceOnDeoptimize = Relocate::NextFlag;
    using IdInput = ReplaceOnDeoptimize::NextFlag;
    using MethodFirstInput = IdInput::NextFlag;
    using LastField = MethodFirstInput;

private:
    IntrinsicId intrinsicId_ {RuntimeInterface::IntrinsicId::COUNT};
    ArenaVector<uint32_t> *imms_ {nullptr};  // record imms appeared in intrinsics
};

#include <get_intrinsics_names.inl>
#include <intrinsics_enum.inl>
#include <can_encode_builtin.inl>

/// Cast instruction
class PANDA_PUBLIC_API CastInst : public InstWithOperandsType<FixedInputsInst1> {
public:
    using BaseInst = InstWithOperandsType<FixedInputsInst1>;
    using BaseInst::BaseInst;

    CastInst(Initializer t, DataType::Type operType) : BaseInst(std::move(t))
    {
        SetOperandsType(operType);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index == 0);
        auto type = GetOperandsType();
        return type != DataType::NO_TYPE ? type : GetInput(0).GetInst()->GetType();
    }

    void SetVnObject(VnObject *vnObj) const override;

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    PANDA_PUBLIC_API Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = static_cast<CastInst *>(FixedInputsInst::Clone(targetGraph));
        ASSERT(clone->GetOperandsType() == GetOperandsType());
        return clone;
    }

    bool IsDynamicCast() const;
};

/// Cmp instruction
class PANDA_PUBLIC_API CmpInst : public InstWithOperandsType<FixedInputsInst2> {
public:
    using BaseInst = InstWithOperandsType<FixedInputsInst2>;
    using BaseInst::BaseInst;

    CmpInst(Initializer t, DataType::Type operType) : BaseInst(std::move(t))
    {
        SetOperandsType(operType);
    }

    bool IsFcmpg() const
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        return GetField<Fcmpg>();
    }
    bool IsFcmpl() const
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        return !GetField<Fcmpg>();
    }
    void SetFcmpg()
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        SetField<Fcmpg>(true);
    }
    void SetFcmpg(bool v)
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        SetField<Fcmpg>(v);
    }
    void SetFcmpl()
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        SetField<Fcmpg>(false);
    }
    void SetFcmpl(bool v)
    {
        ASSERT(DataType::IsFloatType(GetOperandsType()));
        SetField<Fcmpg>(!v);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetOperandsType();
    }

    void SetVnObject(VnObject *vnObj) const override;

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToCmp()->GetOperandsType() == GetOperandsType());
        return clone;
    }

protected:
    using Fcmpg = LastField::NextFlag;
    using LastField = Fcmpg;
};

/// Load value from instance field
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadObjectInst : public ObjectTypeMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>>,
                                        public TypeIdMixin,
                                        public FieldMixin {
public:
    using Base = ObjectTypeMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>>;
    using Base::Base;

    LoadObjectInst(Initializer t, TypeIdMixin m, RuntimeInterface::FieldPtr field, bool isVolatile = false,
                   bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m)), FieldMixin(field)
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 1);
        auto inputType = GetInput(0).GetInst()->GetType();
        ASSERT(inputType == DataType::NO_TYPE || inputType == DataType::REFERENCE || inputType == DataType::ANY);
        return inputType;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadObject()->SetTypeId(GetTypeId());
        clone->CastToLoadObject()->SetMethod(GetMethod());
        clone->CastToLoadObject()->SetObjField(GetObjField());
        ASSERT(clone->CastToLoadObject()->GetVolatile() == GetVolatile());
        ASSERT(clone->CastToLoadObject()->GetObjectType() == GetObjectType());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }
};

/// Load value from memory by offset
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadMemInst : public ScaleMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>> {
public:
    using Base = ScaleMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>>;
    using Base::Base;

    explicit LoadMemInst(Initializer t, bool isVolatile = false, bool needBarrier = false) : Base(std::move(t))
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 2U);
        if (index == 1U) {
            return DataType::IsInt32Bit(GetInput(1).GetInst()->GetType()) ? DataType::INT32 : DataType::INT64;
        }

        ASSERT(index == 0U);
        auto input0Type = GetInput(0).GetInst()->GetType();
        ASSERT((GetInput(0).GetInst()->IsConst() && input0Type == DataType::INT64) || input0Type == DataType::POINTER ||
               input0Type == DataType::REFERENCE);
        return input0Type;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT((clone->GetOpcode() == Opcode::Load && clone->CastToLoad()->GetVolatile() == GetVolatile()) ||
               (clone->GetOpcode() == Opcode::LoadNative && clone->CastToLoadNative()->GetVolatile() == GetVolatile()));
        ASSERT((clone->GetOpcode() == Opcode::Load && clone->CastToLoad()->GetScale() == GetScale()) ||
               (clone->GetOpcode() == Opcode::LoadNative && clone->CastToLoadNative()->GetVolatile() == GetVolatile()));
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }
};

/// Load value from dynamic object
class LoadObjectDynamicInst : public DynObjectAccessMixin<FixedInputsInst3> {
public:
    using Base = DynObjectAccessMixin<FixedInputsInst3>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return true;
    }
};

/// Store value to dynamic object
class StoreObjectDynamicInst : public DynObjectAccessMixin<FixedInputsInst<4U>> {
public:
    using Base = DynObjectAccessMixin<FixedInputsInst<4U>>;
    using Base::Base;

    bool IsBarrier() const override
    {
        return true;
    }
};

/// Resolve instance field
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API ResolveObjectFieldInst : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    ResolveObjectFieldInst(Initializer t, TypeIdMixin m, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(GetInputsCount() == 1U);
        ASSERT(index == 0);
        // This is SaveState input
        return DataType::NO_TYPE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToResolveObjectField()->SetTypeId(GetTypeId());
        clone->CastToResolveObjectField()->SetMethod(GetMethod());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Load value from resolved instance field
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadResolvedObjectFieldInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>,
                                                     public TypeIdMixin {
public:
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;

    LoadResolvedObjectFieldInst(Initializer t, TypeIdMixin m, bool isVolatile = false, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(GetInputsCount() == 2U);
        ASSERT(index < GetInputsCount());
        return index == 0 ? DataType::REFERENCE : DataType::UINT32;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadResolvedObjectField()->SetTypeId(GetTypeId());
        clone->CastToLoadResolvedObjectField()->SetMethod(GetMethod());
        ASSERT(clone->CastToLoadResolvedObjectField()->GetVolatile() == GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Store value into instance field
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreObjectInst : public ObjectTypeMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>>,
                                         public TypeIdMixin,
                                         public FieldMixin {
public:
    using Base = ObjectTypeMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>>;
    using Base::Base;
    static constexpr size_t STORED_INPUT_INDEX = 1;

    StoreObjectInst(Initializer t, TypeIdMixin m, RuntimeInterface::FieldPtr field, bool isVolatile = false,
                    bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m)), FieldMixin(field)
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 2U);
        return index == 0 ? DataType::REFERENCE : GetType();
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreObject()->SetTypeId(GetTypeId());
        clone->CastToStoreObject()->SetMethod(GetMethod());
        clone->CastToStoreObject()->SetObjField(GetObjField());
        ASSERT(clone->CastToStoreObject()->GetVolatile() == GetVolatile());
        ASSERT(clone->CastToStoreObject()->GetObjectType() == GetObjectType());
        return clone;
    }

    // StoreObject call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Store value into resolved instance field
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreResolvedObjectFieldInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst3>>,
                                                      public TypeIdMixin {
public:
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst3>>;
    using Base::Base;
    static constexpr size_t STORED_INPUT_INDEX = 1;

    StoreResolvedObjectFieldInst(Initializer t, TypeIdMixin m, bool isVolatile = false, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetVolatile() || GetNeedBarrier();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        if (index == 0) {
            return DataType::REFERENCE;  // null-check
        }
        if (index == 1) {
            return GetType();  // stored value
        }
        return DataType::UINT32;  // field offset
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreResolvedObjectField()->SetTypeId(GetTypeId());
        clone->CastToStoreResolvedObjectField()->SetMethod(GetMethod());
        ASSERT(clone->CastToStoreResolvedObjectField()->GetVolatile() == GetVolatile());
        return clone;
    }

    // StoreResolvedObjectField calls barriers twice,
    // so we need to save input register for the second call.
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }
};

/// Store value in memory by offset
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreMemInst : public ScaleMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst3>>> {
public:
    using Base = ScaleMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst3>>>;
    using Base::Base;

    static constexpr size_t STORED_INPUT_INDEX = 2;

    explicit StoreMemInst(Initializer t, bool isVolatile = false, bool needBarrier = false) : Base(std::move(t))
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        if (index == 1U) {
            return DataType::IsInt32Bit(GetInput(1).GetInst()->GetType()) ? DataType::INT32 : DataType::INT64;
        }
        if (index == 2U) {
            return GetType();
        }

        ASSERT(index == 0U);
        auto input0Type = GetInput(0).GetInst()->GetType();
        ASSERT(input0Type == DataType::POINTER || input0Type == DataType::REFERENCE);
        return input0Type;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(
            (clone->GetOpcode() == Opcode::Store && clone->CastToStore()->GetVolatile() == GetVolatile()) ||
            (clone->GetOpcode() == Opcode::StoreNative && clone->CastToStoreNative()->GetVolatile() == GetVolatile()));
        ASSERT(
            (clone->GetOpcode() == Opcode::Store && clone->CastToStore()->GetScale() == GetScale()) ||
            (clone->GetOpcode() == Opcode::StoreNative && clone->CastToStoreNative()->GetVolatile() == GetVolatile()));
        return clone;
    }
};

/// Load static field from class.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadStaticInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>,
                                        public TypeIdMixin,
                                        public FieldMixin {
public:
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>;
    using Base::Base;

    LoadStaticInst(Initializer t, TypeIdMixin m, RuntimeInterface::FieldPtr field, bool isVolatile = false,
                   bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m)), FieldMixin(field)
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return GetNeedBarrier() || GetVolatile() || Inst::IsBarrier();
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(index == 0);
        return DataType::REFERENCE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadStatic()->SetTypeId(GetTypeId());
        clone->CastToLoadStatic()->SetMethod(GetMethod());
        clone->CastToLoadStatic()->SetObjField(GetObjField());
        ASSERT(clone->CastToLoadStatic()->GetVolatile() == GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }
};

/// Resolve static instance field
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API ResolveObjectFieldStaticInst : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    ResolveObjectFieldStaticInst(Initializer t, TypeIdMixin m, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToResolveObjectFieldStatic()->SetTypeId(GetTypeId());
        clone->CastToResolveObjectFieldStatic()->SetMethod(GetMethod());
        return clone;
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(GetInputsCount() == 1U);
        ASSERT(index == 0);
        // This is SaveState input
        return DataType::NO_TYPE;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Load value from resolved static instance field
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadResolvedObjectFieldStaticInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>,
                                                           public TypeIdMixin {
public:
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst1>>;
    using Base::Base;

    LoadResolvedObjectFieldStaticInst(Initializer t, TypeIdMixin m, bool isVolatile = false, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetVolatile(isVolatile);
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(GetInputsCount() == 1U);
        ASSERT(index < GetInputsCount());
        return DataType::REFERENCE;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadResolvedObjectFieldStatic()->SetTypeId(GetTypeId());
        clone->CastToLoadResolvedObjectFieldStatic()->SetMethod(GetMethod());
        ASSERT(clone->CastToLoadResolvedObjectFieldStatic()->GetVolatile() == GetVolatile());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Store value into static field.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreStaticInst : public VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>,
                                         public TypeIdMixin,
                                         public FieldMixin {
public:
    using Base = VolatileMixin<NeedBarrierMixin<FixedInputsInst2>>;
    using Base::Base;
    static constexpr size_t STORED_INPUT_INDEX = 1;

    StoreStaticInst(Initializer t, TypeIdMixin m, RuntimeInterface::FieldPtr field, bool isVolatile = false,
                    bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m)), FieldMixin(field)
    {
        SetNeedBarrier(needBarrier);
        SetVolatile(isVolatile);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return GetType();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreStatic()->SetTypeId(GetTypeId());
        clone->CastToStoreStatic()->SetMethod(GetMethod());
        clone->CastToStoreStatic()->SetObjField(GetObjField());
        ASSERT(clone->CastToStoreStatic()->GetVolatile() == GetVolatile());
        return clone;
    }

    // StoreStatic call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }
};

/// Store value into unresolved static field.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API UnresolvedStoreStaticInst : public NeedBarrierMixin<FixedInputsInst2>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;
    static constexpr size_t STORED_INPUT_INDEX = 0U;

    UnresolvedStoreStaticInst(Initializer t, TypeIdMixin m, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return true;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 1) {
            // This is SaveState input
            return DataType::NO_TYPE;
        }
        ASSERT(index == 0);
        return GetType();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToUnresolvedStoreStatic()->SetTypeId(GetTypeId());
        clone->CastToUnresolvedStoreStatic()->SetMethod(GetMethod());
        return clone;
    }
};

/// Store value into resolved static field.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreResolvedObjectFieldStaticInst : public NeedBarrierMixin<FixedInputsInst2>,
                                                            public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;
    static constexpr size_t STORED_INPUT_INDEX = 1U;

    StoreResolvedObjectFieldStaticInst(Initializer t, TypeIdMixin m, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return true;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(GetInputsCount() == 2U);
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return GetType();  // stored value
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreResolvedObjectFieldStatic()->SetTypeId(GetTypeId());
        clone->CastToStoreResolvedObjectFieldStatic()->SetMethod(GetMethod());
        return clone;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Create new object
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API NewObjectInst : public NeedBarrierMixin<FixedInputsInst2>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;

    NewObjectInst(Initializer t, TypeIdMixin m, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToNewObject()->SetTypeId(GetTypeId());
        clone->CastToNewObject()->SetMethod(GetMethod());
        return clone;
    }
};

/// Create new array
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API NewArrayInst : public NeedBarrierMixin<FixedInputsInst3>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst3>;
    using Base::Base;

    static constexpr size_t INDEX_CLASS = 0;
    static constexpr size_t INDEX_SIZE = 1;
    static constexpr size_t INDEX_SAVE_STATE = 2;

    NewArrayInst(Initializer t, TypeIdMixin m, bool needBarrier = false) : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case INDEX_CLASS:
                return GetInput(0).GetInst()->GetType();
            case INDEX_SIZE:
                return DataType::INT32;
            case INDEX_SAVE_STATE:
                // This is SaveState input
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToNewArray()->SetTypeId(GetTypeId());
        clone->CastToNewArray()->SetMethod(GetMethod());
        return clone;
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadConstArrayInst : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    LoadConstArrayInst(Initializer t, TypeIdMixin m, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::NO_TYPE;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadConstArray()->SetTypeId(GetTypeId());
        clone->CastToLoadConstArray()->SetMethod(GetMethod());
        return clone;
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API FillConstArrayInst : public NeedBarrierMixin<FixedInputsInst2>,
                                            public TypeIdMixin,
                                            public ImmediateMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;

    FillConstArrayInst(Initializer t, TypeIdMixin m, uint64_t imm, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m)), ImmediateMixin(imm)
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return GetNeedBarrier() || Inst::IsBarrier();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return index == 0 ? DataType::REFERENCE : DataType::NO_TYPE;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToFillConstArray()->SetTypeId(GetTypeId());
        clone->CastToFillConstArray()->SetMethod(GetMethod());
        clone->CastToFillConstArray()->SetImm(GetImm());
        return clone;
    }
};

/// Checkcast
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API CheckCastInst : public OmitNullCheckMixin<ClassTypeMixin<NeedBarrierMixin<FixedInputsInst3>>>,
                                       public TypeIdMixin {
public:
    using Base = OmitNullCheckMixin<ClassTypeMixin<NeedBarrierMixin<FixedInputsInst3>>>;
    using Base::Base;

    CheckCastInst(Initializer t, TypeIdMixin m, ClassType classType, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetClassType(classType);
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        if (index < 2U) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToCheckCast()->SetTypeId(GetTypeId());
        clone->CastToCheckCast()->SetMethod(GetMethod());
        ASSERT(clone->CastToCheckCast()->GetClassType() == GetClassType());
        ASSERT(clone->CastToCheckCast()->GetOmitNullCheck() == GetOmitNullCheck());
        return clone;
    }
};

/// Is instance
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API IsInstanceInst : public OmitNullCheckMixin<ClassTypeMixin<NeedBarrierMixin<FixedInputsInst3>>>,
                                        public TypeIdMixin {
public:
    using Base = OmitNullCheckMixin<ClassTypeMixin<NeedBarrierMixin<FixedInputsInst3>>>;
    using Base::Base;

    IsInstanceInst(Initializer t, TypeIdMixin m, ClassType classType, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetClassType(classType);
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        if (index < 2U) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToIsInstance()->SetTypeId(GetTypeId());
        clone->CastToIsInstance()->SetMethod(GetMethod());
        ASSERT(clone->CastToIsInstance()->GetClassType() == GetClassType());
        ASSERT(clone->CastToIsInstance()->GetOmitNullCheck() == GetOmitNullCheck());
        return clone;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Load data from constant pool.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadFromPool : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    LoadFromPool(Initializer t, TypeIdMixin m, bool needBarrier = false) : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::NO_TYPE;
    }

    bool IsBarrier() const override
    {
        return GetNeedBarrier() || Inst::IsBarrier();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<LoadFromPool *>(clone)->SetTypeId(GetTypeId());
        static_cast<LoadFromPool *>(clone)->SetMethod(GetMethod());
        return clone;
    }
};

/// Load data from dynamic constant pool.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadFromPoolDynamic : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    LoadFromPoolDynamic(Initializer t, TypeIdMixin m, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m))
    {
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::ANY;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<LoadFromPoolDynamic *>(clone)->SetTypeId(GetTypeId());
        static_cast<LoadFromPoolDynamic *>(clone)->SetMethod(GetMethod());
        return clone;
    }

    bool IsString() const
    {
        return GetField<StringFlag>();
    }

    void SetString(bool v)
    {
        SetField<StringFlag>(v);
    }

    void SetVnObject(VnObject *vnObj) const override;

protected:
    using StringFlag = LastField::NextFlag;
    using LastField = StringFlag;
};

/// Initialization or loading of the class.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API ClassInst : public NeedBarrierMixin<FixedInputsInst1>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst1>;
    using Base::Base;

    ClassInst(Initializer t, TypeIdMixin m, RuntimeInterface::ClassPtr klass, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m)), klass_(klass)
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    void SetVnObject(VnObject *vnObj) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<ClassInst *>(clone)->SetTypeId(GetTypeId());
        static_cast<ClassInst *>(clone)->SetMethod(GetMethod());
        static_cast<ClassInst *>(clone)->SetClass(GetClass());
        return clone;
    }

    RuntimeInterface::ClassPtr GetClass() const
    {
        return klass_;
    }

    void SetClass(RuntimeInterface::ClassPtr klass)
    {
        klass_ = klass;
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        return DataType::NO_TYPE;
    }

private:
    RuntimeInterface::ClassPtr klass_ {nullptr};
};

/// Loading of the runtime class.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API RuntimeClassInst : public NeedBarrierMixin<FixedInputsInst0>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst0>;
    using Base::Base;

    RuntimeClassInst(Inst::Initializer t, TypeIdMixin m, RuntimeInterface::ClassPtr klass, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m)), klass_(klass)
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return GetNeedBarrier() || Inst::IsBarrier();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<RuntimeClassInst *>(clone)->SetTypeId(GetTypeId());
        static_cast<RuntimeClassInst *>(clone)->SetMethod(GetMethod());
        static_cast<RuntimeClassInst *>(clone)->SetClass(GetClass());
        return clone;
    }

    RuntimeInterface::ClassPtr GetClass() const
    {
        return klass_;
    }

    void SetVnObject(VnObject *vnObj) const override;

    void SetClass(RuntimeInterface::ClassPtr klass)
    {
        klass_ = klass;
    }

private:
    RuntimeInterface::ClassPtr klass_ {nullptr};
};

class PANDA_PUBLIC_API WrapObjectNativeInst : public FixedInputsInst1 {
public:
    using Base = FixedInputsInst1;
    using Base::Base;

    explicit WrapObjectNativeInst(Inst::Initializer t) : Base(std::move(t)) {}

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return DataType::REFERENCE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        return FixedInputsInst::Clone(targetGraph);
    }
};

/// Get global var address inst
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API GlobalVarInst : public NeedBarrierMixin<FixedInputsInst2>, public TypeIdMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst2>;
    using Base::Base;

    GlobalVarInst(Initializer t, uintptr_t address) : Base(std::move(t)), address_(address) {}

    GlobalVarInst(Initializer t, TypeIdMixin m, uintptr_t address, bool needBarrier = false)
        : Base(std::move(t)), TypeIdMixin(std::move(m)), address_(address)
    {
        SetNeedBarrier(needBarrier);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        static_cast<GlobalVarInst *>(clone)->SetTypeId(GetTypeId());
        static_cast<GlobalVarInst *>(clone)->SetMethod(GetMethod());
        return clone;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    void SetVnObject(VnObject *vnObj) const override;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::ANY;
        }
        return DataType::NO_TYPE;
    }

    uintptr_t GetAddress() const
    {
        return address_;
    }

private:
    uintptr_t address_ {0};
};

/// Get object pointer from the specific source.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadImmediateInst : public FixedInputsInst<0> {
public:
    using Base = FixedInputsInst;
    using Base::Base;

    enum class ObjectType {
        UNKNOWN,
        CLASS,
        METHOD,
        CONSTANT_POOL,
        STRING,
        PANDA_FILE_OFFSET,
        OBJECT,
        TLS_OFFSET,
        LAST
    };

    LoadImmediateInst(Inst::Initializer t, const void *obj, ObjectType objType = ObjectType::CLASS)
        : Base(std::move(t)), obj_(reinterpret_cast<uint64_t>(obj))
    {
        SetObjectType(objType);
    }

    LoadImmediateInst(Inst::Initializer t, uint64_t obj, ObjectType objType = ObjectType::CLASS)
        : Base(std::move(t)), obj_(obj)
    {
        SetObjectType(objType);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadImmediate()->SetObjectType(GetObjectType());
        clone->CastToLoadImmediate()->obj_ = obj_;
        return clone;
    }

    void *GetObject() const
    {
        return reinterpret_cast<void *>(obj_);
    }

    ObjectType GetObjectType() const
    {
        return GetField<ObjectTypeField>();
    }

    void SetObjectType(ObjectType objType)
    {
        SetField<ObjectTypeField>(objType);
    }

    void SetMethod(RuntimeInterface::MethodPtr obj)
    {
        ASSERT(GetObjectType() == ObjectType::METHOD);
        obj_ = reinterpret_cast<uint64_t>(obj);
    }

    RuntimeInterface::MethodPtr GetMethod() const
    {
        ASSERT(GetObjectType() == ObjectType::METHOD);
        return reinterpret_cast<RuntimeInterface::MethodPtr>(obj_);
    }

    bool IsMethod() const
    {
        return GetField<ObjectTypeField>() == ObjectType::METHOD;
    }

    void SetClass(RuntimeInterface::ClassPtr obj)
    {
        ASSERT(GetObjectType() == ObjectType::CLASS);
        obj_ = reinterpret_cast<uint64_t>(obj);
    }

    RuntimeInterface::ClassPtr GetClass() const
    {
        ASSERT(GetObjectType() == ObjectType::CLASS);
        return reinterpret_cast<RuntimeInterface::ClassPtr>(obj_);
    }

    bool IsConstantPool() const
    {
        return GetField<ObjectTypeField>() == ObjectType::CONSTANT_POOL;
    }

    uintptr_t GetConstantPool() const
    {
        ASSERT(GetObjectType() == ObjectType::CONSTANT_POOL);
        return static_cast<uintptr_t>(obj_);
    }

    bool IsClass() const
    {
        return GetField<ObjectTypeField>() == ObjectType::CLASS;
    }

    bool IsString() const
    {
        return GetField<ObjectTypeField>() == ObjectType::STRING;
    }

    uintptr_t GetString() const
    {
        ASSERT(GetObjectType() == ObjectType::STRING);
        return static_cast<uintptr_t>(obj_);
    }

    bool IsPandaFileOffset() const
    {
        return GetField<ObjectTypeField>() == ObjectType::PANDA_FILE_OFFSET;
    }

    uint64_t GetPandaFileOffset() const
    {
        ASSERT(GetObjectType() == ObjectType::PANDA_FILE_OFFSET);
        return obj_;
    }

    bool IsObject() const
    {
        return GetField<ObjectTypeField>() == ObjectType::OBJECT;
    }

    bool IsTlsOffset() const
    {
        return GetField<ObjectTypeField>() == ObjectType::TLS_OFFSET;
    }

    uint64_t GetTlsOffset() const
    {
        ASSERT(GetObjectType() == ObjectType::TLS_OFFSET);
        return obj_;
    }

    void SetVnObject(VnObject *vnObj) const override;

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

private:
    uint64_t obj_ {0};
    using ObjectTypeField = Base::LastField::NextField<ObjectType, MinimumBitsToStore(ObjectType::LAST)>;
    using LastField = ObjectTypeField;
};

/// Get function from the specific source.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API FunctionImmediateInst : public FixedInputsInst<0> {
public:
    using Base = FixedInputsInst;
    using Base::Base;

    FunctionImmediateInst(Inst::Initializer t, uintptr_t ptr) : Base(std::move(t)), functionPtr_(ptr) {}

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToFunctionImmediate()->functionPtr_ = functionPtr_;
        return clone;
    }

    uintptr_t GetFunctionPtr() const
    {
        return functionPtr_;
    }

    void SetFunctionPtr(uintptr_t ptr)
    {
        functionPtr_ = ptr;
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

private:
    uintptr_t functionPtr_ {0};
};

/// Get object from the specific source(handle).
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadObjFromConstInst : public FixedInputsInst<0> {
public:
    using Base = FixedInputsInst;
    using Base::Base;

    LoadObjFromConstInst(Inst::Initializer t, uintptr_t ptr) : Base(std::move(t)), objectPtr_(ptr) {}

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadObjFromConst()->objectPtr_ = objectPtr_;
        return clone;
    }

    uintptr_t GetObjPtr() const
    {
        return objectPtr_;
    }

    void SetObjPtr(uintptr_t ptr)
    {
        objectPtr_ = ptr;
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

private:
    uintptr_t objectPtr_ {0};
};

/// Select instruction
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API SelectInst : public ConditionMixin<InstWithOperandsType<FixedInputsInst<4U>>> {
public:
    using Base = ConditionMixin<InstWithOperandsType<FixedInputsInst<4U>>>;
    using Base::Base;

    SelectInst(Initializer t, DataType::Type operType, ConditionCode cc) : Base(std::move(t))
    {
        SetOperandsType(operType);
        SetCc(cc);
        if (IsReferenceOrAny()) {
            SetFlag(inst_flags::REF_SPECIAL);
            // Select instruction cannot be generated before LICM where NO_HOIST flag is checked
            // Set it just for consistency
            SetFlag(inst_flags::NO_HOIST);
        }
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    void SetVnObject(VnObject *vnObj) const override;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index < 2U) {
            return GetType();
        }
        return GetOperandsType();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(static_cast<SelectInst *>(clone)->GetCc() == GetCc());
        ASSERT(static_cast<SelectInst *>(clone)->GetOperandsType() == GetOperandsType());
        return clone;
    }
};

class PANDA_PUBLIC_API SelectTransformInst : public SelectTransformTypeMixin<SelectInst> {
public:
    using Base = SelectTransformTypeMixin<SelectInst>;
    using Base::Base;

    SelectTransformInst(Initializer t, DataType::Type operType, ConditionCode cc, SelectTransformType transform)
        : Base(t, operType, cc)
    {
        SetSelectTransformType(transform);
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// SelectImm with comparison with immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API SelectImmInst : public InstWithOperandsType<ConditionMixin<FixedInputsInst3>>,
                                       public ImmediateMixin {
public:
    using Base = InstWithOperandsType<ConditionMixin<FixedInputsInst3>>;
    using Base::Base;

    SelectImmInst(Initializer t, uint64_t imm, DataType::Type operType, ConditionCode cc)
        : Base(std::move(t)), ImmediateMixin(imm)
    {
        SetOperandsType(operType);
        SetCc(cc);
        if (IsReferenceOrAny()) {
            SetFlag(inst_flags::REF_SPECIAL);
            SetFlag(inst_flags::NO_HOIST);
        }
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index < 2U) {
            return GetType();
        }
        return GetOperandsType();
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(static_cast<SelectImmInst *>(clone)->GetCc() == GetCc());
        ASSERT(static_cast<SelectImmInst *>(clone)->GetOperandsType() == GetOperandsType());
        static_cast<SelectImmInst *>(clone)->SetImm(GetImm());
        return clone;
    }
};

class PANDA_PUBLIC_API SelectImmTransformInst : public SelectTransformTypeMixin<SelectImmInst> {
public:
    using Base = SelectTransformTypeMixin<SelectImmInst>;
    using Base::Base;

    SelectImmTransformInst(Initializer t, uint64_t imm, DataType::Type operType, ConditionCode cc,
                           SelectTransformType transform)
        : Base(std::move(t), imm, operType, cc)
    {
        SetSelectTransformType(transform);
    }

    void SetVnObject(VnObject *vnObj) const override;
    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Conditional jump instruction
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API IfInst : public InstWithOperandsType<ConditionMixin<FixedInputsInst2>> {
public:
    using Base = InstWithOperandsType<ConditionMixin<FixedInputsInst2>>;
    using Base::Base;

    IfInst(Initializer t, ConditionCode cc) : Base(std::move(t))
    {
        SetCc(cc);
    }

    IfInst(Initializer t, DataType::Type operType, ConditionCode cc, RuntimeInterface::MethodPtr method = nullptr)
        : Base(std::move(t)), method_(method)
    {
        SetOperandsType(operType);
        SetCc(cc);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetOperandsType();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;

    void SetVnObject(VnObject *vnObj) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(static_cast<IfInst *>(clone)->GetCc() == GetCc());
        ASSERT(static_cast<IfInst *>(clone)->GetOperandsType() == GetOperandsType());
        static_cast<IfInst *>(clone)->SetMethod(GetMethod());
        return clone;
    }

    void SetMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }

    RuntimeInterface::MethodPtr GetMethod() const
    {
        return method_;
    }

private:
    RuntimeInterface::MethodPtr method_ {nullptr};
};

/// IfImm instruction with immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API IfImmInst : public InstWithOperandsType<ConditionMixin<FixedInputsInst1>>,
                                   public ImmediateMixin {
public:
    using Base = InstWithOperandsType<ConditionMixin<FixedInputsInst1>>;
    using Base::Base;

    IfImmInst(Initializer t, ConditionCode cc, uint64_t imm) : Base(std::move(t)), ImmediateMixin(imm)
    {
        SetCc(cc);
    }

    IfImmInst(Initializer t, uint64_t imm, DataType::Type operType, ConditionCode cc,
              RuntimeInterface::MethodPtr method = nullptr)
        : Base(std::move(t)), ImmediateMixin(imm), method_(method)
    {
        SetOperandsType(operType);
        SetCc(cc);
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;
    void SetVnObject(VnObject *vnObj) const override;

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetOperandsType();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToIfImm()->GetCc() == GetCc());
        ASSERT(clone->CastToIfImm()->GetOperandsType() == GetOperandsType());
        clone->CastToIfImm()->SetMethod(GetMethod());
        clone->CastToIfImm()->SetImm(GetImm());
        return clone;
    }

    BasicBlock *GetEdgeIfInputTrue();
    BasicBlock *GetEdgeIfInputFalse();

    void SetMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }

    RuntimeInterface::MethodPtr GetMethod() const
    {
        return method_;
    }

private:
    size_t GetTrueInputEdgeIdx();
    RuntimeInterface::MethodPtr method_ {nullptr};
};

/// Load element from a pair of values, using index as immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadPairPartInst : public FixedInputsInst1, public ImmediateMixin {
public:
    using FixedInputsInst1::FixedInputsInst1;

    LoadPairPartInst(Opcode opcode, uint64_t imm) : FixedInputsInst1(opcode), ImmediateMixin(imm) {}

    LoadPairPartInst(Initializer t, uint64_t imm) : FixedInputsInst1(std::move(t)), ImmediateMixin(imm) {}

    uint32_t GetSrcRegIndex() const override
    {
        return GetImm();
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadPairPart()->SetImm(GetImm());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }
};

/// Load a pair of consecutive values from array
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadArrayPairInst : public NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst2, 2U>>,
                                           public ImmediateMixin {
public:
    using Base = NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst2, 2U>>;
    using Base::Base;

    explicit LoadArrayPairInst(Initializer t, bool needBarrier = false) : Base(std::move(t))
    {
        SetNeedBarrier(needBarrier);
    }

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToLoadArrayPair();
        static_cast<LoadArrayPairInst *>(clone)->SetImm(GetImm());
#ifndef NDEBUG
        for (size_t i = 0; i < GetDstCount(); ++i) {
            clone->SetDstReg(i, GetDstReg(i));
        }
#endif
        return clone;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            default:
                return DataType::NO_TYPE;
        }
    }

    uint32_t Latency() const override
    {
        return 0;
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;
};

/// Load a pair of consecutive values from object
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadObjectPairInst
    : public ObjectTypeMixin<VolatileMixin<NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst1, 2U>>>>,
      public TypeIdMixin2,
      public FieldMixin2 {
public:
    using Base = ObjectTypeMixin<VolatileMixin<NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst1, 2U>>>>;
    using Base::Base;

    explicit LoadObjectPairInst(Initializer t) : Base(std::move(t))
    {
        SetVolatile(false);
        SetNeedBarrier(false);
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 1);
        auto inputType = GetInput(0).GetInst()->GetType();
        ASSERT(inputType == DataType::REFERENCE || inputType == DataType::ANY);
        return inputType;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToLoadObjectPair()->SetTypeId0(GetTypeId0());
        clone->CastToLoadObjectPair()->SetTypeId1(GetTypeId1());
        clone->CastToLoadObjectPair()->SetMethod(GetMethod());
        clone->CastToLoadObjectPair()->SetObjField0(GetObjField0());
        clone->CastToLoadObjectPair()->SetObjField1(GetObjField1());
        ASSERT(clone->CastToLoadObjectPair()->GetVolatile() == GetVolatile());
        ASSERT(clone->CastToLoadObjectPair()->GetObjectType() == GetObjectType());
        return clone;
    }

    uint32_t Latency() const override
    {
        return g_options.GetCompilerSchedLatencyLong();
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Store a pair of consecutive values to array
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreArrayPairInst : public NeedBarrierMixin<FixedInputsInst<4U>>, public ImmediateMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst<4U>>;
    using Base::Base;

    explicit StoreArrayPairInst(Initializer t, bool needBarrier = false) : Base(std::move(t))
    {
        SetNeedBarrier(needBarrier);
    }

    Inst *GetIndex()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetStoredValue(uint64_t index)
    {
        return GetInput(2U + index).GetInst();
    }
    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
                return DataType::INT32;
            case 2U:
            case 3U:
                return GetType();
            default:
                return DataType::NO_TYPE;
        }
    }

    // StoreArrayPair call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToStoreArrayPair();
        static_cast<StoreArrayPairInst *>(clone)->SetImm(GetImm());
        return clone;
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;
};

/// Store a pair of consecutive values to object
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreObjectPairInst : public ObjectTypeMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst3>>>,
                                             public TypeIdMixin2,
                                             public FieldMixin2 {
public:
    using Base = ObjectTypeMixin<VolatileMixin<NeedBarrierMixin<FixedInputsInst3>>>;
    using Base::Base;
    static constexpr size_t STORED_INPUT_INDEX = 2;

    explicit StoreObjectPairInst(Initializer t) : Base(std::move(t))
    {
        SetVolatile(false);
        SetNeedBarrier(false);
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier() || GetVolatile();
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        ASSERT(GetInputsCount() == 3U);
        return index == 0 ? DataType::REFERENCE : GetType();
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreObjectPair()->SetTypeId0(GetTypeId0());
        clone->CastToStoreObjectPair()->SetTypeId1(GetTypeId1());
        clone->CastToStoreObjectPair()->SetMethod(GetMethod());
        clone->CastToStoreObjectPair()->SetObjField0(GetObjField0());
        clone->CastToStoreObjectPair()->SetObjField1(GetObjField1());
        ASSERT(clone->CastToStoreObjectPair()->GetVolatile() == GetVolatile());
        ASSERT(clone->CastToStoreObjectPair()->GetObjectType() == GetObjectType());
        return clone;
    }

    // StoreObject call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

/// Load a pair of consecutive values from array, using array index as immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API LoadArrayPairInstI : public NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst1, 2U>>,
                                            public ImmediateMixin {
public:
    using Base = NeedBarrierMixin<MultipleOutputMixin<FixedInputsInst1, 2U>>;
    using Base::Base;

    explicit LoadArrayPairInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}

    LoadArrayPairInstI(Initializer t, uint64_t imm, bool needBarrier = false) : Base(std::move(t)), ImmediateMixin(imm)
    {
        SetNeedBarrier(needBarrier);
    }

    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph)->CastToLoadArrayPairI();
        clone->SetImm(GetImm());
#ifndef NDEBUG
        for (size_t i = 0; i < GetDstCount(); ++i) {
            clone->SetDstReg(i, GetDstReg(i));
        }
#endif
        return clone;
    }

    uint32_t Latency() const override
    {
        return 0;
    }
};

/// Store a pair of consecutive values to array, using array index as immediate
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StoreArrayPairInstI : public NeedBarrierMixin<FixedInputsInst3>, public ImmediateMixin {
public:
    using Base = NeedBarrierMixin<FixedInputsInst3>;
    using Base::Base;

    explicit StoreArrayPairInstI(Opcode opcode, uint64_t imm) : Base(opcode), ImmediateMixin(imm) {}

    StoreArrayPairInstI(Initializer t, uint64_t imm, bool needBarrier = false) : Base(std::move(t)), ImmediateMixin(imm)
    {
        SetNeedBarrier(needBarrier);
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return DataType::REFERENCE;
            case 1:
            case 2U:
                return GetType();
            default:
                return DataType::NO_TYPE;
        }
    }
    Inst *GetArray()
    {
        return GetInput(0).GetInst();
    }
    Inst *GetFirstValue()
    {
        return GetInput(1).GetInst();
    }
    Inst *GetSecondValue()
    {
        return GetInput(2U).GetInst();
    }

    // StoreArrayPairI call barriers twice,so we need to save input register for second call
    bool IsPropagateLiveness() const override
    {
        return GetType() == DataType::REFERENCE;
    }

    bool IsBarrier() const override
    {
        return Inst::IsBarrier() || GetNeedBarrier();
    }

    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        clone->CastToStoreArrayPairI()->SetImm(GetImm());
        return clone;
    }
};

/// CatchPhiInst instruction
class PANDA_PUBLIC_API CatchPhiInst : public DynamicInputsInst {
public:
    using DynamicInputsInst::DynamicInputsInst;

    explicit CatchPhiInst(Initializer t) : DynamicInputsInst(std::move(t)) {}

    const ArenaVector<const Inst *> *GetThrowableInsts() const
    {
        return throwInsts_;
    }

    const Inst *GetThrowableInst(size_t i) const
    {
        ASSERT(throwInsts_ != nullptr && i < throwInsts_->size());
        return throwInsts_->at(i);
    }

    void AppendThrowableInst(const Inst *inst);
    void ReplaceThrowableInst(const Inst *oldInst, const Inst *newInst);
    void RemoveInput(unsigned index) override;

    bool IsAcc() const
    {
        return GetField<IsAccFlag>();
    }

    void SetIsAcc()
    {
        SetField<IsAccFlag>(true);
    }

protected:
    using IsAccFlag = LastField::NextFlag;
    using LastField = IsAccFlag;

private:
    size_t GetThrowableInstIndex(const Inst *inst)
    {
        ASSERT(throwInsts_ != nullptr);
        auto it = std::find(throwInsts_->begin(), throwInsts_->end(), inst);
        ASSERT(it != throwInsts_->end());
        return std::distance(throwInsts_->begin(), it);
    }

private:
    ArenaVector<const Inst *> *throwInsts_ {nullptr};
};

class PANDA_PUBLIC_API TryInst : public FixedInputsInst0 {
public:
    using FixedInputsInst0::FixedInputsInst0;

    explicit TryInst(Opcode opcode, BasicBlock *endBb = nullptr)
        : FixedInputsInst0({opcode, DataType::NO_TYPE, INVALID_PC}), tryEndBb_(endBb)
    {
    }

    void AppendCatchTypeId(uint32_t id, uint32_t catchEdgeIndex);

    const ArenaVector<uint32_t> *GetCatchTypeIds() const
    {
        return catchTypeIds_;
    }

    const ArenaVector<uint32_t> *GetCatchEdgeIndexes() const
    {
        return catchEdgeIndexes_;
    }

    size_t GetCatchTypeIdsCount() const
    {
        return (catchTypeIds_ == nullptr ? 0 : catchTypeIds_->size());
    }

    Inst *Clone(const Graph *targetGraph) const override;

    void SetTryEndBlock(BasicBlock *tryEndBb)
    {
        tryEndBb_ = tryEndBb;
    }

    BasicBlock *GetTryEndBlock() const
    {
        return tryEndBb_;
    }

private:
    ArenaVector<uint32_t> *catchTypeIds_ {nullptr};
    ArenaVector<uint32_t> *catchEdgeIndexes_ {nullptr};
    BasicBlock *tryEndBb_ {nullptr};
};

PANDA_PUBLIC_API TryInst *GetTryBeginInst(const BasicBlock *tryBeginBb);

/// Mixin for Deoptimize instructions
template <typename T>
class DeoptimizeTypeMixin : public T {
public:
    using T::T;

    void SetDeoptimizeType(DeoptimizeType deoptType)
    {
        T::template SetField<DeoptimizeTypeField>(deoptType);
    }

    void SetDeoptimizeType(Inst *inst)
    {
        switch (inst->GetOpcode()) {
            case Opcode::NullCheck:
                SetDeoptimizeType(DeoptimizeType::NULL_CHECK);
                break;
            case Opcode::BoundsCheck:
                SetDeoptimizeType(DeoptimizeType::BOUNDS_CHECK);
                break;
            case Opcode::ZeroCheck:
                SetDeoptimizeType(DeoptimizeType::ZERO_CHECK);
                break;
            case Opcode::NegativeCheck:
                SetDeoptimizeType(DeoptimizeType::NEGATIVE_CHECK);
                break;
            case Opcode::NotPositiveCheck:
                SetDeoptimizeType(DeoptimizeType::NEGATIVE_CHECK);
                break;
            case Opcode::CheckCast:
                SetDeoptimizeType(DeoptimizeType::CHECK_CAST);
                break;
            case Opcode::AnyTypeCheck: {
                SetDeoptimizeType(inst->CastToAnyTypeCheck()->GetDeoptimizeType());
                break;
            }
            case Opcode::AddOverflowCheck:
            case Opcode::SubOverflowCheck:
            case Opcode::NegOverflowAndZeroCheck:
                SetDeoptimizeType(DeoptimizeType::OVERFLOW_TYPE);
                break;
            case Opcode::DeoptimizeIf:
                SetDeoptimizeType(static_cast<DeoptimizeTypeMixin *>(inst)->GetDeoptimizeType());
                break;
            case Opcode::Intrinsic:
                SetDeoptimizeType(DeoptimizeType::NOT_PROFILED);
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    DeoptimizeType GetDeoptimizeType() const
    {
        return T::template GetField<DeoptimizeTypeField>();
    }

protected:
    using DeoptimizeTypeField =
        typename T::LastField::template NextField<DeoptimizeType, MinimumBitsToStore(DeoptimizeType::COUNT)>;
    using LastField = DeoptimizeTypeField;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API DeoptimizeInst : public DeoptimizeTypeMixin<FixedInputsInst1> {
public:
    using Base = DeoptimizeTypeMixin<FixedInputsInst1>;
    using Base::Base;

    explicit DeoptimizeInst(Initializer t, DeoptimizeType deoptType = DeoptimizeType::NOT_PROFILED) : Base(std::move(t))
    {
        SetDeoptimizeType(deoptType);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToDeoptimize()->GetDeoptimizeType() == GetDeoptimizeType());
        return clone;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API DeoptimizeIfInst : public DeoptimizeTypeMixin<FixedInputsInst2> {
    using Base = DeoptimizeTypeMixin<FixedInputsInst2>;

public:
    using Base::Base;

    DeoptimizeIfInst(Opcode opcode, uint32_t pc, Inst *cond, Inst *ss, DeoptimizeType deoptType)
        : Base({opcode, DataType::NO_TYPE, pc}, cond, ss)
    {
        SetDeoptimizeType(deoptType);
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst::Clone(targetGraph);
        ASSERT(clone->CastToDeoptimizeIf()->GetDeoptimizeType() == GetDeoptimizeType());
        return clone;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return GetInput(0).GetInst()->GetType();
            case 1:
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API DeoptimizeCompareInst
    : public InstWithOperandsType<DeoptimizeTypeMixin<ConditionMixin<FixedInputsInst3>>> {
public:
    using Base = InstWithOperandsType<DeoptimizeTypeMixin<ConditionMixin<FixedInputsInst3>>>;
    using Base::Base;

    explicit DeoptimizeCompareInst(Opcode opcode, const DeoptimizeIfInst *deoptIf, const CompareInst *compare)
        : Base({opcode, deoptIf->GetType(), deoptIf->GetPc()})
    {
        SetDeoptimizeType(deoptIf->GetDeoptimizeType());
        SetOperandsType(compare->GetOperandsType());
        SetCc(compare->GetCc());
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst3::Clone(targetGraph);
        ASSERT(clone->CastToDeoptimizeCompare()->GetDeoptimizeType() == GetDeoptimizeType());
        ASSERT(clone->CastToDeoptimizeCompare()->GetOperandsType() == GetOperandsType());
        ASSERT(clone->CastToDeoptimizeCompare()->GetCc() == GetCc());
        return clone;
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
            case 1:
                return GetInput(index).GetInst()->GetType();
            case 2U:
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API DeoptimizeCompareImmInst
    : public InstWithOperandsType<DeoptimizeTypeMixin<ConditionMixin<FixedInputsInst2>>>,
      public ImmediateMixin {
public:
    using Base = InstWithOperandsType<DeoptimizeTypeMixin<ConditionMixin<FixedInputsInst2>>>;
    using Base::Base;

    explicit DeoptimizeCompareImmInst(Opcode opcode, const DeoptimizeIfInst *deoptIf, const CompareInst *compare,
                                      uint64_t imm)
        : Base({opcode, deoptIf->GetType(), deoptIf->GetPc()}), ImmediateMixin(imm)
    {
        SetDeoptimizeType(deoptIf->GetDeoptimizeType());
        SetOperandsType(compare->GetOperandsType());
        SetCc(compare->GetCc());
    }

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        switch (index) {
            case 0:
                return GetInput(0).GetInst()->GetType();
            case 1:
                return DataType::NO_TYPE;
            default:
                UNREACHABLE();
        }
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        auto clone = FixedInputsInst2::Clone(targetGraph);
        ASSERT(clone->CastToDeoptimizeCompareImm()->GetDeoptimizeType() == GetDeoptimizeType());
        ASSERT(clone->CastToDeoptimizeCompareImm()->GetOperandsType() == GetOperandsType());
        ASSERT(clone->CastToDeoptimizeCompareImm()->GetCc() == GetCc());
        clone->CastToDeoptimizeCompareImm()->SetImm(GetImm());
        return clone;
    }

    PANDA_PUBLIC_API void DumpOpcode(std::ostream *out) const override;
    PANDA_PUBLIC_API bool DumpInputs(std::ostream *out) const override;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ThrowInst : public FixedInputsInst2, public MethodDataMixin {
public:
    using Base = FixedInputsInst2;
    using Base::Base;

    DataType::Type GetInputType(size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        if (index == 0) {
            return DataType::REFERENCE;
        }
        return DataType::NO_TYPE;
    }

    bool IsInlined() const
    {
        auto ss = GetSaveState();
        ASSERT(ss != nullptr);
        return ss->GetCallerInst() != nullptr;
    }

    Inst *Clone(const Graph *targetGraph) const override
    {
        ASSERT(targetGraph != nullptr);
        auto instClone = FixedInputsInst2::Clone(targetGraph);
        auto throwClone = static_cast<ThrowInst *>(instClone);
        throwClone->SetCallMethodId(GetCallMethodId());
        throwClone->SetCallMethod(GetCallMethod());
        return instClone;
    }
};

class BinaryOverflowInst : public IfInst {
public:
    using Base = IfInst;
    using Base::Base;

    BinaryOverflowInst(Initializer t, ConditionCode cc) : Base(t, cc)
    {
        SetOperandsType(GetType());
    }

    DataType::Type GetInputType([[maybe_unused]] size_t index) const override
    {
        ASSERT(index < GetInputsCount());
        return GetType();
    }

    DataType::Type GetOperandsType() const override
    {
        return GetType();
    }
};

bool IsVolatileMemInst(const Inst *inst);

// Check if instruction is pseudo-user for mutli-output instruction
inline bool IsPseudoUserOfMultiOutput(Inst *inst)
{
    ASSERT(inst != nullptr);
    switch (inst->GetOpcode()) {
        case Opcode::LoadPairPart:
            return true;
        default:
            return false;
    }
}

template <typename T>
struct ConstructWrapper : public T {
    template <typename... Args>
    explicit ConstructWrapper([[maybe_unused]] ArenaAllocator *a, Args &&...args)
        : ConstructWrapper(std::forward<Args>(args)...)
    {
    }

    // Try wrap Inst::Initializer:
    template <typename PC, typename... Args>
    ConstructWrapper(Opcode opcode, DataType::Type type, PC pc, Args &&...args)
        : ConstructWrapper(Inst::Initializer {opcode, type, pc}, std::forward<Args>(args)...)
    {
    }

    // Try wrap FixedInputs:
    using Initializer = typename T::Initializer;

    template <typename T0, typename... Args, typename = Inst::CheckBase<T0>>
    ConstructWrapper(Inst::Initializer t, T0 input0, Args &&...args)
        : ConstructWrapper(Initializer {t, std::array<Inst *, 1U> {input0}}, std::forward<Args>(args)...)
    {
    }

    template <typename T0, typename T1, typename... Args, typename = Inst::CheckBase<T0, T1>>
    ConstructWrapper(Inst::Initializer t, T0 input0, T1 input1, Args &&...args)
        : ConstructWrapper(Initializer {t, std::array<Inst *, 2U> {input0, input1}}, std::forward<Args>(args)...)
    {
    }

    template <typename T0, typename T1, typename T2, typename... Args, typename = Inst::CheckBase<T0, T1, T2>>
    ConstructWrapper(Inst::Initializer t, T0 input0, T1 input1, T2 input2, Args &&...args)
        : ConstructWrapper(Initializer {t, std::array<Inst *, 3U> {input0, input1, input2}},
                           std::forward<Args>(args)...)
    {
    }

    template <size_t N, typename... Args>
    ConstructWrapper(Inst::Initializer t, std::array<Inst *, N> &&inputs, Args &&...args)
        : ConstructWrapper(Initializer {t, std::move(inputs)}, std::forward<Args>(args)...)
    {
    }

    // Final forward:
    template <typename... Args>
    explicit ConstructWrapper(Args &&...args) : T(std::forward<Args>(args)...)
    {
    }
};

template <>
struct ConstructWrapper<SpillFillInst> : SpillFillInst {
    ConstructWrapper(ArenaAllocator *allocator, Opcode opcode, SpillFillType type = UNKNOWN)
        : SpillFillInst(allocator, opcode, type)
    {
    }
};

template <typename InstType, typename... Args>
InstType *Inst::ConstructInst(InstType *ptr, ArenaAllocator *allocator, Args &&...args)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
    if constexpr (InstType::INPUT_COUNT == MAX_STATIC_INPUTS) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto opsPtr = reinterpret_cast<char *>(ptr) - sizeof(DynamicOperands);
        [[maybe_unused]] auto ops = new (opsPtr) DynamicOperands(allocator);
        // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
    } else if constexpr (InstType::INPUT_COUNT != 0) {
        constexpr size_t OPERANDS_SIZE = sizeof(Operands<InstType::INPUT_COUNT>);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto operands = new (reinterpret_cast<char *>(ptr) - OPERANDS_SIZE) Operands<InstType::INPUT_COUNT>;
        unsigned idx = InstType::INPUT_COUNT - 1;
        for (auto &user : operands->users) {
            new (&user) User(true, idx--, InstType::INPUT_COUNT);
        }
    }
    auto inst = new (ptr) ConstructWrapper<InstType>(allocator, std::forward<Args>(args)...);
    inst->template SetField<Inst::InputsCount>(InstType::INPUT_COUNT);
    return inst;
}

template <typename InstType, typename... Args>
InstType *Inst::New(ArenaAllocator *allocator, Args &&...args)
{
    static_assert(alignof(InstType) >= alignof(uintptr_t));

    size_t allocSize = sizeof(InstType);
    auto alignment = DEFAULT_ALIGNMENT;
    size_t operandsSize = 0;
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-branch-clone)
    if constexpr (InstType::INPUT_COUNT == MAX_STATIC_INPUTS) {
        constexpr size_t OPERANDS_SIZE = sizeof(DynamicOperands);
        static_assert((OPERANDS_SIZE % alignof(InstType)) == 0);
        operandsSize = OPERANDS_SIZE;
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-branch-clone)
    } else if constexpr (InstType::INPUT_COUNT != 0) {
        constexpr size_t OPERANDS_SIZE = sizeof(Operands<InstType::INPUT_COUNT>);
        static_assert((OPERANDS_SIZE % alignof(InstType)) == 0);
        operandsSize = OPERANDS_SIZE;
        alignment = GetLogAlignment(alignof(Operands<InstType::INPUT_COUNT>));
    }

    auto ptr = reinterpret_cast<uintptr_t>(allocator->Alloc(allocSize + operandsSize, alignment));
    ASSERT(ptr != 0);
    return ConstructInst(reinterpret_cast<InstType *>(ptr + operandsSize), allocator, std::forward<Args>(args)...);
}

INST_CAST_TO_DEF()

template <Opcode OPCODE>
struct OpcodeTraits;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SPECIALIZE_OPCODE_TRAITS(OPCODE, BASE, ...) \
    template <>                                     \
    struct OpcodeTraits<Opcode::OPCODE> {           \
        using BaseType = BASE;                      \
    }; /* CC-OFF(G.PRE.09) code generation */

OPCODE_LIST(SPECIALIZE_OPCODE_TRAITS)
#undef SPECIALIZE_OPCODE_TRAITS

template <Opcode OPCODE>
using BaseTypeOf = typename OpcodeTraits<OPCODE>::BaseType;

inline Inst *User::GetInput()
{
    return GetInst()->GetInput(GetIndex()).GetInst();
}

inline const Inst *User::GetInput() const
{
    return GetInst()->GetInput(GetIndex()).GetInst();
}

inline std::ostream &operator<<(std::ostream &os, const Inst &inst)
{
    inst.Dump(&os, false);
    return os;
}

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_IR_INST_H
