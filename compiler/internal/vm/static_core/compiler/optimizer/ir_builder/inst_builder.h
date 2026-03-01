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

#ifndef PANDA_INST_BUILDER_H
#define PANDA_INST_BUILDER_H

#include "compiler_options.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "code_info/vreg_info.h"
#include "libarkfile/code_data_accessor.h"
#include "libarkfile/file_items.h"
#include "compiler_logger.h"

#include "libarkfile/bytecode_instruction.h"

namespace ark::compiler {
constexpr int64_t INVALID_OFFSET = std::numeric_limits<int64_t>::max();

class InstBuilder {
public:
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ENV_IDX(ENV_TYPE) /* CC-OFFNXT(G.PRE.02, G.PRE.09) namespace member, code generation */ \
    static constexpr uint8_t ENV_TYPE##_IDX = VRegInfo::VRegType::ENV_TYPE - VRegInfo::VRegType::ENV_BEGIN;
    VREGS_ENV_TYPE_DEFS(ENV_IDX)
#undef ENV_IDX

    InstBuilder(Graph *graph, RuntimeInterface::MethodPtr method, CallInst *callerInst, uint32_t inliningDepth);

    NO_COPY_SEMANTIC(InstBuilder);
    NO_MOVE_SEMANTIC(InstBuilder);
    virtual ~InstBuilder()
    {
        GetGraph()->EraseMarker(noTypeMarker_);
        GetGraph()->EraseMarker(visitedBlockMarker_);
    }

    /**
     * Content of this function is auto generated from inst_builder.erb and is located in inst_builder_gen.cpp file
     * @param instruction Pointer to bytecode instruction
     */
    void BuildInstruction(const BytecodeInstruction *instruction);

    void InitEnv(BasicBlock *bb);

    bool IsFailed() const
    {
        return failed_;
    }

    /// Return jump offset for instruction `inst`, 0 if it is not jump instruction.
    static int64_t GetInstructionJumpOffset(const BytecodeInstruction *inst);

    void SetCurrentBlock(BasicBlock *bb);

    BasicBlock *GetCurrentBlock() const
    {
        return currentBb_;
    }

    void Prepare(bool isInlinedGraph);

    void FixType(PhiInst *inst, BasicBlock *bb);
    void FixType(Inst *inst);
    void FixInstructions();
    void ResolveConstants();
    void SplitConstant(ConstantInst *constInst);

    static void RemoveNotDominateInputs(SaveStateInst *saveState);

    size_t GetPc(const uint8_t *instPtr) const;

    auto CreateSafePoint(BasicBlock *bb)
    {
#ifndef NDEBUG
        ResetSafepointDistance();
#endif
        return CreateSaveState(Opcode::SafePoint, bb->GetGuestPc());
    }

    auto CreateSaveStateOsr(BasicBlock *bb)
    {
        return CreateSaveState(Opcode::SaveStateOsr, bb->GetGuestPc());
    }

    auto CreateSaveStateDeoptimize(uint32_t pc)
    {
        return CreateSaveState(Opcode::SaveStateDeoptimize, pc);
    }

    void UpdateDefs();
    bool UpdateDefsForPreds(size_t vreg, std::optional<Inst *> &value);

    const auto &GetCurrentDefs()
    {
        ASSERT(currentDefs_ != nullptr);
        return *currentDefs_;
    }

    bool IsInBootContext()
    {
        auto method = static_cast<ark::Method *>(GetGraph()->GetMethod());
        return method->GetClass()->GetLoadContext()->IsBootContext();
    }

    void AddCatchPhiInputs(const ArenaUnorderedSet<BasicBlock *> &catchHandlers, const InstVector &defs,
                           Inst *throwableInst);

    SaveStateInst *CreateSaveState(Opcode opc, size_t pc);

