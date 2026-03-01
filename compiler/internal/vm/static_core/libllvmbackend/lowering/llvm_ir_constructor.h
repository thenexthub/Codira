/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef LIBLLVMBACKEND_LOWERING_LLVM_IR_CONSTRUCTOR_H
#define LIBLLVMBACKEND_LOWERING_LLVM_IR_CONSTRUCTOR_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/inst.h"

#include "debug_data_builder.h"
#include "llvm_ark_interface.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>

namespace ark::compiler {

class LLVMIrConstructor : public GraphVisitor {
    bool IsSafeCast(Inst *inst, unsigned int index);
    bool TryEmitIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId arkId);

private:
    // Specific intrinsic Emitters
    bool EmitFastPath(Inst *inst, RuntimeInterface::EntrypointId eid, uint32_t numArgs);
    bool EmitStringEquals(Inst *inst);
    bool EmitStringBuilderBool(Inst *inst);
    bool EmitStringBuilderChar(Inst *inst);
    bool EmitStringBuilderString(Inst *inst);
    bool EmitStringCompareTo(Inst *inst);
    bool EmitIsInf(Inst *inst);
    bool EmitMemmoveUnchecked(Inst *inst);
    bool EmitUnreachable(Inst *inst);
    bool EmitNothing(Inst *inst);
    bool EmitSlowPathEntry(Inst *inst);
    bool EmitExclusiveLoadWithAcquire(Inst *inst);
    bool EmitExclusiveStoreWithRelease(Inst *inst);
    bool EmitInterpreterReturn(Inst *inst);
    bool EmitInitInterpreterCallRecord(Inst *inst);
    bool EmitPushInterpreterCallRecord(Inst *inst);
    bool EmitPopInterpreterCallRecord(Inst *inst);
    bool EmitPopMultipleInterpreterCallRecords(Inst *inst);
    bool EmitUpdateInterpreterCallRecord(Inst *inst);
    bool EmitTailCall(Inst *inst);
    bool EmitCompressEightUtf16ToUtf8CharsUsingSimd(Inst *inst);
    bool EmitCompressSixteenUtf16ToUtf8CharsUsingSimd(Inst *inst);
    bool EmitMemCharU8X16UsingSimd(Inst *inst);
    bool EmitMemCharU8X32UsingSimd(Inst *inst);
    bool EmitMemCharU16X8UsingSimd(Inst *inst);
    bool EmitMemCharU16X16UsingSimd(Inst *inst);
    bool EmitMemLastCharU8X16UsingSimd(Inst *inst);
    bool EmitMemLastCharU16X8UsingSimd(Inst *inst);
    bool EmitCharsToUpperCaseU8X16UsingSimd(Inst *inst);
    bool EmitCharsToUpperCaseU8X8UsingSimd(Inst *inst);
    bool EmitCharsToLowerCaseU8X16UsingSimd(Inst *inst);
    bool EmitCharsToLowerCaseU8X8UsingSimd(Inst *inst);
    bool EmitReverseBytes(Inst *inst);
    bool EmitMemoryFenceFull(Inst *inst);
    bool EmitMemoryFenceRelease(Inst *inst);
    bool EmitMemoryFenceAcquire(Inst *inst);
    bool EmitRoundToPInf(Inst *inst);
    bool EmitFround(Inst *inst);
    bool EmitJsCastDoubleToChar(Inst *inst);
    bool EmitCtlz(Inst *inst);
    bool EmitCttz(Inst *inst);
    bool EmitSignbit(Inst *inst);
    bool EmitIsInteger(Inst *inst);
    bool EmitIsSafeInteger(Inst *inst);
    bool EmitRawBitcastToInt(Inst *inst);
    bool EmitRawBitcastToLong(Inst *inst);
    bool EmitRawBitcastFromInt(Inst *inst);
    bool EmitRawBitcastFromLong(Inst *inst);
    bool EmitStringGetCharsTlab(Inst *inst);
    bool EmitStringHashCode(Inst *inst);
    bool EmitWriteTlabStatsSafe(Inst *inst);
    bool EmitExpandU8U16(Inst *inst);
    bool EmitReverseHalfWords(Inst *inst);
    bool EmitAtomicByteOr(Inst *inst);

public:
    llvm::Value *GetMappedValue(Inst *inst, DataType::Type type);
    llvm::Value *GetInputValue(Inst *inst, size_t index, bool skipCoerce = false);
    llvm::Value *GetInputValueFromConstant(ConstantInst *constant, DataType::Type pandaType);
    template <typename T>
    llvm::FunctionType *GetFunctionTypeForCall(T *inst);

