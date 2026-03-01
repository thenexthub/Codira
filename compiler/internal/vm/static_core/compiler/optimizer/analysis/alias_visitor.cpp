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
#include "optimizer/analysis/alias_visitor.h"
#include "compiler_logger.h"

namespace ark::compiler {

void PointerOffset::Dump(std::ostream *out) const
{
    switch (type_) {
        case STATIC_FIELD:
            (*out) << "SF #" << imm_;
            break;
        case POOL_CONSTANT:
            (*out) << "PC #" << imm_;
            break;
        case OBJECT_FIELD:
            (*out) << "#" << imm_;
            break;
        case ARRAY_ELEMENT:
        case RAW_OFFSET:
            (*out) << "[" << imm_ << "]";
            break;
        case UNKNOWN_OFFSET:
            if (*this == CreateDefaultField()) {
                (*out) << "DEFAULT";
            } else if (*this == CreateUnknownOffset()) {
                (*out) << "UNKNOWN";
            } else {
                UNREACHABLE();
            }
            break;
        case DICTIONARY_ELEMENT:
        case OBJECT:
            UNREACHABLE();
        default:
            UNREACHABLE();
    }
}

std::ostream &operator<<(std::ostream &out, const PointerOffset &p)
{
    p.Dump(&out);
    return out;
}

void Pointer::Dump(std::ostream *out) const
{
    switch (GetType()) {
        case OBJECT:
            (*out) << "v" << base_->GetId();
            break;
        case STATIC_FIELD:
            (*out) << "SF #" << GetImm();
            break;
        case POOL_CONSTANT:
            (*out) << "PC #" << GetImm();
            break;
        case OBJECT_FIELD:
            (*out) << "v" << base_->GetId() << " #" << GetImm();
            break;
        case ARRAY_ELEMENT:
        case RAW_OFFSET:
            (*out) << "v" << base_->GetId() << "[";
            if (idx_ != nullptr) {
                (*out) << "v" << idx_->GetId();
                if (GetImm() != 0) {
                    (*out) << "+" << GetImm();
                }
            } else {
                (*out) << GetImm();
            }
            (*out) << "]";
            break;
        case DICTIONARY_ELEMENT:
            ASSERT(idx_ != nullptr);
            (*out) << "v" << base_->GetId() << "[";
            (*out) << "v" << idx_->GetId();
            if (GetImm() != 0) {
                (*out) << "#NAME";
            } else {
                (*out) << "#INDEX";
            }
            (*out) << "]";
            break;
        case UNKNOWN_OFFSET:
            (*out) << "v" << base_->GetId() << " #?";
            break;
        default:
            UNREACHABLE();
    }
}

bool Pointer::IsNotEscapingAlias() const
{
    return GetBase() != nullptr && !IsEscapingAlias(GetBase());
}

bool Pointer::IsLocalCreatedAlias() const
{
    if (base_ == nullptr || base_->GetType() == DataType::POINTER) {
        return false;
    }
    if (!base_->IsReferenceOrAny()) {
        ASSERT(base_->GetBasicBlock()->GetGraph()->IsDynamicMethod() || base_->IsConst());
        // primitive tagged value or null const
        return true;
    }
    switch (base_->GetOpcode()) {
        case Opcode::NullPtr:
            return true;
        case Opcode::NewArray:
        case Opcode::MultiArray:
        case Opcode::NewObject:
        case Opcode::InitObject:
        case Opcode::InitEmptyString:
        case Opcode::InitString:
            return true;
        case Opcode::NullCheck:
        case Opcode::ObjByIndexCheck:
            UNREACHABLE();
            /* fall-through */
        default:
            return false;
    }
}

std::ostream &operator<<(std::ostream &out, const Pointer &p)
{
    p.Dump(&out);
    return out;
}

/**
 * Returns true if a reference escapes the scope of current function:
 * Various function calls, constructors and stores to another objects' fields, arrays
 */
/* static */
// CC-OFFNXT(huge_method[C++], huge_cyclomatic_complexity[C++], G.FUN.01-CPP) big switch case
bool Pointer::IsEscapingAlias(const Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return false;
    }
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->GetType() == DataType::POINTER) {
            return true;
        }
        switch (user.GetInst()->GetOpcode()) {
            case Opcode::RefTypeCheck:
                if (user.GetIndex() == 0) {
                    break;
                }
                [[fallthrough]];
            case Opcode::NullCheck:
            case Opcode::ObjByIndexCheck:
            case Opcode::AnyTypeCheck:
            case Opcode::StringFlatCheck:
                if (IsEscapingAlias(userInst)) {
                    return true;
                }
                break;
            // Currently unhandled
            case Opcode::Store:
            case Opcode::StoreNative:
            case Opcode::StoreI:
                if (user.GetIndex() != 0) {
                    return true;
                }
                break;
            case Opcode::StoreObject:
            case Opcode::StoreObjectPair:
            case Opcode::StoreArray:
            case Opcode::StoreArrayI:
            case Opcode::StoreArrayPair:
            case Opcode::StoreArrayPairI:
            case Opcode::StoreObjectDynamic:
                // Propagate isLocal from base and aliased insts
                break;
            case Opcode::StoreStatic:
            case Opcode::StoreResolvedObjectField:
            case Opcode::StoreResolvedObjectFieldStatic:
            case Opcode::UnresolvedStoreStatic:
            case Opcode::InitObject:
            case Opcode::InitClass:
            case Opcode::LoadAndInitClass:
            case Opcode::UnresolvedLoadAndInitClass:
            case Opcode::GetGlobalVarAddress:
            case Opcode::CallStatic:
            case Opcode::CallResolvedStatic:
            case Opcode::CallVirtual:
            case Opcode::CallResolvedVirtual:
            case Opcode::CallDynamic:
            case Opcode::Call:
            case Opcode::Bitcast:
            case Opcode::Cast:
                return true;
            case Opcode::CallNative:
                if (inst->IsRuntimeCall()) {
                    return true;
                }
                break;
            case Opcode::Intrinsic:
                // if intrinsic has no side effects and has primitive type, it
                //  does not move ref inputs anywhere
                if (!inst->IsNotRemovable() && !inst->IsReferenceOrAny()) {
                    return true;
                }
                break;
            case Opcode::CheckCast:
            case Opcode::HclassCheck:
            case Opcode::IsInstance:
            case Opcode::Compare:
            case Opcode::CompareAnyType:
            case Opcode::DeoptimizeCompare:
            case Opcode::DeoptimizeCompareImm:
            case Opcode::Return:
            case Opcode::LiveOut:
            case Opcode::Throw:
            case Opcode::LenArray:
            case Opcode::InitString:
            case Opcode::IfImm:
            case Opcode::If:
            case Opcode::ResolveVirtual:
            case Opcode::Monitor:
            case Opcode::FillConstArray:
            case Opcode::GetInstanceClass:
            case Opcode::ResolveByName:
                break;
            case Opcode::Phi:
            case Opcode::CatchPhi:
            case Opcode::Select:
            case Opcode::SelectImm:
            case Opcode::SelectTransform:
            case Opcode::SelectImmTransform:
            case Opcode::CastValueToAnyType:
            case Opcode::CastAnyTypeValue:
                // Propagate isLocal from users
                break;
            case Opcode::NewArray:
            case Opcode::NewObject:
            case Opcode::MultiArray:
                ASSERT(user.GetIndex() == 0);  // class input
                break;
            default:
                ASSERT_DO(userInst->IsLoad() || userInst->IsSaveState(),
                          (std::cerr << "unknown user: " << *userInst << std::endl,
                           inst->GetBasicBlock()->GetGraph()->Dump(&std::cerr)));
        }
    }
    return false;
}

