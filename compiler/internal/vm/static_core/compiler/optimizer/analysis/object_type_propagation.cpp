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

#include "alias_visitor.h"
#include "loop_analyzer.h"
#include "object_type_propagation.h"
#include "typed_ref_set.h"
#include "optimizer/analysis/live_in_analysis.h"
#include "optimizer/ir/basicblock.h"
#include "compiler_logger.h"

namespace ark::compiler {

ObjectTypeInfo Merge(ObjectTypeInfo lhs, ObjectTypeInfo rhs)
{
    if (lhs == ObjectTypeInfo::INVALID || rhs == ObjectTypeInfo::UNKNOWN) {
        return lhs;
    }
    if (rhs == ObjectTypeInfo::INVALID || lhs == ObjectTypeInfo::UNKNOWN) {
        return rhs;
    }
    // can be improved by finding a common superclass
    if (lhs.GetClass() != rhs.GetClass()) {
        return ObjectTypeInfo::INVALID;
    }
    return {lhs.GetClass(), lhs.IsExact() && rhs.IsExact()};
}

void TryImproveTypeInfo(ObjectTypeInfo &lhs, ObjectTypeInfo rhs)
{
    if (!lhs || (rhs && !lhs.IsExact() && rhs.IsExact())) {
        lhs = rhs;
    }
}

struct EmptyDeleter {
    void operator()([[maybe_unused]] void *ptr) {}
};

template <typename T>
class COWPtr {
public:
    COWPtr() : ptr_(nullptr) {}
    explicit COWPtr(ArenaAllocator *alloc) : ptr_(alloc->New<T>(alloc->Adapter()), EmptyDeleter {}, alloc->Adapter()) {}
    COWPtr(T *obj, ArenaAllocator *alloc) : ptr_(obj, EmptyDeleter {}, alloc->Adapter()) {}

    const T &operator*() const
    {
        return *ptr_;
    }

    const T *operator->() const
    {
        return Get();
    }

    const T *Get() const
    {
        return ptr_.get();
    }

    T *Mut(ArenaAllocator *alloc)
    {
        if (ptr_.unique()) {
            return ptr_.get();
        }
        T *copy = alloc->New<T>(*ptr_);
        ptr_ = std::shared_ptr<T> {copy, EmptyDeleter {}, alloc->Adapter()};
        ASSERT(ptr_.unique());
        ASSERT(ptr_.get() == copy);
        return copy;
    }

private:
    std::shared_ptr<T> ptr_;
};

namespace {

class ObjectTypePropagationVisitor;
constexpr Ref NULL_REF = 0;

class BasicBlockState {
public:
    BasicBlockState(ObjectTypePropagationVisitor *visitor, BasicBlock *bb);
    BasicBlockState(const BasicBlockState &other, BasicBlock *bb);
    ~BasicBlockState() = default;

    const ArenaTypedRefSet &GetFieldRefSet(Ref base, const PointerOffset &offset);
    void CreateFieldRefSetForNewObject(Ref base, bool defaultConstructed);
    ArenaTypedRefSet &CreateFieldRefSet(Ref base, const PointerOffset &offset, bool &changed);
    const ArenaTypedRefSet &NewUnknownRef();
    const ArenaTypedRefSet &GetNullRef();
    void Merge(BasicBlockState *other);
    void TryEscape(Ref ref);
    void ForceEscape(Ref ref);
    void Escape(const Inst *inst);
    void InvalidateEscaped();
    bool IsEscaped(Ref ref) const;
    template <typename F>
    void Cleanup(const F &remove);
    size_t GetBlockId() const;

    constexpr static size_t REAL_REF_START = 3U;

private:
    void Escape(ArenaVector<Ref> &worklist, const ArenaTypedRefSet &refs);
    void Escape(ArenaVector<Ref> &worklist);
    void InvalidateEscapedRef(Ref ref);

private:
    DEFAULT_COPY_CTOR(BasicBlockState);
    NO_COPY_OPERATOR(BasicBlockState);
    NO_MOVE_SEMANTIC(BasicBlockState);

    ArenaAllocator *GetLocalAllocator();
    friend std::ostream &operator<<(std::ostream &os, const BasicBlockState &state);

    ObjectTypePropagationVisitor *visitor_;
    ArenaMap<Ref, COWPtr<PointerOffset::Map<ArenaTypedRefSet>>> fieldRefs_;
    ArenaTypedRefSet escaped_;
#ifndef NDEBUG
    BasicBlock *bb_ {nullptr};
#endif
};

class TypePropagationVisitor : public GraphVisitor {
public:
    explicit TypePropagationVisitor(Graph *graph) : graph_(graph) {}
    ~TypePropagationVisitor() override = default;
    NO_COPY_SEMANTIC(TypePropagationVisitor);
    NO_MOVE_SEMANTIC(TypePropagationVisitor);

    static void VisitNewObject(GraphVisitor *v, Inst *i);
    static void VisitParameter(GraphVisitor *v, Inst *i);
    static void VisitNewArray(GraphVisitor *v, Inst *i);
    static void VisitLoadArray(GraphVisitor *v, Inst *i);
    static void VisitLoadString(GraphVisitor *v, Inst *i);
    static void VisitLoadObject(GraphVisitor *v, Inst *i);
    static void VisitLoadStatic(GraphVisitor *v, Inst *i);
    static void VisitCallStatic(GraphVisitor *v, Inst *i);
    static void VisitCallVirtual(GraphVisitor *v, Inst *i);
    static void VisitRefTypeCheck(GraphVisitor *v, Inst *i);

#include "optimizer/ir/visitor.inc"

protected:
    virtual void SetTypeInfo(Inst *inst, ObjectTypeInfo info) = 0;

private:
    static void ProcessManagedCall(GraphVisitor *v, CallInst *inst);
    Graph *GetGraph()
    {
        return graph_;
    }

    Graph *graph_;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LoopPropagationVisitor : public AliasVisitor, public TypePropagationVisitor {
public:
    explicit LoopPropagationVisitor(ObjectTypePropagationVisitor *parent);
    ~LoopPropagationVisitor() override = default;
    NO_COPY_SEMANTIC(LoopPropagationVisitor);
    NO_MOVE_SEMANTIC(LoopPropagationVisitor);

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        UNREACHABLE();
    }

    void AddDirectEdge(const Pointer &p) override;

    void VisitAllocation(Inst *inst) override;

    void AddConstantDirectEdge(Inst *inst, uint32_t id) override;

    void AddCopyEdge(const Pointer &from, const Pointer &to) override;
    void AddPseudoCopyEdge(const Pointer &base, const Pointer &field) override;

    void VisitHeapInv([[maybe_unused]] Inst *inst) override
    {
        heapInv_ = true;
    }

