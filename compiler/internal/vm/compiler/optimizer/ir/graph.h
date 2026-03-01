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

#ifndef COMPILER_OPTIMIZER_IR_GRAPH_H
#define COMPILER_OPTIMIZER_IR_GRAPH_H

#include <cstdint>
#include <algorithm>
#include <optional>
#include "compiler_events_gen.h"
#include "inst.h"
#include "marker.h"
#include "optimizer/pass_manager.h"
#include "utils/arena_containers.h"

namespace panda {
class Method;
class CodeAllocator;
}  // namespace panda

namespace panda::compiler {
class BasicBlock;
class Graph;
class RuntimeInfo;
class PassManager;
class LivenessAnalyzer;
class DominatorsTree;
class Rpo;
class Loop;
class ParameterInfo;

/**
 * Specifies graph compilation mode.
 */
class GraphMode {
public:
    explicit GraphMode(uint32_t value) : value_(value) {}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_GRAPH_MODE(name)                    \
    static GraphMode name(bool set = true)          \
    {                                               \
        return GraphMode(Flag##name ::Encode(set)); \
    }                                               \
    void Set##name(bool v)                          \
    {                                               \
        Flag##name ::Set(v, &value_);               \
    }                                               \
    bool Is##name() const                           \
    {                                               \
        return Flag##name ::Get(value_);            \
    }

    DECLARE_GRAPH_MODE(Osr);
    // The graph is used in BytecodeOptimizer mode
    DECLARE_GRAPH_MODE(BytecodeOpt);
    // The method from dynamic language
    DECLARE_GRAPH_MODE(DynamicMethod);
    // Graph will be compiled with native calling convention
    DECLARE_GRAPH_MODE(Native);
    // FastPath from compiled code to runtime
    DECLARE_GRAPH_MODE(FastPath);
    // Boundary frame is used for compiled code
    DECLARE_GRAPH_MODE(Boundary);
    // Graph will be compiled for calling inside interpreter
    DECLARE_GRAPH_MODE(Interpreter);
    // Graph will be compiled for interpreter main loop
    DECLARE_GRAPH_MODE(InterpreterEntry);

#undef DECLARE_GRAPH_MODE

    bool SupportManagedCode() const
    {
        return !IsNative() && !IsFastPath() && !IsBoundary() && !IsInterpreter() && !IsInterpreterEntry();
    }

    void Dump(std::ostream &stm);

private:
    using FlagOsr = BitField<bool, 0, 1>;
    using FlagBytecodeOpt = FlagOsr::NextFlag;
    using FlagDynamicMethod = FlagBytecodeOpt::NextFlag;
    using FlagNative = FlagDynamicMethod::NextFlag;
    using FlagFastPath = FlagNative::NextFlag;
    using FlagBoundary = FlagFastPath::NextFlag;
    using FlagInterpreter = FlagBoundary::NextFlag;
    using FlagInterpreterEntry = FlagInterpreter::NextFlag;

    uint32_t value_ {0};

    friend GraphMode operator|(GraphMode a, GraphMode b);
};

inline GraphMode operator|(GraphMode a, GraphMode b)
{
    return GraphMode(a.value_ | b.value_);
}

using EncodeDataType = Span<uint8_t>;

class Graph final : public MarkerMgr {
public:
    explicit Graph(ArenaAllocator *allocator, ArenaAllocator *local_allocator, Arch arch)
        : Graph(allocator, local_allocator, arch, false)
    {
    }

    Graph(ArenaAllocator *allocator, ArenaAllocator *local_allocator, Arch arch, bool osr_mode)
        : Graph(allocator, local_allocator, arch, nullptr, GetDefaultRuntime(), osr_mode)
    {
    }

    Graph(ArenaAllocator *allocator, ArenaAllocator *local_allocator, Arch arch, bool dynamic_method, bool bytecode_opt)
        : Graph(allocator, local_allocator, arch, nullptr, GetDefaultRuntime(), false, nullptr, dynamic_method,
                bytecode_opt)
    {
    }

    Graph(ArenaAllocator *allocator, ArenaAllocator *local_allocator, Arch arch, RuntimeInterface::MethodPtr method,
          RuntimeInterface *runtime, bool osr_mode)
        : Graph(allocator, local_allocator, arch, method, runtime, osr_mode, nullptr)
    {
    }

