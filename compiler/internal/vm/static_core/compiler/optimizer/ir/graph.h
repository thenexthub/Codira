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

#include "aot_data.h"
#include "basicblock.h"
#include "compiler_events_gen.h"
#include "inst.h"
#include "marker.h"
#include "optimizer/code_generator/method_properties.h"
#include "optimizer/pass_manager.h"
#include "optimizer/optimizations/string_flat_check.h"
#include "libarkbase/utils/arena_containers.h"
#include <algorithm>
#include <optional>

// defines required for AbcKit
#if !defined(NDEBUG) || defined(ENABLE_LIBABCKIT)
#define COMPILER_DEBUG_CHECKS
#endif
#ifdef ENABLE_LIBABCKIT
// CC-OFFNXT(G.PRE.02) should be with define
#define ABCKIT_MODE_CHECK(cond, action) \
    if (cond) {                         \
        action;                         \
    }
#else
// CC-OFFNXT(G.PRE.02) should be with define
#define ABCKIT_MODE_CHECK(cond, action)
#endif

namespace ark {
class Method;
class CodeAllocator;
}  // namespace ark

namespace ark::compiler {
class BasicBlock;
class Graph;
class RuntimeInfo;
class PassManager;
class LivenessAnalyzer;
class DominatorsTree;
class Rpo;
class BoundsRangeInfo;
class Loop;
class CodeInfoBuilder;

class Encoder;
class CallingConvention;
class ParameterInfo;
class RegistersDescription;
class RelocationHandler;

enum AliasType : uint8_t;

/// Specifies graph compilation mode.
class GraphMode {
public:
    explicit GraphMode(uint32_t value) : value_(value) {}
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_GRAPH_MODE_MODIFIERS(name)     \
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
#define DECLARE_GRAPH_MODE(name)                    \
    static GraphMode name(bool set = true)          \
    {                                               \
        /* CC-OFFNXT(G.PRE.05) function gen */      \
        return GraphMode(Flag##name ::Encode(set)); \
    }                                               \
    DECLARE_GRAPH_MODE_MODIFIERS(name)

    DECLARE_GRAPH_MODE(Osr);
    // The graph is used in BytecodeOptimizer mode
    DECLARE_GRAPH_MODE(BytecodeOpt);
    // The method from dynamic language
    DECLARE_GRAPH_MODE(DynamicMethod);
    // The method from dynamic language uses common calling convention
    DECLARE_GRAPH_MODE(DynamicStub);
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
    // Graph will be compiled for abckit
    DECLARE_GRAPH_MODE(AbcKit);
    // Declare an interpreter mode to use by the IrToc
    DECLARE_GRAPH_MODE(NativePlus);

#undef DECLARE_GRAPH_MODE
#undef DECLARE_GRAPH_MODE_MODIFIERS

    bool SupportManagedCode() const
    {
        return !IsNative() && !IsFastPath() && !IsBoundary() && !IsInterpreter() && !IsInterpreterEntry() &&
               !IsNativePlus();
    }

    void Dump(std::ostream &stm);

private:
    using FlagOsr = BitField<bool, 0, 1>;
    using FlagBytecodeOpt = FlagOsr::NextFlag;
    using FlagDynamicMethod = FlagBytecodeOpt::NextFlag;
    using FlagDynamicStub = FlagDynamicMethod::NextFlag;
    using FlagNative = FlagDynamicStub::NextFlag;
    using FlagFastPath = FlagNative::NextFlag;
    using FlagBoundary = FlagFastPath::NextFlag;
    using FlagInterpreter = FlagBoundary::NextFlag;
    using FlagInterpreterEntry = FlagInterpreter::NextFlag;
    using FlagAbcKit = FlagInterpreterEntry::NextFlag;
    using FlagNativePlus = FlagAbcKit::NextFlag;

    uint32_t value_ {0};

    friend GraphMode operator|(GraphMode a, GraphMode b);
};

inline GraphMode operator|(GraphMode a, GraphMode b)
{
    return GraphMode(a.value_ | b.value_);
}

using EncodeDataType = Span<uint8_t>;

class PANDA_PUBLIC_API Graph final : public MarkerMgr {
public:
    struct GraphArgs {
        ArenaAllocator *allocator;
        ArenaAllocator *localAllocator;
        Arch arch;
        RuntimeInterface::MethodPtr method;
        RuntimeInterface *runtime;
    };

    explicit Graph(ArenaAllocator *allocator, ArenaAllocator *localAllocator, Arch arch)
        : Graph({allocator, localAllocator, arch, nullptr, GetDefaultRuntime()}, false)
    {
    }

    Graph(ArenaAllocator *allocator, ArenaAllocator *localAllocator, Arch arch, bool osrMode)
        : Graph({allocator, localAllocator, arch, nullptr, GetDefaultRuntime()}, osrMode)
    {
    }

    Graph(ArenaAllocator *allocator, ArenaAllocator *localAllocator, Arch arch, bool dynamicMethod, bool bytecodeOpt)
        : Graph({allocator, localAllocator, arch, nullptr, GetDefaultRuntime()}, nullptr, false, dynamicMethod,
                bytecodeOpt)
    {
    }