    void Escape(const Inst *inst) override;

    void SetTypeInfo(Inst *inst, ObjectTypeInfo info) override;

    void VisitLoop(Loop *loop);

    void VisitBlock(BasicBlock *bb) override;

    void VisitLoopRec(Loop *loop);

private:
    ObjectTypePropagationVisitor *parent_;
    bool heapInv_ {false};
    BasicBlockState *headerState_ {nullptr};
    ArenaSet<const Inst *> escapedInsts_;
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ObjectTypePropagationVisitor : public Analysis, public AliasVisitor, public TypePropagationVisitor {
public:
    explicit ObjectTypePropagationVisitor(Graph *graph)
        : Analysis(graph),
          TypePropagationVisitor(graph),
          states_(graph->GetVectorBlocks().size(), graph->GetLocalAllocator()->Adapter()),
          refInfos_(BasicBlockState::REAL_REF_START, ObjectTypeInfo::INVALID, graph->GetLocalAllocator()->Adapter()),
          instRefSets_(graph->GetLocalAllocator()->Adapter()),
          loopVisitor_(this),
          liveIns_(graph),
          loopEdges_(graph->GetLocalAllocator()->Adapter()),
          loopStoreEdges_(graph->GetLocalAllocator()->Adapter()),
          workSet_(graph->GetLocalAllocator()->Adapter()),
          nullSet_(graph->GetLocalAllocator(), ObjectTypeInfo::UNKNOWN),
          allSet_(graph->GetLocalAllocator(), ObjectTypeInfo::INVALID)
    {
        AliasVisitor::Init(graph->GetLocalAllocator());
        refInfos_[NULL_REF] = ObjectTypeInfo::UNKNOWN;
        nullSet_.SetBit(NULL_REF, ObjectTypeInfo::UNKNOWN);
        allSet_.SetBit(ALL_REF, ObjectTypeInfo::INVALID);
    }

    const char *GetPassName() const override
    {
        return "ObjectTypePropagationVisitor";
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        // We use only VisitBlock
        UNREACHABLE();
    }

    using Analysis::GetGraph;

    bool RunImpl() override;
    void SetTypeInfosInGraph();
    void ResetTypeInfosInGraph();
    void WalkOutgoingEdges(const Inst *fromInst, const PointerOffset::Map<ArenaVector<Pointer>> &edges);
    void WalkStoreEdges(const Inst *toInst, const PointerOffset::Map<ArenaVector<const Inst *>> &edges);
    void WalkEdges();
    Ref NewRef(ObjectTypeInfo info = ObjectTypeInfo::INVALID);
    ObjectTypeInfo GetRefTypeInfo(Ref ref) const;
    auto Printer(ObjectTypeInfo typeInfo);
    template <typename T>
    auto Printer(const TypedRefSet<T> &typedRefSet);
    template <typename T>
    auto Printer(TypedRefSet<T> &&typedRefSet);
    void DumpRefInfos(std::ostream &os);
    ArenaTypedRefSet &GetInstRefSet(const Inst *inst);
    ArenaTypedRefSet &TryGetInstRefSet(const Inst *inst);
    const ArenaTypedRefSet &GetNullSet() const;
    const ArenaTypedRefSet &GetAllSet() const;
    ArenaTypedRefSet &GetOrCreateInstRefSet(const Inst *inst, bool &changed);
    const auto &GetInstRefSets() const
    {
        return instRefSets_;
    }

protected:
    void AddDirectEdge(const Pointer &p) override;
    void AddConstantDirectEdge(Inst *inst, uint32_t id) override;

    void AddCopyEdge(const Pointer &from, const Pointer &to) override;
    void AddPseudoCopyEdge([[maybe_unused]] const Pointer &base, [[maybe_unused]] const Pointer &field) override
    {
        // we don't do that here
    }

    void VisitHeapInv(Inst *inst) override;
    void Escape(const Inst *inst) override;
    void VisitAllocation(Inst *inst) override;
    void SetTypeInfo(Inst *inst, ObjectTypeInfo info) override;

private:
    BasicBlockState *GetState(const BasicBlock *block);
    void CleanupState(BasicBlock *block);
    void VisitInstsInBlock(BasicBlock *bb);
    bool VisitBlockInternal(BasicBlock *block);
    void VisitLoop(Loop *loop);
    void RollbackChains();
    void AddTempEdge(const Pointer &from, const Pointer &to);
    bool AddEdge(const Pointer &from, const Pointer &to);
    bool AddLoadEdge(const PointerOffset &fromOffset, const Inst *toObj, const ArenaTypedRefSet &srcRefSet);
    bool AddStoreEdge(const Inst *fromObj, const Inst *toObj, const PointerOffset &toOffset,
                      const ArenaTypedRefSet &srcRefSet);

    friend class LoopPropagationVisitor;

private:
    // main state:
    Marker visited_ {};
    ArenaVector<BasicBlockState *> states_;
    ArenaVector<ObjectTypeInfo> refInfos_;
    ArenaMap<const Inst *, ArenaTypedRefSet> instRefSets_;
    // helper structs:
    LoopPropagationVisitor loopVisitor_;
    LiveInAnalysis liveIns_;

    // helper containers:
    // src base -> src offset -> dest pointers
    ArenaMap<const Inst *, PointerOffset::Map<ArenaVector<Pointer>>> loopEdges_;
    // store base -> store offset -> stored objects
    ArenaMap<const Inst *, PointerOffset::Map<ArenaVector<const Inst *>>> loopStoreEdges_;
    ArenaSet<const Inst *> workSet_;
    BasicBlockState *currentBlockState_ {nullptr};
    ArenaTypedRefSet nullSet_;
    ArenaTypedRefSet allSet_;
    bool inLoop_ {false};
};

template <typename LambdaT>
struct LambdaPrinter : LambdaT {
    explicit LambdaPrinter(LambdaT &&self) : LambdaT(std::move(self)) {}

