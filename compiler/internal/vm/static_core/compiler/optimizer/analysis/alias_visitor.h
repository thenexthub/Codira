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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_ALIAS_VISITOR_H
#define COMPILER_OPTIMIZER_ANALYSIS_ALIAS_VISITOR_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {

enum PointerType {
    // Reference to unknown object.
    // Valid fields: base
    OBJECT,
    // Constant from pool
    // Valid fields: imm
    POOL_CONSTANT,
    // Object's field
    // Valid fields: base, imm, type_ptr
    OBJECT_FIELD,
    // Static field of the object
    // Valid fields: imm, type_ptr
    STATIC_FIELD,
    // Array pointer
    // Valid fields
    // * in common: base, idx
    // * in case LoadArrayPair/StoreArrayPair: base, idx, imm
    // * in case LoadArrayPairI/StoreArrayPairI, LoadArrayI/StoreArrayI: base, imm
    ARRAY_ELEMENT,
    // Dictionary pointer
    // Valid fields: base, idx
    DICTIONARY_ELEMENT,
    // Valid fields: base, idx, imm
    RAW_OFFSET,
    // Can be any other type of offset, so can alias with all of them.
    // Valid fields: base
    UNKNOWN_OFFSET
};

class PointerOffset {
public:
    PointerOffset(PointerType type, uint64_t imm, const void *typePtr) : type_(type), imm_(imm), typePtr_(typePtr) {};
    PointerOffset() = default;

    // Used for fields with offset not known in compile-time
    static PointerOffset CreateUnknownOffset()
    {
        return {UNKNOWN_OFFSET, 0, nullptr};
    }

    // Used for default fields in TypePropagation
    static PointerOffset CreateDefaultField()
    {
        return {UNKNOWN_OFFSET, 1, nullptr};
    }

    PointerType GetType() const
    {
        return type_;
    }

    uint64_t GetImm() const
    {
        return imm_;
    }

    const void *GetTypePtr() const
    {
        return typePtr_;
    }

    bool IsObject() const
    {
        return type_ == OBJECT;
    }

    struct Hash {
        uint32_t operator()(PointerOffset const &p) const
        {
            if (p.GetTypePtr() == nullptr) {
                return std::hash<uint64_t> {}(p.GetImm()) ^ std::hash<PointerType> {}(p.type_);
            }
            return std::hash<const void *> {}(p.GetTypePtr());
        }
    };

    bool operator==(const PointerOffset &p) const
    {
        return GetType() == p.GetType() && HasSameOffset(p);
    }

    bool HasSameOffset(const PointerOffset &p) const
    {
        if (typePtr_ == nullptr && p.typePtr_ == nullptr) {
            return imm_ == p.imm_;
        }
        return typePtr_ == p.typePtr_;
    }

    void Dump(std::ostream *out) const;

    template <class T>
    using Map = ArenaUnorderedMap<PointerOffset, T, Hash>;

private:
    PointerType type_ = OBJECT;
    uint64_t imm_ = 0;
    const void *typePtr_ {nullptr};
};

std::ostream &operator<<(std::ostream &out, const PointerOffset &p);

class Pointer {
private:
    Pointer(PointerType type, const Inst *base, const Inst *idx, uint64_t imm, const void *typePtr)
        : base_(base), idx_(idx), off_(type, imm, typePtr) {};

public:
    Pointer(const Inst *base, const PointerOffset &offset)
        : Pointer(offset.GetType(), base, nullptr, offset.GetImm(), offset.GetTypePtr())
    {
    }
    Pointer() = default;

    static Pointer CreateObject(const Inst *base)
    {
        ASSERT(base != nullptr);
        ASSERT(base->IsReferenceOrAny() || base->GetBasicBlock()->GetGraph()->IsDynamicMethod() || base->IsConst() ||
               base->GetType() == DataType::POINTER);
        return Pointer(OBJECT, base, nullptr, 0, nullptr);
    }

    static Pointer CreatePoolConstant(uint32_t typeId)
    {
        return Pointer(POOL_CONSTANT, nullptr, nullptr, typeId, nullptr);
    }

    static Pointer CreateStaticField(uint32_t typeId, const void *typePtr = nullptr)
    {
        return Pointer(STATIC_FIELD, nullptr, nullptr, typeId, typePtr);
    }

    static Pointer CreateObjectField(const Inst *base, uint32_t typeId, const void *typePtr = nullptr)
    {
        ASSERT(base != nullptr);
        return Pointer(OBJECT_FIELD, base, nullptr, typeId, typePtr);
    }

    static Pointer CreateArrayElement(const Inst *array, const Inst *idx, uint64_t imm = 0)
    {
        ASSERT(array != nullptr);
        return Pointer(ARRAY_ELEMENT, array, idx, imm, nullptr);
    }

    static Pointer CreateRawOffset(const Inst *obj, const Inst *idx, uint64_t imm = 0)
    {
        ASSERT(obj != nullptr);
        return Pointer(RAW_OFFSET, obj, idx, imm, nullptr);
    }

