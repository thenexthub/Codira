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

#ifndef COMPILER_OPTIMIZER_CODEGEN_ENCODE_VISITOR_H
#define COMPILER_OPTIMIZER_CODEGEN_ENCODE_VISITOR_H

#include "codegen.h"

namespace ark::compiler {

class Encoder;
class CodeBuilder;
class OsrEntryStub;

class EncodeVisitor : public GraphVisitor {
    using EntrypointId = RuntimeInterface::EntrypointId;

public:
    explicit EncodeVisitor(Codegen *cg) : cg_(cg), arch_(cg->GetArch()) {}

    EncodeVisitor() = delete;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return cg_->GetGraph()->GetBlocksRPO();
    }
    Codegen *GetCodegen() const
    {
        return cg_;
    }
    Encoder *GetEncoder()
    {
        return cg_->GetEncoder();
    }
    Arch GetArch() const
    {
        return arch_;
    }
    CallingConvention *GetCallingConvention()
    {
        return cg_->GetCallingConvention();
    }

    RegistersDescription *GetRegfile()
    {
        return cg_->GetRegfile();
    }

    bool GetResult()
    {
        return success_ && cg_->GetEncoder()->GetResult();
    }

    // For each group of SpillFillData representing spill or fill operations and
    // sharing the same source and destination types order by stack slot number in descending order.
    static void SortSpillFillData(ArenaVector<SpillFillData> *spillFills);
    // Checks if two spill-fill operations could be coalesced into single operation over pair of arguments.
    static bool CanCombineSpillFills(SpillFillData pred, SpillFillData succ, const CFrameLayout &fl,
                                     const Graph *graph);

protected:
    // UnaryOperation
    static void VisitMov(GraphVisitor *visitor, Inst *inst);
    static void VisitNeg(GraphVisitor *visitor, Inst *inst);
    static void VisitAbs(GraphVisitor *visitor, Inst *inst);
    static void VisitNot(GraphVisitor *visitor, Inst *inst);
    static void VisitSqrt(GraphVisitor *visitor, Inst *inst);

    // BinaryOperation
    static void VisitAdd(GraphVisitor *visitor, Inst *inst);
    static void VisitSub(GraphVisitor *visitor, Inst *inst);
    static void VisitMul(GraphVisitor *visitor, Inst *inst);
    static void VisitShl(GraphVisitor *visitor, Inst *inst);
    static void VisitAShr(GraphVisitor *visitor, Inst *inst);
    static void VisitAnd(GraphVisitor *visitor, Inst *inst);
    static void VisitOr(GraphVisitor *visitor, Inst *inst);
    static void VisitXor(GraphVisitor *visitor, Inst *inst);

    // Binary Overflow Operation
    static void VisitAddOverflow(GraphVisitor *visitor, Inst *inst);
    static void VisitAddOverflowCheck(GraphVisitor *visitor, Inst *inst);
    static void VisitSubOverflow(GraphVisitor *visitor, Inst *inst);
    static void VisitSubOverflowCheck(GraphVisitor *visitor, Inst *inst);
    static void VisitNegOverflowAndZeroCheck(GraphVisitor *visitor, Inst *inst);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_IMM_OPERATION(opc) static void Visit##opc##I(GraphVisitor *visitor, Inst *inst)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_IMM_OPS(DEF) \
    DEF(Add);               \
    DEF(Sub);               \
    DEF(Shl);               \
    DEF(AShr);              \
    DEF(And);               \
    DEF(Or);                \
    DEF(Xor);               \
    DEF(Div);               \
    DEF(Mod)

    BINARY_IMM_OPS(BINARY_IMM_OPERATION);

#undef BINARY_IMM_OPS
#undef BINARY_IMM_OPERATION

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_SIGN_UNSIGN_OPERATION(opc) static void Visit##opc(GraphVisitor *visitor, Inst *inst)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SIGN_UNSIGN_OPS(DEF) \
    DEF(Div);                \
    DEF(Mod);                \
    DEF(Min);                \
    DEF(Max);                \
    DEF(Shr)

    SIGN_UNSIGN_OPS(BINARY_SIGN_UNSIGN_OPERATION);

#undef SIGN_UNSIGN_OPS
#undef BINARY_SIGN_UNSIGN_OPERATION

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_SHIFTED_REGISTER_OPERATION_DEF(opc, ignored) \
    /* CC-OFFNXT(G.PRE.09) code generation */               \
    static void Visit##opc##SR(GraphVisitor *visitor, Inst *inst);

    ENCODE_INST_WITH_SHIFTED_OPERAND(BINARY_SHIFTED_REGISTER_OPERATION_DEF)

#undef BINARY_SHIFTED_REGISTER_OPERATION_DEF

    static void VisitExtractBitfield(GraphVisitor *visitor, Inst *inst);

    static void VisitShrI(GraphVisitor *visitor, Inst *inst);

    static void VisitCast(GraphVisitor *visitor, Inst *inst);