    friend std::ostream &operator<<(std::ostream &o, const LambdaPrinter &p)
    {
        return p(o);
    }
};

auto ObjectTypePropagationVisitor::Printer(ObjectTypeInfo typeInfo)
{
    // CC-OFFNXT(G.FMT.14-CPP) project code style
    return LambdaPrinter([this, typeInfo](std::ostream &o) -> std::ostream & {
        if (typeInfo == ObjectTypeInfo::UNKNOWN) {
            return o << "UNKNOWN";
        }
        if (typeInfo == ObjectTypeInfo::INVALID) {
            return o << "INVALID";
        }
        return o << GetGraph()->GetRuntime()->GetClassName(typeInfo.GetClass());
    });
}

template <typename T>
auto ObjectTypePropagationVisitor::Printer(const TypedRefSet<T> &typedRefSet)
{
    // CC-OFFNXT(G.FMT.14-CPP) project code style
    return LambdaPrinter([this, &typedRefSet](std::ostream &o) -> std::ostream & {
        return o << typedRefSet.GetRefSet() << " :: " << Printer(typedRefSet.GetTypeInfo());
    });
}

template <typename T>
auto ObjectTypePropagationVisitor::Printer(TypedRefSet<T> &&typedRefSet)
{
    // CC-OFFNXT(G.FMT.14-CPP) project code style
    return LambdaPrinter([this, localTypedRefSet = std::move(typedRefSet)](std::ostream &o) -> std::ostream & {
        return o << localTypedRefSet.GetRefSet() << " :: " << Printer(localTypedRefSet.GetTypeInfo());
    });
}

bool ObjectTypePropagationVisitor::RunImpl()
{
    if (!liveIns_.Run(false)) {
        UNREACHABLE();
    }
    auto *graph = GetGraph();
    ASSERT(graph != nullptr);
    graph->RunPass<LoopAnalyzer>();
    MarkerHolder holder(graph);
    visited_ = holder.GetMarker();
    for (auto *block : graph->GetBlocksRPO()) {
        if (!VisitBlockInternal(block)) {
            ResetTypeInfosInGraph();
            return false;
        }
    }
    SetTypeInfosInGraph();
    return true;
}

void ObjectTypePropagationVisitor::SetTypeInfosInGraph()
{
    for (auto &[inst, refs] : instRefSets_) {
        if (inst->IsParameter() && inst->GetObjectTypeInfo()) {
            // type info was set during inlining
            continue;
        }
        ObjectTypeInfo info = ObjectTypeInfo::UNKNOWN;
        [[maybe_unused]] bool isNull = true;
        refs.Visit([this, &info, &isNull](Ref ref) {
            info = Merge(info, GetRefTypeInfo(ref));
            if (ref != NULL_REF) {
                isNull = false;
            }
        });
        ASSERT_DO(info != ObjectTypeInfo::UNKNOWN || isNull,
                  (std::cerr << "Inst shoulnd't have UNKNOWN TypeInfo: " << *inst << "\n"
                             << "refs: " << Printer(refs) << "\n",
                   GetGraph()->Dump(&std::cerr)));
        auto commonInfo = refs.GetTypeInfo();
        TryImproveTypeInfo(info, commonInfo);
        const_cast<Inst *>(inst)->SetObjectTypeInfo(info);
    }
}

void ObjectTypePropagationVisitor::ResetTypeInfosInGraph()
{
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInsts()) {
            inst->SetObjectTypeInfo(ObjectTypeInfo::INVALID);
        }
    }
}

void ObjectTypePropagationVisitor::WalkOutgoingEdges(const Inst *fromInst,
                                                     const PointerOffset::Map<ArenaVector<Pointer>> &edges)
{
    for (const auto &[fromOffset, toPtrs] : edges) {
        for (const auto &toPtr : toPtrs) {
            Pointer fromPtr {fromInst, fromOffset};
            bool changed = AddEdge(fromPtr, toPtr);
            if (changed) {
                ASSERT(toPtr.GetBase() != nullptr);
                workSet_.insert(toPtr.GetBase());
            }
        }
    }
}

void ObjectTypePropagationVisitor::WalkStoreEdges(const Inst *toInst,
                                                  const PointerOffset::Map<ArenaVector<const Inst *>> &edges)
{
    for (const auto &[toOffset, fromInsts] : edges) {
        Pointer toPtr {toInst, toOffset};
        for (const auto *fromInst : fromInsts) {
            auto fromPtr = Pointer::CreateObject(fromInst);
            bool changed = AddEdge(fromPtr, toPtr);
            if (changed) {
                ASSERT(toPtr.GetBase() != nullptr);
            }
        }
    }
}

void ObjectTypePropagationVisitor::WalkEdges()
{
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "LOOP - WalkEdges";
    workSet_ = {};
    for (const auto &[fromInst, edges] : loopEdges_) {
        ASSERT(fromInst != nullptr);
        WalkOutgoingEdges(fromInst, edges);
    }
    while (!workSet_.empty()) {
        auto inst = *workSet_.begin();
        workSet_.erase(workSet_.begin());

        if (auto it = loopStoreEdges_.find(inst); it != loopStoreEdges_.end()) {
            WalkStoreEdges(inst, it->second);
        }
        if (auto it = loopEdges_.find(inst); it != loopEdges_.end()) {
            WalkOutgoingEdges(inst, it->second);
        }
    }
}

Ref ObjectTypePropagationVisitor::NewRef(ObjectTypeInfo info)
{
    refInfos_.push_back(info);
    return refInfos_.size() - 1;
}

ObjectTypeInfo ObjectTypePropagationVisitor::GetRefTypeInfo(Ref ref) const
{
    if (ref == ALL_REF) {
        return ObjectTypeInfo::INVALID;
    }
    return refInfos_.at(ref);
}

void ObjectTypePropagationVisitor::DumpRefInfos(std::ostream &os)
{
    for (Ref ref = BasicBlockState::REAL_REF_START; ref < refInfos_.size(); ref++) {
        auto typeInfo = GetRefTypeInfo(ref);
        os << ref << ": " << Printer(typeInfo) << "\n";
    }
}

ArenaTypedRefSet &ObjectTypePropagationVisitor::GetInstRefSet(const Inst *inst)
{
    switch (inst->GetOpcode()) {
        // No passes that check class references aliasing
        case Opcode::GetInstanceClass:
        case Opcode::LoadImmediate:
            return nullSet_;
        default:
            break;
    }
    if (instRefSets_.find(inst) == instRefSets_.end()) {
        std::cerr << "no inst: " << *inst << '\n';
        GetGraph()->Dump(&std::cerr);
        UNREACHABLE();
    }
    return instRefSets_.at(inst);
}

ArenaTypedRefSet &ObjectTypePropagationVisitor::TryGetInstRefSet(const Inst *inst)
{
    auto it = instRefSets_.find(inst);
    if (it == instRefSets_.end()) {
        return nullSet_;
    }
    return it->second;
}

const ArenaTypedRefSet &ObjectTypePropagationVisitor::GetNullSet() const
{
    ASSERT(nullSet_.PopCount() == 1);
    return nullSet_;
}

const ArenaTypedRefSet &ObjectTypePropagationVisitor::GetAllSet() const
{
    return allSet_;
}

ArenaTypedRefSet &ObjectTypePropagationVisitor::GetOrCreateInstRefSet(const Inst *inst, bool &changed)
{
    ASSERT(inst != nullptr);
    auto ret = instRefSets_.try_emplace(inst, GetGraph()->GetLocalAllocator(), ObjectTypeInfo::UNKNOWN);
    changed |= ret.second;
    return ret.first->second;
}

void ObjectTypePropagationVisitor::AddDirectEdge(const Pointer &p)
{
    if (p.GetType() == STATIC_FIELD) {
        // Load/Store Static - processed in AddCopyEdge
        return;
    }
    auto inst = p.GetBase();
    ASSERT(inst != nullptr);
    auto &refs = IsZeroConstantOrNullPtr(inst) ? nullSet_ : currentBlockState_->NewUnknownRef();
    instRefSets_.try_emplace(inst, refs);
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "Add Direct " << *inst << ": " << Printer(instRefSets_.at(inst));
}

void ObjectTypePropagationVisitor::AddConstantDirectEdge(Inst *inst, [[maybe_unused]] uint32_t id)
{
    ASSERT(inst != nullptr);
    instRefSets_.try_emplace(inst, currentBlockState_->NewUnknownRef());
}

void ObjectTypePropagationVisitor::AddCopyEdge(const Pointer &from, const Pointer &to)
{
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "Add Copy " << from << " -> " << to;
    [[maybe_unused]] auto changed = AddEdge(from, to);
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION)
        << "  ToSet: " << Printer(TryGetInstRefSet(to.GetBase())) << " changed: " << changed;
}