    Graph(ArenaAllocator *allocator, ArenaAllocator *local_allocator, Arch arch, RuntimeInterface::MethodPtr method,
          RuntimeInterface *runtime, bool osr_mode, Graph *parent, bool dynamic_method = false,
          bool bytecode_opt = false)
        : Graph(allocator, local_allocator, arch, method, runtime, parent,
                GraphMode::Osr(osr_mode) | GraphMode::BytecodeOpt(bytecode_opt) |
                    GraphMode::DynamicMethod(dynamic_method))
    {
    }

    Graph(ArenaAllocator *allocator, ArenaAllocator *local_allocator, Arch arch, RuntimeInterface::MethodPtr method,
          RuntimeInterface *runtime, Graph *parent, GraphMode mode)
        : ALLOCATOR(allocator),
          LOCAL_ALLOCATOR(local_allocator),
          arch_(arch),
          vector_bb_(allocator->Adapter()),
          throwable_insts_(allocator->Adapter()),
          runtime_(runtime),
          method_(method),
          pass_manager_(this, parent != nullptr ? parent->GetPassManager() : nullptr),
          event_writer_(runtime->GetClassNameFromMethod(method), runtime->GetMethodName(method)),
          mode_(mode),
          single_implementation_list_(allocator->Adapter()),
          try_begin_blocks_(allocator->Adapter()),
          spilled_constants_(allocator->Adapter()),
          parent_graph_(parent)
    {
        SetNeedCleanup(true);
    }

    ~Graph() override;

    Graph *CreateChildGraph(RuntimeInterface::MethodPtr method)
    {
        auto graph = GetAllocator()->New<Graph>(GetAllocator(), GetLocalAllocator(), GetArch(), method, GetRuntime(),
                                                this, mode_);
        return graph;
    }

    /// Get default runtime interface object
    static RuntimeInterface *GetDefaultRuntime()
    {
        static RuntimeInterface runtime_interface;
        return &runtime_interface;
    }

    Arch GetArch() const
    {
        return arch_;
    }

    void AddBlock(BasicBlock *block);
#ifndef NDEBUG
    void AddBlock(BasicBlock *block, uint32_t id);
#endif
    void DisconnectBlock(BasicBlock *block, bool remove_last_inst = true, bool fix_dom_tree = true);
    void DisconnectBlockRec(BasicBlock *block, bool remove_last_inst = true, bool fix_dom_tree = true);

    void EraseBlock(BasicBlock *block);
    void RestoreBlock(BasicBlock *block);
    // Remove empty block. Block must have one successor and no Phis.
    void RemoveEmptyBlock(BasicBlock *block);

    // Remove empty block. Block may have Phis and can't be a loop pre-header.
    void RemoveEmptyBlockWithPhis(BasicBlock *block, bool irr_loop = false);

    // Remove block predecessors.
    void RemovePredecessors(BasicBlock *block, bool remove_last_inst = true);

    // Remove block successors.
    void RemoveSuccessors(BasicBlock *block);

    // Remove unreachable blocks.
    void RemoveUnreachableBlocks();

    // get end block
    BasicBlock *GetEndBlock()
    {
        return end_block_;
    }

    BasicBlock *GetEndBlock() const
    {
        return end_block_;
    }
    // set end block
    void SetEndBlock(BasicBlock *end_block)
    {
        end_block_ = end_block;
    }
    bool HasEndBlock()
    {
        return end_block_ != nullptr;
    }
    // get start block
    BasicBlock *GetStartBlock()
    {
        return start_block_;
    }
    BasicBlock *GetStartBlock() const
    {
        return start_block_;
    }
    // set start block
    void SetStartBlock(BasicBlock *start_block)
    {
        start_block_ = start_block;
    }
    // get vector_bb_
    const ArenaVector<BasicBlock *> &GetVectorBlocks() const
    {
        return vector_bb_;
    }

    size_t GetAliveBlocksCount() const
    {
        return std::count_if(vector_bb_.begin(), vector_bb_.end(), [](BasicBlock *block) { return block != nullptr; });
    }

    PassManager *GetPassManager()
    {
        return &pass_manager_;
    }
    const PassManager *GetPassManager() const
    {
        return &pass_manager_;
    }