static Pointer GetDynamicAccessPointer(Inst *inst, Inst *base, DynObjectAccessType type, DynObjectAccessMode mode)
{
    if (type == DynObjectAccessType::UNKNOWN || mode == DynObjectAccessMode::UNKNOWN) {
        return Pointer::CreateUnknownOffset(base);
    }

    Inst *idx = inst->GetDataFlowInput(1);
    uint64_t imm = 0;
    if (type == DynObjectAccessType::BY_NAME) {
        ASSERT_DO(mode != DynObjectAccessMode::ARRAY,
                  (std::cerr << "Unexpected access type BY_NAME for access mode ARRAY: ", inst->Dump(&std::cerr)));
        imm = UINT64_MAX;
    } else {
        ASSERT_DO(type == DynObjectAccessType::BY_INDEX,
                  (std::cerr << "Unsupported dynamic access type in alias analysis: ", inst->Dump(&std::cerr)));
    }
    if (mode == DynObjectAccessMode::ARRAY) {
        return Pointer::CreateArrayElement(base, idx, imm);
    }
    ASSERT_DO(mode == DynObjectAccessMode::DICTIONARY,
              (std::cerr << "Unsupported dynamic access mode in alias analysis: ", inst->Dump(&std::cerr)));
    return Pointer::CreateDictionaryElement(base, idx, imm);
}

static Pointer ParseArrayElement(Inst *inst)
{
    uint32_t imm = 0;
    Inst *offset = nullptr;
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::StoreArray:
            offset = inst->GetDataFlowInput(1);
            break;
        case Opcode::LoadArrayI:
            imm = inst->CastToLoadArrayI()->GetImm();
            break;
        case Opcode::StoreArrayI:
            imm = inst->CastToStoreArrayI()->GetImm();
            break;
        default:
            UNREACHABLE();
    }
    auto base = inst->GetDataFlowInput(0);
    return Pointer::CreateArrayElement(base, offset, imm);
}