void ObjectTypePropagationVisitor::VisitHeapInv([[maybe_unused]] Inst *inst)
{
    currentBlockState_->InvalidateEscaped();
}

void ObjectTypePropagationVisitor::Escape(const Inst *inst)
{
    currentBlockState_->Escape(inst);
}

void ObjectTypePropagationVisitor::VisitAllocation(Inst *inst)
{
    bool defaultConstructed = inst->GetOpcode() != Opcode::InitObject;
    auto ref = NewRef();
    ASSERT(inst != nullptr);
    auto [it, inserted] = instRefSets_.try_emplace(inst, GetGraph()->GetLocalAllocator(), ObjectTypeInfo::UNKNOWN);
    it->second.SetBit(ref, ObjectTypeInfo::UNKNOWN);
    currentBlockState_->CreateFieldRefSetForNewObject(ref, defaultConstructed);
}

void ObjectTypePropagationVisitor::SetTypeInfo(Inst *inst, ObjectTypeInfo info)
{
    // should be data-flow input instead of check inst
    ASSERT(!inst->IsCheck() || inst->GetOpcode() == Opcode::RefTypeCheck);
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "SetTypeInfo inst " << inst->GetId() << " " << Printer(info);
    auto &refSet = TryGetInstRefSet(inst);
    if (&refSet == &nullSet_) {
        ASSERT(inLoop_);
        COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  -> now has type " << Printer(refSet.GetTypeInfo());
        return;
    }
    if (refSet.PopCount() == 1) {
        auto ref = refSet.GetSingle();
        TryImproveTypeInfo(refInfos_[ref], info);
    }
    refSet.TryImproveTypeInfo(info);
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  -> now has type " << Printer(refSet.GetTypeInfo());
}

BasicBlockState *ObjectTypePropagationVisitor::GetState(const BasicBlock *block)
{
    ASSERT(block->GetId() < states_.size());
    return states_[block->GetId()];
}

// Remove escaped (!) refs dead in the beginning of the block
// Non-escaped refs cannot be removed because they can be part of escape-chain later
void ObjectTypePropagationVisitor::CleanupState(BasicBlock *block)
{
    ASSERT(currentBlockState_ == GetState(block));
    ArenaBitVector liveRefs(GetGraph()->GetLocalAllocator());
    liveIns_.VisitAlive(block, [this, &liveRefs](const Inst *inst) {
        // Currently CatchPhi's inputs can be marked live in the beginning of the start block,
        // so there is not *always* RefSet for inst
        TryGetInstRefSet(inst).Visit([&liveRefs](Ref ref) { liveRefs.SetBit(ref); });
    });
    currentBlockState_->Cleanup([&liveRefs](Ref ref) { return !liveRefs.GetBit(ref); });
}

void ObjectTypePropagationVisitor::VisitInstsInBlock(BasicBlock *bb)
{
    for (auto inst : bb->AllInsts()) {
        AliasVisitor::VisitInstruction(inst);
        TypePropagationVisitor::VisitInstruction(inst);
    }
}

bool ObjectTypePropagationVisitor::VisitBlockInternal(BasicBlock *block)
{
    Loop *loop = nullptr;
    if (block->IsLoopHeader()) {
        loop = block->GetLoop();
        if (loop->IsIrreducible()) {
            return false;
        }
    }

    BasicBlockState *state = nullptr;
    for (auto *pred : block->GetPredsBlocks()) {
        if (!pred->IsMarked(visited_)) {
            ASSERT(block->IsLoopHeader());
            [[maybe_unused]] auto *predLoop = pred->GetLoop();
            ASSERT(predLoop != nullptr);
            ASSERT(predLoop == loop || predLoop->IsInside(loop));
            continue;
        }
        if (state == nullptr) {
            state = GetGraph()->GetLocalAllocator()->New<BasicBlockState>(*GetState(pred), block);
        } else {
            state->Merge(GetState(pred));
        }
    }
    if (state == nullptr) {
        state = GetGraph()->GetLocalAllocator()->New<BasicBlockState>(this, block);
    }
    states_[block->GetId()] = state;
    currentBlockState_ = state;
    if (loop != nullptr) {
        VisitLoop(loop);
    }
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "before visit: " << *state;
    block->SetMarker(visited_);
    CleanupState(block);
    VisitInstsInBlock(block);
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "after visit: " << *state;
    currentBlockState_ = nullptr;
    return true;
}

void ObjectTypePropagationVisitor::VisitLoop(Loop *loop)
{
    ASSERT(!inLoop_);
    ASSERT(GetGraph()->IsAnalysisValid<LoopAnalyzer>());
    inLoop_ = true;
    loopVisitor_.VisitLoop(loop);
    RollbackChains();
    inLoop_ = false;
}

void ObjectTypePropagationVisitor::RollbackChains()
{
    for (auto &[base, edges] : loopEdges_) {
        edges.clear();
    }
    for (auto &[base, edges] : loopStoreEdges_) {
        edges.clear();
    }
}