    static void VisitBitcast(GraphVisitor *visitor, Inst *inst);

    static void VisitPhi([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst);

    static void VisitConstant(GraphVisitor *visitor, Inst *inst);

    static void VisitNullPtr(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadUniqueObject(GraphVisitor *visitor, Inst *inst);

    static void VisitIf(GraphVisitor *visitor, Inst *inst);

    static void VisitIfImm(GraphVisitor *visitor, Inst *inst);

    static void VisitCompare(GraphVisitor *visitor, Inst *inst);

    static void VisitCmp(GraphVisitor *visitor, Inst *inst);

    // All next visitors use execution model for implementation
    static void VisitReturnVoid(GraphVisitor *visitor, Inst * /* unused */);

    static void VisitReturn(GraphVisitor *visitor, Inst *inst);

    static void VisitReturnI(GraphVisitor *visitor, Inst *inst);

    static void VisitReturnInlined(GraphVisitor *visitor, Inst * /* unused */);

    static void VisitNewArray(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadConstArray(GraphVisitor *visitor, Inst *inst);

    static void VisitFillConstArray(GraphVisitor *visitor, Inst *inst);

    static void VisitParameter(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreArray(GraphVisitor *visitor, Inst *inst);

    static void VisitSpillFill(GraphVisitor *visitor, Inst *inst);

    static void VisitSaveState(GraphVisitor *visitor, Inst *inst);

    static void VisitSaveStateDeoptimize(GraphVisitor *visitor, Inst *inst);

    static void VisitSaveStateOsr(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadArray(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadCompressedStringChar(GraphVisitor *visitor, Inst *inst);

    static void VisitLenArray(GraphVisitor *visitor, Inst *inst);

    static void VisitNullCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitBoundsCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitZeroCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitRefTypeCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitNegativeCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitNotPositiveCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadString(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveObjectField(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadResolvedObjectField(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadObject(GraphVisitor *visitor, Inst *inst);

    static void VisitLoad(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreObject(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreResolvedObjectField(GraphVisitor *visitor, Inst *inst);

    static void VisitStore(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveObjectFieldStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadResolvedObjectFieldStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedStoreStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreResolvedObjectFieldStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitNewObject(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadRuntimeClass(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadClass(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadAndInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitGetGlobalVarAddress(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedLoadAndInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedInitClass(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadType(GraphVisitor *visitor, Inst *inst);

    static void VisitUnresolvedLoadType(GraphVisitor *visitor, Inst *inst);

    static void VisitCheckCast(GraphVisitor *visitor, Inst *inst);

    static void VisitIsInstance(GraphVisitor *visitor, Inst *inst);

    static void VisitMonitor(GraphVisitor *visitor, Inst *inst);

    static void VisitIntrinsic(GraphVisitor *visitor, Inst *inst);

    static void VisitBuiltin(GraphVisitor *visitor, Inst *inst);

    static void VisitBoundsCheckI(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreArrayI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadArrayI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadCompressedStringCharI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadI(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreI(GraphVisitor *visitor, Inst *inst);

    static void VisitMultiArray(GraphVisitor *visitor, Inst *inst);
    static void VisitInitEmptyString(GraphVisitor *visitor, Inst *inst);
    static void VisitInitString(GraphVisitor *visitor, Inst *inst);

    static void VisitCallStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveStatic(GraphVisitor *visitor, Inst *inst);
    static void VisitCallResolvedStatic(GraphVisitor *visitor, Inst *inst);

    static void VisitCallVirtual(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveVirtual(GraphVisitor *visitor, Inst *inst);
    static void VisitCallResolvedVirtual(GraphVisitor *visitor, Inst *inst);

    static void VisitCallDynamic(GraphVisitor *visitor, Inst *inst);

    static void VisitCallNative(GraphVisitor *visitor, Inst *inst);
    static void VisitWrapObjectNative(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadConstantPool(GraphVisitor *visitor, Inst *inst);
    static void VisitLoadLexicalEnv(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadFromConstantPool(GraphVisitor *visitor, Inst *inst);

    static void VisitSafePoint(GraphVisitor *visitor, Inst *inst);

    static void VisitSelect(GraphVisitor *visitor, Inst *inst);

    static void VisitSelectImm(GraphVisitor *visitor, Inst *inst);

    static void VisitSelectTransform(GraphVisitor *visitor, Inst *inst);

    static void VisitSelectImmTransform(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadArrayPair(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadArrayPairI(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadPairPart(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadObjectPair(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreArrayPair(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreArrayPairI(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreObjectPair(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadExclusive(GraphVisitor *visitor, Inst *inst);

    static void VisitStoreExclusive(GraphVisitor *visitor, Inst *inst);

    static void VisitNOP(GraphVisitor *visitor, Inst *inst);

    static void VisitThrow(GraphVisitor *visitor, Inst *inst);

    static void VisitDeoptimizeIf(GraphVisitor *visitor, Inst *inst);

    static void VisitDeoptimizeCompare(GraphVisitor *visitor, Inst *inst);

    static void VisitDeoptimizeCompareImm(GraphVisitor *visitor, Inst *inst);

    static void VisitDeoptimize(GraphVisitor *visitor, Inst *inst);

    static void VisitIsMustDeoptimize(GraphVisitor *visitor, Inst *inst);

    static void VisitMAdd(GraphVisitor *visitor, Inst *inst);
    static void VisitMSub(GraphVisitor *visitor, Inst *inst);
    static void VisitMNeg(GraphVisitor *visitor, Inst *inst);
    static void VisitOrNot(GraphVisitor *visitor, Inst *inst);
    static void VisitAndNot(GraphVisitor *visitor, Inst *inst);
    static void VisitXorNot(GraphVisitor *visitor, Inst *inst);
    static void VisitNegSR(GraphVisitor *visitor, Inst *inst);

    static void VisitGetInstanceClass(GraphVisitor *visitor, Inst *inst);
    static void VisitGetManagedClassObject(GraphVisitor *visito, Inst *inst);
    static void VisitLoadImmediate(GraphVisitor *visitor, Inst *inst);
    static void VisitFunctionImmediate(GraphVisitor *visitor, Inst *inst);
    static void VisitLoadObjFromConst(GraphVisitor *visitor, Inst *inst);
    static void VisitRegDef(GraphVisitor *visitor, Inst *inst);
    static void VisitLiveIn(GraphVisitor *visitor, Inst *inst);
    static void VisitLiveOut(GraphVisitor *visitor, Inst *inst);
    static void VisitCallIndirect(GraphVisitor *visitor, Inst *inst);
    static void VisitCall(GraphVisitor *visitor, Inst *inst);

    static void VisitResolveByName(GraphVisitor *visitor, Inst *inst);

    // Dyn inst.
    static void VisitCompareAnyType(GraphVisitor *visitor, Inst *inst);
    static void VisitGetAnyTypeName(GraphVisitor *visitor, Inst *inst);
    static void VisitCastAnyTypeValue(GraphVisitor *visitor, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *visitor, Inst *inst);
    static void VisitAnyTypeCheck(GraphVisitor *visitor, Inst *inst);
    static void VisitHclassCheck(GraphVisitor *visitor, Inst *inst);
    static void VisitObjByIndexCheck(GraphVisitor *visitor, Inst *inst);

    static void VisitLoadObjectDynamic(GraphVisitor *visitor, Inst *inst);
    static void VisitStoreObjectDynamic(GraphVisitor *visitor, Inst *inst);
    static void VisitStringFlatCheck(GraphVisitor *visitor, Inst *inst);

    void VisitDefault([[maybe_unused]] Inst *inst) override
    {
#ifndef NDEBUG
        COMPILER_LOG(DEBUG, CODEGEN) << "Can't encode instruction " << GetOpcodeString(inst->GetOpcode())
                                     << " with type " << DataType::ToString(inst->GetType());
#endif
        success_ = false;
    }

    // Helper functions
    static void FillUnresolvedClass(GraphVisitor *visitor, Inst *inst);
    static void FillObjectClass(GraphVisitor *visitor, Reg tmpReg, LabelHolder::LabelId throwLabel);
    static void FillOtherClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId throwLabel);
    static void FillArrayObjectClass(GraphVisitor *visitor, Reg tmpReg, LabelHolder::LabelId throwLabel);
    static void FillArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId throwLabel);
    static void FillInterfaceClass(GraphVisitor *visitor, Inst *inst);
    static void FillUnionClass(GraphVisitor *visitor, Inst *inst);

    static void FillLoadClassUnresolved(GraphVisitor *visitor, Inst *inst);

    static void FillCheckCast(GraphVisitor *visitor, Inst *inst, Reg src, LabelHolder::LabelId endLabel,
                              compiler::ClassType klassType);

    static void FillIsInstanceUnresolved(GraphVisitor *visitor, Inst *inst);

    static void FillIsInstanceCaseObject(GraphVisitor *visitor, Inst *inst, Reg tmpReg);

    static void FillIsInstanceCaseOther(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId endLabel);

    static void FillIsInstanceCaseArrayObject(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                              LabelHolder::LabelId endLabel);

    static void FillIsInstanceCaseArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                             LabelHolder::LabelId endLabel);

    static void FillIsInstanceCaseInterface(GraphVisitor *visitor, Inst *inst);

    static void FillIsInstance(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId endLabel);
    static void EncodeLoadAndInitClassInAot(EncodeVisitor *enc, Encoder *encoder, Inst *inst, uint32_t classId,
                                            Reg dst);

#include "optimizer/ir/visitor.inc"

private:
    Codegen *cg_;
    Arch arch_;
    bool success_ {true};
};  // EncodeVisitor

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_ENCODE_VISITOR_H
