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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H
#define COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H

#include "alias_visitor.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/pass.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {
class BasicBlock;
class Graph;

enum AliasType : uint8_t {
    // Proved that references are not aliases
    NO_ALIAS,
    // References may or may not alias each other (cannot be proven statically)
    MAY_ALIAS,
    // References are proven aliases
    MUST_ALIAS,
    // Variation of MAY_ALIAS
    ALIAS_IF_BASE_EQUALS,
};

class PointerWithInfo;

struct PointerInfo {
    Pointer::Set pointsTo;
    bool local;
    bool isVolatile;
};

class PointerWithInfo : private std::pair<const Pointer, PointerInfo> {
private:
    using Base = std::pair<const Pointer, PointerInfo>;
    void SetLocal(bool isLocal)
    {
        second.local = isLocal;
    }

public:
    // PointerWithInfo(const std::pair<const Pointer, PointerInfo> &pair) {}
    PointerWithInfo() = delete;
    NO_COPY_SEMANTIC(PointerWithInfo);
    NO_MOVE_SEMANTIC(PointerWithInfo);
    ~PointerWithInfo() = delete;

    static PointerWithInfo &FromPair(Base &base)
    {
        return static_cast<PointerWithInfo &>(base);
    }

    static const PointerWithInfo &FromPair(const Base &base)
    {
        return static_cast<const PointerWithInfo &>(base);
    }

    const Pointer &GetPointer() const
    {
        return first;
    }

    PointerType GetType() const
    {
        return GetPointer().GetType();
    }

    const Inst *GetBase() const
    {
        return GetPointer().GetBase();
    }

    const Inst *GetIdx() const
    {
        return GetPointer().GetIdx();
    }

    uint64_t GetImm() const
    {
        return GetPointer().GetImm();
    }

    const void *GetTypePtr() const
    {
        return GetPointer().GetTypePtr();
    }

    // true if all aliases are created locally and don't escape
    bool IsLocal() const
    {
        return second.local;
    }

    // true if all aliases are created locally
    bool IsLocalCreated() const;

    bool IsVolatile() const
    {
        return second.isVolatile;
    }

    const Pointer::Set &PointsTo() const
    {
        return second.pointsTo;
    }

    void SetVolatile(bool isVolatile)
    {
        second.isVolatile = isVolatile;
    }

    bool UpdateLocal(bool sourceIsLocal)
    {
        if (IsLocal() && !sourceIsLocal) {
            SetLocal(false);
            return true;
        }
        return false;
    }

    bool Insert(Pointer p)
    {
        return second.pointsTo.insert(p).second;
    }
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class AliasAnalysis : public Analysis, public AliasVisitor {
public:
    enum class Trilean {
        TRUE,
        UNKNOWN,
        FALSE,
    };

    using PointerPairVector = ArenaVector<std::pair<Pointer, Pointer>>;

    explicit AliasAnalysis(Graph *graph);
    NO_MOVE_SEMANTIC(AliasAnalysis);
    NO_COPY_SEMANTIC(AliasAnalysis);
    ~AliasAnalysis() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "AliasAnalysis";
    }

    template <bool REFINED = false>
    AliasType CheckInstAlias(Inst *mem1, Inst *mem2) const;
    AliasType CheckRefAlias(Inst *ref1, Inst *ref2) const;
    void Dump(std::ostream *out) const;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override;

    const PointerWithInfo &GetPointerInfo(const Pointer &pointer) const;
    PointerWithInfo &GetPointerInfo(const Pointer &pointer);

protected:
    void AddDirectEdge(const Pointer &p) override
    {
        direct_->push_back({p, p});
    }

    void VisitAllocation(Inst *inst) override
    {
        AddDirectEdge(Pointer::CreateObject(inst));
    }

    void AddConstantDirectEdge(Inst *inst, uint32_t id) override;

    void AddCopyEdge(const Pointer &from, const Pointer &to) override
    {
        copy_->push_back({from, to});
        AddEscapeEdge(to, from);
    }

    void AddPseudoCopyEdge(const Pointer &base, const Pointer &field) override
    {
        copy_->push_back({base, field});
        AddEscapeEdge(base, field);
        AddEscapeEdge(field, base);
    }

    void SetVolatile(const Pointer &pointer, Inst *memInst) override;

private:
    void Init();
    void CreatePointerInfo(const Pointer &pointer, bool isVolatile = false);

    void FindEscapingPointers();
    void AddEscapeEdge(const Pointer &from, const Pointer &to);
    void PropagateEscaped(const ArenaVector<Pointer> &escaping, ArenaVector<const PointerWithInfo *> &worklist);
    void SolveConstraints();

    static Trilean IsSameOffsets(const Inst *off1, const Inst *off2);
    static AliasType AliasingTwoArrayPointers(const Pointer *p1, const Pointer *p2);

    template <bool REFINED = false>
    AliasType CheckMemAddress(const Pointer &p1, const Pointer &p2, bool isVolatile) const;
    template <bool REFINED = false>
    AliasType SingleIntersectionAliasing(const Pointer &p1, const Pointer &p2, const PointerWithInfo *commonBase,
                                         bool isVolatile) const;
    AliasType CheckMemAddressEmptyIntersectionCase(const PointerWithInfo &base1, const PointerWithInfo &base2,
                                                   const Pointer &p1, const Pointer &p2) const;
    void SolveConstraintsMainLoop(const PointerWithInfo *ref, PointerWithInfo *edge, bool &added);
    void DumpChains(std::ostream *out) const;
    void DumpDirect(std::ostream *out) const;
    void DumpCopy(std::ostream *out) const;

private:
    ArenaUnorderedMap<Pointer, PointerInfo, Pointer::Hash> pointerInfo_;

    // Local containers:
    Pointer::Map<ArenaVector<Pointer>> *chains_ {nullptr};
    Pointer::Map<ArenaVector<Pointer>> *reverseChains_ {nullptr};
    PointerPairVector *direct_ {nullptr};
    PointerPairVector *copy_ {nullptr};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_ALIAS_ANALYSIS_H