void ObjectTypePropagationVisitor::AddTempEdge(const Pointer &from, const Pointer &to)
{
    ASSERT(from.GetBase() != nullptr);
    {
        auto [fromObj, fromOffset] = from.DropIdx();
        auto &instEdges = loopEdges_.try_emplace(fromObj, GetGraph()->GetLocalAllocator()->Adapter()).first->second;
        auto &ptrEdges = instEdges.try_emplace(fromOffset, GetGraph()->GetLocalAllocator()->Adapter()).first->second;
        ptrEdges.push_back(to);
    }
    if (from.IsObject() && !to.IsObject() && to.GetType() != STATIC_FIELD && to.GetType() != POOL_CONSTANT) {
        // store
        auto [toObj, toOffset] = to.DropIdx();
        auto &instEdges = loopStoreEdges_.try_emplace(toObj, GetGraph()->GetLocalAllocator()->Adapter()).first->second;
        auto &ptrEdges = instEdges.try_emplace(toOffset, GetGraph()->GetLocalAllocator()->Adapter()).first->second;
        ptrEdges.push_back(from.GetBase());
    }
}

bool ObjectTypePropagationVisitor::AddEdge(const Pointer &from, const Pointer &to)
{
    auto [fromObj, fromOffset] = from.DropIdx();
    auto [toObj, toOffset] = to.DropIdx();
    auto it = instRefSets_.find(fromObj);
    const ArenaTypedRefSet *srcRefSet = nullptr;
    if (fromObj == nullptr) {
        ASSERT(fromOffset.GetType() == STATIC_FIELD || fromOffset.GetType() == POOL_CONSTANT);
        srcRefSet = &currentBlockState_->NewUnknownRef();
    } else if (it != instRefSets_.end()) {
        srcRefSet = &it->second;
    } else if (!fromObj->IsReferenceOrAny()) {
        // inputs of instructions with ANY type may all have primitive types
        srcRefSet = &currentBlockState_->NewUnknownRef();
    } else {
        ASSERT(inLoop_ || toObj->IsPhi() || toObj->IsCatchPhi());
        return false;
    }
    ASSERT(currentBlockState_ != nullptr);
    if (fromOffset.IsObject() && toOffset.IsObject()) {
        // copy
        bool changed = false;
        auto &destRefSet = GetOrCreateInstRefSet(toObj, changed);
        if (!destRefSet.Includes(*srcRefSet)) {
            destRefSet |= *srcRefSet;
            changed = true;
        }
        return changed;
    }
    if (toOffset.IsObject()) {
        return AddLoadEdge(fromOffset, toObj, *srcRefSet);
    }
    if (fromOffset.IsObject()) {
        return AddStoreEdge(fromObj, toObj, toOffset, *srcRefSet);
    }
    // both are not objects
    UNREACHABLE();
}

bool ObjectTypePropagationVisitor::AddLoadEdge(const PointerOffset &fromOffset, const Inst *toObj,
                                               const ArenaTypedRefSet &srcRefSet)
{
    bool changed = false;
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << " LOAD";
    auto &destRefSet = GetOrCreateInstRefSet(toObj, changed);
    if (fromOffset.GetType() == STATIC_FIELD || fromOffset.GetType() == POOL_CONSTANT) {
        COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  STATIC FIELD -> " << toObj->GetId();
        changed |= destRefSet != srcRefSet;
        destRefSet = srcRefSet;
        return changed;
    }
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  initial destRefSet: " << Printer(destRefSet);
    srcRefSet.Visit([&fromOffset, &destRefSet, &changed, this](Ref ref) {
        COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  Visit " << ref;
        auto &srcFieldSet = currentBlockState_->GetFieldRefSet(ref, fromOffset);
        COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  srcFieldSet: " << Printer(srcFieldSet);
        if (!destRefSet.Includes(srcFieldSet)) {
            destRefSet |= srcFieldSet;
            changed = true;
        }
    });
    return changed;
}

bool ObjectTypePropagationVisitor::AddStoreEdge(const Inst *fromObj, const Inst *toObj, const PointerOffset &toOffset,
                                                const ArenaTypedRefSet &srcRefSet)
{
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << " STORE";
    ASSERT(fromObj != nullptr);
    if (!fromObj->IsReferenceOrAny()) {
        // primitive ANY type or null as integer is stored
        return false;
    }
    if (toOffset.GetType() == STATIC_FIELD || toOffset.GetType() == POOL_CONSTANT) {
        return false;
    }
    if (instRefSets_.find(toObj) == instRefSets_.end()) {
        ASSERT(inLoop_);
        return true;
    }
    auto &destRefSet = instRefSets_.at(toObj);
    bool changed = false;
    bool escape = false;
    destRefSet.Visit([&toOffset, &srcRefSet, &changed, &escape, this](Ref ref) {
        if (currentBlockState_->IsEscaped(ref)) {
            escape = true;
        }
        if (ref == ALL_REF) {
            return;
        }
        auto &dstFieldSet = currentBlockState_->CreateFieldRefSet(ref, toOffset, changed);
        COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  ref: " << ref;
        COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  src: " << Printer(srcRefSet);
        if (!inLoop_ && toOffset.GetType() != UNKNOWN_OFFSET) {
            changed |= dstFieldSet != srcRefSet;
            dstFieldSet = srcRefSet;
        } else if (!dstFieldSet.Includes(srcRefSet)) {
            dstFieldSet |= srcRefSet;
            changed = true;
        }
        COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "  dst: " << Printer(dstFieldSet);
    });
    if (escape) {
        srcRefSet.Visit([this](Ref ref) { currentBlockState_->TryEscape(ref); });
    }
    return changed;
}

LoopPropagationVisitor::LoopPropagationVisitor(ObjectTypePropagationVisitor *parent)
    : TypePropagationVisitor(parent->GetGraph()),
      parent_(parent),
      escapedInsts_(parent_->GetGraph()->GetLocalAllocator()->Adapter())
{
    AliasVisitor::Init(parent->GetGraph()->GetLocalAllocator());
}

void LoopPropagationVisitor::AddDirectEdge(const Pointer &p)
{
    parent_->AddDirectEdge(p);
}

void LoopPropagationVisitor::VisitAllocation(Inst *inst)
{
    parent_->VisitAllocation(inst);
}

void LoopPropagationVisitor::AddConstantDirectEdge(Inst *inst, uint32_t id)
{
    parent_->AddConstantDirectEdge(inst, id);
}

void LoopPropagationVisitor::AddCopyEdge(const Pointer &from, const Pointer &to)
{
    if (from.GetType() == STATIC_FIELD || from.GetType() == POOL_CONSTANT) {
        ASSERT(to.IsObject());
        AddDirectEdge(to);
    } else if (to.GetType() == STATIC_FIELD || to.GetType() == POOL_CONSTANT) {
        ASSERT(from.IsObject());
        Escape(from.GetBase());
    } else {
        parent_->AddTempEdge(from, to);
    }
}

void LoopPropagationVisitor::AddPseudoCopyEdge([[maybe_unused]] const Pointer &base,
                                               [[maybe_unused]] const Pointer &field)
{
    // no action
}

void LoopPropagationVisitor::Escape(const Inst *inst)
{
    escapedInsts_.insert(inst);
}