    static void SetParamSpillFill(Graph *graph, ParameterInst *paramInst, size_t numArgs, size_t i,
                                  DataType::Type type);

#ifndef NDEBUG
    void TryInsertSafepoint(BasicBlock *bb = nullptr, bool insertSP = false)
    {
        auto curBb = bb != nullptr ? bb : currentBb_;
        if (GetGraph()->IsBytecodeOptimizer() || curBb->IsOsrEntry() ||
            !g_options.IsCompilerEnforceSafepointPlacement()) {
            return;
        }
        if ((curBb->GetLastInst() == nullptr || curBb->GetLastInst()->GetOpcode() != Opcode::SafePoint) &&
            (insertSP || --safepointDistance_ <= 0)) {
            auto *sp = CreateSafePoint(curBb);
            currentBb_->AppendInst(sp);
#ifdef PANDA_COMPILER_DEBUG_INFO
            if (sp->GetPc() != INVALID_PC) {
                sp->SetCurrentMethod(method_);
            }
#endif
            COMPILER_LOG(DEBUG, IR_BUILDER) << *sp;
        }
    }
#endif

protected:
    template <typename T>
    void AddInstruction(T inst)
    {
        ASSERT(currentBb_);
        currentBb_->AppendInst(inst);
#ifdef PANDA_COMPILER_DEBUG_INFO
        if (inst->GetPc() != INVALID_PC) {
            inst->SetCurrentMethod(method_);
        }
#endif
        COMPILER_LOG(DEBUG, IR_BUILDER) << *inst;
    }

    template <typename T, typename... Ts>
    void AddInstruction(T inst, Ts... insts)
    {
        AddInstruction(inst);
        AddInstruction(insts...);
    }

    void UpdateDefinition(size_t vreg, Inst *inst);
    void UpdateDefinitionAcc(Inst *inst);
    void UpdateDefinitionLexEnv(Inst *inst);
    Inst *GetDefinition(size_t vreg);
    Inst *GetDefinitionAcc();
    Inst *GetEnvDefinition(uint8_t envIdx);

    void BuildCastToAnyString(const BytecodeInstruction *bcInst);

    Graph *GetGraph()
    {
        return graph_;
    }

    const Graph *GetGraph() const
    {
        return graph_;
    }

    const RuntimeInterface *GetRuntime() const
    {
        return runtime_;
    }

    RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }

    RuntimeInterface::MethodPtr GetMethod() const
    {
        return method_;
    }

    /// Get count of arguments for the method specified by id
    size_t GetMethodArgumentsCount(uintptr_t id) const;

private:
    void SyncWithGraph();

    void UpdateDefsForCatch();
    void UpdateDefsForLoopHead();

    size_t GetVRegsCount() const
    {
        return vregsAndArgsCount_ + 1 + GetGraph()->GetEnvCount();
    }

    void SetCallNativeFlags(CallInst *callInst, RuntimeInterface::MethodPtr method) const
    {
        bool isNativeApi = method != nullptr && GetRuntime()->IsMethodNativeApi(method);
        callInst->SetIsNative(isNativeApi);
        callInst->SetCanNativeException(isNativeApi && GetRuntime()->HasNativeException(method));
    }

    ConstantInst *FindOrCreate32BitConstant(uint32_t value);
    ConstantInst *FindOrCreateConstant(uint64_t value);
    ConstantInst *FindOrCreateAnyConstant(DataType::Any value);
    ConstantInst *FindOrCreateDoubleConstant(double value);
    ConstantInst *FindOrCreateFloatConstant(float value);

    enum SaveStateType {
        CHECK = 0,  // side_exit = true,  move_to_side_exit = true
        CALL,       // side_exit = false,  move_to_side_exit = false
        VIRT_CALL   // side_exit = true,  move_to_side_exit = false
    };

    ClassInst *CreateLoadAndInitClassGeneric(uint32_t classId, size_t pc);

    Inst *CreateCast(Inst *input, DataType::Type type, DataType::Type operandsType, size_t pc)
    {
        auto cast = GetGraph()->CreateInstCast(type, pc, input, operandsType);
        if (!input->HasType()) {
            input->SetType(operandsType);
        }
        return cast;
    }

