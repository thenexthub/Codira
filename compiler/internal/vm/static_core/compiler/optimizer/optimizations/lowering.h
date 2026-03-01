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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H

#include "compiler_logger.h"
#include "compiler_options.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API Lowering : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit Lowering(Graph *graph) : Optimization(graph) {}

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerLowering();
    }

    const char *GetPassName() const override
    {
        return "Lowering";
    }

    void InvalidateAnalyses() override;

    static constexpr uint8_t HALF_SIZE = 32;
    static constexpr uint8_t WORD_SIZE = 64;

private:
    /**
     * Utility template classes aimed to simplify pattern matching over IR-graph.
     * Patterns are trees declared as a type using Any, UnaryOp, BinaryOp and Operand composition.
     * Then IR-subtree could be tested for matching by calling static Capture method.
     * To capture operands from matched subtree Operand<Index> should be used, where
     * Index is an operand's index within OperandsCapture.
     *
     * For example, suppose that we want to test if IR-subtree rooted by some Inst matches add or sub instruction
     * pattern:
     *
     * Inst* inst = ...;
     * using Predicate = Any<BinaryOp<Opcode::Add, Operand<0>, Operand<1>,
     *                       BinaryOp<Opcode::Sub, Operand<0>, Operand<1>>;
     * OperandsCapture<2> capture{};
     * bool is_add_or_sub = Predicate::Capture(inst, capture);
     *
     * If inst is a binary instruction with opcode Add or Sub then Capture will return `true`,
     * capture.Get(0) will return left inst's input and capture.Get(1) will return right inst's input.
     */

    // Flags altering matching behavior.
    enum Flags {
        NONE = 0,
        COMMUTATIVE = 1,  // binary operation is commutative
        C = COMMUTATIVE,
        SINGLE_USER = 2,  // operation must have only single user
        S = SINGLE_USER
    };

    static bool IsSet(uint64_t flags, Flags flag)
    {
        return (flags & flag) != 0;
    }

    static bool IsNotSet(uint64_t flags, Flags flag)
    {
        return (flags & flag) == 0;
    }

    template <size_t MAX_OPERANDS>
    class OperandsCapture {
    public:
        Inst *Get(size_t i)
        {
            ASSERT(i < MAX_OPERANDS);
            return operands_[i];
        }

        void Set(size_t i, Inst *inst)
        {
            ASSERT(i < MAX_OPERANDS);
#pragma GCC diagnostic push
// CC-OFFNXT(warning_suppression) GCC false positive
#pragma GCC diagnostic ignored "-Warray-bounds"
            operands_[i] = inst;
#pragma GCC diagnostic pop
        }

        // Returns true if all non-constant operands have the same common type (obtained using GetCommonType) as all
        // other operands.
        bool HaveCommonType() const
        {
            auto nonConstType = DataType::LAST;
            for (size_t i = 0; i < MAX_OPERANDS; i++) {
                if (operands_[i]->GetOpcode() != Opcode::Constant) {
                    nonConstType = GetCommonType(operands_[i]->GetType());
                    break;
                }
            }
            // all operands are constants
            if (nonConstType == DataType::LAST) {
                nonConstType = operands_[0]->GetType();
            }
            for (size_t i = 0; i < MAX_OPERANDS; i++) {
                if (operands_[i]->GetOpcode() == Opcode::Constant) {
                    if (GetCommonType(operands_[i]->GetType()) != GetCommonType(nonConstType)) {
                        return false;
                    }
                } else if (nonConstType != GetCommonType(operands_[i]->GetType())) {
                    return false;
                }
            }
            return true;
        }

    private:
        std::array<Inst *, MAX_OPERANDS> operands_;
    };

    template <size_t MAX_INSTS>
    class InstructionsCapture {
    public:
        void Add(Inst *inst)
        {
            ASSERT(currentIdx_ < MAX_INSTS);
            insts_[currentIdx_++] = inst;
        }

        size_t GetCurrentIndex() const
        {
            return currentIdx_;
        }

        void SetCurrentIndex(size_t idx)
        {
            ASSERT(idx < MAX_INSTS);
            currentIdx_ = idx;
        }

        // Returns true if all non-constant operands have exactly the same type and all
        // constant arguments have the same common type (obtained using GetCommonType) as all other operands.
        bool HaveSameType() const
        {
            ASSERT(currentIdx_ == MAX_INSTS);
            auto nonConstType = DataType::LAST;
            for (size_t i = 0; i < MAX_INSTS; i++) {
                if (insts_[i]->GetOpcode() != Opcode::Constant) {
                    nonConstType = insts_[i]->GetType();
                    break;
                }
            }
            // all operands are constants
            if (nonConstType == DataType::LAST) {
                nonConstType = insts_[0]->GetType();
            }
            for (size_t i = 0; i < MAX_INSTS; i++) {
                if (insts_[i]->GetOpcode() == Opcode::Constant) {
                    if (GetCommonType(insts_[i]->GetType()) != GetCommonType(nonConstType)) {
                        return false;
                    }
                } else if (nonConstType != insts_[i]->GetType()) {
                    return false;
                }
            }
            return true;
        }

        InstructionsCapture &ResetIndex()
        {
            currentIdx_ = 0;
            return *this;
        }

    private:
        std::array<Inst *, MAX_INSTS> insts_ {};
        size_t currentIdx_ = 0;
    };

    template <Opcode OPCODE, typename L, typename R, uint64_t FLAGS = Flags::NONE>
    struct BinaryOp {
        template <size_t MAX_OPERANDS, size_t MAX_INSTS>
        static bool Capture(Inst *inst, OperandsCapture<MAX_OPERANDS> &args, InstructionsCapture<MAX_INSTS> &insts)
        {
            constexpr auto INPUTS_NUM = 2;
            // NOLINTNEXTLINE(readability-magic-numbers)
            if (inst->GetOpcode() != OPCODE || inst->GetInputsCount() != INPUTS_NUM ||
                (IsSet(FLAGS, Flags::SINGLE_USER) && !inst->HasSingleUser())) {
                return false;
            }
            if (L::Capture(inst->GetInput(0).GetInst(), args, insts) &&
                R::Capture(inst->GetInput(1).GetInst(), args, insts)) {
                insts.Add(inst);
                return true;
            }
            if (IsSet(FLAGS, Flags::COMMUTATIVE) && L::Capture(inst->GetInput(1).GetInst(), args, insts) &&
                R::Capture(inst->GetInput(0).GetInst(), args, insts)) {
                insts.Add(inst);
                return true;
            }
            return false;
        }
    };

    template <Opcode OPCODE, typename T, uint64_t FLAGS = Flags::NONE>
    struct UnaryOp {
        template <size_t MAX_OPERANDS, size_t MAX_INSTS>
        static bool Capture(Inst *inst, OperandsCapture<MAX_OPERANDS> &args, InstructionsCapture<MAX_INSTS> &insts)
        {
            // NOLINTNEXTLINE(readability-magic-numbers)
            bool matched = inst->GetOpcode() == OPCODE && inst->GetInputsCount() == 1 &&
                           (IsNotSet(FLAGS, Flags::SINGLE_USER) || inst->HasSingleUser()) &&
                           T::Capture(inst->GetInput(0).GetInst(), args, insts);
            if (matched) {
                insts.Add(inst);
            }
            return matched;
        }
    };

    template <size_t IDX>
    struct Operand {
        template <size_t MAX_OPERANDS, size_t MAX_INSTS>
        static bool Capture(Inst *inst, OperandsCapture<MAX_OPERANDS> &args,
                            [[maybe_unused]] InstructionsCapture<MAX_INSTS> &insts)
        {
            static_assert(IDX < MAX_OPERANDS, "Operand's index should not exceed OperandsCapture size");

            args.Set(IDX, inst);
            return true;
        }
    };

    template <typename T, typename... Args>
    struct AnyOf {
        template <size_t MAX_OPERANDS, size_t MAX_INSTS>
        static bool Capture(Inst *inst, OperandsCapture<MAX_OPERANDS> &args, InstructionsCapture<MAX_INSTS> &insts)
        {
            size_t instIdx = insts.GetCurrentIndex();
            if (T::Capture(inst, args, insts)) {
                return true;
            }
            insts.SetCurrentIndex(instIdx);
            return AnyOf<Args...>::Capture(inst, args, insts);
        }
    };

    template <typename T>
    struct AnyOf<T> {
        template <size_t MAX_OPERANDS, size_t MAX_INSTS>
        static bool Capture(Inst *inst, OperandsCapture<MAX_OPERANDS> &args, InstructionsCapture<MAX_INSTS> &insts)
        {
            return T::Capture(inst, args, insts);
        }
    };

    template <bool ENABLED, typename T>
    struct MatchIf : public T {
    };

    template <typename T>
    struct MatchIf<false, T> {
        template <size_t MAX_OPERANDS, size_t MAX_INSTS>
        // NOLINTNEXTLINE(readability-named-parameter)
        static bool Capture(Inst *, OperandsCapture<MAX_OPERANDS> &, InstructionsCapture<MAX_INSTS> &)
        {
            return false;
        }
    };

    using SRC0 = Operand<0>;
    using SRC1 = Operand<1>;
    using SRC2 = Operand<2U>;

    template <typename L, typename R, uint64_t F = Flags::NONE>
    using ADD = BinaryOp<Opcode::Add, L, R, F | Flags::COMMUTATIVE>;
    template <typename L, typename R, uint64_t F = Flags::NONE>
    using SUB = BinaryOp<Opcode::Sub, L, R, F>;
    template <typename L, typename R, uint64_t F = Flags::NONE>
    using MUL = BinaryOp<Opcode::Mul, L, R, F | Flags::COMMUTATIVE>;
    template <typename T, uint64_t F = Flags::NONE>
    using NEG = UnaryOp<Opcode::Neg, T, F>;
    template <typename T, uint64_t F = Flags::SINGLE_USER>
    using NOT = UnaryOp<Opcode::Not, T, F>;
    template <typename T, uint64_t F = Flags::SINGLE_USER>
    using SHRI = UnaryOp<Opcode::ShrI, T, F>;
    template <typename T, uint64_t F = Flags::SINGLE_USER>
    using ASHRI = UnaryOp<Opcode::AShrI, T, F>;
    template <typename T, uint64_t F = Flags::SINGLE_USER>
    using SHLI = UnaryOp<Opcode::ShlI, T, F>;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    static void VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCastValueToAnyType([[maybe_unused]] GraphVisitor *v, Inst *inst);

    template <Opcode OPC>
    static void VisitBitwiseBinaryOperation([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitOr(GraphVisitor *v, Inst *inst);
    static void VisitAnd(GraphVisitor *v, Inst *inst);
    static void VisitXor(GraphVisitor *v, Inst *inst);

    static void VisitAndNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitXorNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitOrNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSaveState([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSafePoint([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSaveStateOsr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSaveStateDeoptimize([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitBoundsCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadArray([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadCompressedStringChar([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreArray([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoad([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadNative([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStore([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreNative([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitReturn([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMul([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitDeoptimizeIf(GraphVisitor *v, Inst *inst);
    static void VisitLoadFromConstantPool(GraphVisitor *v, Inst *inst);
    static void VisitCompare(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"

    static void ReplaceInstruction(Inst *inst, Inst *newInst)
    {
        inst->ReplaceUsers(newInst);
        newInst->GetBasicBlock()->GetGraph()->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()),
                                                                             inst->GetId(), inst->GetPc());
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
    }

    static void InsertInstruction(Inst *inst, Inst *newInst)
    {
        inst->InsertBefore(newInst);
        ReplaceInstruction(inst, newInst);
    }

    template <size_t MAX_OPERANDS>
    static void SetInputsAndInsertInstruction(OperandsCapture<MAX_OPERANDS> &operands, Inst *inst, Inst *newInst);

    static constexpr Opcode GetInstructionWithShiftedOperand(Opcode opcode);
    static constexpr Opcode GetInstructionWithInvertedOperand(Opcode opcode);
    static ShiftType GetShiftTypeByOpcode(Opcode opcode);
    static ConstantInst *GetCheckInstAndGetConstInput(Inst *inst);
    static ShiftOpcode ConvertOpcode(Opcode newOpcode);

    static void LowerMemInstScale(Inst *inst);
    static void LowerShift(Inst *inst);
    static bool TryLowerShrAsBitfieldExtraction(Inst *inst, uint64_t shrValue);
    static bool TryLowerShrAsBitfieldExtractionCase1(Inst *inst, Inst *input, uint64_t shlValue, uint64_t shrValue);
    static bool TryLowerShrAsBitfieldExtractionCase2(Inst *inst, Inst *input, uint64_t mask, uint64_t shrValue);
    static bool ConstantFitsCompareImm(Inst *cst, uint32_t size, ConditionCode cc);
    static Inst *LowerAddSub(Inst *inst);
    template <Opcode OPCODE>
    static void LowerMulDivMod(Inst *inst);
    static Inst *LowerMultiplyAddSub(Inst *inst);
    static Inst *LowerNegateMultiply(Inst *inst);
    static void LowerLogicWithInvertedOperand(Inst *inst);
    static bool LowerCastValueToAnyTypeWithConst(Inst *inst);
    template <typename T, size_t MAX_OPERANDS>
    static Inst *LowerOperationWithShiftedOperand(Inst *inst, OperandsCapture<MAX_OPERANDS> &operands, Inst *shiftInst,
                                                  Opcode newOpcode);
    template <Opcode OPCODE, bool IS_COMMUTATIVE = true>
    static Inst *LowerBinaryOperationWithShiftedOperand(Inst *inst);
    template <Opcode OPCODE>
    static void LowerUnaryOperationWithShiftedOperand(Inst *inst);
    static Inst *LowerLogic(Inst *inst);
    static Inst *LowerAndAsBitfieldExtraction(Inst *inst);
    template <typename LowLevelType>
    static void LowerConstArrayIndex(Inst *inst, Opcode lowLevelOpcode);
    static void LowerStateInst(SaveStateInst *saveState);
    static void LowerReturnInst(FixedInputsInst1 *ret);
    // We'd like to swap only to make second operand immediate
    static bool BetterToSwapCompareInputs(Inst *cmp);
    // Optimize order of input arguments for decreasing using accumulator (Bytecodeoptimizer only).
    static void OptimizeIfInput(compiler::Inst *ifInst);
    static void JoinFcmpInst(IfImmInst *inst, CmpInst *input);
    void LowerIf(IfImmInst *inst);
    static void InPlaceLowerIfImm(IfImmInst *inst, Inst *input, Inst *cst, ConditionCode cc, DataType::Type inputType);
    static void LowerIfImmToIf(IfImmInst *inst, Inst *input, ConditionCode cc, DataType::Type inputType);
    static void LowerToDeoptimizeCompare(Inst *inst);
    static bool SatisfyReplaceDivMovConditions(Inst *inst);
    static bool TryReplaceDivPowerOfTwo(GraphVisitor *v, Inst *inst);
    static bool TryReplaceDivModNonPowerOfTwo(GraphVisitor *v, Inst *inst);
    static bool TryReplaceModPowerOfTwo(GraphVisitor *v, Inst *inst);
    static void ReplaceSignedModPowerOfTwo(GraphVisitor *v, Inst *inst, uint64_t absValue);
    static void ReplaceUnsignedModPowerOfTwo(GraphVisitor *v, Inst *inst, uint64_t absValue);
    static void ReplaceSignedDivPowerOfTwo(GraphVisitor *v, Inst *inst, int64_t sValue);
    static void ReplaceUnsignedDivPowerOfTwo(GraphVisitor *v, Inst *inst, uint64_t uValue);
    static bool TryReplaceModIfModInputIsAdd(GraphVisitor *v, Inst *inst);
    static Inst *IdentifyBaseModIfModInputIsAdd(Inst *inst, const Inst *modInst, uint64_t modVal);

private:
    SaveStateBridgesBuilder ssb_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H