void LoopPropagationVisitor::SetTypeInfo(Inst *inst, ObjectTypeInfo info)
{
    parent_->SetTypeInfo(inst, info);
}

void LoopPropagationVisitor::VisitLoop(Loop *loop)
{
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "Visit loop " << loop->GetId();
    ASSERT(loop != nullptr && !loop->IsIrreducible());
    heapInv_ = false;
    headerState_ = parent_->GetState(loop->GetHeader());
    escapedInsts_.clear();
    VisitLoopRec(loop);
    parent_->WalkEdges();
    for (auto *inst : escapedInsts_) {
        headerState_->Escape(inst);
    }
    if (heapInv_) {
        headerState_->InvalidateEscaped();
    }
}

void LoopPropagationVisitor::VisitBlock(BasicBlock *bb)
{
    for (auto inst : bb->AllInsts()) {
        AliasVisitor::VisitInstruction(inst);
        TypePropagationVisitor::VisitInstruction(inst);
    }
}

void LoopPropagationVisitor::VisitLoopRec(Loop *loop)
{
    ASSERT(loop != nullptr && !loop->IsRoot());
    for (auto *block : loop->GetBlocks()) {
        VisitBlock(block);
    }
    for (auto *inner : loop->GetInnerLoops()) {
        VisitLoopRec(inner);
    }
}

BasicBlockState::BasicBlockState(ObjectTypePropagationVisitor *visitor, [[maybe_unused]] BasicBlock *bb)
    : visitor_(visitor),
      fieldRefs_(visitor->GetGraph()->GetLocalAllocator()->Adapter()),
      escaped_(visitor->GetGraph()->GetLocalAllocator(), ObjectTypeInfo::UNKNOWN)
{
    escaped_.SetBit(NULL_REF, ObjectTypeInfo::UNKNOWN);
    // two refs representing unknown objects
    // two of them to express that 2 unknown objects are not equal (intersection size > 1)
    escaped_.SetBit(1U, ObjectTypeInfo::INVALID);
    escaped_.SetBit(2U, ObjectTypeInfo::INVALID);
#ifndef NDEBUG
    bb_ = bb;
#endif
}

BasicBlockState::BasicBlockState(const BasicBlockState &other, [[maybe_unused]] BasicBlock *bb) : BasicBlockState(other)
{
#ifndef NDEBUG
    this->bb_ = bb;
#endif
}

const ArenaTypedRefSet &BasicBlockState::GetFieldRefSet(Ref base, const PointerOffset &offset)
{
    if (base == ALL_REF) {
        return visitor_->GetAllSet();
    }
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "    GetFieldRefSet " << base << ' ' << offset;
    if (offset.GetType() == UNKNOWN_OFFSET) {
        return escaped_;
    }
    auto it = fieldRefs_.find(base);
    if (it == fieldRefs_.end()) {
        return escaped_;
    }
    auto &fieldRefsOfBase = it->second;
    if (fieldRefsOfBase->find(offset) == fieldRefsOfBase->end()) {
        return escaped_;
    }
    // lazy propagation of refs on unknown offset
    if (auto overwriting = fieldRefsOfBase->find(PointerOffset::CreateUnknownOffset());
        overwriting != fieldRefsOfBase->end()) {
        fieldRefsOfBase.Mut(GetLocalAllocator())->at(offset) |= overwriting->second;
    }
    return fieldRefsOfBase->at(offset);
}

void BasicBlockState::CreateFieldRefSetForNewObject(Ref base, bool defaultConstructed)
{
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "    CreateFieldRefSetForNewObject " << base;
    auto [it, inserted] = fieldRefs_.try_emplace(base, GetLocalAllocator());
    ASSERT(inserted);
    auto &objectInfos = it->second;
    const auto &refs = defaultConstructed ? visitor_->GetNullSet() : escaped_;
    auto mut = objectInfos.Mut(GetLocalAllocator());
    ASSERT(mut != nullptr);
    // create "other refs" entry
    if (!mut->try_emplace(PointerOffset::CreateDefaultField(), refs).second) {
        UNREACHABLE();
    }
}

ArenaTypedRefSet &BasicBlockState::CreateFieldRefSet(Ref base, const PointerOffset &offset, bool &changed)
{
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "    CreateFieldRefSet " << base << ' ' << offset;
    auto [it1, ins1] = fieldRefs_.try_emplace(base, GetLocalAllocator());
    changed |= ins1;
    auto &objectInfos = it1->second;
    if (ins1) {
        auto mut = objectInfos.Mut(GetLocalAllocator());
        ASSERT(mut != nullptr);
        // create "other refs" entry
        if (!mut->try_emplace(PointerOffset::CreateDefaultField(), escaped_).second) {
            UNREACHABLE();
        }
    }
    auto [it2, ins2] =
        objectInfos.Mut(GetLocalAllocator())->try_emplace(offset, GetLocalAllocator(), ObjectTypeInfo::UNKNOWN);
    changed |= ins2;
    return it2->second;
}

const ArenaTypedRefSet &BasicBlockState::NewUnknownRef()
{
    return escaped_;
}

template <typename T>
class IsMap : public std::false_type {
};

template <typename K, typename T, typename C, typename A>
class IsMap<std::map<K, T, C, A>> : public std::true_type {
};

template <typename T>
constexpr bool IS_ORDERED_MAP_V = IsMap<std::decay_t<T>>::value;

// Harsh - set intersection for keys + set union for keys in intersection
// Not harsh (more precise) - set union for keys + set union for keys in intersection
template <bool HARSH = false>
void MergeFieldSets(PointerOffset::Map<ArenaTypedRefSet> &to, const PointerOffset::Map<ArenaTypedRefSet> &from)
{
    static_assert(!IS_ORDERED_MAP_V<decltype(to)>);
    ASSERT(to.count(PointerOffset::CreateDefaultField()));
    ASSERT(from.count(PointerOffset::CreateDefaultField()));
    auto &defaultRefsTo = to.at(PointerOffset::CreateDefaultField());
    auto defaultRefsFrom = from.at(PointerOffset::CreateDefaultField());
    for (auto it = to.begin(); it != to.end();) {
        auto fromIt = from.find(it->first);
        if (fromIt != from.end()) {
            it->second |= fromIt->second;
            it++;
        } else if constexpr (!HARSH) {
            it->second |= defaultRefsFrom;
            it++;
        } else {
            defaultRefsTo |= it->second;
            it = to.erase(it);
        }
    }
    for (const auto &[offset, refs] : from) {
        if (to.find(offset) == to.end()) {
            if constexpr (HARSH) {
                defaultRefsTo |= refs;
            } else {
                auto it = to.emplace(offset, refs).first;
                // defaultRefsTo may be invalidated, recompute
                it->second |= to.at(PointerOffset::CreateDefaultField());
            }
        }
    }
}