    NewObjectInst *CreateNewObjectInst(size_t pc, uint32_t typeId, SaveStateInst *saveState, Inst *initClass)
    {
        auto newObj = graph_->CreateInstNewObject(DataType::REFERENCE, pc, initClass, saveState,
                                                  TypeIdMixin {typeId, graph_->GetMethod()});
        return newObj;
    }

    template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE = true>
    class BuildCallHelper {
    public:
        BuildCallHelper(const BytecodeInstruction *bcInst, InstBuilder *builder, Inst *additionalInput = nullptr);

        bool TryBuildIntrinsic();
        void BuildIntrinsic();
        void BuildDefaultIntrinsic(RuntimeInterface::IntrinsicId intrinsicId, bool isVirtual);
        void BuildDefaultStaticIntrinsic(RuntimeInterface::IntrinsicId intrinsicId);
        void BuildDefaultVirtualCallIntrinsic(RuntimeInterface::IntrinsicId intrinsicId);
        void BuildMonitorIntrinsic(bool isEnter);

        void BuildStaticCallIntrinsic(RuntimeInterface::IntrinsicId intrinsicId);
        void BuildVirtualCallIntrinsic(RuntimeInterface::IntrinsicId intrinsicId);

        void AddCallInstruction();
        void BuildCallInst(uint32_t classId);
        void BuildCallStaticInst(uint32_t classId);
        void BuildInitClassInstForCallStatic(uint32_t classId);

        void BuildCallVirtualInst();
        void SetCallArgs(Inst *additionalInput = nullptr);
        uint32_t GetClassId();
        auto GetGraph()
        {
            return builder_->GetGraph();
        }
        auto GetRuntime()
        {
            return builder_->GetRuntime();
        }
        auto GetMethod()
        {
            return builder_->GetMethod();
        }
        auto Builder()
        {
            return builder_;
        }