    static Pointer CreateDictionaryElement(const Inst *array, const Inst *idx, uint64_t imm = 0)
    {
        ASSERT(array != nullptr);
        return Pointer(DICTIONARY_ELEMENT, array, idx, imm, nullptr);
    }

    static Pointer CreateUnknownOffset(const Inst *obj)
    {
        return Pointer(obj, PointerOffset::CreateUnknownOffset());
    }

    const Inst *GetBase() const
    {
        return base_;
    }

    const Inst *GetIdx() const
    {
        return idx_;
    }

    PointerType GetType() const
    {
        return off_.GetType();
    }

    uint64_t GetImm() const
    {
        return off_.GetImm();
    }

    const void *GetTypePtr() const
    {
        return off_.GetTypePtr();
    }

    bool IsNotEscapingAlias() const;

    /// Returns true if reference is created in the current function
    bool IsLocalCreatedAlias() const;

    bool IsObject() const
    {
        return GetType() == OBJECT;
    }

    struct Hash {
        uint32_t operator()(Pointer const &p) const
        {
            auto instHasher = std::hash<const Inst *> {};
            uint32_t hash = instHasher(p.GetBase());
            hash += instHasher(p.GetIdx());
            if (p.GetTypePtr() == nullptr) {
                hash += std::hash<uint64_t> {}(p.GetImm());
            } else {
                hash += std::hash<const void *> {}(p.GetTypePtr());
            }
            return hash;
        }
    };

    bool operator==(const Pointer &p) const
    {
        auto ret = GetOffset() == p.GetOffset() && GetBase() == p.GetBase() && GetIdx() == p.GetIdx();
        return ret;
    }

    bool HasSameOffsetDroppedIdx(const Pointer &p) const
    {
        return GetOffsetDroppedIdx().HasSameOffset(p.GetOffsetDroppedIdx());
    }

    PointerOffset GetOffsetDroppedIdx() const
    {
        if (idx_ == nullptr) {
            return GetOffset();
        }
        ASSERT(GetTypePtr() == nullptr);
        if (!idx_->IsConst() || GetType() == DICTIONARY_ELEMENT) {
            return PointerOffset::CreateUnknownOffset();
        }
        ASSERT(GetType() == ARRAY_ELEMENT || GetType() == RAW_OFFSET);
        auto newImm = GetImm() + idx_->CastToConstant()->GetIntValue();
        return PointerOffset(GetType(), newImm, GetTypePtr());
    }

    std::pair<const Inst *, PointerOffset> DropIdx() const
    {
        return {GetBase(), GetOffsetDroppedIdx()};
    }

    void Dump(std::ostream *out) const;

    using Set = ArenaUnorderedSet<Pointer, Hash>;
    template <class T>
    using Map = ArenaUnorderedMap<Pointer, T, Hash>;

private:
    const PointerOffset &GetOffset() const
    {
        return off_;
    }

    /**
     * Returns true if a reference escapes the scope of current function:
     * Various function calls, constructors and stores to another objects' fields, arrays
     */
    static bool IsEscapingAlias(const Inst *inst);

    const Inst *base_ {nullptr};
    const Inst *idx_ {nullptr};
    PointerOffset off_;
};

std::ostream &operator<<(std::ostream &out, const Pointer &p);

class AliasVisitor : public GraphVisitor {
public:
    void Init(ArenaAllocator *allocator);

    ArenaSet<Inst *> *GetClearInputsSet()
    {
        ASSERT(inputsSet_ != nullptr);
        inputsSet_->clear();
        return inputsSet_;
    }

    static bool ParseInstruction(Inst *inst, Pointer *pointer);

    /**
     * Sort IR instructions into two constraint groups:
     *     Direct: introduce the alias
     *     Copy: copy one alias to another
     */