    const ArenaVector<BasicBlock *> &GetBlocksRPO() const;

    const ArenaVector<BasicBlock *> &GetBlocksLinearOrder() const;

    template <class Callback>
    void VisitAllInstructions(Callback callback);

    /// Main allocator for graph, all related to Graph data should be allocated via this allocator.
    ArenaAllocator *GetAllocator() const
    {
        return ALLOCATOR;
    }
    /// Allocator for temproray usage, when allocated data is no longer needed after optimization/analysis finished.
    ArenaAllocator *GetLocalAllocator() const
    {
        return LOCAL_ALLOCATOR;
    }
    bool IsDFConstruct() const
    {
        return FlagDFConstruct::Get(bit_fields_);
    }
    void SetDFConstruct()
    {
        FlagDFConstruct::Set(true, &bit_fields_);
    }

    bool IsAotMode() const
    {
        return false;
    }

    bool IsOfflineCompilationMode() const
    {
        return IsAotMode() || GetMode().IsInterpreter();
    }

    bool IsDefaultLocationsInit() const
    {
        return FlagDefaultLocationsInit::Get(bit_fields_);
    }
    void SetDefaultLocationsInit()
    {
        FlagDefaultLocationsInit::Set(true, &bit_fields_);
    }
#ifndef NDEBUG
    bool IsRegAllocApplied() const
    {
        return FlagRegallocApplied::Get(bit_fields_);
    }
    void SetRegAllocApplied()
    {
        FlagRegallocApplied::Set(true, &bit_fields_);
    }
    bool IsRegAccAllocApplied() const
    {
        return FlagRegaccallocApplied::Get(bit_fields_);
    }
    void SetRegAccAllocApplied()
    {
        FlagRegaccallocApplied::Set(true, &bit_fields_);
    }
    bool IsInliningComplete() const
    {
        return FlagInliningComplete::Get(bit_fields_);
    }
    void SetInliningComplete()
    {
        FlagInliningComplete::Set(true, &bit_fields_);
    }
    bool IsSchedulerComplete() const
    {
        return FlagSchedulerComplete::Get(bit_fields_);
    }
    void SetSchedulerComplete()
    {
        FlagSchedulerComplete::Set(true, &bit_fields_);
    }
    bool IsLowLevelInstructionsEnabled() const
    {
        return FlagLowLevelInstnsEnabled::Get(bit_fields_);
    }
    void SetLowLevelInstructionsEnabled()
    {
        FlagLowLevelInstnsEnabled::Set(true, &bit_fields_);
    }
#else
    bool IsRegAllocApplied() const
    {
        return false;
    }
#endif  // NDEBUG

    void SetData(EncodeDataType data)
    {
        data_ = data;
    }

    EncodeDataType GetData() const
    {
        return data_;
    }

    EncodeDataType GetData()
    {
        return data_;
    }

    void SetCodeInfo(Span<uint8_t> data)
    {
        code_info_data_ = data.SubSpan<const uint8_t>(0, data.size());
    }

    Span<const uint8_t> GetCodeInfoData() const
    {
        return code_info_data_;
    }

    void DumpUsedRegs(std::ostream &out = std::cerr, const char *prefix = nullptr) const
    {
        if (prefix != nullptr) {
            out << prefix;
        }
        out << "'\n  used scalar regs: ";
        if (used_regs_ != nullptr) {
            for (unsigned i = 0; i < used_regs_->size(); ++i) {
                if (used_regs_->at(i)) {
                    out << i << " ";
                }
            }
        }
        out << "\n  used float  regs: ";
        if (used_regs_ != nullptr) {
            for (unsigned i = 0; i < used_vregs_->size(); ++i) {
                if (used_vregs_->at(i)) {
                    out << i << " ";
                }
            }
        }
        out << std::endl;
    }

    // Get registers mask which used in graph
    template <DataType::Type reg_type>
    ArenaVector<bool> *GetUsedRegs() const
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (reg_type == DataType::INT64) {
            return used_regs_;
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (reg_type == DataType::FLOAT64) {
            return used_vregs_;
        }
        UNREACHABLE();
        return nullptr;
    }