    Graph *GetGraph() const
    {
        return graph_;
    }

    llvm::Function *GetFunc()
    {
        return func_;
    }

    llvm::IRBuilder<> *GetBuilder()
    {
        return &builder_;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return graph_->GetBlocksRPO();
    }

private:
    // Initializers. BuildIr calls them
    void BuildBasicBlocks(Marker normal);
    void BuildInstructions(Marker normal);
    void FillPhiInputs(BasicBlock *block, Marker normal);

    // Creator functions for internal usage

    llvm::CallInst *CreateEntrypointCall(RuntimeInterface::EntrypointId eid, Inst *inst,
                                         llvm::ArrayRef<llvm::Value *> args = llvm::None);
    llvm::CallInst *CreateIntrinsicCall(Inst *inst);
    llvm::CallInst *CreateIntrinsicCall(Inst *inst, RuntimeInterface::IntrinsicId entryId,
                                        llvm::ArrayRef<llvm::Value *> arguments);
    llvm::Value *CreateAllocaForArgs(llvm::Type *type, uint32_t arraySize);
    llvm::CallInst *CreateFastPathCall(Inst *inst, RuntimeInterface::EntrypointId eid,
                                       llvm::ArrayRef<llvm::Value *> args = llvm::None);

    llvm::Value *CreateIsInstanceEntrypointCall(Inst *inst);
    llvm::Value *CreateIsInstanceObject(llvm::Value *klassObj);
    llvm::Value *CreateIsInstanceOther(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId);
    llvm::Value *CreateIsInstanceArray(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId);
    llvm::Value *CreateIsInstanceArrayObject(Inst *inst, llvm::Value *klassObj);
    llvm::Value *CreateIsInstanceInnerBlock(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId);

    void CreateCheckCastEntrypointCall(Inst *inst);
    void CreateCheckCastObject(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId);
    void CreateCheckCastOther(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId);
    void CreateCheckCastArray(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId);
    void CreateCheckCastArrayObject(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId);
    void CreateCheckCastInner(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId);

    void CreateInterpreterReturnRestoreRegs(RegMask &regMask, size_t offset, bool fp);
    llvm::Value *CreateLoadClassById(Inst *inst, uint32_t typeId, bool init);
    llvm::Value *CreateBinaryOp(Inst *inst, llvm::Instruction::BinaryOps opcode);
    llvm::Value *CreateBinaryImmOp(Inst *inst, llvm::Instruction::BinaryOps opcode, uint64_t c);
    llvm::Value *CreateShiftOp(Inst *inst, llvm::Instruction::BinaryOps opcode);
    llvm::Value *CreateSignDivMod(Inst *inst, llvm::Instruction::BinaryOps opcode);
    llvm::Value *CreateFloatComparison(CmpInst *cmpInst, llvm::Value *x, llvm::Value *y);
    llvm::Value *CreateIntegerComparison(CmpInst *inst, llvm::Value *x, llvm::Value *y);
    llvm::Value *CreateNewArrayWithRuntime(Inst *inst);
    llvm::Value *CreateNewObjectWithRuntime(Inst *inst);
    llvm::Value *CreateResolveVirtualCallBuiltin(Inst *inst, llvm::Value *thiz, uint32_t methodId);
    llvm::Value *CreateLoadManagedClassFromClass(llvm::Value *klass);
    llvm::Value *CreateIsNan(llvm::Value *value)
    {
        return builder_.CreateFCmp(llvm::FCmpInst::Predicate::FCMP_UNE, value, value);
    }