    Graph(const GraphArgs &args, bool osrMode) : Graph(args, nullptr, osrMode) {}

    Graph(const GraphArgs &args, Graph *parent, bool osrMode, bool dynamicMethod = false, bool bytecodeOpt = false)
        : Graph(args, parent,
                GraphMode::Osr(osrMode) | GraphMode::BytecodeOpt(bytecodeOpt) | GraphMode::DynamicMethod(dynamicMethod))
    {
    }

    Graph(const GraphArgs &args, Graph *parent, GraphMode mode)
        : allocator_(args.allocator),
          localAllocator_(args.localAllocator),
          arch_(args.arch),
          vectorBb_(args.allocator->Adapter()),
          throwableInsts_(args.allocator->Adapter()),
          runtime_(args.runtime),
          method_(args.method),
          passManager_(this, parent != nullptr ? parent->GetPassManager() : nullptr),
          eventWriter_(args.runtime->GetClassNameFromMethod(args.method), args.runtime->GetMethodName(args.method)),
          mode_(mode),
          singleImplementationList_(args.allocator->Adapter()),
          tryBeginBlocks_(args.allocator->Adapter()),
          throwBlocks_(args.allocator->Adapter()),
          spilledConstants_(args.allocator->Adapter()),
          parentGraph_(parent),
          stringFlatCheckConstraint_(runtime_->IsUseAllStrings() && !StringFlatCheck::IsEnable(runtime_))
    {
        SetNeedCleanup(true);
        SetCanOptimizeNativeMethods(g_options.IsCompilerOptimizeNativeCalls() && (GetArch() != Arch::AARCH32) &&
                                    GetRuntime()->IsNativeMethodOptimizationEnabled());
    }

    ~Graph() override;

    Graph *CreateChildGraph(RuntimeInterface::MethodPtr method)
    {
        auto graph = GetAllocator()->New<Graph>(
            GraphArgs {GetAllocator(), GetLocalAllocator(), GetArch(), method, GetRuntime()}, this, mode_);
        ASSERT(graph != nullptr);
        graph->SetAotData(GetAotData());
        return graph;
    }

    /// Get default runtime interface object
    static RuntimeInterface *GetDefaultRuntime()
    {
        static RuntimeInterface runtimeInterface;
        return &runtimeInterface;
    }

    Arch GetArch() const
    {
        return arch_;
    }

    SourceLanguage GetLanguage() const
    {
        return lang_;
    }

    void SetLanguage(SourceLanguage lang)
    {
        lang_ = lang;
    }

    void AddBlock(BasicBlock *block);
#ifndef NDEBUG
    void AddBlock(BasicBlock *block, uint32_t id);
#endif
    void DisconnectBlock(BasicBlock *block, bool removeLastInst = true, bool fixDomTree = true);
    void DisconnectBlockRec(BasicBlock *block, bool removeLastInst = true, bool fixDomTree = true);

    void EraseBlock(BasicBlock *block);
    void RestoreBlock(BasicBlock *block);
    // Remove empty block. Block must have one successor and no Phis.
    void RemoveEmptyBlock(BasicBlock *block);

    // Remove empty block. Block may have Phis and can't be a loop pre-header.
    void RemoveEmptyBlockWithPhis(BasicBlock *block, bool irrLoop = false);

    // Remove block predecessors.
    void RemovePredecessors(BasicBlock *block, bool removeLastInst = true);

    // Remove block successors.
    void RemoveSuccessors(BasicBlock *block);

    // Remove unreachable blocks.
    void RemoveUnreachableBlocks();

    // get end block
    BasicBlock *GetEndBlock()
    {
        return endBlock_;
    }
    // set end block
    void SetEndBlock(BasicBlock *endBlock)
    {
        endBlock_ = endBlock;
    }
    bool HasEndBlock()
    {
        return endBlock_ != nullptr;
    }
    // get start block
    BasicBlock *GetStartBlock()
    {
        return startBlock_;
    }
    BasicBlock *GetStartBlock() const
    {
        return startBlock_;
    }
    // set start block
    void SetStartBlock(BasicBlock *startBlock)
    {
        startBlock_ = startBlock;
    }
    // get vector_bb_
    const ArenaVector<BasicBlock *> &GetVectorBlocks() const
    {
        return vectorBb_;
    }

    size_t GetAliveBlocksCount() const
    {
        return std::count_if(vectorBb_.begin(), vectorBb_.end(), [](BasicBlock *block) { return block != nullptr; });
    }

    PassManager *GetPassManager()
    {
        return &passManager_;
    }
    const PassManager *GetPassManager() const
    {
        return &passManager_;
    }

    const BoundsRangeInfo *GetBoundsRangeInfo() const;

    PANDA_PUBLIC_API const ArenaVector<BasicBlock *> &GetBlocksRPO() const;

    PANDA_PUBLIC_API const ArenaVector<BasicBlock *> &GetBlocksLinearOrder() const;

    template <class Callback>
    void VisitAllInstructions(Callback callback);

    AliasType CheckInstAlias(Inst *mem1, Inst *mem2);