    private:
        InstBuilder *builder_ {};
        const BytecodeInstruction *bcInst_ {};
        RuntimeInterface::MethodPtr method_ {};
        uint32_t methodId_ {};
        uint32_t pc_ {};
        InputTypesMixin<DynamicInputsInst> *call_ {};
        Inst *resolver_ {};
        Inst *nullCheck_ {};
        SaveStateInst *saveState_ {};
        bool hasImplicitArg_ {};
    };
    Inst *GetArgDefinition(const BytecodeInstruction *bcInst, size_t idx, bool accRead, bool isRange = false);
    Inst *GetArgDefinitionRange(const BytecodeInstruction *bcInst, size_t idx);
    template <bool IS_VIRTUAL>
    void AddArgNullcheckIfNeeded(RuntimeInterface::IntrinsicId intrinsic, Inst *inst, Inst *saveState, size_t bcAddr);
    void BuildMonitor(const BytecodeInstruction *bcInst, Inst *def, bool isEnter);
    Inst *BuildFloatInst(const BytecodeInstruction *bcInst);
    template <bool IS_RANGE, bool ACC_READ>
    void BuildIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead);
    template <bool IS_RANGE, bool ACC_READ>
    void BuildDefaultIntrinsic(bool isVirtual, const BytecodeInstruction *bcInst);
    void BuildAbsIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    template <Opcode OPCODE>
    void BuildBinaryOperationIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildSqrtIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildIsNanIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildStringLengthIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildStringIsEmptyIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildCharIsUpperCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildCharToUpperCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildCharIsLowerCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildCharToLowerCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
    void BuildMonitorIntrinsic(const BytecodeInstruction *bcInst, bool isEnter, bool accRead);
    virtual void BuildThrow(const BytecodeInstruction *bcInst);
    void BuildLenArray(const BytecodeInstruction *bcInst);
    virtual void BuildNewArray(const BytecodeInstruction *bcInst);
    virtual void BuildNewObject(const BytecodeInstruction *bcInst);
    virtual void BuildLoadConstArray(const BytecodeInstruction *bcInst);
    void BuildLoadConstStringArray(const BytecodeInstruction *bcInst);
    template <typename T>
    void BuildUnfoldLoadConstArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                   const pandasm::LiteralArray &litArray);
    template <typename T>
    void BuildUnfoldLoadConstPrimitiveArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                            const pandasm::LiteralArray &litArray, NewArrayInst *arrayInst);
    template <typename T>
    void BuildUnfoldLoadConstStringArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                         const pandasm::LiteralArray &litArray, NewArrayInst *arrayInst);
    void BuildInitString(const BytecodeInstruction *bcInst);
    virtual void BuildInitObject(const BytecodeInstruction *bcInst, bool isRange);
    CallInst *BuildCallStaticForInitObject(const BytecodeInstruction *bcInst, uint32_t methodId, Inst **resolver);
    void BuildMultiDimensionalArrayObject(const BytecodeInstruction *bcInst, bool isRange);
    void BuildInitObjectMultiDimensionalArray(const BytecodeInstruction *bcInst, bool isRange);
    template <bool IS_ACC_WRITE>
    void BuildLoadObject(const BytecodeInstruction *bcInst, DataType::Type type);
    template <bool IS_ACC_READ>
    void BuildStoreObject(const BytecodeInstruction *bcInst, DataType::Type type);
    Inst *BuildStoreObjectInst(const BytecodeInstruction *bcInst, DataType::Type type, RuntimeInterface::FieldPtr field,
                               uint32_t fieldId, Inst **resolveInst);
    virtual void BuildLoadStatic(const BytecodeInstruction *bcInst, DataType::Type type);
    Inst *BuildLoadStaticInst(size_t pc, DataType::Type type, uint32_t typeId, Inst *saveState);
    virtual void BuildStoreStatic(const BytecodeInstruction *bcInst, DataType::Type type);
    Inst *BuildStoreStaticInst(const BytecodeInstruction *bcInst, DataType::Type type, uint32_t typeId,
                               Inst *storeInput, Inst *saveState);
    virtual void BuildCheckCast(const BytecodeInstruction *bcInst);
    virtual void BuildIsInstance(const BytecodeInstruction *bcInst);
    Inst *BuildLoadClass(RuntimeInterface::IdType typeId, size_t pc, Inst *saveState);
    virtual void BuildLoadArray(const BytecodeInstruction *bcInst, DataType::Type type);
    virtual void BuildStoreArray(const BytecodeInstruction *bcInst, DataType::Type type);
    template <bool CREATE_REF_CHECK>
    void BuildStoreArrayInst(const BytecodeInstruction *bcInst, DataType::Type type, Inst *arrayRef, Inst *index,
                             Inst *value);
    std::tuple<SaveStateInst *, Inst *, LengthMethodInst *, BoundsCheckInst *> BuildChecksBeforeArray(
        size_t pc, Inst *arrayRef, bool withNullcheck = true);
    template <Opcode OPCODE>
    void BuildLoadFromPool(const BytecodeInstruction *bcInst);
    void BuildCastToAnyNumber(const BytecodeInstruction *bcInst);
    AnyTypeCheckInst *BuildAnyTypeCheckInst(size_t bcAddr, Inst *input, Inst *saveState,
                                            AnyBaseType type = AnyBaseType::UNDEFINED_TYPE);
    void InitAnyTypeCheckInst(AnyTypeCheckInst *anyCheck, bool typeWasProfiled = false,
                              profiling::AnyInputType allowedInputType = {})
    {
        anyCheck->SetAllowedInputType(allowedInputType);
        anyCheck->SetIsTypeWasProfiled(typeWasProfiled);
    }

    void BuildStringGetIntrinsic(const BytecodeInstruction *bcInst, bool accRead,
                                 RuntimeInterface::IntrinsicId intrinsicId);
    bool TryBuildStringCharAtIntrinsic(const BytecodeInstruction *bcInst, bool accRead);

    template <bool IS_ACC_WRITE>
    void BuildLoadFromAnyByName(const BytecodeInstruction *bcInst, DataType::Type type);
    template <bool IS_ACC_WRITE>
    void BuildStoreFromAnyByName(const BytecodeInstruction *bcInst, DataType::Type type);
    void BuildLoadFromAnyByIdx(const BytecodeInstruction *bcInst, DataType::Type type);
    void BuildStoreFromAnyByIdx(const BytecodeInstruction *bcInst, DataType::Type type);
    void BuildLoadFromAnyByVal(const BytecodeInstruction *bcInst, DataType::Type type);
    void BuildStoreFromAnyByVal(const BytecodeInstruction *bcInst, DataType::Type type);
    void BuildIsInstanceAny(const BytecodeInstruction *bcInst, DataType::Type type);
    template <bool IS_ACC_WRITE>
    void BuildAnyCall(const BytecodeInstruction *bcInst);