    void SetRegUsage(Register reg, DataType::Type type)
    {
        ASSERT(reg != INVALID_REG);
        if (DataType::IsFloatType(type)) {
            SetUsedReg<DataType::FLOAT64>(reg);
        } else {
            SetUsedReg<DataType::INT64>(reg);
        }
    }

    template <DataType::Type reg_type>
    void SetUsedReg(Register reg)
    {
        ArenaVector<bool> *graph_regs = nullptr;
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (reg_type == DataType::INT64) {
            graph_regs = used_regs_;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (reg_type == DataType::FLOAT64) {
            graph_regs = used_vregs_;
        } else {
            UNREACHABLE();
        }
        CHECK_NOT_NULL(graph_regs);
        ASSERT(reg < graph_regs->size());
        (*graph_regs)[reg] = true;
    }

    template <DataType::Type reg_type>
    void InitUsedRegs(const ArenaVector<bool> *used_regs)
    {
        if (used_regs == nullptr) {
            return;
        }
        ArenaVector<bool> *graph_regs = nullptr;
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (reg_type == DataType::INT64) {
            used_regs_ = GetAllocator()->New<ArenaVector<bool>>(GetAllocator()->Adapter());
            graph_regs = used_regs_;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (reg_type == DataType::FLOAT64) {
            used_vregs_ = GetAllocator()->New<ArenaVector<bool>>(GetAllocator()->Adapter());
            graph_regs = used_vregs_;
        } else {
            UNREACHABLE();
        }
        CHECK_NOT_NULL(graph_regs);
        graph_regs->resize(used_regs->size());
        std::copy(used_regs->begin(), used_regs->end(), graph_regs->begin());
    }

    uint32_t GetStackSlotsCount() const
    {
        return stack_slot_count_;
    }

    void SetStackSlotsCount(uint32_t stack_slot_count)
    {
        stack_slot_count_ = stack_slot_count;
    }

    void UpdateStackSlotsCount(uint32_t stack_slot_count)
    {
        stack_slot_count_ = std::max(stack_slot_count_, stack_slot_count);
    }

    uint32_t GetParametersSlotsCount() const;

    uint32_t GetExtSlotsStart() const
    {
        return ext_stack_slot_;
    }

    void SetExtSlotsStart(uint32_t ext_stack_slot)
    {
        ext_stack_slot_ = ext_stack_slot;
    }

    BasicBlock *CreateEmptyBlock(uint32_t guest_pc = INVALID_PC);
    BasicBlock *CreateEmptyBlock(BasicBlock *base_block);
#ifndef NDEBUG
    BasicBlock *CreateEmptyBlock(uint32_t id, uint32_t guest_pc);
#endif
    BasicBlock *CreateStartBlock();
    BasicBlock *CreateEndBlock(uint32_t guest_pc = INVALID_PC);
    ConstantInst *GetFirstConstInst()
    {
        return first_const_inst_;
    }
    void SetFirstConstInst(ConstantInst *const_inst)
    {
        first_const_inst_ = const_inst;
    }

    Inst *GetNullPtrInst() const
    {
        return nullptr_inst_;
    }
    bool HasNullPtrInst() const
    {
        return nullptr_inst_ != nullptr;
    }
    void UnsetNullPtrInst()
    {
        ASSERT(HasNullPtrInst());
        nullptr_inst_ = nullptr;
    }

    /// Find constant in the list, return nullptr if not found
    ConstantInst *FindConstant(DataType::Type type, uint64_t value);
    /// Find constant in the list or create new one and insert at the end
    template <typename T>
    ConstantInst *FindOrCreateConstant(T value);

    /**
     * Find constant that is equal to the given one specified by inst. If not found, add inst to the graph.
     * @param inst Constant instruction to be added
     * @return Found instruction or inst if not found
     */
    ConstantInst *FindOrAddConstant(ConstantInst *inst);

    ParameterInst *AddNewParameter(uint16_t arg_number);

    ParameterInst *AddNewParameter(uint16_t arg_number, DataType::Type type)
    {
        ParameterInst *param = AddNewParameter(arg_number);
        param->SetType(type);
        return param;
    }

    /*
     * The function remove the ConstantInst from the graph list
     * !NOTE ConstantInst isn't removed from BasicBlock list
     */
    void RemoveConstFromList(ConstantInst *const_inst);

    ConstantInst *GetSpilledConstant(ImmTableSlot slot)
    {
        ASSERT(static_cast<size_t>(slot) < spilled_constants_.size());
        return spilled_constants_[slot];
    }

    ImmTableSlot AddSpilledConstant(ConstantInst *const_inst)
    {
        // Constant already in the table
        auto current_slot = const_inst->GetImmTableSlot();
        if (current_slot != INVALID_IMM_TABLE_SLOT) {
            ASSERT(spilled_constants_[current_slot] == const_inst);
            return current_slot;
        }

        auto count = spilled_constants_.size();
        if (count >= MAX_NUM_IMM_SLOTS) {
            return INVALID_IMM_TABLE_SLOT;
        }
        spilled_constants_.push_back(const_inst);
        const_inst->SetImmTableSlot(count);
        return ImmTableSlot(count);
    }

    ImmTableSlot FindSpilledConstantSlot(ConstantInst *const_inst) const
    {
        auto slot = std::find(spilled_constants_.begin(), spilled_constants_.end(), const_inst);
        if (slot == spilled_constants_.end()) {
            return INVALID_IMM_TABLE_SLOT;
        }
        return std::distance(spilled_constants_.begin(), slot);
    }

    size_t GetSpilledConstantsCount() const
    {
        return spilled_constants_.size();
    }

    bool HasAvailableConstantSpillSlots() const
    {
        return GetSpilledConstantsCount() < MAX_NUM_IMM_SLOTS;
    }

    auto begin()  // NOLINT(readability-identifier-naming)
    {
        return vector_bb_.begin();
    }
    auto begin() const  // NOLINT(readability-identifier-naming)
    {
        return vector_bb_.begin();
    }
    auto end()  // NOLINT(readability-identifier-naming)
    {
        return vector_bb_.end();
    }
    auto end() const  // NOLINT(readability-identifier-naming)
    {
        return vector_bb_.end();
    }

    void Dump(std::ostream *out) const;

    Loop *GetRootLoop()
    {
        return root_loop_;
    }
    const Loop *GetRootLoop() const
    {
        return root_loop_;
    }

    void SetRootLoop(Loop *root_loop)
    {
        root_loop_ = root_loop;
    }

    void SetHasIrreducibleLoop(bool has_irr_loop)
    {
        FlagIrredicibleLoop::Set(has_irr_loop, &bit_fields_);
    }

    void SetHasInfiniteLoop(bool has_inf_loop)
    {
        FlagInfiniteLoop::Set(has_inf_loop, &bit_fields_);
    }

    void SetHasFloatRegs()
    {
        FlagFloatRegs::Set(true, &bit_fields_);
    }

    bool HasLoop() const;
    bool HasIrreducibleLoop() const;
    bool HasInfiniteLoop() const;
    bool HasFloatRegs() const;

    /**
     * Try-catch info
     * Vector of begin try-blocks in order they were declared in the bytecode
     */
    void AppendTryBeginBlock(const BasicBlock *block)
    {
        try_begin_blocks_.push_back(block);
    }

    void EraseTryBeginBlock(const BasicBlock *block)
    {
        auto it = std::find(try_begin_blocks_.begin(), try_begin_blocks_.end(), block);
        if (it == try_begin_blocks_.end()) {
            ASSERT(false && "Trying to remove non try_begin block");
            return;
        }
        try_begin_blocks_.erase(it);
    }

    const auto &GetTryBeginBlocks() const
    {
        return try_begin_blocks_;
    }

    void AppendThrowableInst(const Inst *inst, BasicBlock *catch_handler)
    {
        auto it = throwable_insts_.emplace(inst, GetAllocator()->Adapter()).first;
        it->second.push_back(catch_handler);
    }

    bool IsInstThrowable(const Inst *inst) const
    {
        return throwable_insts_.count(inst) > 0;
    }

    void RemoveThrowableInst(const Inst *inst);
    void ReplaceThrowableInst(Inst *old_inst, Inst *new_inst);

    const auto &GetThrowableInstHandlers(const Inst *inst) const
    {
        ASSERT(IsInstThrowable(inst));
        return throwable_insts_.at(inst);
    }

    void ClearTryCatchInfo()
    {
        throwable_insts_.clear();
        try_begin_blocks_.clear();
    }

    void DumpThrowableInsts(std::ostream *out) const;

    /**
     * Run pass specified by template argument T.
     * Optimization passes might take additional arguments that will passed to Optimization's constructor.
     * Analyses can't take additional arguments.
     * @tparam T Type of pass
     * @param args Additional arguments for optimizations passes
     * @return true if pass was successful
     */
    template <typename T, typename... Args>
    bool RunPass(Args... args)
    {
        ASSERT(GetPassManager());
        return pass_manager_.RunPass<T>(std::forward<Args>(args)...);
    }
    template <typename T, typename... Args>
    bool RunPass(Args... args) const
    {
        ASSERT(GetPassManager());
        return pass_manager_.RunPass<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    bool RunPass(T *pass)
    {
        ASSERT(GetPassManager());
        return pass_manager_.RunPass(pass, GetLocalAllocator()->GetAllocatedSize());
    }

    /**
     * Get analysis instance.
     * All analyses are reside in Graph object in composition relationship.
     * @tparam T Type of analysis
     * @return Reference to analysis instance
     */
    template <typename T>
    T &GetAnalysis()
    {
        ASSERT(GetPassManager());
        return GetPassManager()->GetAnalysis<T>();
    }
    template <typename T>
    const T &GetAnalysis() const
    {
        ASSERT(GetPassManager());
        return pass_manager_.GetAnalysis<T>();
    }

    /**
     * Same as GetAnalysis but additionaly checck that analysis in valid state.
     * @tparam T Type of analysis
     * @return Reference to analysis instance
     */
    template <typename T>
    T &GetValidAnalysis()
    {
        RunPass<T>();
        ASSERT(IsAnalysisValid<T>());
        return GetAnalysis<T>();
    }
    template <typename T>
    const T &GetValidAnalysis() const
    {
        RunPass<T>();
        ASSERT(IsAnalysisValid<T>());
        return GetAnalysis<T>();
    }

    /**
     * Return true if Analysis valid, false otherwise
     * @tparam T Type of analysis
     */
    template <typename T>
    bool IsAnalysisValid() const
    {
        return GetAnalysis<T>().IsValid();
    }

    /**
     * Reset valid state of specified analysis
     * @tparam T Type of analysis
     */
    template <typename T>
    void InvalidateAnalysis()
    {
        ASSERT(GetPassManager());
        GetPassManager()->GetAnalysis<T>().SetValid(false);
    }

    /// Accessors to the number of current instruction id.
    auto GetCurrentInstructionId() const
    {
        return instr_current_id_;
    }
    auto SetCurrentInstructionId(size_t v)
    {
        instr_current_id_ = v;
    }

    /// RuntimeInterface accessors
    RuntimeInterface *GetRuntime() const
    {
        return runtime_;
    }
    void SetRuntime(RuntimeInterface *runtime)
    {
        runtime_ = runtime;
    }
    auto GetMethod() const
    {
        return method_;
    }
    auto SetMethod(RuntimeInterface::MethodPtr method)
    {
        method_ = method;
    }

    void ResetParameterInfo();

    EventWriter &GetEventWriter()
    {
        return event_writer_;
    }

    // clang-format off

    /**
     * Create instruction by opcode
     */
    [[nodiscard]] Inst* CreateInst(Opcode opc) const
    {
        switch (opc) {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, BASE, ...)                                      \
            case Opcode::OPCODE: {                                       \
                auto inst = Inst::New<BASE>(ALLOCATOR, Opcode::OPCODE);  \
                inst->SetId(instr_current_id_++);                        \
                return inst;                                             \
                }
                OPCODE_LIST(INST_DEF)

#undef INST_DEF
            default:
                return nullptr;
        }
    }
    /**
     * Define creation methods for all opcodes
     */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, BASE, ...)                                               \
    template <typename... Args>                                                   \
    [[nodiscard]] BASE* CreateInst##OPCODE(Args&&... args) const {                \
        auto inst = Inst::New<BASE>(ALLOCATOR, Opcode::OPCODE, std::forward<Args>(args)...);  \
        inst->SetId(instr_current_id_++); \
        return inst; \
    }
    OPCODE_LIST(INST_DEF)

#undef INST_DEF
    // clang-format on