void BasicBlockState::Merge(BasicBlockState *other)
{
    // in-place set intersection
    auto otherIt = other->fieldRefs_.begin();
    static_assert(IS_ORDERED_MAP_V<decltype(fieldRefs_)>);
    for (auto it = fieldRefs_.begin(); it != fieldRefs_.end();) {
        otherIt = std::find_if(otherIt, other->fieldRefs_.end(),
                               [&it](auto otherElem) { return otherElem.first >= it->first; });
        auto &[ref, refFields] = *it;
        if (otherIt != other->fieldRefs_.end() && otherIt->first == ref) {
            auto mut = refFields.Mut(GetLocalAllocator());
            ASSERT(mut != nullptr);
            MergeFieldSets(*mut, *otherIt->second);
            if (IsEscaped(ref) != other->IsEscaped(ref)) {
                ForceEscape(ref);
            }
            it++;
        } else {
            ASSERT(otherIt == other->fieldRefs_.end() || otherIt->first > it->first);
            it = fieldRefs_.erase(it);
        }
    }
    escaped_ |= other->escaped_;
}

void BasicBlockState::Escape(ArenaVector<Ref> &worklist, const ArenaTypedRefSet &refs)
{
    refs.Visit([this, &worklist](Ref fieldRef) {
        if (!IsEscaped(fieldRef)) {
            escaped_.SetBit(fieldRef, ObjectTypeInfo::INVALID);
            worklist.push_back(fieldRef);
        }
    });
}

void BasicBlockState::Escape(ArenaVector<Ref> &worklist)
{
    while (!worklist.empty()) {
        auto ref = worklist.back();
        worklist.pop_back();
        auto it = fieldRefs_.find(ref);
        if (it == fieldRefs_.end()) {
            continue;
        }
        for (const auto &fieldRefs : *it->second) {
            Escape(worklist, fieldRefs.second);
        }
    }
}

void BasicBlockState::Escape(const Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "ESCAPE inst: " << *inst;

    ArenaVector<Ref> worklist(visitor_->GetGraph()->GetLocalAllocator()->Adapter());
    visitor_->GetInstRefSet(inst).Visit([this, &worklist](Ref ref) {
        if (!IsEscaped(ref)) {
            COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "ESCAPE ref: " << ref;
            escaped_.SetBit(ref, ObjectTypeInfo::INVALID);
            worklist.push_back(ref);
        }
    });

    Escape(worklist);
}

void BasicBlockState::TryEscape(Ref ref)
{
    if (IsEscaped(ref)) {
        return;
    }

    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "ESCAPE ref: " << ref;
    escaped_.SetBit(ref, ObjectTypeInfo::INVALID);
    ArenaVector<Ref> worklist({ref}, visitor_->GetGraph()->GetLocalAllocator()->Adapter());
    Escape(worklist);
}

/* Updates `escaped_` RefSet if:
 * - ref has escaped already, but new refs to non-escaped refs were added;
 * - or ref has not escaped yet.
 */
void BasicBlockState::ForceEscape(Ref ref)
{
    escaped_.SetBit(ref, ObjectTypeInfo::INVALID);
    ArenaVector<Ref> worklist({ref}, visitor_->GetGraph()->GetLocalAllocator()->Adapter());
    Escape(worklist);
}

void BasicBlockState::InvalidateEscapedRef(Ref ref)
{
    if (ref == ALL_REF) {
        if (fieldRefs_.size() == 1 && fieldRefs_.begin()->first == ALL_REF) {
            // already invalidated
            return;
        }
        fieldRefs_.clear();
        fieldRefs_.try_emplace(ALL_REF, GetLocalAllocator());
        return;
    }
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "INVALIDATE ref " << ref;
    auto it = fieldRefs_.find(ref);
    if (it == fieldRefs_.end()) {
        return;
    }
    auto *refFields = it->second.Mut(GetLocalAllocator());
    ASSERT(refFields != nullptr);
    auto oldDefault = refFields->find(PointerOffset::CreateDefaultField());
    ASSERT(oldDefault != refFields->end());
    // erase all except `default value`
    refFields->erase(std::next(oldDefault), refFields->end());
    refFields->erase(refFields->begin(), oldDefault);
    ASSERT(refFields->size() == 1);
    ASSERT(refFields->begin()->first == PointerOffset::CreateDefaultField());
    // invoke copy-assignment - no additional space is consumed on repeated invalidations
    refFields->begin()->second = escaped_;
}

void BasicBlockState::InvalidateEscaped()
{
    COMPILER_LOG(DEBUG, TYPE_PROPAGATION) << "INVALIDATE BB " << GetBlockId();
    escaped_.Visit([this](Ref ref) {
        if (ref >= REAL_REF_START) {
            InvalidateEscapedRef(ref);
        }
    });
}

bool BasicBlockState::IsEscaped(Ref ref) const
{
    if (ref == ALL_REF) {
        return true;
    }
    return escaped_.GetBit(ref);
}

template <typename F>
void BasicBlockState::Cleanup(const F &remove)
{
    for (auto it = fieldRefs_.begin(); it != fieldRefs_.end();) {
        if (IsEscaped(it->first) && remove(it->first)) {
            it = fieldRefs_.erase(it);
        } else {
            it++;
        }
    }
}

size_t BasicBlockState::GetBlockId() const
{
#ifndef NDEBUG
    return bb_->GetId();
#else
    return 0;
#endif  // NDEBUG
}

ArenaAllocator *BasicBlockState::GetLocalAllocator()
{
    return visitor_->GetGraph()->GetLocalAllocator();
}

[[maybe_unused]] std::ostream &operator<<(std::ostream &os, const BasicBlockState &state)
{
    auto *visitor = state.visitor_;
    os << "BB " << state.GetBlockId() << " state:\n";
    for (auto [ref, refInfo] : state.fieldRefs_) {
        os << "  ref " << ref << " " << visitor->Printer(visitor->GetRefTypeInfo(ref)) << ":\n";
        for (auto [offset, refs] : *refInfo) {
            os << "    " << offset << " -> " << visitor->Printer(refs) << "\n";
        }
    }
    os << "Escaped: " << visitor->Printer(state.escaped_) << "\n";
    os << "Instructions RefSets (global):\n";
    for (auto &[inst, refs] : visitor->GetInstRefSets()) {
        // short inst dump without inputs/outputs
        os << "  " << inst->GetId() << "." << inst->GetType() << " ";
        inst->DumpOpcode(&os);
        os << "-> " << visitor->Printer(refs) << "\n";
    }
    visitor->DumpRefInfos(os);
    return os;
}