    /// Main allocator for graph, all related to Graph data should be allocated via this allocator.
    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }
    /// Allocator for temporary usage, when allocated data is no longer needed after optimization/analysis finished.
    ArenaAllocator *GetLocalAllocator() const
    {
        return localAllocator_;
    }
    bool IsDFConstruct() const
    {
        return FlagDFConstruct::Get(bitFields_);
    }
    void SetDFConstruct()
    {
        FlagDFConstruct::Set(true, &bitFields_);
    }

    void SetAotData(AotData *data)
    {
        aotData_ = data;
    }
    AotData *GetAotData()
    {
        return aotData_;
    }
    const AotData *GetAotData() const
    {
        return aotData_;
    }

    bool IsAotMode() const
    {
        return aotData_ != nullptr;
    }

    bool IsAotNoChaMode() const
    {
        return aotData_ != nullptr && !aotData_->GetUseCha();
    }

    bool IsOfflineCompilationMode() const
    {
        return IsAotMode() || GetMode().IsInterpreter() || GetMode().IsFastPath() || GetMode().IsNativePlus() ||
               GetMode().IsInterpreterEntry();
    }

    bool IsDefaultLocationsInit() const
    {
        return FlagDefaultLocationsInit::Get(bitFields_);
    }
    void SetDefaultLocationsInit()
    {
        FlagDefaultLocationsInit::Set(true, &bitFields_);
    }
    bool IsIrtocPrologEpilogOptimized() const
    {
        return FlagIrtocPrologEpilogOptimized::Get(bitFields_);
    }
    void SetIrtocPrologEpilogOptimized()
    {
        FlagIrtocPrologEpilogOptimized::Set(true, &bitFields_);
    }
    bool IsUnrollComplete() const
    {
        return FlagUnrollComplete::Get(bitFields_);
    }
    void SetUnrollComplete()
    {
        FlagUnrollComplete::Set(true, &bitFields_);
    }
#ifdef COMPILER_DEBUG_CHECKS
    bool IsRegAllocApplied() const
    {
        return FlagRegallocApplied::Get(bitFields_);
    }
    void SetRegAllocApplied()
    {
        FlagRegallocApplied::Set(true, &bitFields_);
    }
    bool IsRegAccAllocApplied() const
    {
        return FlagRegaccallocApplied::Get(bitFields_);
    }
    void SetRegAccAllocApplied()
    {
        FlagRegaccallocApplied::Set(true, &bitFields_);
    }
    bool IsInliningComplete() const
    {
        return FlagInliningComplete::Get(bitFields_) || IsOsrMode();
    }
    void SetInliningComplete()
    {
        FlagInliningComplete::Set(true, &bitFields_);
    }
    bool IsLowLevelInstructionsEnabled() const
    {
        return FlagLowLevelInstnsEnabled::Get(bitFields_);
    }
    void SetLowLevelInstructionsEnabled()
    {
        FlagLowLevelInstnsEnabled::Set(true, &bitFields_);
    }
    bool IsDynUnitTest() const
    {
        return FlagDynUnitTest::Get(bitFields_);
    }
    void SetDynUnitTestFlag()
    {
        FlagDynUnitTest::Set(true, &bitFields_);
    }
    bool IsUnitTest() const
    {
        static constexpr uintptr_t FAKE_FILE = 0xdeadf;
        return method_ == nullptr || ToUintPtr(runtime_->GetBinaryFileForMethod(method_)) == FAKE_FILE;
    }
#else
    bool IsRegAllocApplied() const
    {
        return false;
    }
#endif  // COMPILER_DEBUG_CHECKS

    bool IsThrowApplied() const
    {
        return FlagThrowApplied::Get(bitFields_);
    }
    void SetThrowApplied()
    {
        FlagThrowApplied::Set(true, &bitFields_);
    }
    void UnsetThrowApplied()
    {
        FlagThrowApplied::Set(false, &bitFields_);
    }

#ifdef PANDA_COMPILER_DEBUG_INFO
    bool IsLineDebugInfoEnabled() const
    {
        return FlagLineDebugInfoEnabled::Get(bitFields_);
    }
    void SetLineDebugInfoEnabled()
    {
        FlagLineDebugInfoEnabled::Set(true, &bitFields_);
    }
#endif

    void SetCode(EncodeDataType data)
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

    EncodeDataType GetCode() const
    {
        return data_;
    }

    EncodeDataType GetCode()
    {
        return data_;
    }

    void SetCodeInfo(Span<uint8_t> data)
    {
        codeInfoData_ = data.SubSpan<const uint8_t>(0, data.size());
    }

    Span<const uint8_t> GetCodeInfoData() const
    {
        return codeInfoData_;
    }

    void DumpUsedRegs(std::ostream &out = std::cerr, const char *prefix = nullptr) const
    {
        if (prefix != nullptr) {
            out << prefix;
        }
        out << "'\n  used scalar regs: ";
        if (usedRegs_ != nullptr) {
            for (unsigned i = 0; i < usedRegs_->size(); ++i) {
                if (usedRegs_->at(i)) {
                    out << i << " ";
                }
            }
        }
        out << "\n  used float  regs: ";
        if (usedRegs_ != nullptr) {
            for (unsigned i = 0; i < usedVregs_->size(); ++i) {
                if (usedVregs_->at(i)) {
                    out << i << " ";
                }
            }
        }
        out << std::endl;
    }