    llvm::Value *CreateIsInf(llvm::Value *input);
    llvm::Value *CreateIsInteger(Inst *inst, llvm::Value *input);
    llvm::Value *CreateCastToInt(Inst *inst);
    llvm::Value *CreateLoadWithOrdering(Inst *inst, llvm::Value *value, llvm::AtomicOrdering ordering,
                                        const llvm::Twine &name = "");
    llvm::Value *CreateStoreWithOrdering(llvm::Value *value, llvm::Value *ptr, llvm::AtomicOrdering ordering);
    llvm::Value *CreateZerosCount(Inst *inst, llvm::Intrinsic::ID llvmId);
    llvm::Value *CreateRoundArm64(Inst *inst, bool is64);
    llvm::Value *CreateNewStringFromCharsTlab(Inst *inst, llvm::Value *offset, llvm::Value *length, llvm::Value *array);
    llvm::Value *CreateNewStringFromStringTlab(Inst *inst, llvm::Value *stringVal);
    void CreateDeoptimizationBranch(Inst *inst, llvm::Value *deoptimize, RuntimeInterface::EntrypointId exception,
                                    llvm::ArrayRef<llvm::Value *> arguments = llvm::None);
    llvm::CallInst *CreateDeoptimizeCall(Inst *inst, llvm::ArrayRef<llvm::Value *> arguments,
                                         RuntimeInterface::EntrypointId exception);
    ArenaVector<llvm::OperandBundleDef> CreateSaveStateBundle(Inst *inst, bool noReturn = false);
    void EncodeSaveStateInputs(ArenaVector<llvm::Value *> *vals, SaveStateInst *ss);
    void EncodeInlineInfo(Inst *inst, llvm::Instruction *instruction);
    void CreatePreWRB(Inst *inst, llvm::Value *mem);
    void CreatePostWRB(Inst *inst, llvm::Value *mem, llvm::Value *offset, llvm::Value *value);
    llvm::Value *CreateMemoryFence(memory_order::Order order);
    llvm::Value *CreateCondition(ConditionCode cc, llvm::Value *x, llvm::Value *y);
    void CreateIf(Inst *inst, llvm::Value *cond, bool likely, bool unlikely);
    llvm::Value *CreateReturn(llvm::Value *value);
    llvm::CallInst *CreateTailCallFastPath(Inst *inst);
    llvm::CallInst *CreateTailCallInterpreter(Inst *inst);
    template <uint32_t VECTOR_SIZE>
    void CreateCompressUtf16ToUtf8CharsUsingSimd(Inst *inst);
    template <uint32_t VECTOR_SIZE>
    void CreateCharsToCaseU8UsingSimd(Inst *inst, uint8_t begin, uint8_t end);

    // Getters
    llvm::FunctionType *GetEntryFunctionType();

    llvm::IntegerType *GetEntrypointSizeType()
    {
        return builder_.getIntNTy(func_->getParent()->getDataLayout().getPointerSizeInBits(0));
    }

    llvm::Value *ToSizeT(llvm::Value *value);
    llvm::Value *ToSSizeT(llvm::Value *value);

    ArenaVector<llvm::Value *> GetArgumentsForCall(llvm::Value *callee, CallInst *call, bool skipFirst = false);
    ArenaVector<llvm::Value *> GetIntrinsicArguments(llvm::FunctionType *intrinsicFunctionType, IntrinsicInst *inst);
    void SetIntrinsicParamAttrs(llvm::CallInst *call, IntrinsicInst *inst, llvm::ArrayRef<llvm::Value *> args);

    llvm::Value *GetThreadRegValue();
    llvm::Value *GetRealFrameRegValue();