    uint32_t GetBitFields()
    {
        return bit_fields_;
    }

    void SetBitFields(uint32_t bit_fields)
    {
        bit_fields_ = bit_fields;
    }

    bool NeedCleanup() const
    {
        return FlagNeedCleanup::Get(bit_fields_);
    }

    void SetNeedCleanup(bool v)
    {
        FlagNeedCleanup::Set(v, &bit_fields_);
    }

    bool IsOsrMode() const
    {
        return mode_.IsOsr();
    }

    bool IsBytecodeOptimizer() const
    {
        return mode_.IsBytecodeOpt();
    }

    bool IsDynamicMethod() const
    {
        return mode_.IsDynamicMethod();
    }

    bool SupportManagedCode() const
    {
        return mode_.SupportManagedCode();
    }

    GraphMode GetMode() const
    {
        return mode_;
    }

    void SetMode(GraphMode mode)
    {
        mode_ = mode;
    }

#ifndef NDEBUG
    compiler::inst_modes::Mode GetCompilerMode()
    {
        if (IsBytecodeOptimizer()) {
            return compiler::inst_modes::BYTECODE_OPT;
        }
        if (SupportManagedCode()) {
            return compiler::inst_modes::JIT_AOT;
        }
        return compiler::inst_modes::IRTOC;
    }
#endif

