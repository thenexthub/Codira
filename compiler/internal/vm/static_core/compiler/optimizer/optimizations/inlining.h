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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H

#include <string>
#include "optimizer/pass.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/runtime_interface.h"
#include "compiler_options.h"
#include "libarkbase/utils/arena_containers.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage,-warnings-as-errors)
#define LOG_INLINING(level) COMPILER_LOG(level, INLINING) << GetLogIndent()

namespace ark::compiler {
struct InlineContext {
    RuntimeInterface::MethodPtr method {};
    bool chaDevirtualize {false};
    bool replaceToStatic {false};
    RuntimeInterface::IntrinsicId intrinsicId {RuntimeInterface::IntrinsicId::INVALID};
};

struct InlinedGraph {
    Graph *graph {nullptr};
    bool hasRuntimeCalls {false};
};

class Inlining : public Optimization {
public:
    static constexpr auto MAX_CALL_DEPTH = 20U;
    using InstPair = std::pair<Inst *, Inst *>;
    using InstTriple = std::tuple<Inst *, Inst *, Inst *>;
    using BasicBlockPair = std::pair<BasicBlock *, BasicBlock *>;
    using FlagPair = std::pair<bool *, bool *>;

    explicit Inlining(Graph *graph) : Inlining(graph, 0, 0, nullptr) {}
    Inlining(Graph *graph, bool resolveWoInline) : Inlining(graph)
    {
        resolveWoInline_ = resolveWoInline;
    }

    Inlining(Graph *graph, uint32_t instructionsCount, uint32_t methodsInlined,
             const ArenaVector<RuntimeInterface::MethodPtr> *inlinedStack);

    NO_MOVE_SEMANTIC(Inlining);
    NO_COPY_SEMANTIC(Inlining);
    ~Inlining() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerInlining();
    }

    const char *GetPassName() const override
    {
        return "Inline";
    }
    auto GetCurrentDepth() const
    {
        ASSERT(!inlinedStack_.empty());
        return inlinedStack_.size() - 1;
    }

    void InvalidateAnalyses() override;

protected:
    virtual void RunOptimizations() const;
    virtual bool IsInstSuitableForInline(Inst *inst) const;
    virtual bool TryInline(CallInst *callInst);
    bool TryInlineWithInlineCaches(CallInst *callInst);
    bool TryInlineExternal(CallInst *callInst, InlineContext *ctx);
    bool TryInlineExternalAot(CallInst *callInst, InlineContext *ctx);

    Inst *GetNewDefAndCorrectDF(Inst *callInst, Inst *oldDef);

    bool Do();
    bool DoInline(CallInst *callInst, InlineContext *ctx);
    bool DoInlineMethod(CallInst *callInst, InlineContext *ctx);
    bool DoInlineIntrinsic(CallInst *callInst, InlineContext *ctx);
    bool DoInlineIntrinsicByExpansion(CallInst *callInst, InlineContext *ctx);
    bool DoInlineIntrinsicByEncoding(CallInst *callInst, InlineContext *ctx);
#include "intrinsics_inlining_expansion.inl.h"

    bool DoInlineMonomorphic(CallInst *callInst, RuntimeInterface::ClassPtr receiver);
    bool DoInlinePolymorphic(CallInst *callInst, ArenaVector<RuntimeInterface::ClassPtr> *receivers);
    SaveStateInst *GetOrCloneSaveState(CallInst *callInst, BasicBlock *callBb);
    void CreateCompareClass(CallInst *callInst, Inst *getClsInst, RuntimeInterface::ClassPtr receiver,
                            BasicBlock *callBb);
    BasicBlockPair MakeCallBbs(InstPair insts, BasicBlockPair bbs, [[maybe_unused]] PhiInst **phiInst,
                               [[maybe_unused]] size_t receiversSize);
    void InlineReceiver(InstTriple insts, BasicBlockPair bbs, FlagPair flags, size_t receiversSize,
                        InlinedGraph inlGraph);
    bool FinalizeInlineReceiver(InstTriple insts, BasicBlockPair bbs, FlagPair flags, bool needToDeoptimize);
    void InsertDeoptimizeInst(CallInst *callInst, BasicBlock *callBb,
                              DeoptimizeType deoptType = DeoptimizeType::INLINE_IC);
    void InsertCallInst(CallInst *callInst, BasicBlock *callBb, BasicBlock *retBb, Inst *phiInst);

    void UpdateDataflow(Graph *graphInl, Inst *callInst, std::variant<BasicBlock *, PhiInst *> use,
                        Inst *newDef = nullptr);
    void UpdateDataflowForEmptyGraph(Inst *callInst, std::variant<BasicBlock *, PhiInst *> use, BasicBlock *endBlock);
    void UpdateParameterDataflow(Graph *graphInl, Inst *callInst);
    void UpdateControlflow(Graph *graphInl, BasicBlock *callBb, BasicBlock *callContBb);
    void MoveConstants(Graph *graphInl);

    template <bool CHECK_EXTERNAL, bool CHECK_INTRINSICS = false>
    bool CheckMethodCanBeInlined(const CallInst *callInst, InlineContext *ctx);
    bool CheckDepthLimit(InlineContext *ctx);
    template <bool CHECK_EXTERNAL>
    bool CheckTooBigMethodCanBeInlined(const CallInst *callInst, InlineContext *ctx, bool methodIsTooBig);
    template <bool CHECK_EXTERNAL>
    bool CheckMethodSize(const CallInst *callInst, InlineContext *ctx);
    bool ResolveTarget(CallInst *callInst, InlineContext *ctx);
    bool CanUseTypeInfo(ObjectTypeInfo typeInfo, RuntimeInterface::MethodPtr method);
    void InsertChaGuard(CallInst *callInst, InlineContext *ctx);
    bool IsManualDevirtualize(CallInst *callInst, RuntimeInterface *runtime);

    InlinedGraph BuildGraph(InlineContext *ctx, CallInst *callInst, CallInst *polyCallInst = nullptr);
    bool CheckBytecode(CallInst *callInst, const InlineContext &ctx, bool *calleeCallRuntime);
    bool TryBuildGraph(const InlineContext &ctx, Graph *graphInl, CallInst *callInst, CallInst *polyCallInst);
    bool CheckLoops(bool *calleeCallRuntime, Graph *graphInl);
    static void PropagateObjectInfo(Graph *graphInl, CallInst *callInst);

    void ProcessCallReturnInstructions(CallInst *callInst, BasicBlock *callContBb, bool hasRuntimeCalls,
                                       bool needBarriers = false);
    size_t CalculateInstructionsCount(Graph *graph);

    IClassHierarchyAnalysis *GetCha()
    {
        return cha_;
    }

    bool IsInlineCachesEnabled() const;

    std::string GetLogIndent() const
    {
        return std::string(GetCurrentDepth() * 2U, ' ');
    }

    bool IsIntrinsic(const InlineContext *ctx) const
    {
        return ctx->intrinsicId != RuntimeInterface::IntrinsicId::INVALID;
    }

    virtual bool SkipBlock(const BasicBlock *block) const;

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    uint32_t methodsInlined_ {0};
    uint32_t instructionsCount_ {0};
    uint32_t instructionsLimit_ {0};
    ArenaVector<BasicBlock *> returnBlocks_;
    ArenaUnorderedSet<std::string> blacklist_;
    ArenaVector<RuntimeInterface::MethodPtr> inlinedStack_;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

private:
    uint32_t vregsCount_ {0};
    bool resolveWoInline_ {false};
    IClassHierarchyAnalysis *cha_ {nullptr};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H
