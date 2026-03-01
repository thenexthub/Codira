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

#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/alias_analysis.h"
#include "compiler_logger.h"

/**
 * See  "Efficient Field-sensitive pointer analysis for C" by "David J. Pearce
 * and Paul H. J. Kelly and Chris Hankin
 *
 * We treat each IR Inst as a constraint that may be applied to a set of
 * aliases of some virtual register.  Virtual registers are used as constraint
 * variables as well.
 *
 * In order to solve the system of set constraints, the following is done:
 *
 * 1. Each constraint variable x has a solution set associated with it, Sol(x).
 * Implemented through AliasAnalysis::points_to_ that contains mapping of
 * virtual register to possible aliases.
 *
 * 2. Constraints are separated into direct, copy.
 *
 * - Direct constraints are constraints that require no extra processing, such
 * as P = &Q.
 *
 * - Copy constraints are those of the form P = Q.  Such semantic can be
 * obtained through NullCheck, Mov, and Phi instructions.
 *
 * 3. All direct constraints of the form P = &Q are processed, such that Q is
 * added to Sol(P).
 *
 * 4. A directed graph is built out of the copy constraints.  Each constraint
 * variable is a node in the graph, and an edge from Q to P is added for each
 * copy constraint of the form P = Q.
 *
 * 5. The graph is then walked, and solution sets are propagated along the copy
 * edges, such that an edge from Q to P causes Sol(P) <- Sol(P) union Sol(Q).
 *
 * 6. The process of walking the graph is iterated until no solution sets
 * change.
 *
 * To add new instructions to alias analysis please consider following:
 *     - IsLocalAlias method: to add instructions that create new object
 *     - ParseInstruction method: to add a new instruction alias analysis works for
 *     - AliasAnalysis class: to add a visitor for a new instruction that should be analyzed
 *
 * NOTE(Evgenii Kudriashov): Prior to walking the graph in steps 5 and 6, We
 * need to perform static cycle elimination on the constraint graph, as well as
 * off-line variable substitution.
 *
 * NOTE(Evgenii Kudriashov): To add flow-sensitivity the "Flow-sensitive
 * pointer analysis for millions of lines of code" by Ben Hardekopf and Calvin
 * Lin may be considered.
 *
 * NOTE(Evgenii Kudriashov): After implementing VRP and SCEV the "Loop-Oriented
 * Array- and Field-Sensitive Pointer Analysis for Automatic SIMD
 * Vectorization" by Yulei Sui, Xiaokang Fan, Hao Zhou, and Jingling Xue may be
 * considered to add advanced analysis of array indices.
 */