    llvm::Argument *GetMethodArgument()
    {
        ASSERT(func_ != nullptr);
        ASSERT(GetGraph()->SupportManagedCode());
        auto offset = 0;
        return func_->arg_begin() + offset;
    }

    llvm::Argument *GetArgument(size_t index)
    {
        ASSERT(func_ != nullptr);
        auto offset = 0;
        if (GetGraph()->SupportManagedCode()) {
            offset++;
        }
        return func_->arg_begin() + offset + index;
    }

    llvm::Function *GetOrCreateFunctionForCall(ark::compiler::CallInst *call, void *method);

    llvm::Type *GetType(DataType::Type pandaType);
    llvm::Type *GetExactType(DataType::Type targetType);

    llvm::Instruction::CastOps GetCastOp(DataType::Type from, DataType::Type to);

    llvm::Value *CoerceValue(llvm::Value *value, DataType::Type sourceType, DataType::Type targetType);
    llvm::Value *CoerceValue(llvm::Value *value, llvm::Type *targetType);

    void ValueMapAdd(Inst *inst, llvm::Value *value, bool setName = true);
    void FillValueMapForUsers(ArenaUnorderedMap<DataType::Type, llvm::Value *> *map, Inst *inst, llvm::Value *value);

    void WrapArkCall(Inst *orig, llvm::CallInst *call);

    void AddBlock(BasicBlock *pb, llvm::BasicBlock *lb)
    {
        ASSERT(blockTailMap_.count(pb) == 0);
        blockTailMap_.insert({pb, lb});
        blockHeadMap_.insert({pb, lb});
    }

    void SetCurrentBasicBlock(llvm::BasicBlock *block)
    {
        builder_.SetInsertPoint(block);
    }

    llvm::BasicBlock *GetCurrentBasicBlock()
    {
        return builder_.GetInsertBlock();
    }

    void ReplaceTailBlock(BasicBlock *pandaBlock, llvm::BasicBlock *llvmBlock)
    {
        auto it = blockTailMap_.find(pandaBlock);
        ASSERT(it != blockTailMap_.end());
        it->second = llvmBlock;
    }

    llvm::BasicBlock *GetHeadBlock(BasicBlock *block)
    {
        ASSERT(blockHeadMap_.count(block) == 1);
        auto result = blockHeadMap_.at(block);
        ASSERT(result != nullptr);
        return result;
    }

    llvm::BasicBlock *GetTailBlock(BasicBlock *block)
    {
        ASSERT(blockTailMap_.count(block) == 1);
        auto result = blockTailMap_.at(block);
        ASSERT(result != nullptr);
        return result;
    }

    void InitializeEntryBlock(bool noInline);

    void MarkAsAllocation(llvm::CallInst *call);

protected:
    // Instruction Visitors