static Pointer ParsePoolConstant(Inst *inst)
{
    uint32_t typeId = 0;
    switch (inst->GetOpcode()) {
        case Opcode::LoadString:
            typeId = inst->CastToLoadString()->GetTypeId();
            break;
        case Opcode::LoadType:
            typeId = inst->CastToLoadType()->GetTypeId();
            break;
        case Opcode::UnresolvedLoadType:
            typeId = inst->CastToUnresolvedLoadType()->GetTypeId();
            break;
        default:
            UNREACHABLE();
    }
    return Pointer::CreatePoolConstant(typeId);
}

static Pointer ParseStaticField(Inst *inst)
{
    uint32_t typeId = 0;
    void *typePtr = nullptr;
    switch (inst->GetOpcode()) {
        case Opcode::LoadStatic:
            typeId = inst->CastToLoadStatic()->GetTypeId();
            typePtr = inst->CastToLoadStatic()->GetObjField();
            break;
        case Opcode::LoadResolvedObjectFieldStatic:
            typeId = inst->CastToLoadResolvedObjectFieldStatic()->GetTypeId();
            break;
        case Opcode::StoreStatic:
            typeId = inst->CastToStoreStatic()->GetTypeId();
            typePtr = inst->CastToStoreStatic()->GetObjField();
            break;
        case Opcode::UnresolvedStoreStatic:
            typeId = inst->CastToUnresolvedStoreStatic()->GetTypeId();
            break;
        case Opcode::StoreResolvedObjectFieldStatic:
            typeId = inst->CastToStoreResolvedObjectFieldStatic()->GetTypeId();
            break;
        default:
            UNREACHABLE();
    }
    return Pointer::CreateStaticField(typeId, typePtr);
}

static Pointer ParseObjectField(Inst *inst)
{
    uint32_t typeId = 0;
    void *typePtr = nullptr;
    bool isStatic = false;
    switch (inst->GetOpcode()) {
        case Opcode::LoadObject:
            isStatic = inst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_STATIC;
            typeId = inst->CastToLoadObject()->GetTypeId();
            typePtr = inst->CastToLoadObject()->GetObjField();
            break;
        case Opcode::LoadResolvedObjectField:
            typeId = inst->CastToLoadResolvedObjectField()->GetTypeId();
            break;
        case Opcode::StoreObject:
            isStatic = inst->CastToStoreObject()->GetObjectType() == ObjectType::MEM_STATIC;
            typeId = inst->CastToStoreObject()->GetTypeId();
            typePtr = inst->CastToStoreObject()->GetObjField();
            break;
        case Opcode::StoreResolvedObjectField:
            typeId = inst->CastToStoreResolvedObjectField()->GetTypeId();
            break;
        default:
            UNREACHABLE();
    }
    auto base = inst->GetDataFlowInput(0);
    return isStatic ? Pointer::CreateStaticField(typeId, typePtr) : Pointer::CreateObjectField(base, typeId, typePtr);
}

static Pointer ParseDynamicField(Inst *inst)
{
    auto base = inst->GetDataFlowInput(0);

    DynObjectAccessType type;
    DynObjectAccessMode mode;
    switch (inst->GetOpcode()) {
        case Opcode::LoadObjectDynamic:
            type = inst->CastToLoadObjectDynamic()->GetAccessType();
            mode = inst->CastToLoadObjectDynamic()->GetAccessMode();
            break;
        case Opcode::StoreObjectDynamic:
            type = inst->CastToStoreObjectDynamic()->GetAccessType();
            mode = inst->CastToStoreObjectDynamic()->GetAccessMode();
            break;
        default:
            UNREACHABLE();
    }

    return GetDynamicAccessPointer(inst, base, type, mode);
}

/// Selects the address from instruction that should be checked on alias
bool AliasVisitor::ParseInstruction(Inst *inst, Pointer *pointer)
{
    Pointer p {};
    switch (inst->GetOpcode()) {
        case Opcode::LoadArray:
        case Opcode::LoadArrayI:
        case Opcode::StoreArray:
        case Opcode::StoreArrayI:
            p = ParseArrayElement(inst);
            break;
        case Opcode::LoadString:
        case Opcode::LoadType:
        case Opcode::UnresolvedLoadType:
            p = ParsePoolConstant(inst);
            break;
        case Opcode::LoadStatic:
        case Opcode::StoreStatic:
        case Opcode::UnresolvedStoreStatic:
        case Opcode::LoadResolvedObjectFieldStatic:
        case Opcode::StoreResolvedObjectFieldStatic:
            p = ParseStaticField(inst);
            break;
        case Opcode::LoadObject:
        case Opcode::StoreObject:
        case Opcode::LoadResolvedObjectField:
        case Opcode::StoreResolvedObjectField:
            p = ParseObjectField(inst);
            break;
        case Opcode::LoadObjectDynamic:
        case Opcode::StoreObjectDynamic:
            p = ParseDynamicField(inst);
            break;
        default:
            return false;
    }

    auto base = p.GetBase();
    if (base != nullptr) {
        // Currently unhandled and return always MAY_ALIAS
        if (base->GetOpcode() == Opcode::LoadArrayPair || base->GetOpcode() == Opcode::LoadArrayPairI ||
            base->GetOpcode() == Opcode::LoadPairPart || base->GetOpcode() == Opcode::CatchPhi ||
            base->GetOpcode() == Opcode::Load || base->GetOpcode() == Opcode::LoadI ||
            base->GetOpcode() == Opcode::Store || base->GetOpcode() == Opcode::StoreI) {
            return false;
        }
        ASSERT(base->IsDominate(inst));
    }

    *pointer = p;
    return true;
}