    void AddSingleImplementationMethod(RuntimeInterface::MethodPtr method)
    {
        single_implementation_list_.push_back(method);
    }

    void SetDynamicMethod()
    {
        mode_.SetDynamicMethod(true);
    }

    auto &GetSingleImplementationList()
    {
        return single_implementation_list_;
    }

    Graph *GetParentGraph()
    {
        return parent_graph_;
    }

    Graph *GetOutermostParentGraph()
    {
        auto graph = this;
        while (graph->GetParentGraph() != nullptr) {
            graph = graph->GetParentGraph();
        }
        return graph;
    }

    void SetVRegsCount(size_t count)
    {
        vregs_count_ = count;
    }

    size_t GetVRegsCount() const
    {
        return vregs_count_;
    }

    int64_t GetBranchCounter(const BasicBlock *block, bool true_succ);

    /**
     * This class provides methods for ranged-based `for` loop over all parameters in the graph.
     */
    class ParameterList {
    public:
        class Iterator {
        public:
            explicit Iterator(Inst *inst) : inst_(inst) {}

            Iterator &operator++()
            {
                for (inst_ = inst_->GetNext(); inst_ != nullptr && inst_->GetOpcode() != Opcode::Parameter;
                     inst_ = inst_->GetNext()) {
                }
                return *this;
            }
            bool operator!=(const Iterator &other)
            {
                return inst_ != other.inst_;
            }
            Inst *operator*()
            {
                return inst_;
            }
            Inst *operator->()
            {
                return inst_;
            }