    // Get registers mask which used in graph
    template <DataType::Type REG_TYPE>
    ArenaVector<bool> *GetUsedRegs() const
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REG_TYPE == DataType::INT64) {
            return usedRegs_;
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REG_TYPE == DataType::FLOAT64) {
            return usedVregs_;
        }
        UNREACHABLE();
        return nullptr;
    }

    void SetRegUsage(Register reg, DataType::Type type)
    {
        ASSERT(reg != GetInvalidReg());
        if (DataType::IsFloatType(type)) {
            SetUsedReg<DataType::FLOAT64>(reg);
        } else {
            SetUsedReg<DataType::INT64>(reg);
        }
    }

    void SetRegUsage(Location location)
    {
        ASSERT(location.IsFixedRegister());
        if (location.IsFpRegister()) {
            SetUsedReg<DataType::FLOAT64>(location.GetValue());
        } else {
            SetUsedReg<DataType::INT64>(location.GetValue());
        }
    }

    template <DataType::Type REG_TYPE>
    void SetUsedReg(Register reg)
    {
        ArenaVector<bool> *graphRegs = nullptr;
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REG_TYPE == DataType::INT64) {
            graphRegs = usedRegs_;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (REG_TYPE == DataType::FLOAT64) {
            graphRegs = usedVregs_;
        } else {
            UNREACHABLE();
        }
        ASSERT(graphRegs != nullptr);
        ASSERT(reg < graphRegs->size());
        (*graphRegs)[reg] = true;
    }

    template <DataType::Type REG_TYPE>
    void InitUsedRegs(const ArenaVector<bool> *usedRegs)
    {
        if (usedRegs == nullptr) {
            return;
        }
        ArenaVector<bool> *graphRegs = nullptr;
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (REG_TYPE == DataType::INT64) {
            usedRegs_ = GetAllocator()->New<ArenaVector<bool>>(GetAllocator()->Adapter());
            graphRegs = usedRegs_;
            // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
        } else if constexpr (REG_TYPE == DataType::FLOAT64) {
            usedVregs_ = GetAllocator()->New<ArenaVector<bool>>(GetAllocator()->Adapter());
            graphRegs = usedVregs_;
        } else {
            UNREACHABLE();
        }
        ASSERT(graphRegs != nullptr);
        graphRegs->resize(usedRegs->size());
        std::copy(usedRegs->begin(), usedRegs->end(), graphRegs->begin());
    }

    Register GetZeroReg() const;
    Register GetArchTempReg() const;
    Register GetArchTempVReg() const;
    // Get registers mask which used in codegen, runtime e.t.c
    RegMask GetArchUsedRegs();
    void SetArchUsedRegs(RegMask mask);

    // Get vector registers mask which used in codegen, runtime e.t.c
    VRegMask GetArchUsedVRegs();

    // Return true if one 64-bit scalar register can be split to 2 32-bit
    bool IsRegScalarMapped() const;

    uint32_t GetStackSlotsCount() const
    {
        return stackSlotCount_;
    }

    void SetStackSlotsCount(uint32_t stackSlotCount)
    {
        stackSlotCount_ = stackSlotCount;
    }

    void UpdateStackSlotsCount(uint32_t stackSlotCount)
    {
        stackSlotCount_ = std::max(stackSlotCount_, stackSlotCount);
    }

    uint32_t GetParametersSlotsCount() const;

    uint32_t GetExtSlotsStart() const
    {
        return extStackSlot_;
    }

    void SetExtSlotsStart(uint32_t extStackSlot)
    {
        extStackSlot_ = extStackSlot;
    }

    BasicBlock *CreateEmptyBlock(uint32_t guestPc = INVALID_PC);
    BasicBlock *CreateEmptyBlock(BasicBlock *baseBlock);
#ifndef NDEBUG
    BasicBlock *CreateEmptyBlock(uint32_t id, uint32_t guestPc);