void TypePropagationVisitor::VisitNewObject(GraphVisitor *v, Inst *i)
{
    auto *self = static_cast<TypePropagationVisitor *>(v);
    auto inst = i->CastToNewObject();
    auto klass = self->GetGraph()->GetRuntime()->GetClass(inst->GetMethod(), inst->GetTypeId());
    if (klass != nullptr) {
        self->SetTypeInfo(inst, {klass, true});
    }
}

void TypePropagationVisitor::VisitNewArray(GraphVisitor *v, Inst *i)
{
    auto *self = static_cast<TypePropagationVisitor *>(v);
    auto inst = i->CastToNewArray();
    auto klass = self->GetGraph()->GetRuntime()->GetClass(inst->GetMethod(), inst->GetTypeId());
    if (klass != nullptr) {
        self->SetTypeInfo(inst, {klass, true});
    }
}

void TypePropagationVisitor::VisitLoadString(GraphVisitor *v, Inst *i)
{
    auto *self = static_cast<TypePropagationVisitor *>(v);
    auto inst = i->CastToLoadString();
    auto klass = self->GetGraph()->GetRuntime()->GetLineStringClass(inst->GetMethod(), nullptr);
    if (klass != nullptr) {
        self->SetTypeInfo(inst, {klass, true});
    }
}

void TypePropagationVisitor::VisitLoadArray([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *i)
{
    if (i->GetType() != DataType::REFERENCE) {
        return;
    }
    VisitRefTypeCheck(v, i);
}

void TypePropagationVisitor::VisitLoadObject(GraphVisitor *v, Inst *i)
{
    if (i->GetType() != DataType::REFERENCE || i->CastToLoadObject()->GetObjectType() != ObjectType::MEM_OBJECT) {
        return;
    }
    auto *self = static_cast<TypePropagationVisitor *>(v);
    auto inst = i->CastToLoadObject();
    auto fieldId = inst->GetTypeId();
    if (fieldId == 0) {
        return;
    }
    auto runtime = self->GetGraph()->GetRuntime();
    auto method = inst->GetMethod();
    auto typeId = runtime->GetFieldValueTypeId(method, fieldId);
    auto klass = runtime->GetClass(method, typeId);
    if (klass != nullptr) {
        auto isExact = runtime->GetClassType(method, typeId) == ClassType::FINAL_CLASS;
        self->SetTypeInfo(inst, {klass, isExact});
    }
}

void TypePropagationVisitor::VisitLoadStatic(GraphVisitor *v, Inst *i)
{
    if (i->GetType() != DataType::REFERENCE) {
        return;
    }
    auto *self = static_cast<TypePropagationVisitor *>(v);
    auto inst = i->CastToLoadStatic();
    auto fieldId = inst->GetTypeId();
    if (fieldId == 0) {
        return;
    }
    auto runtime = self->GetGraph()->GetRuntime();
    auto method = inst->GetMethod();
    auto typeId = runtime->GetFieldValueTypeId(method, fieldId);
    auto klass = runtime->GetClass(method, typeId);
    if (klass != nullptr) {
        auto isExact = runtime->GetClassType(method, typeId) == ClassType::FINAL_CLASS;
        self->SetTypeInfo(inst, {klass, isExact});
    }
}

void TypePropagationVisitor::VisitCallStatic(GraphVisitor *v, Inst *i)
{
    ProcessManagedCall(v, i->CastToCallStatic());
}

void TypePropagationVisitor::VisitCallVirtual(GraphVisitor *v, Inst *i)
{
    ProcessManagedCall(v, i->CastToCallVirtual());
}

void TypePropagationVisitor::VisitRefTypeCheck(GraphVisitor *v, Inst *i)
{
    auto arrayTypeInfo = i->GetDataFlowInput(0)->GetObjectTypeInfo();
    if (!arrayTypeInfo) {
        return;
    }
    auto *self = static_cast<TypePropagationVisitor *>(v);
    auto runtime = self->GetGraph()->GetRuntime();
    if (!runtime->IsArrayClass(arrayTypeInfo.GetClass())) {
        return;
    }
    if (runtime->GetArrayComponentType(arrayTypeInfo.GetClass()) != DataType::REFERENCE) {
        return;
    }
    auto storedClass = runtime->GetArrayElementClass(arrayTypeInfo.GetClass());
    if (storedClass == nullptr) {
        return;
    }
    auto isExact = runtime->GetClassType(storedClass) == ClassType::FINAL_CLASS;
    self->SetTypeInfo(i, {storedClass, isExact});
}

void TypePropagationVisitor::VisitParameter(GraphVisitor *v, Inst *i)
{
    auto inst = i->CastToParameter();
    auto graph = i->GetBasicBlock()->GetGraph();
    if (inst->GetType() != DataType::REFERENCE || graph->IsBytecodeOptimizer() || inst->HasObjectTypeInfo()) {
        return;
    }
    auto refNum = inst->GetArgRefNumber();
    auto runtime = graph->GetRuntime();
    auto method = graph->GetMethod();
    RuntimeInterface::ClassPtr klass;
    if (refNum == ParameterInst::INVALID_ARG_REF_NUM) {
        // This parametr doesn't have ArgRefNumber
        if (inst->GetArgNumber() != 0 || runtime->IsMethodStatic(method)) {
            return;
        }
        klass = runtime->GetClass(method);
    } else {
        auto typeId = runtime->GetMethodArgReferenceTypeId(method, refNum);
        klass = runtime->GetClass(method, typeId);
    }
    if (klass != nullptr) {
        auto isExact = runtime->GetClassType(klass) == ClassType::FINAL_CLASS;
        auto *self = static_cast<TypePropagationVisitor *>(v);
        self->SetTypeInfo(inst, {klass, isExact});
    }
}

void TypePropagationVisitor::ProcessManagedCall(GraphVisitor *v, CallInst *inst)
{
    if (inst->GetType() != DataType::REFERENCE) {
        return;
    }
    if (inst->IsInlined()) {
        return;
    }
    auto *self = static_cast<TypePropagationVisitor *>(v);
    auto runtime = self->GetGraph()->GetRuntime();
    auto method = inst->GetCallMethod();
    auto typeId = runtime->GetMethodReturnTypeId(method);
    auto klass = runtime->GetClass(method, typeId);
    if (klass != nullptr) {
        auto isExact = runtime->GetClassType(method, typeId) == ClassType::FINAL_CLASS;
        self->SetTypeInfo(inst, {klass, isExact});
    }
}

}  // namespace

ObjectTypePropagation::ObjectTypePropagation(Graph *graph) : Analysis(graph) {}

bool ObjectTypePropagation::RunImpl()
{
    ObjectTypePropagationVisitor visitor(GetGraph());
    visitor.RunImpl();
    return true;
}

}  // namespace ark::compiler