void AliasVisitor::Init(ArenaAllocator *allocator)
{
    inputsSet_ = allocator->New<ArenaSet<Inst *>>(allocator->Adapter());
    ASSERT(inputsSet_ != nullptr);
}

/// Instructions that definitely are not an alias of anything.
void AliasVisitor::VisitNullPtr(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasVisitor::VisitLoadUniqueObject(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasVisitor::VisitLoadUndefined(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasVisitor::VisitInitObject(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->VisitAllocation(inst);
}
void AliasVisitor::VisitNewObject(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->VisitAllocation(inst);
}
void AliasVisitor::VisitNewArray(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->VisitAllocation(inst);
}
void AliasVisitor::VisitMultiArray(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->VisitAllocation(inst);
}
void AliasVisitor::VisitInitEmptyString(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasVisitor::VisitInitString(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}
void AliasVisitor::VisitStringFlatCheck(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->IsReferenceOrAny());
    static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}

/**
 * Instructions that can introduce references that are an alias of
 * something already existed.
 */
void AliasVisitor::VisitConstant(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasVisitor::VisitParameter(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasVisitor::VisitLiveIn(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasVisitor::VisitLoadImmediate(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasVisitor::VisitBitcast(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

void AliasVisitor::VisitIntrinsic(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
    if (inst->GetFlag(inst_flags::HEAP_INV)) {
        // NB: we assume that for not HEAP_INV intrinsics their inputs don't escape
        static_cast<AliasVisitor *>(v)->EscapeInputsAndInv(inst);
    }
}

void AliasVisitor::VisitBuiltin(GraphVisitor *v, Inst *inst)
{
    VisitCall(v, inst);
}
void AliasVisitor::VisitCallStatic(GraphVisitor *v, Inst *inst)
{
    VisitCall(v, inst);
}
void AliasVisitor::VisitCallResolvedStatic(GraphVisitor *v, Inst *inst)
{
    VisitCall(v, inst);
}
void AliasVisitor::VisitCallVirtual(GraphVisitor *v, Inst *inst)
{
    VisitCall(v, inst);
}
void AliasVisitor::VisitCallResolvedVirtual(GraphVisitor *v, Inst *inst)
{
    VisitCall(v, inst);
}
void AliasVisitor::VisitCallDynamic(GraphVisitor *v, Inst *inst)
{
    VisitCall(v, inst);
}
void AliasVisitor::VisitCallNative(GraphVisitor *v, Inst *inst)
{
    VisitCall(v, inst);
}
void AliasVisitor::VisitCall(GraphVisitor *v, Inst *inst)
{
    if (static_cast<CallInst *>(inst)->IsInlined()) {
        ASSERT(inst->GetUsers().Empty());
        return;
    }
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
    static_cast<AliasVisitor *>(v)->EscapeInputsAndInv(inst);
}

void AliasVisitor::VisitGetManagedClassObject(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}
void AliasVisitor::VisitResolveObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

/// Instructions that introduce static fields (global variables).
void AliasVisitor::VisitLoadStatic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToLoadStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId, typedInst->GetObjField());

    visitor->SetVolatile(sfield, inst);

    visitor->AddDirectEdge(sfield);
    visitor->AddCopyEdge(sfield, Pointer::CreateObject(inst));
}

void AliasVisitor::VisitLoadResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToLoadResolvedObjectFieldStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId);

    visitor->SetVolatile(sfield, inst);

    visitor->AddDirectEdge(sfield);
    visitor->AddCopyEdge(sfield, Pointer::CreateObject(inst));
}

void AliasVisitor::VisitStoreStatic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToStoreStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId, typedInst->GetObjField());

    visitor->SetVolatile(sfield, inst);

    visitor->AddDirectEdge(sfield);
    auto stored = inst->GetDataFlowInput(typedInst->STORED_INPUT_INDEX);
    visitor->AddCopyEdge(Pointer::CreateObject(stored), sfield);
    visitor->Escape(stored);
}

void AliasVisitor::VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    if (inst->GetType() != DataType::REFERENCE) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToStoreResolvedObjectFieldStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId);

    visitor->SetVolatile(sfield, inst);

    visitor->AddDirectEdge(sfield);
    auto stored = inst->GetDataFlowInput(typedInst->STORED_INPUT_INDEX);
    visitor->AddCopyEdge(Pointer::CreateObject(stored), sfield);
    visitor->Escape(stored);
}