#endif
    BasicBlock *CreateStartBlock();
    BasicBlock *CreateEndBlock(uint32_t guestPc = INVALID_PC);
    ConstantInst *GetFirstConstInst()
    {
        return firstConstInst_;
    }
    void SetFirstConstInst(ConstantInst *constInst)
    {
        firstConstInst_ = constInst;
    }

    Inst *GetNullPtrInst() const
    {
        return nullptrInst_;
    }
    bool HasNullPtrInst() const
    {
        return nullptrInst_ != nullptr;
    }
    void UnsetNullPtrInst()
    {
        ASSERT(HasNullPtrInst());
        nullptrInst_ = nullptr;
    }
    Inst *GetOrCreateNullPtr();

    Inst *GetUniqueObjectInst() const
    {
        return uniqueObjectInst_;
    }
    bool HasUniqueObjectInst() const
    {
        return uniqueObjectInst_ != nullptr;
    }
    void UnsetUniqueObjectInst()
    {
        ASSERT(HasUniqueObjectInst());
        uniqueObjectInst_ = nullptr;
    }
    Inst *GetOrCreateUniqueObjectInst();

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

    ParameterInst *AddNewParameter(uint16_t argNumber);

    ParameterInst *AddNewParameter(uint16_t argNumber, DataType::Type type)
    {
        ParameterInst *param = AddNewParameter(argNumber);
        param->SetType(type);
        return param;
    }

    ParameterInst *FindParameter(uint16_t argNumber);

    /*
     * The function remove the ConstantInst from the graph list
     * !NOTE ConstantInst isn't removed from BasicBlock list
     */
    void RemoveConstFromList(ConstantInst *constInst);

    ConstantInst *GetSpilledConstant(ImmTableSlot slot)
    {
        ASSERT(static_cast<size_t>(slot) < spilledConstants_.size());
        return spilledConstants_[slot];
    }

    ImmTableSlot AddSpilledConstant(ConstantInst *constInst)
    {
        // Constant already in the table
        auto currentSlot = constInst->GetImmTableSlot();
        if (currentSlot != GetInvalidImmTableSlot()) {
            ASSERT(spilledConstants_[currentSlot] == constInst);
            return currentSlot;
        }

        auto count = spilledConstants_.size();
        if (count >= GetMaxNumImmSlots()) {
            return GetInvalidImmTableSlot();
        }
        spilledConstants_.push_back(constInst);
        constInst->SetImmTableSlot(count);
        return ImmTableSlot(count);
    }

    ImmTableSlot FindSpilledConstantSlot(ConstantInst *constInst) const
    {
        auto slot = std::find(spilledConstants_.begin(), spilledConstants_.end(), constInst);
        if (slot == spilledConstants_.end()) {
            return GetInvalidImmTableSlot();
        }
        return std::distance(spilledConstants_.begin(), slot);
    }

    size_t GetSpilledConstantsCount() const
    {
        return spilledConstants_.size();
    }

    bool HasAvailableConstantSpillSlots() const
    {
        return GetSpilledConstantsCount() < GetMaxNumImmSlots();
    }

    auto begin()  // NOLINT(readability-identifier-naming)
    {
        return vectorBb_.begin();
    }
    auto begin() const  // NOLINT(readability-identifier-naming)
    {
        return vectorBb_.begin();
    }
    auto end()  // NOLINT(readability-identifier-naming)
    {
        return vectorBb_.end();
    }
    auto end() const  // NOLINT(readability-identifier-naming)
    {
        return vectorBb_.end();
    }

    void Dump(std::ostream *out) const;

    Loop *GetRootLoop()
    {
        return rootLoop_;
    }
    const Loop *GetRootLoop() const
    {
        return rootLoop_;
    }

    void SetRootLoop(Loop *rootLoop)
    {
        rootLoop_ = rootLoop;
    }

    void SetHasIrreducibleLoop(bool hasIrrLoop)
    {
        FlagIrredicibleLoop::Set(hasIrrLoop, &bitFields_);
    }

    void SetHasInfiniteLoop(bool hasInfLoop)
    {
        FlagInfiniteLoop::Set(hasInfLoop, &bitFields_);
    }

    void SetHasFloatRegs()
    {
        FlagFloatRegs::Set(true, &bitFields_);
    }

    bool HasLoop() const;
    PANDA_PUBLIC_API bool HasIrreducibleLoop() const;
    bool HasInfiniteLoop() const;
    bool HasFloatRegs() const;

    /**
     * Try-catch info
     * Vector of begin try-blocks in order they were declared in the bytecode
     */
    void AppendTryBeginBlock(const BasicBlock *block)
    {
        tryBeginBlocks_.push_back(block);
    }

    void EraseTryBeginBlock(const BasicBlock *block)
    {
        auto it = std::find(tryBeginBlocks_.begin(), tryBeginBlocks_.end(), block);
        if (it == tryBeginBlocks_.end()) {
            ASSERT(false && "Trying to remove non try_begin block");
            return;
        }
        tryBeginBlocks_.erase(it);
    }

    const auto &GetTryBeginBlocks() const
    {
        return tryBeginBlocks_;
    }

    void RemovePredecessorUpdateDF(BasicBlock *block, BasicBlock *rmPred);

    bool FindThrowBlock(BasicBlock *block)
    {
        auto it = std::find(throwBlocks_.begin(), throwBlocks_.end(), block);
        return (it != throwBlocks_.end());
    }

    bool AppendThrowBlock(BasicBlock *block)
    {
        if (!FindThrowBlock(block)) {
            throwBlocks_.insert(block);
            return true;
        }
        return false;
    }

    bool EraseThrowBlock(BasicBlock *block)
    {
        auto it = std::find(throwBlocks_.begin(), throwBlocks_.end(), block);
        if (it == throwBlocks_.end()) {
            return false;
        }
        throwBlocks_.erase(it);
        return true;
    }

    const auto &GetThrowBlocks() const
    {
        return throwBlocks_;
    }

    void ClearThrowBlocks()
    {
        throwBlocks_.clear();
    }

    void AppendThrowableInst(const Inst *inst, BasicBlock *catchHandler)
    {
        auto it = throwableInsts_.emplace(inst, GetAllocator()->Adapter()).first;
        it->second.push_back(catchHandler);
    }

    bool IsInstThrowable(const Inst *inst) const
    {
        return throwableInsts_.count(inst) > 0;
    }

    void RemoveThrowableInst(const Inst *inst);
    PANDA_PUBLIC_API void ReplaceThrowableInst(Inst *oldInst, Inst *newInst);

    const auto &GetThrowableInstHandlers(const Inst *inst) const
    {
        ASSERT(IsInstThrowable(inst));
        return throwableInsts_.at(inst);
    }

    void ClearTryCatchInfo()
    {
        throwableInsts_.clear();
        tryBeginBlocks_.clear();
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
        return passManager_.RunPass<T>(std::forward<Args>(args)...);
    }
    template <typename T, typename... Args>
    bool RunPass(Args... args) const
    {
        ASSERT(GetPassManager());
        return passManager_.RunPass<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    bool RunPass(T *pass)
    {
        ASSERT(GetPassManager());
        return passManager_.RunPass(pass, GetLocalAllocator()->GetAllocatedSize());
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
        return passManager_.GetAnalysis<T>();
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
        return instrCurrentId_;
    }
    auto SetCurrentInstructionId(size_t v)
    {
        instrCurrentId_ = v;
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

    Encoder *GetEncoder();
    RegistersDescription *GetRegisters() const;
    CallingConvention *GetCallingConvention();
    const MethodProperties &GetMethodProperties();
    void ResetParameterInfo();
    SpillFillData GetDataForNativeParam(DataType::Type type);

    template <bool GRAPH_ENCODED = false>
    size_t EstimateCodeSize()
    {
        if constexpr (GRAPH_ENCODED) {
            return encoder_->BufferSize();
        }
        auto maxIrInstsCount = GetCurrentInstructionId();
        auto encoder = GetEncoder();
        ASSERT(encoder != nullptr);
        auto maxArchInstsPerIrInsts = encoder->MaxArchInstPerEncoded();
        auto maxBytesInArchInst = GetInstructionSizeBits(GetArch());
        return maxIrInstsCount * maxArchInstsPerIrInsts * maxBytesInArchInst;
    }

    EventWriter &GetEventWriter()
    {
        return eventWriter_;
    }

    void SetCodeBuilder(CodeInfoBuilder *builder)
    {
        ciBuilder_ = builder;
    }

    // clang-format off

    /// Create instruction by opcode
    // NOLINTNEXTLINE(readability-function-size)
    [[nodiscard]] Inst* CreateInst(Opcode opc) const
    {
        switch (opc) {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RETURN_INST(OPCODE, BASE, ...)                                   \
            case Opcode::OPCODE: {                                       \
                auto inst = Inst::New<BASE>(allocator_, Opcode::OPCODE); \
                inst->SetId(instrCurrentId_++);                          \
                if (IsAbcKit()) {                                        \
                    SetAbcKitFlags(inst);                                \
                }                                                        \
                return inst;                                             \
            }
            OPCODE_LIST(RETURN_INST)

#undef RETURN_INST
            default:
                return nullptr;
        }
    }

    template <Opcode OPCODE, typename... Args>
    [[nodiscard]] BaseTypeOf<OPCODE> *CreateInst(Args &&...args) const
    {
        auto *inst = Inst::New<BaseTypeOf<OPCODE>>(allocator_, OPCODE, std::forward<Args>(args)...);
        inst->SetId(instrCurrentId_++);
        if (IsAbcKit()) {
            SetAbcKitFlags(inst);
        }
        if constexpr (inst_flags::HasFlag(OPCODE, inst_flags::READ_BARRIER)) {
            bool needsBarrier = GetRuntime()->NeedsPreReadBarrier();
            if (needsBarrier && inst->GetType() == DataType::REFERENCE) {
                inst->SetNeedBarrier(true);
            }
        }
        if constexpr (inst_flags::HasFlag(OPCODE, inst_flags::WRITE_BARRIER)) {
            bool needsBarrier = GetRuntime()->NeedsPreWriteBarrier() || GetRuntime()->NeedsPostWriteBarrier();
            if (needsBarrier && inst->GetType() == DataType::REFERENCE) {
                inst->SetNeedBarrier(true);
            }
        }
        return inst;
    }

    /// Define creation methods for all opcodes
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RETURN_INST(OPCODE, BASE, ...)                                  \
    template <typename... Args>                                         \
    [[nodiscard]] BASE* CreateInst##OPCODE(Args&&... args) const {      \
        return CreateInst<Opcode::OPCODE>(std::forward<Args>(args)...); \
    }
    OPCODE_LIST(RETURN_INST)

#undef RETURN_INST

#ifdef PANDA_COMPILER_DEBUG_INFO
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RETURN_INST(OPCODE, BASE, ...)                                               \
    template <typename... Args>                                                   \
    [[nodiscard]] BASE* CreateInst##OPCODE(Inst* inst, Args&&... args) const {    \
        auto new_inst = CreateInst##OPCODE(inst->GetType(), inst->GetPc(), std::forward<Args>(args)...);  \
        new_inst->SetCurrentMethod(inst->GetCurrentMethod());                     \
        return new_inst;                                                          \
    }
    OPCODE_LIST(RETURN_INST)

