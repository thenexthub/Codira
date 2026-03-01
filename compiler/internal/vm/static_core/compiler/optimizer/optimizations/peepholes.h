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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_PEEPHOLES_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_PEEPHOLES_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

#include "compiler_logger.h"
#include "compiler_options.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/graph_visitor.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PEEPHOLE_IS_APPLIED(visitor, inst) (visitor)->SetIsApplied((inst), true, __FILE__, __LINE__)

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API Peepholes : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit Peepholes(Graph *graph) : Optimization(graph), inputs_(graph->GetLocalAllocator()->Adapter()) {}

    NO_MOVE_SEMANTIC(Peepholes);
    NO_COPY_SEMANTIC(Peepholes);
    ~Peepholes() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "Peepholes";
    }

    bool IsApplied() const
    {
        return isApplied_;
    }

    bool IsEnable() const override
    {
        return g_options.IsCompilerPeepholes();
    }

    void InvalidateAnalyses() override;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }
#include "intrinsics_peephole.inl.h"

    static void VisitSafePoint([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAbs([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAddFinalize([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1);
    static void VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSubFinalize([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1);
    static void VisitMulOneConst([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1);
    static void VisitMul([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMin([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMax([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAnd([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitOr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitXor([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCmp(GraphVisitor *v, Inst *inst);
    static void VisitCompare([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitIf(GraphVisitor *v, Inst *inst);
    static void VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCastCase1([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCastCase2([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCastCase3([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLenArray(GraphVisitor *v, Inst *inst);
    static void VisitPhi([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSqrt([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitIsInstance(GraphVisitor *v, Inst *inst);
    static void VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst);
    static void VisitCompareAnyType(GraphVisitor *v, Inst *inst);
    static void VisitStore([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreObject([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreStatic([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAddOverflowCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSubOverflowCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNegOverflowAndZeroCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitGetInstanceClass([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadAndInitClass([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitInitClass([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitIntrinsic([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadClass([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadConstantPool([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadFromConstantPool([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadObject([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadStatic([[maybe_unused]] GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"

private:
    void SetIsApplied(Inst *inst, bool altFormat = false, const char *file = nullptr, int line = 0)
    {
        isApplied_ = true;
        if (altFormat) {
            COMPILER_LOG(DEBUG, PEEPHOLE) << "Peephole (" << file << ":" << line << ") is applied for " << *inst;
        } else {
            COMPILER_LOG(DEBUG, PEEPHOLE) << "Peephole is applied for " << GetOpcodeString(inst->GetOpcode());
        }
        GetGraph()->GetEventWriter().EventPeephole(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
        if (g_options.IsCompilerDumpPeepholes()) {
            std::string name(GetPassName());
            name += "_" + std::to_string(applyIndex_);
            GetGraph()->GetPassManager()->DumpGraph(name.c_str());
            applyIndex_++;
        }
    }

    // This function check that we can merge two Phi instructions in one basic block.
    static bool IsPhiUnionPossible(PhiInst *phi1, PhiInst *phi2);

    // Create new instruction instead of current inst
    static Inst *CreateAndInsertInst(Opcode newOpc, Inst *currInst, Inst *input0, Inst *input1 = nullptr);

    // Try put constant in second input
    void TrySwapInputs(Inst *inst);

    // Try to remove redundant cast
    void TrySimplifyCastToOperandsType(Inst *inst);
    void TrySimplifyShifts(Inst *inst);
    bool TrySimplifyAddSubWithZeroInput(Inst *inst);
    bool TrySimplifyAddSubWithConstInput(Inst *inst);
    template <Opcode OPC, int IDX>
    void TrySimplifyAddSub(Inst *inst, Inst *input0, Inst *input1);
    bool TrySimplifyAddSubAdd(Inst *inst, Inst *input0, Inst *input1);
    bool TrySimplifyAddSubSub(Inst *inst, Inst *input0, Inst *input1);
    bool TrySimplifySubAddAdd(Inst *inst, Inst *input0, Inst *input1);
    bool TrySimplifyShlShlAdd(Inst *inst);
    bool TryReassociateShlShlAddSub(Inst *inst);
    void TrySimplifyNeg(Inst *inst);
    void TryReplaceDivByShift(Inst *inst);
    bool TrySimplifyCompareCaseInputInv(Inst *inst, Inst *input);
    bool TrySimplifyCompareWithBoolInput(Inst *inst, bool *isOsrBlocked);
    bool TrySimplifyCmpCompareWithZero(Inst *inst, bool *isOsrBlocked);
    bool TrySimplifyFloatCmpCompare(Inst **input0, Inst **input1, DataType::Type *cmpOpType, Inst *compareInput,
                                    bool *swap);
    bool TrySimplifyTestEqualInputs(Inst *inst);
    void TryRemoveOverflowCheck(Inst *inst);
    static bool TrySimplifyCompareAndZero(Inst *inst, bool *isOsrBlocked);
    static bool TrySimplifyCompareAnyType(Inst *inst);
    static bool TrySimplifyCompareAnyTypeCase1(Inst *inst, Inst *input0, Inst *input1);
    static bool TrySimplifyCompareAnyTypeCase2(Inst *inst, Inst *input0, Inst *input1);
    static bool TrySimplifyCompareLenArrayWithZero(Inst *inst);
    // Try to combine constants when arithmetic operations with constants are repeated
    template <typename T>
    static bool TryCombineConst(Inst *inst, ConstantInst *cnst1, T combine, bool *isOsrBlocked);
    static bool TryCombineAddSubConst(Inst *inst, ConstantInst *cnst1, bool *isOsrBlocked);
    static bool TryCombineShiftConst(Inst *inst, ConstantInst *cnst1, bool *isOsrBlocked);
    static bool TryCombineMulConst(Inst *inst, ConstantInst *cnst1, bool *isOsrBlocked);

    static bool GetInputsOfCompareWithConst(const Inst *inst, Inst **input, ConstantInst **constInput,
                                            bool *inputsSwapped);

    static bool CreateCompareInsteadOfXorAdd(Inst *oldInst);

    bool OptimizeLenArrayForMultiArray(Inst *lenArray, Inst *inst, size_t indexSize);
    bool TryOptimizeLenArrayForPhi(Inst *lenArray, PhiInst *phi);
    static bool TryEliminatePhi(PhiInst *phi);
    // It is check can we use this peephole in OSR or not
    static bool SkipThisPeepholeInOSR(Inst *inst, Inst *newInput);

    // Table for eliminating 'Compare <CC> <INPUT>, <CONST>'
    enum InputCode {
        INPUT_TRUE,   // Result of compare is TRUE whatever the INPUT is
        INPUT_ORIG,   // Result of compare is equal to the INPUT itself
        INPUT_INV,    // Result of compare is a negation of the INPUT
        INPUT_FALSE,  // Result of compare is FALSE whatever the INPUT is
        INPUT_UNDEF   // Result of compare is undefined
    };
    // clang-format off
    static constexpr std::array<std::array<InputCode, 4>, CC_LAST + 1> VALUES = {{
        /* CONST < 0  CONST == 0   CONST == 1   CONST > 1        CC    */
        {INPUT_FALSE, INPUT_INV,   INPUT_ORIG,  INPUT_FALSE}, /* CC_EQ */
        {INPUT_TRUE,  INPUT_ORIG,  INPUT_INV,   INPUT_TRUE},  /* CC_NE */
        {INPUT_FALSE, INPUT_FALSE, INPUT_INV,   INPUT_TRUE},  /* CC_LT */
        {INPUT_FALSE, INPUT_INV,   INPUT_TRUE,  INPUT_TRUE},  /* CC_LE */
        {INPUT_TRUE,  INPUT_ORIG,  INPUT_FALSE, INPUT_FALSE}, /* CC_GT */
        {INPUT_TRUE,  INPUT_TRUE,  INPUT_ORIG,  INPUT_FALSE}, /* CC_GE */
        /* For unsigned comparison 'CONST < 0' equals to 'CONST > 1'   */
        {INPUT_TRUE,  INPUT_FALSE, INPUT_INV,   INPUT_TRUE},  /* CC_B  */
        {INPUT_TRUE,  INPUT_INV,   INPUT_TRUE,  INPUT_TRUE},  /* CC_BE */
        {INPUT_FALSE, INPUT_ORIG,  INPUT_FALSE, INPUT_FALSE}, /* CC_A  */
        {INPUT_FALSE, INPUT_TRUE,  INPUT_ORIG,  INPUT_FALSE}  /* CC_AE */
    }};
    // clang-format on
    static InputCode GetInputCode(const ConstantInst *inst, ConditionCode cc)
    {
        if (inst->GetType() == DataType::INT32) {
            auto i = std::min<int32_t>(std::max<int32_t>(-1, static_cast<int32_t>(inst->GetInt32Value())), 2U) + 1;
            return VALUES[cc][i];
        }
        if (inst->GetType() == DataType::INT64) {
            auto i = std::min<int64_t>(std::max<int64_t>(-1, static_cast<int64_t>(inst->GetIntValue())), 2U) + 1;
            return VALUES[cc][i];
        }
        UNREACHABLE();
        return InputCode::INPUT_UNDEF;
    }
    template <typename T>
    static void EliminateInstPrecedingStore(GraphVisitor *v, Inst *inst);
    bool TrySimplifyCompareNegation(Inst *inst);
    bool IsNegationPattern(Inst *inst);
    bool TrySimplifyNegationPattern(Inst *inst);
    static CallInst *FindTypeArrayCtorCall(Inst *newObject);
    static Inst *GetTypedArraySize(Inst *newInst);
    static bool CheckGetLengthLoad(Inst *inst);
    static bool TryOptimizeBoxedLoadStoreObject(Inst *inst);
    static bool TryOptimizeGetLengthLoadObject(Inst *inst);
    static bool TryOptimizeXorChain(Inst *inst);

private:
    // Each peephole application has own unique index, it will be used in peepholes dumps
    uint32_t applyIndex_ {0};

    bool isApplied_ {false};
    ArenaVector<Inst *> inputs_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_PEEPHOLES_H