void AliasVisitor::VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToUnresolvedStoreStatic();
    uint32_t typeId = typedInst->GetTypeId();
    Pointer sfield = Pointer::CreateStaticField(typeId);

    visitor->SetVolatile(sfield, inst);

    visitor->AddDirectEdge(sfield);
    auto stored = inst->GetDataFlowInput(typedInst->STORED_INPUT_INDEX);
    visitor->AddCopyEdge(Pointer::CreateObject(stored), sfield);
    visitor->Escape(stored);
}

/// Instructions that introduce unique constant references (global constants).
void AliasVisitor::VisitLoadRuntimeClass(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadRuntimeClass()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}

void AliasVisitor::VisitLoadClass(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadClass()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasVisitor::VisitLoadAndInitClass(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadAndInitClass()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasVisitor::VisitUnresolvedLoadAndInitClass(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToUnresolvedLoadAndInitClass()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasVisitor::VisitGetGlobalVarAddress(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToGetGlobalVarAddress()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasVisitor::VisitLoadString(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadString()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasVisitor::VisitLoadConstArray(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadConstArray()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasVisitor::VisitLoadType(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToLoadType()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}
void AliasVisitor::VisitUnresolvedLoadType(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        uint32_t typeId = inst->CastToUnresolvedLoadType()->GetTypeId();
        static_cast<AliasVisitor *>(v)->AddConstantDirectEdge(inst, typeId);
    }
}

void AliasVisitor::VisitLoadObjFromConst(GraphVisitor *v, Inst *inst)
{
    if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

/// Instructions that introduce aliases.
void AliasVisitor::VisitLoadArray(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *arr = inst->GetDataFlowInput(0);
    Inst *idx = inst->GetDataFlowInput(1);
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elem = Pointer::CreateArrayElement(arr, idx);
    Pointer val = Pointer::CreateObject(inst);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasVisitor::VisitStoreArray(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *arr = inst->GetDataFlowInput(0);
    Inst *idx = inst->GetDataFlowInput(1);
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elem = Pointer::CreateArrayElement(arr, idx);
    Pointer val = Pointer::CreateObject(inst->GetDataFlowInput(2U));

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(val, elem);
}

void AliasVisitor::VisitLoadArrayI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *arr = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elem = Pointer::CreateArrayElement(arr, nullptr, inst->CastToLoadArrayI()->GetImm());
    Pointer val = Pointer::CreateObject(inst);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasVisitor::VisitStoreArrayI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *arr = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elem = Pointer::CreateArrayElement(arr, nullptr, inst->CastToStoreArrayI()->GetImm());
    Pointer val = Pointer::CreateObject(inst->GetDataFlowInput(1));

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(val, elem);
}

void AliasVisitor::VisitLoadArrayPair(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto *load = inst->CastToLoadArrayPair();
    Inst *arr = load->GetDataFlowInput(load->GetArray());
    Pointer obj = Pointer::CreateObject(arr);
    for (auto &user : load->GetUsers()) {
        ASSERT(user.GetInst()->GetOpcode() == Opcode::LoadPairPart);
        auto uinst = user.GetInst()->CastToLoadPairPart();

        Pointer elem = Pointer::CreateArrayElement(arr, load->GetIndex(), uinst->GetImm());
        Pointer val = Pointer::CreateObject(uinst);
        visitor->AddPseudoCopyEdge(obj, elem);
        visitor->AddCopyEdge(elem, val);
        visitor->AddCopyEdge(elem, Pointer::CreateObject(load));
    }
}

void AliasVisitor::VisitStoreArrayPair(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto *store = inst->CastToStoreArrayPair();
    Inst *arr = store->GetDataFlowInput(store->GetArray());
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elFst = Pointer::CreateArrayElement(arr, store->GetIndex());
    Pointer elSnd = Pointer::CreateArrayElement(arr, store->GetIndex(), 1);
    Pointer valFst = Pointer::CreateObject(store->GetDataFlowInput(store->GetStoredValue(0)));
    Pointer valSnd = Pointer::CreateObject(store->GetDataFlowInput(store->GetStoredValue(1)));

    visitor->AddPseudoCopyEdge(obj, elFst);
    visitor->AddPseudoCopyEdge(obj, elSnd);
    visitor->AddCopyEdge(valFst, elFst);
    visitor->AddCopyEdge(valSnd, elSnd);
}

void AliasVisitor::VisitLoadObjectPair(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToLoadObjectPair();
    uint32_t typeId0 = typedInst->GetTypeId0();
    uint32_t typeId1 = typedInst->GetTypeId1();
    ASSERT(typedInst->GetObjectType() != ObjectType::MEM_STATIC);
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer field0 = Pointer::CreateObjectField(dfobj, typeId0, typedInst->GetObjField0());
    Pointer field1 = Pointer::CreateObjectField(dfobj, typeId1, typedInst->GetObjField1());

    visitor->SetVolatile(field0, inst);
    visitor->SetVolatile(field1, inst);

    visitor->AddPseudoCopyEdge(obj, field0);
    visitor->AddPseudoCopyEdge(obj, field1);

    for (auto &user : inst->GetUsers()) {
        ASSERT(user.GetInst()->GetOpcode() == Opcode::LoadPairPart);
        auto uinst = user.GetInst()->CastToLoadPairPart();
        const auto &field = uinst->GetImm() == 0 ? field0 : field1;
        visitor->AddCopyEdge(field, Pointer::CreateObject(user.GetInst()));
    }
}

void AliasVisitor::VisitStoreObjectPair(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToStoreObjectPair();
    uint32_t typeId0 = typedInst->GetTypeId0();
    uint32_t typeId1 = typedInst->GetTypeId1();
    ASSERT(typedInst->GetObjectType() != ObjectType::MEM_STATIC);
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer field0 = Pointer::CreateObjectField(dfobj, typeId0, typedInst->GetObjField0());
    Pointer val0 = Pointer::CreateObject(inst->GetDataFlowInput(1));
    Pointer field1 = Pointer::CreateObjectField(dfobj, typeId1, typedInst->GetObjField1());
    Pointer val1 = Pointer::CreateObject(inst->GetDataFlowInput(2));

    visitor->SetVolatile(field0, inst);
    visitor->SetVolatile(field1, inst);

    visitor->AddPseudoCopyEdge(obj, field0);
    visitor->AddCopyEdge(val0, field0);
    visitor->AddPseudoCopyEdge(obj, field1);
    visitor->AddCopyEdge(val1, field1);
}

void AliasVisitor::VisitLoadArrayPairI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto *load = inst->CastToLoadArrayPairI();
    Inst *arr = load->GetDataFlowInput(load->GetArray());
    Pointer obj = Pointer::CreateObject(arr);
    for (auto &user : load->GetUsers()) {
        ASSERT(user.GetInst()->GetOpcode() == Opcode::LoadPairPart);
        auto uinst = user.GetInst()->CastToLoadPairPart();

        Pointer elem = Pointer::CreateArrayElement(arr, nullptr, load->GetImm() + uinst->GetImm());
        Pointer val = Pointer::CreateObject(uinst);
        visitor->AddPseudoCopyEdge(obj, elem);
        visitor->AddCopyEdge(elem, val);
    }
}

void AliasVisitor::VisitStoreArrayPairI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto *store = inst->CastToStoreArrayPairI();
    Inst *arr = store->GetDataFlowInput(store->GetArray());
    Pointer obj = Pointer::CreateObject(arr);
    Pointer elFst = Pointer::CreateArrayElement(arr, nullptr, store->GetImm());
    Pointer elSnd = Pointer::CreateArrayElement(arr, nullptr, store->GetImm() + 1);
    Pointer valFst = Pointer::CreateObject(store->GetDataFlowInput(store->GetFirstValue()));
    Pointer valSnd = Pointer::CreateObject(store->GetDataFlowInput(store->GetSecondValue()));

    visitor->AddPseudoCopyEdge(obj, elFst);
    visitor->AddPseudoCopyEdge(obj, elSnd);
    visitor->AddCopyEdge(valFst, elFst);
    visitor->AddCopyEdge(valSnd, elSnd);
}

void AliasVisitor::VisitLoadObject(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToLoadObject();
    uint32_t typeId = typedInst->GetTypeId();
    if (inst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_STATIC) {
        Pointer sfield = Pointer::CreateStaticField(typeId, typedInst->GetObjField());

        visitor->SetVolatile(sfield, inst);

        visitor->AddDirectEdge(sfield);
        visitor->AddCopyEdge(sfield, Pointer::CreateObject(inst));
    } else {
        Inst *dfobj = inst->GetDataFlowInput(0);
        Pointer obj = Pointer::CreateObject(dfobj);
        Pointer ifield = Pointer::CreateObjectField(dfobj, typeId, typedInst->GetObjField());
        Pointer to = Pointer::CreateObject(inst);

        visitor->SetVolatile(ifield, inst);

        visitor->AddPseudoCopyEdge(obj, ifield);
        visitor->AddCopyEdge(ifield, to);
    }
}

void AliasVisitor::VisitLoadResolvedObjectField(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToLoadResolvedObjectField();
    uint32_t typeId = typedInst->GetTypeId();
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer ifield = Pointer::CreateObjectField(dfobj, typeId);
    Pointer to = Pointer::CreateObject(inst);

    visitor->SetVolatile(ifield, inst);

    visitor->AddPseudoCopyEdge(obj, ifield);
    visitor->AddCopyEdge(ifield, to);
}

void AliasVisitor::VisitStoreObject(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToStoreObject();
    uint32_t typeId = typedInst->GetTypeId();
    if (inst->CastToStoreObject()->GetObjectType() == ObjectType::MEM_STATIC) {
        Pointer sfield = Pointer::CreateStaticField(typeId, typedInst->GetObjField());

        visitor->SetVolatile(sfield, inst);

        visitor->AddDirectEdge(sfield);
        visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), sfield);
    } else {
        Inst *dfobj = inst->GetDataFlowInput(0);
        Pointer obj = Pointer::CreateObject(dfobj);
        Pointer ifield = Pointer::CreateObjectField(dfobj, typeId, typedInst->GetObjField());
        Pointer val = Pointer::CreateObject(inst->GetDataFlowInput(1));

        visitor->SetVolatile(ifield, inst);

        visitor->AddPseudoCopyEdge(obj, ifield);
        visitor->AddCopyEdge(val, ifield);
    }
}

void AliasVisitor::VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto typedInst = inst->CastToStoreResolvedObjectField();
    uint32_t typeId = typedInst->GetTypeId();
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer ifield = Pointer::CreateObjectField(dfobj, typeId);
    Pointer val = Pointer::CreateObject(inst->GetDataFlowInput(typedInst->STORED_INPUT_INDEX));

    visitor->SetVolatile(ifield, inst);

    visitor->AddPseudoCopyEdge(obj, ifield);
    visitor->AddCopyEdge(val, ifield);
}

void AliasVisitor::VisitLoad(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *dfobj = inst->GetDataFlowInput(0);
    if (dfobj->GetType() == DataType::POINTER) {
        visitor->AddDirectEdge(Pointer::CreateObject(inst));
        return;
    }
    Pointer obj = Pointer::CreateObject(dfobj);
    Inst *idx = inst->GetDataFlowInput(1);
    Pointer elem = Pointer::CreateRawOffset(dfobj, idx);
    Pointer val = Pointer::CreateObject(inst);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasVisitor::VisitLoadNative(GraphVisitor *v, Inst *inst)
{
    VisitLoad(v, inst);
}

void AliasVisitor::VisitStore(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto stored = inst->GetDataFlowInput(2U);
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *dfobj = inst->GetDataFlowInput(0);
    if (dfobj->GetType() == DataType::POINTER) {
        // Treat store to pointer as escaping for simplicity
        visitor->Escape(stored);
        return;
    }
    Pointer obj = Pointer::CreateObject(dfobj);
    Inst *idx = inst->GetDataFlowInput(1);
    Pointer elem = Pointer::CreateRawOffset(dfobj, idx);
    Pointer val = Pointer::CreateObject(stored);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(val, elem);
}

void AliasVisitor::VisitStoreNative(GraphVisitor *v, Inst *inst)
{
    VisitStore(v, inst);
}

void AliasVisitor::VisitLoadI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *dfobj = inst->GetDataFlowInput(0);
    if (dfobj->GetType() == DataType::POINTER) {
        visitor->AddDirectEdge(Pointer::CreateObject(inst));
        return;
    }
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer elem = Pointer::CreateRawOffset(dfobj, nullptr, inst->CastToLoadI()->GetImm());
    Pointer val = Pointer::CreateObject(inst);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasVisitor::VisitStoreI(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto stored = inst->GetDataFlowInput(1U);
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *dfobj = inst->GetDataFlowInput(0);
    if (dfobj->GetType() == DataType::POINTER) {
        // Treat store to pointer as escaping for simplicity
        visitor->Escape(stored);
        return;
    }
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer elem = Pointer::CreateRawOffset(dfobj, nullptr, inst->CastToStoreI()->GetImm());
    Pointer val = Pointer::CreateObject(stored);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(val, elem);
}

void AliasVisitor::VisitCatchPhi(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto inputsSet = visitor->GetClearInputsSet();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        inputsSet->insert(inst->GetDataFlowInput(i));
    }

    for (auto inputInst : *inputsSet) {
        visitor->AddCopyEdge(Pointer::CreateObject(inputInst), Pointer::CreateObject(inst));
    }
    if (inputsSet->empty()) {
        // TryCatchResolving hasn't run yet
        visitor->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

void AliasVisitor::VisitPhi(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(i)), Pointer::CreateObject(inst));
    }
}