        private:
            Inst *inst_ {nullptr};
        };

        explicit ParameterList(const Graph *graph) : graph_(graph) {}

        // NOLINTNEXTLINE(readability-identifier-naming)
        Iterator begin();
        // NOLINTNEXTLINE(readability-identifier-naming)
        static Iterator end()
        {
            return Iterator(nullptr);
        }

    private:
        const Graph *graph_ {nullptr};
    };

    /**
     * Get list of all parameters
     * @return instance of the ParameterList class
     */
    ParameterList GetParameters() const
    {
        return ParameterList(this);
    }

    void InitDefaultLocations();

private:
    void AddConstInStartBlock(ConstantInst *const_inst);

    NO_MOVE_SEMANTIC(Graph);
    NO_COPY_SEMANTIC(Graph);

private:
    ArenaAllocator *const ALLOCATOR;
    ArenaAllocator *const LOCAL_ALLOCATOR;

    Arch arch_ {RUNTIME_ARCH};

    // List of blocks in insertion order.
    ArenaVector<BasicBlock *> vector_bb_;
    BasicBlock *start_block_ {nullptr};
    BasicBlock *end_block_ {nullptr};

    Loop *root_loop_ {nullptr};

    uint32_t bit_fields_ {0};
    using FlagDFConstruct = BitField<bool, 0, 1>;
    using FlagNeedCleanup = FlagDFConstruct::NextFlag;
    using FlagIrredicibleLoop = FlagNeedCleanup::NextFlag;
    using FlagInfiniteLoop = FlagIrredicibleLoop::NextFlag;
    using FlagFloatRegs = FlagInfiniteLoop::NextFlag;
    using FlagDefaultLocationsInit = FlagFloatRegs::NextFlag;
#ifndef NDEBUG
    using FlagRegallocApplied = FlagDefaultLocationsInit::NextFlag;
    using FlagRegaccallocApplied = FlagRegallocApplied::NextFlag;
    using FlagInliningComplete = FlagRegaccallocApplied::NextFlag;
    using FlagSchedulerComplete = FlagInliningComplete::NextFlag;
    using FlagLowLevelInstnsEnabled = FlagSchedulerComplete::NextFlag;
#endif  // NDEBUG