#undef RETURN_INST
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RETURN_INST(OPCODE, BASE, ...)                                               \
    template <typename... Args>                                                   \
    [[nodiscard]] BASE* CreateInst##OPCODE(Inst* inst, Args&&... args) const {    \
        auto new_inst = CreateInst##OPCODE(inst->GetType(), inst->GetPc(), std::forward<Args>(args)...);  \
        return new_inst;                                                          \
    }
    OPCODE_LIST(RETURN_INST)

#undef RETURN_INST
#endif

    // clang-format on

    uint32_t GetBitFields()
    {
        return bitFields_;
    }

    void SetBitFields(uint32_t bitFields)
    {
        bitFields_ = bitFields;
    }

    bool NeedCleanup() const
    {
        return FlagNeedCleanup::Get(bitFields_);
    }

    void SetNeedCleanup(bool v)
    {
        FlagNeedCleanup::Set(v, &bitFields_);
    }

    bool CanOptimizeNativeMethods() const
    {
        return FlagCanOptimizeNativeMethods::Get(bitFields_);
    }

    void SetCanOptimizeNativeMethods(bool v)
    {
        FlagCanOptimizeNativeMethods::Set(v, &bitFields_);
    }

    bool IsJitOrOsrMode() const
    {
        return !IsAotMode() && !IsBytecodeOptimizer() && SupportManagedCode();
    }

    bool IsOsrMode() const
    {
        return mode_.IsOsr();
    }

    bool IsJitMode() const
    {
        return !IsOsrMode() && IsJitOrOsrMode();
    }

    bool IsBytecodeOptimizer() const
    {
        return mode_.IsBytecodeOpt();
    }

    bool IsDynamicMethod() const
    {
        return mode_.IsDynamicMethod();
    }

    bool IsAbcKit() const
    {
#ifdef ENABLE_LIBABCKIT
        return mode_.IsAbcKit();
#else
        return false;
#endif
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
        singleImplementationList_.push_back(method);
    }

    void SetDynamicMethod()
    {
        mode_.SetDynamicMethod(true);
    }

    void SetAbcKit()
    {
        mode_.SetAbcKit(true);
    }

    void SetDynamicStub()
    {
        mode_.SetDynamicStub(true);
    }

    auto &GetSingleImplementationList()
    {
        return singleImplementationList_;
    }

    Graph *GetParentGraph() const
    {
        return parentGraph_;
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
        vregsCount_ = count;
    }

    size_t GetVRegsCount() const
    {
        return vregsCount_;
    }

    size_t GetEnvCount() const
    {
        return (IsDynamicMethod() && !IsBytecodeOptimizer()) ? VRegInfo::ENV_COUNT : 0;
    }

    RelocationHandler *GetRelocationHandler()
    {
        return relocationHandler_;
    }

    void SetRelocationHandler(RelocationHandler *handler)
    {
        relocationHandler_ = handler;
    }

    int64_t GetBranchCounter(const BasicBlock *block, bool trueSucc);

    int64_t GetThrowCounter(const BasicBlock *block);

    /// This class provides methods for ranged-based `for` loop over all parameters in the graph.
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
        PANDA_PUBLIC_API Iterator begin();
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

    bool SupportsIrtocBarriers() const
    {
        return (GetMode().IsFastPath() || GetMode().IsNativePlus() || IsJitOrOsrMode() || IsAotMode() ||
                GetMode().IsInterpreter() || GetMode().IsInterpreterEntry()) &&
               !IsDynamicMethod() && GetArch() != Arch::AARCH32;
    }

    void SetMaxInliningDepth(uint32_t depth)
    {
        maxInliningDepth_ = std::max(maxInliningDepth_, depth);
    }

    uint32_t GetMaxInliningDepth()
    {
        return maxInliningDepth_;
    }

    bool IsStringFlatCheckConstraint() const
    {
        return stringFlatCheckConstraint_;
    }