void AliasVisitor::VisitSelect(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    ASSERT(inst->GetInputsCount() == 4U);
    auto visitor = static_cast<AliasVisitor *>(v);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), Pointer::CreateObject(inst));
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(1)), Pointer::CreateObject(inst));
}

void AliasVisitor::VisitSelectImm(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    ASSERT(inst->GetInputsCount() == 3U);
    auto visitor = static_cast<AliasVisitor *>(v);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), Pointer::CreateObject(inst));
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(1)), Pointer::CreateObject(inst));
}

void AliasVisitor::VisitMov(GraphVisitor *v, Inst *inst)
{
    UNREACHABLE();
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)), Pointer::CreateObject(inst));
}

// RefTypeCheck is always of a known type, but dataflow input isn't until the check actually happens;
// So it makes sence to add a copy edge to new pointer
void AliasVisitor::VisitRefTypeCheck(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->IsReferenceOrAny());
    auto visitor = static_cast<AliasVisitor *>(v);
    visitor->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(1U)), Pointer::CreateObject(inst));
}

void AliasVisitor::VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst)
{
    if (inst->GetType() == DataType::REFERENCE) {
        static_cast<AliasVisitor *>(v)->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)),
                                                    Pointer::CreateObject(inst));
    }
}

void AliasVisitor::VisitCastValueToAnyType(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToCastValueToAnyType()->GetInputType(0) == DataType::REFERENCE) {
        static_cast<AliasVisitor *>(v)->AddCopyEdge(Pointer::CreateObject(inst->GetDataFlowInput(0)),
                                                    Pointer::CreateObject(inst));
    } else if (inst->IsReferenceOrAny()) {
        static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
    }
}