namespace ark::compiler {

bool PointerWithInfo::IsLocalCreated() const
{
    for (const auto &pointer : PointsTo()) {
        if (!(pointer.IsObject() && pointer.IsLocalCreatedAlias())) {
            return false;
        }
    }
    return true;
}

AliasAnalysis::AliasAnalysis(Graph *graph) : Analysis(graph), pointerInfo_(graph->GetAllocator()->Adapter()) {}

const ArenaVector<BasicBlock *> &AliasAnalysis::GetBlocksToVisit() const
{
    return GetGraph()->GetBlocksRPO();
}

[[maybe_unused]] static std::string TypeToStr(AliasType t)
{
    switch (t) {
        case MAY_ALIAS:
            return "MAY_ALIAS";
        case NO_ALIAS:
            return "NO_ALIAS";
        case MUST_ALIAS:
            return "MUST_ALIAS";
        case ALIAS_IF_BASE_EQUALS:
            return "ALIAS_IF_BASE_EQUALS";
        default:
            UNREACHABLE();
    }
}

bool AliasAnalysis::RunImpl()
{
    Init();

    VisitGraph();

    for (const auto &[from, _] : *direct_) {
        CreatePointerInfo(from);
    }
    for (const auto &[from, to] : *copy_) {
        CreatePointerInfo(from);
        CreatePointerInfo(to);
    }

    // Initialize solution sets
    for (auto pair : *direct_) {
        ASSERT(pair.first.GetBase() == nullptr || pair.first.GetBase()->GetOpcode() != Opcode::NullCheck);
        ASSERT(pair.second.GetBase() == nullptr || pair.second.GetBase()->GetOpcode() != Opcode::NullCheck);
        GetPointerInfo(pair.first).Insert(pair.second);
    }

    // Build graphs
    for (auto pair : *copy_) {
        auto it = chains_->try_emplace(pair.first, GetGraph()->GetLocalAllocator()->Adapter());
        ASSERT(pair.first.GetBase() == nullptr || pair.first.GetBase()->GetOpcode() != Opcode::NullCheck);
        ASSERT(pair.second.GetBase() == nullptr || pair.second.GetBase()->GetOpcode() != Opcode::NullCheck);
        it.first->second.push_back(pair.second);
    }

    FindEscapingPointers();
    SolveConstraints();

#ifndef NDEBUG
    if (CompilerLogger::IsComponentEnabled(CompilerLoggerComponents::ALIAS_ANALYSIS)) {
        std::ostringstream out;
        DumpCopy(&out);
        DumpDirect(&out);
        DumpChains(&out);
        Dump(&out);
        COMPILER_LOG(DEBUG, ALIAS_ANALYSIS) << out.str();
    }
#endif
    return true;
}

void AliasAnalysis::Init()
{
    auto allocator = GetGraph()->GetLocalAllocator();
    chains_ = allocator->New<Pointer::Map<ArenaVector<Pointer>>>(allocator->Adapter());
    reverseChains_ = allocator->New<Pointer::Map<ArenaVector<Pointer>>>(allocator->Adapter());
    direct_ = allocator->New<PointerPairVector>(allocator->Adapter());
    copy_ = allocator->New<PointerPairVector>(allocator->Adapter());
    ASSERT(chains_ != nullptr);
    ASSERT(reverseChains_ != nullptr);
    ASSERT(direct_ != nullptr);
    ASSERT(copy_ != nullptr);
    AliasVisitor::Init(allocator);
    pointerInfo_.clear();
}

static bool PointerLess(const Pointer &lhs, const Pointer &rhs)
{
    if (lhs.GetBase() == rhs.GetBase()) {
        return lhs.GetImm() < rhs.GetImm();
    }
    if (lhs.GetBase() == nullptr) {
        return true;
    }
    if (rhs.GetBase() == nullptr) {
        return false;
    }
    return lhs.GetBase()->GetId() < rhs.GetBase()->GetId();
}

void AliasAnalysis::DumpChains(std::ostream *out) const
{
    ArenaVector<Pointer> sortedKeys(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto &pair : *chains_) {
        sortedKeys.push_back(pair.first);
    }
    if (sortedKeys.empty()) {
        (*out) << "\nThe chains is empty" << std::endl;
        return;
    }
    std::sort(sortedKeys.begin(), sortedKeys.end(), PointerLess);

    (*out) << "\nThe chains are the following: {" << std::endl;
    for (auto &p : sortedKeys) {
        (*out) << "\t";
        p.Dump(out);
        (*out) << ": {";

        // Sort by instruction ID to add more readability to logs
        ArenaVector<Pointer> sorted(chains_->at(p), GetGraph()->GetLocalAllocator()->Adapter());
        std::sort(sorted.begin(), sorted.end(), PointerLess);
        auto edge = sorted.begin();
        if (edge != sorted.end()) {
            edge->Dump(out);
            while (++edge != sorted.end()) {
                (*out) << ", ";
                edge->Dump(out);
            }
        }
        (*out) << "}" << std::endl;
    }
    (*out) << "}" << std::endl;
}

void AliasAnalysis::DumpDirect(std::ostream *out) const
{
    (*out) << "\nThe direct are the following:" << std::endl;
    for (auto &pair : *direct_) {
        (*out) << "{ ";
        pair.first.Dump(out);
        (*out) << " , ";
        pair.second.Dump(out);
        (*out) << " }" << std::endl;
    }
}

void AliasAnalysis::DumpCopy(std::ostream *out) const
{
    (*out) << "\nThe copy are the following:" << std::endl;
    for (auto &pair : *copy_) {
        (*out) << "{ ";
        pair.first.Dump(out);
        (*out) << " , ";
        pair.second.Dump(out);
        (*out) << " }" << std::endl;
    }
}

void AliasAnalysis::Dump(std::ostream *out) const
{
    ArenaVector<Pointer> sortedKeys(GetGraph()->GetLocalAllocator()->Adapter());
    for (const auto &pair : pointerInfo_) {
        sortedKeys.push_back(pair.first);
    }
    if (sortedKeys.empty()) {
        (*out) << "\nThe solution set is empty" << std::endl;
        return;
    }
    std::sort(sortedKeys.begin(), sortedKeys.end(), PointerLess);

    (*out) << "\nThe solution set is the following:" << std::endl;
    for (auto &p : sortedKeys) {
        (*out) << "\t";
        p.Dump(out);
        const auto &info = GetPointerInfo(p);
        if (info.IsLocal()) {
            (*out) << "(local)";
        }
        if (info.IsVolatile()) {
            (*out) << "(v)";
        }
        (*out) << ": {";

        // Sort by instruction ID to add more readability to logs
        auto values = info.PointsTo();
        ArenaVector<Pointer> sorted(values.begin(), values.end(), GetGraph()->GetLocalAllocator()->Adapter());
        std::sort(sorted.begin(), sorted.end(), PointerLess);
        auto iter = sorted.begin();
        if (iter != sorted.end()) {
            iter->Dump(out);
            while (++iter != sorted.end()) {
                (*out) << ", ";
                iter->Dump(out);
            }
        }
        (*out) << "}" << std::endl;
    }
}

template <bool REFINED>
AliasType AliasAnalysis::CheckInstAlias(Inst *mem1, Inst *mem2) const
{
    ASSERT(mem1->IsMemory() && mem2->IsMemory());
    Pointer p1 = {};
    Pointer p2 = {};

    if (!ParseInstruction(mem1, &p1) || !ParseInstruction(mem2, &p2)) {
        return MAY_ALIAS;
    }

    // Instructions with different types cannot alias each other. Handle
    // difference only in numeric types for now.
    if (IsTypeNumeric(mem1->GetType()) && IsTypeNumeric(mem2->GetType()) &&
        GetTypeSize(mem1->GetType(), GetGraph()->GetArch()) != GetTypeSize(mem2->GetType(), GetGraph()->GetArch())) {
        return NO_ALIAS;
    }
    // Instructions with a primitive type and the reference type cannot alias each other.
    if ((IsTypeNumeric(mem1->GetType()) && mem2->IsReferenceOrAny()) ||
        (IsTypeNumeric(mem2->GetType()) && mem1->IsReferenceOrAny())) {
        return NO_ALIAS;
    }

    return CheckMemAddress<REFINED>(p1, p2, IsVolatileMemInst(mem1) || IsVolatileMemInst(mem2));
}

template AliasType AliasAnalysis::CheckInstAlias<false>(Inst *, Inst *) const;
template AliasType AliasAnalysis::CheckInstAlias<true>(Inst *, Inst *) const;

AliasType AliasAnalysis::CheckRefAlias(Inst *ref1, Inst *ref2) const
{
    ASSERT(ref1->IsReferenceOrAny());
    ASSERT(ref2->IsReferenceOrAny());
    return CheckMemAddress<false>(Pointer::CreateObject(ref1), Pointer::CreateObject(ref2), false);
}

AliasType AliasAnalysis::CheckMemAddressEmptyIntersectionCase(const PointerWithInfo &base1,
                                                              const PointerWithInfo &base2, const Pointer &p1,
                                                              const Pointer &p2) const
{
    // If at least one set of aliases consists of only local aliases then there is NO_ALIAS
    if (base1.IsLocal() || base2.IsLocal()) {
        return NO_ALIAS;
    }
    // If BOTH sets of aliases consists of only local created aliases then there is NO_ALIAS
    if (base1.IsLocalCreated() && base2.IsLocalCreated()) {
        return NO_ALIAS;
    }
    // Different fields cannot alias each other even if they are not created locally
    if (p1.GetType() == OBJECT_FIELD && !p1.HasSameOffsetDroppedIdx(p2)) {
        return NO_ALIAS;
    }
    if (p1.GetType() == ARRAY_ELEMENT) {
        auto equal = IsSameOffsets(p1.GetIdx(), p2.GetIdx());
        // If it is known that indices are different OR Imm indices are different then there is
        // no alias.  If they are both different we can't certainly say so.
        if ((equal == Trilean::FALSE && p1.GetImm() == p2.GetImm()) ||
            (equal == Trilean::TRUE && p1.GetImm() != p2.GetImm())) {
            return NO_ALIAS;
        }
    }
    if (p1.GetType() == DICTIONARY_ELEMENT) {
        auto equal = IsSameOffsets(p1.GetIdx(), p2.GetIdx());
        if (equal == Trilean::FALSE && p1.GetImm() == p2.GetImm()) {
            return NO_ALIAS;
        }
    }
    return MAY_ALIAS;
}

/**
 * We have 5 types of pointers: OBJECT, OBJECT_FIELD, POOL_CONSTANT,
 * STATIC_FIELD and ARRAY_ELEMENT.  They correspond to groups of memory storing
 * and loading instructions.  Assuming that these groups cannot load the same
 * memory address (at IR level), it is true that different types of pointers
 * cannot alias each other.
 *
 * Global pointers such as STATIC_FIELD and POOL_CONSTANT are checked through
 * their unique TypeId.
 *
 * For OBJECT_FIELDs we look at objects they referenced to.  If they refer to
 * the same objects then depending on TypeId they MUST_ALIAS or NO_ALIAS.  If
 * the situation of the referenced objects is unclear then they MAY_ALIAS.
 *
 * The same is for ARRAY_ELEMENT.  Instead of TypeId we look at index and compare them.
 *
 * All function's arguments MAY_ALIAS each other.  Created objects in the
 * function may not alias arguments.
 */
template <bool REFINED>
AliasType AliasAnalysis::CheckMemAddress(const Pointer &p1, const Pointer &p2, bool isVolatile) const
{
    ASSERT(p1.GetType() != UNKNOWN_OFFSET && p2.GetType() != UNKNOWN_OFFSET);
    if (p1.GetType() != p2.GetType()) {
        return NO_ALIAS;
    }

    if (p1.GetType() == STATIC_FIELD || p1.GetType() == POOL_CONSTANT) {
        if (p1.HasSameOffsetDroppedIdx(p2)) {
            return MUST_ALIAS;
        }
        return NO_ALIAS;
    }
    ASSERT(p1.GetBase() != nullptr && p2.GetBase() != nullptr);

    if (auto base = p1.GetBase(); base == p2.GetBase()) {
        return SingleIntersectionAliasing(p1, p2, nullptr, isVolatile);
    }

    auto baseObj1 = Pointer::CreateObject(p1.GetBase());
    auto baseObj2 = Pointer::CreateObject(p2.GetBase());
    ASSERT_DO(pointerInfo_.find(baseObj1) != pointerInfo_.end(),
              (std::cerr << "Undefined inst in AliasAnalysis: ", p1.GetBase()->Dump(&std::cerr)));
    ASSERT_DO(pointerInfo_.find(baseObj2) != pointerInfo_.end(),
              (std::cerr << "Undefined inst in AliasAnalysis: ", p2.GetBase()->Dump(&std::cerr)));
    const auto &base1 = GetPointerInfo(baseObj1);
    const auto &base2 = GetPointerInfo(baseObj2);
    const auto &aliases1 = base1.PointsTo();
    const auto &aliases2 = base2.PointsTo();
    // Find the intersection
    const Pointer *intersection = nullptr;
    for (auto &alias : aliases1) {
        if (aliases2.find(alias) != aliases2.end()) {
            intersection = &alias;
            break;
        }
    }

    // The only common intersection
    if (intersection != nullptr && aliases1.size() == 1 && aliases2.size() == 1) {
        return SingleIntersectionAliasing<REFINED>(p1, p2, &base1, isVolatile);
    }
    // Empty intersection: check that both addresses are not parameters
    if (intersection == nullptr) {
        return CheckMemAddressEmptyIntersectionCase(base1, base2, p1, p2);
    }
    return MAY_ALIAS;
}

static uint64_t CombineIdxAndImm(const Pointer *p)
{
    uint64_t sum = 0;
    if (p->GetIdx() != nullptr) {
        auto idx = p->GetIdx();
        ASSERT(idx->IsConst());
        ASSERT(!DataType::IsFloatType(idx->GetType()));
        sum = idx->CastToConstant()->GetRawValue();
    }
    sum += p->GetImm();
    return sum;
}

AliasType AliasAnalysis::AliasingTwoArrayPointers(const Pointer *p1, const Pointer *p2)
{
    // Structure of Pointer: base, idx, imm
    // Base may be same or not. We should compare fields in order: idx, combine {idx + imm}.
    // Is necessary compare sum, because LoadArrayI (..., nullptr, 2) and StoreArray(..., Const{2}, 0) will alias,
    // but in separate idx and imm are different.

    // 1) Compare idx. If they same, compare Imm part -> ALIAS_IF_BASE_EQUALS, NO_ALIAS
    if (AliasAnalysis::IsSameOffsets(p1->GetIdx(), p2->GetIdx()) == Trilean::TRUE) {
        return p1->GetImm() == p2->GetImm() ? ALIAS_IF_BASE_EQUALS : NO_ALIAS;
    }
    // 2) If still one of indexes is not constant or nullptr -> MAY_ALIAS
    for (auto *pointer : {p1, p2}) {
        if (pointer->GetIdx() != nullptr && !pointer->GetIdx()->IsConst()) {
            return MAY_ALIAS;
        }
    }
    // 3) Combine idx(is constant) and imm for compare
    if (CombineIdxAndImm(p1) == CombineIdxAndImm(p2)) {
        return ALIAS_IF_BASE_EQUALS;
    }
    return NO_ALIAS;
}

/// Checks aliasing if P1 and P2 point to the one single object.
/* static */
template <bool REFINED>
AliasType AliasAnalysis::SingleIntersectionAliasing(const Pointer &p1, const Pointer &p2,
                                                    const PointerWithInfo *commonBase, bool isVolatile) const
{
    // check that PointerInfo for intersection exists
    ASSERT(commonBase == nullptr || commonBase->PointsTo().size() == 1U);
    ASSERT(p1.GetType() == p2.GetType());
    switch (p1.GetType()) {
        case ARRAY_ELEMENT: {
            if (auto type = AliasingTwoArrayPointers(&p1, &p2); type != ALIAS_IF_BASE_EQUALS) {
                return type;
            }
            break;
        }
        case DICTIONARY_ELEMENT:
            // It is dinamic case, there are less guarantees
            if (IsSameOffsets(p1.GetIdx(), p2.GetIdx()) != Trilean::TRUE) {
                return MAY_ALIAS;
            }
            if (p1.GetImm() != p2.GetImm()) {
                return MAY_ALIAS;
            }
            break;
        case OBJECT_FIELD:
            if (!p1.HasSameOffsetDroppedIdx(p2)) {
                return NO_ALIAS;
            }
            break;
        case OBJECT:
            break;
        default:
            UNREACHABLE();
    }
    if (commonBase != nullptr && commonBase->IsLocal()) {
        // if base is local, volatility does not matter
        return MUST_ALIAS;
    }
    if (isVolatile) {
        return MAY_ALIAS;
    }
    if (commonBase == nullptr) {
        // p1 and p2 have the same base instruction
        return MUST_ALIAS;
    }
    ASSERT(commonBase->GetType() == OBJECT);
    if (commonBase->IsVolatile()) {
        // base of p1 and base of p2 can be different if they are loaded from some object
        // at different moments
        return MAY_ALIAS;
    }

    auto basePointsTo = *commonBase->PointsTo().begin();
    if (basePointsTo.GetType() == OBJECT || basePointsTo.GetType() == POOL_CONSTANT) {
        // p1 and p2 have the same base instruction
        return MUST_ALIAS;
    }
    if (REFINED) {
        return ALIAS_IF_BASE_EQUALS;
    }
    return MAY_ALIAS;
}

void AliasAnalysis::SolveConstraintsMainLoop(const PointerWithInfo *ref, PointerWithInfo *edge, bool &added)
{
    added |= edge->UpdateLocal(ref->IsLocal());
    if (ref->IsVolatile() && !edge->IsVolatile()) {
        added = true;
        edge->SetVolatile(true);
    }
    if (ref->GetBase() != edge->GetBase() || edge->GetType() == STATIC_FIELD || edge->GetType() == OBJECT) {
        for (const auto &alias : ref->PointsTo()) {
            ASSERT(alias.GetBase() == nullptr || alias.GetBase()->GetOpcode() != Opcode::NullCheck);
            added |= edge->Insert(alias);
        }
        return;
    }

    ASSERT(ref->GetBase() == edge->GetBase());
    ASSERT(edge->GetType() == OBJECT_FIELD || edge->GetType() == ARRAY_ELEMENT ||
           edge->GetType() == DICTIONARY_ELEMENT || edge->GetType() == RAW_OFFSET || edge->GetType() == UNKNOWN_OFFSET);

    bool onlyObjects = true;
    for (auto alias : ref->PointsTo()) {
        ASSERT(alias.GetBase() == nullptr || alias.GetBase()->GetOpcode() != Opcode::NullCheck);
        if (alias.GetType() != OBJECT) {
            onlyObjects = false;
            continue;
        }

        // NOTE: do we need it if onlyObjects is false?
        Pointer p {};
        if (edge->GetType() == OBJECT_FIELD) {
            // Propagating from object to fields: A{a} -> A.F{a.f}
            p = Pointer::CreateObjectField(alias.GetBase(), edge->GetImm(), edge->GetTypePtr());
        } else if (edge->GetType() == ARRAY_ELEMENT) {
            // Propagating from object to elements: A{a} -> A[i]{a[i]}
            p = Pointer::CreateArrayElement(alias.GetBase(), edge->GetIdx(), edge->GetImm());
        } else if (edge->GetType() == RAW_OFFSET) {
            // Propagating from object to elements: A{a} -> A[i]{a[i]}
            p = Pointer::CreateRawOffset(alias.GetBase(), edge->GetIdx(), edge->GetImm());
        } else if (edge->GetType() == DICTIONARY_ELEMENT) {
            // Propagating from object to elements: A{a} -> A[i]{a[i]}
            p = Pointer::CreateDictionaryElement(alias.GetBase(), edge->GetIdx());
        } else if (edge->GetType() == UNKNOWN_OFFSET) {
            p = Pointer::CreateUnknownOffset(alias.GetBase());
        } else {
            UNREACHABLE();
        }
        added |= edge->Insert(p);
    }

    if (!onlyObjects) {
        // In case A{a[j]} -> A[i] we propagate symbolic name: A{a[j]} -> A[i]{A[i]}
        added |= edge->Insert(edge->GetPointer());
    }
}

void AliasAnalysis::CreatePointerInfo(const Pointer &pointer, bool isVolatile)
{
    auto it = pointerInfo_.find(pointer);
    if (it == pointerInfo_.end()) {
        pointerInfo_.emplace(pointer, PointerInfo {Pointer::Set {GetGraph()->GetAllocator()->Adapter()},
                                                   pointer.IsNotEscapingAlias(), isVolatile});
    } else if (isVolatile) {
        it->second.isVolatile = true;
    }
}

const PointerWithInfo &AliasAnalysis::GetPointerInfo(const Pointer &pointer) const
{
    auto it = pointerInfo_.find(pointer);
    ASSERT_DO(it != pointerInfo_.end(),
              (std::cerr << "No info for pointer: ", pointer.Dump(&std::cerr), GetGraph()->Dump(&std::cerr)));
    return PointerWithInfo::FromPair(*it);
}

PointerWithInfo &AliasAnalysis::GetPointerInfo(const Pointer &pointer)
{
    return const_cast<PointerWithInfo &>(static_cast<const AliasAnalysis *>(this)->GetPointerInfo(pointer));
}

void AliasAnalysis::SetVolatile(const Pointer &pointer, Inst *memInst)
{
    ASSERT(memInst->IsMemory());
    if (IsVolatileMemInst(memInst)) {
        CreatePointerInfo(pointer, true);
        ASSERT(GetPointerInfo(pointer).IsVolatile());
    }
}

void AliasAnalysis::AddConstantDirectEdge(Inst *inst, uint32_t id)
{
    direct_->push_back({Pointer::CreateObject(inst), Pointer::CreatePoolConstant(id)});
}

void AliasAnalysis::FindEscapingPointers()
{
    ArenaVector<const PointerWithInfo *> worklist(GetGraph()->GetLocalAllocator()->Adapter());
    // propagate "escaping" property backward by copy-edges
    for (const auto &pair : pointerInfo_) {
        const auto *info = &PointerWithInfo::FromPair(pair);
        if (info->IsLocal()) {
            continue;
        }
        worklist.push_back(info);
        while (!worklist.empty()) {
            auto to = worklist.back();
            worklist.pop_back();
            if (auto it = reverseChains_->find(to->GetPointer()); it != reverseChains_->end()) {
                PropagateEscaped(it->second, worklist);
            }
        }
    }
    // pointer is local alias if it is created locally and does not escape
    for (auto &pair : pointerInfo_) {
        auto &info = PointerWithInfo::FromPair(pair);
        info.UpdateLocal(info.GetPointer().IsLocalCreatedAlias());
    }
}

void AliasAnalysis::AddEscapeEdge(const Pointer &from, const Pointer &to)
{
    auto it = reverseChains_->try_emplace(from, GetGraph()->GetLocalAllocator()->Adapter());
    it.first->second.push_back(to);
}

void AliasAnalysis::PropagateEscaped(const ArenaVector<Pointer> &escaping,
                                     ArenaVector<const PointerWithInfo *> &worklist)
{
    for (const auto &from : escaping) {
        auto &fromInfo = GetPointerInfo(from);
        if (fromInfo.UpdateLocal(false)) {
            worklist.push_back(&fromInfo);
        }
    }
}

/**
 * Here we propagate solutions obtained from direct constraints through copy
 * constraints e.g: we have a node A with solution {a} and the node A was
 * copied to B and C (this->chains_ maintains these links), and C was copied to
 * D.
 *
 *    A{a} -> B
 *        \-> C -> D
 *
 * After first iteration (iterating A node) we will obtain
 *
 *     A{a} -> B{a}
 *         \-> C{a} -> D
 *
 * After second iteration (iterating B node) nothing changes
 *
 * After third iteration (iterating C node):
 *
 * A{a} -> B{a}
 *     \-> C{a} -> D{a}
 *
 * For complex nodes (OBJECT_FIELD, ARRAY_ELEMENT) we create auxiliary nodes e.g.
 * if a field F was accessed from object A then we have two nodes:
 *
 * A{a} -> A.F
 *
 * And solutions from A would be propagated as following:
 *
 * A{a} -> A.F{a.F}
 *
 * The function works using worklist to process only updated nodes.
 */
void AliasAnalysis::SolveConstraints()
{
    ArenaQueue<PointerWithInfo *> worklist(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto &pair : *direct_) {
        if (chains_->find(pair.first) != chains_->end()) {
            worklist.push(&GetPointerInfo(pair.first));
        }
    }

    while (!worklist.empty()) {
        const PointerWithInfo *ref = worklist.front();
        ASSERT(ref->GetBase() == nullptr || ref->GetBase()->GetOpcode() != Opcode::NullCheck);
        for (const auto &edgePointer : chains_->at(ref->GetPointer())) {
            auto *edge = &GetPointerInfo(edgePointer);
            // POOL_CONSTANT cannot be assignee
            ASSERT(edge->GetType() != POOL_CONSTANT);
            bool added = false;

            SolveConstraintsMainLoop(ref, edge, added);

            if (added && chains_->find(edgePointer) != chains_->end()) {
                worklist.push(edge);
            }
            ASSERT(!edge->PointsTo().empty());
        }
        worklist.pop();
    }
}

/**
 * Compares offsets. Since we have a situation when we cannot determine either equal offsets or not,
 * we use three valued logic. It is handy when we want to know whether offsets are definitely
 * different or definitely equal.
 */
/* static */
AliasAnalysis::Trilean AliasAnalysis::IsSameOffsets(const Inst *off1, const Inst *off2)
{
    if (off1 == off2) {
        return Trilean::TRUE;
    }
    if (off1 == nullptr || off2 == nullptr) {
        return Trilean::FALSE;
    }

    if (off1->GetVN() != INVALID_VN && off2->GetVN() != INVALID_VN && off1->GetVN() == off2->GetVN()) {
        return Trilean::TRUE;
    }

    if (off1->IsConst() && off2->IsConst() &&
        off1->CastToConstant()->GetRawValue() != off2->CastToConstant()->GetRawValue()) {
        return Trilean::FALSE;
    }

    return Trilean::UNKNOWN;
}

}  // namespace ark::compiler