    /// Instructions that definitely are not an alias of anything.
    static void VisitNullPtr(GraphVisitor *v, Inst *inst);
    static void VisitLoadUniqueObject(GraphVisitor *v, Inst *inst);
    static void VisitLoadUndefined(GraphVisitor *v, Inst *inst);
    static void VisitInitObject(GraphVisitor *v, Inst *inst);
    static void VisitNewObject(GraphVisitor *v, Inst *inst);
    static void VisitNewArray(GraphVisitor *v, Inst *inst);
    static void VisitMultiArray(GraphVisitor *v, Inst *inst);
    static void VisitInitEmptyString(GraphVisitor *v, Inst *inst);
    static void VisitInitString(GraphVisitor *v, Inst *inst);
    static void VisitStringFlatCheck(GraphVisitor *v, Inst *inst);
    /**
     * Instructions that can introduce references that are an alias of
     * something already existed.
     */
    static void VisitConstant(GraphVisitor *v, Inst *inst);
    static void VisitParameter(GraphVisitor *v, Inst *inst);
    static void VisitLiveIn(GraphVisitor *v, Inst *inst);
    static void VisitLoadImmediate(GraphVisitor *v, Inst *inst);
    static void VisitBitcast(GraphVisitor *v, Inst *inst);
    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);
    static void VisitBuiltin(GraphVisitor *v, Inst *inst);
    static void VisitCallStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallResolvedStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallVirtual(GraphVisitor *v, Inst *inst);
    static void VisitCallResolvedVirtual(GraphVisitor *v, Inst *inst);
    static void VisitCallDynamic(GraphVisitor *v, Inst *inst);
    static void VisitCallNative(GraphVisitor *v, Inst *inst);
    static void VisitCall(GraphVisitor *v, Inst *inst);
    static void VisitGetManagedClassObject(GraphVisitor *v, Inst *inst);
    static void VisitResolveObjectFieldStatic(GraphVisitor *v, Inst *inst);

    /// Instructions that introduce static fields (global variables).
    static void VisitLoadStatic(GraphVisitor *v, Inst *inst);
    static void VisitLoadResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitStoreStatic(GraphVisitor *v, Inst *inst);
    static void VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst);

    /// Instructions that introduce unique constant references (global constants).
    static void VisitLoadRuntimeClass(GraphVisitor *v, Inst *inst);
    static void VisitLoadClass(GraphVisitor *v, Inst *inst);
    static void VisitLoadAndInitClass(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadAndInitClass(GraphVisitor *v, Inst *inst);
    static void VisitGetGlobalVarAddress(GraphVisitor *v, Inst *inst);
    static void VisitLoadString(GraphVisitor *v, Inst *inst);
    static void VisitLoadConstArray(GraphVisitor *v, Inst *inst);
    static void VisitLoadType(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedLoadType(GraphVisitor *v, Inst *inst);
    static void VisitLoadObjFromConst(GraphVisitor *v, Inst *inst);

    /// Instructions that introduce aliases.
    static void VisitLoadArray(GraphVisitor *v, Inst *inst);
    static void VisitStoreArray(GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayI(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayI(GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayPair(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayPair(GraphVisitor *v, Inst *inst);
    static void VisitLoadArrayPairI(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayPairI(GraphVisitor *v, Inst *inst);
    static void VisitLoadObject(GraphVisitor *v, Inst *inst);
    static void VisitLoadResolvedObjectField(GraphVisitor *v, Inst *inst);
    static void VisitStoreObject(GraphVisitor *v, Inst *inst);
    static void VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst);
    static void VisitLoad(GraphVisitor *v, Inst *inst);
    static void VisitLoadNative(GraphVisitor *v, Inst *inst);
    static void VisitStore(GraphVisitor *v, Inst *inst);
    static void VisitStoreNative(GraphVisitor *v, Inst *inst);
    static void VisitLoadI(GraphVisitor *v, Inst *inst);
    static void VisitStoreI(GraphVisitor *v, Inst *inst);
    static void VisitCatchPhi(GraphVisitor *v, Inst *inst);
    static void VisitPhi(GraphVisitor *v, Inst *inst);
    static void VisitSelect(GraphVisitor *v, Inst *inst);
    static void VisitSelectImm(GraphVisitor *v, Inst *inst);
    static void VisitMov(GraphVisitor *v, Inst *inst);
    static void VisitRefTypeCheck(GraphVisitor *v, Inst *inst);
    static void VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst);
    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst);
    static void VisitGetAnyTypeName(GraphVisitor *v, Inst *inst);
    static void VisitLoadConstantPool(GraphVisitor *v, Inst *inst);
    static void VisitLoadLexicalEnv(GraphVisitor *v, Inst *inst);
    static void VisitLoadObjectPair(GraphVisitor *v, Inst *inst);
    static void VisitStoreObjectPair(GraphVisitor *v, Inst *inst);

    /// Dynamic instructions
    static void VisitLoadObjectDynamic(GraphVisitor *v, Inst *inst);
    static void VisitStoreObjectDynamic(GraphVisitor *v, Inst *inst);
    static void VisitLoadFromConstantPool(GraphVisitor *v, Inst *inst);

    void VisitDefault([[maybe_unused]] Inst *inst) override;

protected:
    virtual void AddDirectEdge(const Pointer &p) = 0;
    virtual void AddConstantDirectEdge(Inst *inst, uint32_t id) = 0;
    virtual void AddCopyEdge(const Pointer &from, const Pointer &to) = 0;
    virtual void AddPseudoCopyEdge(const Pointer &base, const Pointer &field) = 0;
    virtual void SetVolatile([[maybe_unused]] const Pointer &pointer, [[maybe_unused]] Inst *memInst) {}
    virtual void VisitHeapInv([[maybe_unused]] Inst *inst) {}
    virtual void Escape([[maybe_unused]] const Inst *inst) {}
    virtual void VisitAllocation([[maybe_unused]] Inst *inst) = 0;

#include "optimizer/ir/visitor.inc"

private:
    void EscapeInputsAndInv(Inst *inst);

private:
    ArenaSet<Inst *> *inputsSet_ {nullptr};
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_ALIAS_VISITOR_H