void AliasVisitor::VisitGetAnyTypeName(GraphVisitor *v, Inst *inst)
{
    static_cast<AliasVisitor *>(v)->AddDirectEdge(Pointer::CreateObject(inst));
}

void AliasVisitor::VisitLoadConstantPool(GraphVisitor *v, Inst *inst)
{
    Inst *dfobj = inst->GetDataFlowInput(0);
    // fake offset to avoid aliasing with real fields
    // also avoid cross_values usage here to get real offset
    Pointer elem = Pointer::CreateArrayElement(dfobj, nullptr, UINT64_MAX - 2U);
    Pointer to = Pointer::CreateObject(inst);
    static_cast<AliasVisitor *>(v)->AddCopyEdge(elem, to);
}

void AliasVisitor::VisitLoadLexicalEnv(GraphVisitor *v, Inst *inst)
{
    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer elem = Pointer::CreateArrayElement(dfobj, nullptr, UINT64_MAX - 1U);
    Pointer to = Pointer::CreateObject(inst);
    static_cast<AliasVisitor *>(v)->AddCopyEdge(elem, to);
}

void AliasVisitor::VisitLoadObjectDynamic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto visitor = static_cast<AliasVisitor *>(v);
    auto type = inst->CastToLoadObjectDynamic()->GetAccessType();
    auto mode = inst->CastToLoadObjectDynamic()->GetAccessMode();

    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer val = Pointer::CreateObject(inst);
    Pointer elem = GetDynamicAccessPointer(inst, dfobj, type, mode);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasVisitor::VisitStoreObjectDynamic(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    auto stored = inst->GetDataFlowInput(2U);
    auto visitor = static_cast<AliasVisitor *>(v);
    auto type = inst->CastToStoreObjectDynamic()->GetAccessType();
    auto mode = inst->CastToStoreObjectDynamic()->GetAccessMode();

    Inst *dfobj = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(dfobj);
    Pointer val = Pointer::CreateObject(stored);
    Pointer elem = GetDynamicAccessPointer(inst, dfobj, type, mode);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(val, elem);
}