    static void VisitConstant(GraphVisitor *v, Inst *inst);
    static void VisitNullPtr(GraphVisitor *v, Inst *inst);
    static void VisitLiveIn(GraphVisitor *v, Inst *inst);
    static void VisitParameter(GraphVisitor *v, Inst *inst);
    static void VisitReturnVoid(GraphVisitor *v, Inst *inst);
    static void VisitReturn(GraphVisitor *v, Inst *inst);
    static void VisitReturnInlined(GraphVisitor *v, Inst *inst);
    static void VisitReturnI(GraphVisitor *v, Inst *inst);
    static void VisitTry(GraphVisitor *v, Inst *inst);
    static void VisitSaveState(GraphVisitor *v, Inst *inst);
    static void VisitSaveStateDeoptimize(GraphVisitor *v, Inst *inst);
    static void VisitSafePoint(GraphVisitor *v, Inst *inst);
    static void VisitNOP(GraphVisitor *v, Inst *inst);
    static void VisitLiveOut(GraphVisitor *v, Inst *inst);
    static void VisitSubOverflowCheck(GraphVisitor *v, Inst *inst);
    static void VisitDeoptimize(GraphVisitor *v, Inst *inst);
    static void VisitDeoptimizeIf(GraphVisitor *v, Inst *inst);
    static void VisitNegativeCheck(GraphVisitor *v, Inst *inst);
    static void VisitZeroCheck(GraphVisitor *v, Inst *inst);
    static void VisitNullCheck(GraphVisitor *v, Inst *inst);
    static void VisitBoundsCheck(GraphVisitor *v, Inst *inst);
    static void VisitRefTypeCheck(GraphVisitor *v, Inst *inst);
    static void VisitLoadString(GraphVisitor *v, Inst *inst);
    static void VisitLenArray(GraphVisitor *v, Inst *inst);
    static void VisitLoadArray(GraphVisitor *v, Inst *inst);
    static void VisitLoadCompressedStringChar(GraphVisitor *v, Inst *inst);
    static void VisitStoreArray(GraphVisitor *v, Inst *inst);
    static void VisitLoad(GraphVisitor *v, Inst *inst);
    static void VisitLoadNative(GraphVisitor *v, Inst *inst);
    static void VisitStore(GraphVisitor *v, Inst *inst);
    static void VisitStoreNative(GraphVisitor *v, Inst *inst);
    static void VisitLoadI(GraphVisitor *v, Inst *inst);
    static void VisitStoreI(GraphVisitor *v, Inst *inst);
    static void VisitLoadObject(GraphVisitor *v, Inst *inst);
    static void VisitStoreObject(GraphVisitor *v, Inst *inst);
    static void VisitResolveObjectField(GraphVisitor *v, Inst *inst);
    static void VisitLoadResolvedObjectField(GraphVisitor *v, Inst *inst);
    static void VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst);
    static void VisitResolveObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitLoadResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitBitcast(GraphVisitor *v, Inst *inst);
    static void VisitCast(GraphVisitor *v, Inst *inst);
    static void VisitAnd(GraphVisitor *v, Inst *inst);
    static void VisitAndI(GraphVisitor *v, Inst *inst);
    static void VisitOr(GraphVisitor *v, Inst *inst);
    static void VisitOrI(GraphVisitor *v, Inst *inst);
    static void VisitXor(GraphVisitor *v, Inst *inst);
    static void VisitXorI(GraphVisitor *v, Inst *inst);
    static void VisitShl(GraphVisitor *v, Inst *inst);
    static void VisitShlI(GraphVisitor *v, Inst *inst);
    static void VisitShr(GraphVisitor *v, Inst *inst);
    static void VisitShrI(GraphVisitor *v, Inst *inst);
    static void VisitAShr(GraphVisitor *v, Inst *inst);
    static void VisitAShrI(GraphVisitor *v, Inst *inst);
    static void VisitAdd(GraphVisitor *v, Inst *inst);
    static void VisitAddI(GraphVisitor *v, Inst *inst);
    static void VisitSub(GraphVisitor *v, Inst *inst);
    static void VisitSubI(GraphVisitor *v, Inst *inst);
    static void VisitMul(GraphVisitor *v, Inst *inst);
    static void VisitMulI(GraphVisitor *v, Inst *inst);
    static void VisitDiv(GraphVisitor *v, Inst *inst);
    static void VisitMod(GraphVisitor *v, Inst *inst);
    static void VisitMin(GraphVisitor *v, Inst *inst);
    static void VisitMax(GraphVisitor *v, Inst *inst);
    static void VisitCompare(GraphVisitor *v, Inst *inst);
    static void VisitCmp(GraphVisitor *v, Inst *inst);
    static void VisitNeg(GraphVisitor *v, Inst *inst);
    static void VisitNot(GraphVisitor *v, Inst *inst);
    static void VisitIfImm(GraphVisitor *v, Inst *inst);
    static void VisitIf(GraphVisitor *v, Inst *inst);
    static void VisitCallIndirect(GraphVisitor *v, Inst *inst);
    static void VisitCall(GraphVisitor *v, Inst *inst);
    static void VisitPhi(GraphVisitor *v, Inst *inst);
    static void VisitMultiArray(GraphVisitor *v, Inst *inst);
    static void VisitInitEmptyString(GraphVisitor *v, Inst *inst);
    static void VisitInitString(GraphVisitor *v, Inst *inst);
    static void VisitNewArray(GraphVisitor *v, Inst *inst);
    static void VisitNewObject(GraphVisitor *v, Inst *inst);
    static void VisitCallStatic(GraphVisitor *v, Inst *inst);
    static void VisitResolveStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallResolvedStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallVirtual(GraphVisitor *v, Inst *inst);
    static void VisitResolveVirtual(GraphVisitor *v, Inst *inst);
    static void VisitCallResolvedVirtual(GraphVisitor *v, Inst *inst);
    static void VisitAbs(GraphVisitor *v, Inst *inst);
    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);
    static void VisitMonitor(GraphVisitor *v, Inst *inst);
    static void VisitSqrt(GraphVisitor *v, Inst *inst);
    static void VisitInitClass(GraphVisitor *v, Inst *inst);
    static void VisitLoadClass(GraphVisitor *v, Inst *inst);
    static void VisitLoadAndInitClass(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadAndInitClass(GraphVisitor *v, Inst *inst);
    static void VisitLoadStatic(GraphVisitor *v, Inst *inst);
    static void VisitStoreStatic(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst);
    static void VisitLoadConstArray(GraphVisitor *v, Inst *inst);
    static void VisitFillConstArray(GraphVisitor *v, Inst *inst);
    static void VisitIsInstance(GraphVisitor *v, Inst *inst);
    static void VisitCheckCast(GraphVisitor *v, Inst *inst);
    static void VisitLoadType(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadType(GraphVisitor *v, Inst *inst);
    static void VisitGetInstanceClass(GraphVisitor *v, Inst *inst);
    static void VisitThrow(GraphVisitor *v, Inst *inst);
    static void VisitCatchPhi(GraphVisitor *v, Inst *inst);
    static void VisitLoadRuntimeClass(GraphVisitor *v, Inst *inst);
    static void VisitLoadUniqueObject(GraphVisitor *v, Inst *inst);
    static void VisitLoadImmediate(GraphVisitor *v, Inst *inst);
    static void VisitStringFlatCheck(GraphVisitor *v, Inst *inst);

    void VisitDefault(Inst *inst) override;

public:
    explicit LLVMIrConstructor(Graph *graph, llvm::Module *module, llvm::LLVMContext *context,
                               llvmbackend::LLVMArkInterface *arkInterface,
                               const std::unique_ptr<llvmbackend::DebugDataBuilder> &debugData);

    bool BuildIr(bool preventInlining);

    static void InsertArkFrameInfo(llvm::Module *module, Arch arch);
    static void ProvideSafepointPoll(llvm::Module *module, llvmbackend::LLVMArkInterface *arkInterface, Arch arch);

    static std::string CheckGraph(Graph *graph);
    static bool CanCompile(Inst *inst);

#ifndef NDEBUG
    void BreakIrIfNecessary();
#endif

#include "llvm_ir_constructor_gen.h.inl"

#include "optimizer/ir/visitor.inc"

private:
    Graph *graph_ {nullptr};
    llvm::Function *func_;
    llvm::IRBuilder<> builder_;
    ArenaDoubleUnorderedMap<Inst *, DataType::Type, llvm::Value *> inputMap_;
    ArenaUnorderedMap<BasicBlock *, llvm::BasicBlock *> blockTailMap_;
    ArenaUnorderedMap<BasicBlock *, llvm::BasicBlock *> blockHeadMap_;
    llvmbackend::LLVMArkInterface *arkInterface_;
    const std::unique_ptr<llvmbackend::DebugDataBuilder> &debugData_;
    ArenaVector<uint8_t> cc_;
    ArenaVector<llvm::Value *> ccValues_;
};

}  // namespace ark::compiler

#endif  // LIBLLVMBACKEND_LOWERING_LLVM_IR_CONSTRUCTOR_H