#include "inst_builder_extensions.inl.h"

    auto GetClassId() const
    {
        return classId_;
    }

    Marker GetNoTypeMarker() const
    {
        return noTypeMarker_;
    }

    Marker GetVisitedBlockMarker() const
    {
        return visitedBlockMarker_;
    }

    bool ForceUnresolved() const
    {
#ifndef NDEBUG
        return g_options.IsCompilerForceUnresolved() && !graph_->IsBytecodeOptimizer();
#else
        return false;
#endif
    }

    void SetTypeRec(Inst *inst, DataType::Type type);

    /// Convert Panda bytecode type to COMPILER IR type
    static DataType::Type ConvertPbcType(panda_file::Type type);

    /// Get return type of the method specified by id
    DataType::Type GetMethodReturnType(uintptr_t id) const;
    /// Get type of argument of the method specified by id
    DataType::Type GetMethodArgumentType(uintptr_t id, size_t index) const;
    /// Get return type of currently compiling method
    DataType::Type GetCurrentMethodReturnType() const;
    /// Get type of argument of currently compiling method
    DataType::Type GetCurrentMethodArgumentType(size_t index) const;
    /// Get count of arguments of currently compiling method
    size_t GetCurrentMethodArgumentsCount() const;

    template <bool IS_STATIC>
    bool IsInConstructor() const;

#ifndef PANDA_ETS_INTEROP_JS
    bool TryBuildInteropCall([[maybe_unused]] const BytecodeInstruction *bcInst, [[maybe_unused]] bool isRange,
                             [[maybe_unused]] bool accRead)
    {
        return false;
    }
#endif

#ifndef NDEBUG
    void ResetSafepointDistance()
    {
        safepointDistance_ = static_cast<int32_t>(g_options.GetCompilerSafepointDistanceLimit());
    }
#endif

private:
    static constexpr size_t ONE_FOR_OBJECT = 1;
    static constexpr size_t ONE_FOR_SSTATE = 1;

    Graph *graph_ {nullptr};
    RuntimeInterface *runtime_ {nullptr};
    BasicBlock *currentBb_ {nullptr};

    RuntimeInterface::MethodProfile methodProfile_ {};

    // Definitions vector of currently processed basic block
    InstVector *currentDefs_ {nullptr};
    // Result of LoadFromConstantPool which will be added to SaveState inputs
    Inst *additionalDef_ {nullptr};
    // Contains definitions of the virtual registers in all basic blocks
    ArenaVector<InstVector> defs_;

    RuntimeInterface::MethodPtr method_ {nullptr};
    // Set to true if builder failed to build IR
    bool failed_ {false};
    // Number of virtual registers and method arguments
    const size_t vregsAndArgsCount_;
    // Marker for instructions with undefined type in the building phase
    Marker noTypeMarker_;
    Marker visitedBlockMarker_;

    // Pointer to start position of bytecode instructions buffer
    const uint8_t *instructionsBuf_ {nullptr};

    CallInst *callerInst_ {nullptr};
    uint32_t inliningDepth_ {0};
    size_t classId_;
#ifndef NDEBUG
    int32_t safepointDistance_ {0};
#endif
#include "intrinsics_ir_build.inl.h"
};
}  // namespace ark::compiler

#endif  // PANDA_INST_BUILDER_H