void AliasVisitor::VisitLoadFromConstantPool(GraphVisitor *v, Inst *inst)
{
    if (!inst->IsReferenceOrAny()) {
        return;
    }
    uint32_t typeId = inst->CastToLoadFromConstantPool()->GetTypeId();
    auto visitor = static_cast<AliasVisitor *>(v);
    Inst *constpool = inst->GetDataFlowInput(0);
    Pointer obj = Pointer::CreateObject(constpool);
    Pointer elem = Pointer::CreateArrayElement(constpool, nullptr, typeId);
    Pointer val = Pointer::CreateObject(inst);

    visitor->AddPseudoCopyEdge(obj, elem);
    visitor->AddCopyEdge(elem, val);
}

void AliasVisitor::VisitDefault([[maybe_unused]] Inst *inst)
{
    /* Ignore the following instructions with REFERENCE type intentionally */
    switch (inst->GetOpcode()) {
        // Handled on its input
        case Opcode::LoadPairPart:
        // No passes that check class references aliasing
        case Opcode::GetInstanceClass:
        case Opcode::LoadImmediate:
        // NOTE(compiler): Probably should be added
        case Opcode::Monitor:
        // Mitigated by using GetDataFlowInput
        case Opcode::NullCheck:
        case Opcode::RefTypeCheck:
        case Opcode::AnyTypeCheck:
        case Opcode::ObjByIndexCheck:
        case Opcode::HclassCheck:
        // Irrelevant for analysis
        case Opcode::Return:
        case Opcode::ReturnI:
        // No need to analyze
        case Opcode::LiveOut:
        case Opcode::FunctionImmediate:
            return;
        default:
            ASSERT_DO(!inst->IsReferenceOrAny(),
                      (std::cerr << "Unsupported instruction in alias analysis: ", inst->Dump(&std::cerr)));
            return;
    }
}

void AliasVisitor::EscapeInputsAndInv(Inst *inst)
{
    for (auto &input : inst->GetInputs()) {
        auto *inputInst = Inst::GetDataFlowInput(input.GetInst());
        if (inputInst->IsReferenceOrAny()) {
            Escape(inputInst);
        }
    }
    VisitHeapInv(inst);
}

}  // namespace ark::compiler