private:
    void AddConstInStartBlock(ConstantInst *constInst);

    NO_MOVE_SEMANTIC(Graph);
    NO_COPY_SEMANTIC(Graph);

private:
    uint32_t maxInliningDepth_ {0};
    ArenaAllocator *const allocator_;
    ArenaAllocator *const localAllocator_;

    Arch arch_ {RUNTIME_ARCH};

    // List of blocks in insertion order.
    ArenaVector<BasicBlock *> vectorBb_;
    BasicBlock *startBlock_ {nullptr};
    BasicBlock *endBlock_ {nullptr};

    Loop *rootLoop_ {nullptr};

    AotData *aotData_ {nullptr};

    uint32_t bitFields_ {0};
    using FlagDFConstruct = BitField<bool, 0, 1>;
    using FlagNeedCleanup = FlagDFConstruct::NextFlag;
    using FlagIrredicibleLoop = FlagNeedCleanup::NextFlag;
    using FlagInfiniteLoop = FlagIrredicibleLoop::NextFlag;
    using FlagFloatRegs = FlagInfiniteLoop::NextFlag;
    using FlagDefaultLocationsInit = FlagFloatRegs::NextFlag;
    using FlagIrtocPrologEpilogOptimized = FlagDefaultLocationsInit::NextFlag;
    using FlagThrowApplied = FlagIrtocPrologEpilogOptimized::NextFlag;
    using FlagCanOptimizeNativeMethods = FlagThrowApplied::NextFlag;
    using FlagUnrollComplete = FlagCanOptimizeNativeMethods::NextFlag;