    // codegen data
    EncodeDataType data_;
    Span<const uint8_t> code_info_data_;
    ArenaVector<bool> *used_regs_ {nullptr};
    ArenaVector<bool> *used_vregs_ {nullptr};

    // TODO (a.popov) Replace by ArenaMap from throwable_inst* to try_inst*
    ArenaMap<const Inst *, ArenaVector<BasicBlock *>> throwable_insts_;

    mutable size_t instr_current_id_ {0};
    // first constant instruction in graph !TODO rewrite it to hash-map
    ConstantInst *first_const_inst_ {nullptr};
    Inst *nullptr_inst_ {nullptr};

    RuntimeInterface *runtime_ {nullptr};
    RuntimeInterface::MethodPtr method_ {nullptr};

    ParameterInfo *param_info_ {nullptr};

    mutable PassManager pass_manager_;
    EventWriter event_writer_;

    GraphMode mode_;

    ArenaVector<RuntimeInterface::MethodPtr> single_implementation_list_;
    ArenaVector<const BasicBlock *> try_begin_blocks_;
    ArenaVector<ConstantInst *> spilled_constants_;
    // Graph that inlines this graph
    Graph *parent_graph_ {nullptr};
    // Number of used stack slots
    uint32_t stack_slot_count_ {0};
    // Number of used stack slots for parameters
    uint32_t param_slots_count_ {0};
    // First language extension slot
    uint32_t ext_stack_slot_ {0};
    // Number of the virtual registers used in the compiled method (inlined methods aren't included).
    uint32_t vregs_count_ {0};
};

class MarkerHolder {
public:
    NO_COPY_SEMANTIC(MarkerHolder);
    NO_MOVE_SEMANTIC(MarkerHolder);

    explicit MarkerHolder(const Graph *graph) : graph_(graph), marker_(graph->NewMarker())
    {
        ASSERT(marker_ != UNDEF_MARKER);
    }

    ~MarkerHolder()
    {
        graph_->EraseMarker(marker_);
    }

    Marker GetMarker()
    {
        return marker_;
    }

private:
    const Graph *graph_;
    Marker marker_ {UNDEF_MARKER};
};

template <typename T>
ConstantInst *Graph::FindOrCreateConstant(T value)
{
    bool is_support_int32 = IsBytecodeOptimizer();
    if (first_const_inst_ == nullptr) {
        first_const_inst_ = CreateInstConstant(value, is_support_int32);
        AddConstInStartBlock(first_const_inst_);
        return first_const_inst_;
    }
    ConstantInst *current_const = first_const_inst_;
    ConstantInst *prev_const = nullptr;
    while (current_const != nullptr) {
        if (current_const->IsEqualConst(value, is_support_int32)) {
            return current_const;
        }
        prev_const = current_const;
        current_const = current_const->GetNextConst();
    }
    ASSERT(prev_const != nullptr);
    auto *new_const = CreateInstConstant(value, is_support_int32);
    AddConstInStartBlock(new_const);

    prev_const->SetNextConst(new_const);
    return new_const;
}

void InvalidateBlocksOrderAnalyzes(Graph *graph);
void MarkLoopExits(const Graph *graph, Marker marker);
void RemovePredecessorUpdateDF(BasicBlock *block, BasicBlock *rm_pred);
std::string GetMethodFullName(const Graph *graph, RuntimeInterface::MethodPtr method);
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_GRAPH_H