#if defined(NDEBUG) && !defined(ENABLE_LIBABCKIT)
    using LastField = FlagUnrollComplete;
#else
    using FlagRegallocApplied = FlagUnrollComplete::NextFlag;
    using FlagRegaccallocApplied = FlagRegallocApplied::NextFlag;
    using FlagInliningComplete = FlagRegaccallocApplied::NextFlag;
    using FlagLowLevelInstnsEnabled = FlagInliningComplete::NextFlag;
    using FlagDynUnitTest = FlagLowLevelInstnsEnabled::NextFlag;
    using LastField = FlagDynUnitTest;
#endif  // NDEBUG

#ifdef PANDA_COMPILER_DEBUG_INFO
    using FlagLineDebugInfoEnabled = LastField::NextFlag;
#endif

    // codegen data
    EncodeDataType data_;
    Span<const uint8_t> codeInfoData_;
    ArenaVector<bool> *usedRegs_ {nullptr};
    ArenaVector<bool> *usedVregs_ {nullptr};

    // NOTE (a.popov) Replace by ArenaMap from throwable_inst* to try_inst*
    ArenaMap<const Inst *, ArenaVector<BasicBlock *>> throwableInsts_;

    RegMask archUsedRegs_ {0};

    mutable size_t instrCurrentId_ {0};
    // first constant instruction in graph !NOTE rewrite it to hash-map
    ConstantInst *firstConstInst_ {nullptr};
    Inst *nullptrInst_ {nullptr};
    Inst *uniqueObjectInst_ {nullptr};
    RuntimeInterface *runtime_ {nullptr};
    RuntimeInterface::MethodPtr method_ {nullptr};

    Encoder *encoder_ {nullptr};

    mutable RegistersDescription *registers_ {nullptr};

    CallingConvention *callconv_ {nullptr};

    std::optional<MethodProperties> methodProperties_ {std::nullopt};

    ParameterInfo *paramInfo_ {nullptr};

    RelocationHandler *relocationHandler_ {nullptr};

    mutable PassManager passManager_;
    EventWriter eventWriter_;

    GraphMode mode_;

    CodeInfoBuilder *ciBuilder_ {nullptr};

    ArenaVector<RuntimeInterface::MethodPtr> singleImplementationList_;
    ArenaVector<const BasicBlock *> tryBeginBlocks_;
    ArenaSet<BasicBlock *> throwBlocks_;
    ArenaVector<ConstantInst *> spilledConstants_;
    // Graph that inlines this graph
    Graph *parentGraph_ {nullptr};
    // Number of used stack slots
    uint32_t stackSlotCount_ {0};
    // Number of used stack slots for parameters
    uint32_t paramSlotsCount_ {0};
    // First language extension slot
    uint32_t extStackSlot_ {0};
    // Number of the virtual registers used in the compiled method (inlined methods aren't included).
    uint32_t vregsCount_ {0};
    // Source language of the method being compiled
    SourceLanguage lang_ {SourceLanguage::PANDA_ASSEMBLY};
    // true if tree strings are enabled but StringFlatCheck pass is disabled
    bool stringFlatCheckConstraint_;
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
    bool isSupportInt32 = IsBytecodeOptimizer();
    if (firstConstInst_ == nullptr) {
        firstConstInst_ = CreateInstConstant(value, isSupportInt32);
        AddConstInStartBlock(firstConstInst_);
        return firstConstInst_;
    }
    ConstantInst *currentConst = firstConstInst_;
    ConstantInst *prevConst = nullptr;
    while (currentConst != nullptr) {
        if (currentConst->IsEqualConst(value, isSupportInt32)) {
            return currentConst;
        }
        prevConst = currentConst;
        currentConst = currentConst->GetNextConst();
    }
    ASSERT(prevConst != nullptr);
    auto *newConst = CreateInstConstant(value, isSupportInt32);
    AddConstInStartBlock(newConst);

    prevConst->SetNextConst(newConst);
    return newConst;
}

void InvalidateBlocksOrderAnalyzes(Graph *graph);
void MarkLoopExits(const Graph *graph, Marker marker);
std::string GetMethodFullName(const Graph *graph, RuntimeInterface::MethodPtr method);
size_t GetObjectOffset(const Graph *graph, ObjectType objType, RuntimeInterface::FieldPtr field, uint32_t typeId);
}  // namespace ark::compiler
#endif  // COMPILER_OPTIMIZER_IR_GRAPH_H
