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

#include "libarkbase/events/events.h"
#include "runtime/bridge/bridge.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/locks.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/method-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/value-inl.h"
#include "runtime/interpreter/frame.h"
#include "runtime/interpreter/interpreter.h"
#include "libarkbase/utils/hash.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/os/mutex.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/debug_helpers.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/line_number_program.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkfile/shorty_iterator.h"
#include "runtime/handle_base-inl.h"
#include "runtime/handle_scope-inl.h"
#include "libarkfile/type_helper.h"
#include "verification/public.h"
#include "verification/util/is_system.h"

namespace ark {

Method::Proto::Proto(const panda_file::File &pf, panda_file::File::EntityId protoId)
{
    panda_file::ProtoDataAccessor pda(pf, protoId);

    pda.EnumerateTypes([this](panda_file::Type type) { shorty_.push_back(type); });

    auto refNum = pda.GetRefNum();
    for (size_t refIdx = 0; refIdx < refNum; ++refIdx) {
        auto id = pda.GetReferenceType(refIdx);
        refTypes_.emplace_back(utf::Mutf8AsCString(pf.GetStringData(id).data));
    }
}

bool Method::ProtoId::operator==(const Method::ProtoId &other) const
{
    if (&pf_ == &other.pf_) {
        return protoId_ == other.protoId_;
    }

    panda_file::ProtoDataAccessor pda1(pf_, protoId_);
    panda_file::ProtoDataAccessor pda2(other.pf_, other.protoId_);
    return pda1.IsEqual(&pda2);
}

bool Method::ProtoId::operator==(const Proto &other) const
{
    const auto &shorties = other.GetShorty();
    const auto &refTypes = other.GetRefTypes();

    bool equal = true;
    size_t shortyIdx = 0;
    panda_file::ProtoDataAccessor pda(pf_, protoId_);
    pda.EnumerateTypes([&equal, &shorties, &shortyIdx](panda_file::Type type) {
        equal = equal && shortyIdx < shorties.size() && type == shorties[shortyIdx];
        ++shortyIdx;
    });
    if (!equal || shortyIdx != shorties.size() || pda.GetRefNum() != refTypes.size()) {
        return false;
    }

    // check ref types
    for (size_t refIdx = 0; refIdx < refTypes.size(); ++refIdx) {
        auto id = pda.GetReferenceType(refIdx);
        if (refTypes[refIdx] != utf::Mutf8AsCString(pf_.GetStringData(id).data)) {
            return false;
        }
    }

    return true;
}

std::string_view Method::Proto::GetReturnTypeDescriptor() const
{
    auto retType = GetReturnType();
    if (!retType.IsPrimitive()) {
        return refTypes_[0];
    }

    switch (retType.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return "V";
        case panda_file::Type::TypeId::U1:
            return "Z";
        case panda_file::Type::TypeId::I8:
            return "B";
        case panda_file::Type::TypeId::U8:
            return "H";
        case panda_file::Type::TypeId::I16:
            return "S";
        case panda_file::Type::TypeId::U16:
            return "C";
        case panda_file::Type::TypeId::I32:
            return "I";
        case panda_file::Type::TypeId::U32:
            return "U";
        case panda_file::Type::TypeId::F32:
            return "F";
        case panda_file::Type::TypeId::I64:
            return "J";
        case panda_file::Type::TypeId::U64:
            return "Q";
        case panda_file::Type::TypeId::F64:
            return "D";
        case panda_file::Type::TypeId::TAGGED:
            return "A";
        default:
            UNREACHABLE();
    }
}

PandaString Method::Proto::GetSignature(bool includeReturnType)
{
    PandaOStringStream signature;
    signature << "(";
    auto &shorty = GetShorty();
    auto &refTypes = GetRefTypes();

    auto refIt = refTypes.begin();
    panda_file::Type returnType = shorty[0];
    if (!returnType.IsPrimitive()) {
        ++refIt;
    }
    auto shortyEnd = shorty.end();
    auto shortyIt = shorty.begin() + 1;
    for (; shortyIt != shortyEnd; ++shortyIt) {
        if ((*shortyIt).IsPrimitive()) {
            signature << panda_file::Type::GetSignatureByTypeId(*shortyIt);
        } else {
            signature << *refIt;
            ++refIt;
        }
    }
    signature << ")";
    if (includeReturnType) {
        if (returnType.IsPrimitive()) {
            signature << panda_file::Type::GetSignatureByTypeId(returnType);
        } else {
            signature << refTypes[0];
        }
    }

    return signature.str();
}

uint32_t Method::GetFullNameHashFromString(const PandaString &str)
{
    return GetHash32String(reinterpret_cast<const uint8_t *>(str.c_str()));
}

uint32_t Method::GetClassNameHashFromString(const PandaString &str)
{
    return GetHash32String(reinterpret_cast<const uint8_t *>(str.c_str()));
}

Method::UniqId Method::CalcUniqId(const uint8_t *classDescr, const uint8_t *name)
{
    auto constexpr HALF = 32ULL;
    constexpr uint64_t NO_FILE = 0xFFFFFFFFULL << HALF;
    uint64_t hash = PseudoFnvHashString(classDescr);
    hash = PseudoFnvHashString(name, hash);
    return NO_FILE | hash;
}

Method::Method(Class *klass, const panda_file::File *pf, panda_file::File::EntityId fileId,
               panda_file::File::EntityId codeId, uint32_t accessFlags, uint32_t numArgs, const uint16_t *shorty)
    : accessFlags_(accessFlags),
      numArgs_(numArgs),
      stor16Pair_({0, 0}),
      classWord_(static_cast<ClassHelper::ClassWordSize>(ToObjPtrType(klass))),
      compiledEntryPoint_(nullptr),
      pandaFile_(pf),
      pointer_(),

      fileId_(fileId),
      codeId_(codeId),
      shorty_(shorty),
      instructions_(nullptr),
      numVregs_(NUM_VREGS_UNKNOWN)
{
    ResetHotnessCounter();
    ResetSaverTryCounter();
    // Atomic with relaxed order reason: data race with native_pointer_ with no synchronization or ordering constraints
    // imposed on other reads or writes NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    pointer_.nativePointer.store(nullptr, std::memory_order_relaxed);
    SetCompilationStatus(CompilationStage::NOT_COMPILED);
}

Value Method::Invoke(ManagedThread *thread, Value *args, bool proxyCall)
{
#ifndef NDEBUG
    if (thread->HasPendingException()) {
        LOG(ERROR, INTERPRETER) << "Has pending exception " << thread->GetException()->ClassAddr<Class>()->GetName();
        LOG(ERROR, INTERPRETER) << "Before call the method " << GetFullName();
        StackWalker::Create(ManagedThread::GetCurrent()).Dump(std::cerr);
    }
#endif
    if (thread->HasPendingException()) {
        return Value();
    }
    return InvokeImpl<InvokeHelperStatic>(thread, GetNumArgs(), args, proxyCall);
}

PANDA_PUBLIC_API panda_file::Type Method::GetReturnType() const
{
    panda_file::ShortyIterator it(shorty_);
    return *it;
}

panda_file::Type Method::GetArgType(size_t idx) const
{
    if (!IsStatic()) {
        if (idx == 0) {
            return panda_file::Type(panda_file::Type::TypeId::REFERENCE);
        }

        --idx;
    }

    panda_file::ProtoDataAccessor pda(*(pandaFile_),
                                      panda_file::MethodDataAccessor::GetProtoId(*(pandaFile_), fileId_));
    return pda.GetArgType(idx);
}

panda_file::File::StringData Method::GetRefReturnType() const
{
    ASSERT(GetReturnType().IsReference());
    panda_file::ProtoDataAccessor pda(*(pandaFile_),
                                      panda_file::MethodDataAccessor::GetProtoId(*(pandaFile_), fileId_));
    panda_file::File::EntityId classId = pda.GetReferenceType(0);
    return pandaFile_->GetStringData(classId);
}

panda_file::File::StringData Method::GetRefArgType(size_t idx) const
{
    if (!IsStatic()) {
        if (idx == 0) {
            return pandaFile_->GetStringData(panda_file::MethodDataAccessor::GetClassId(*(pandaFile_), fileId_));
        }

        --idx;
    }

    // in case of reference return type first idx corresponds to it
    if (GetReturnType().IsReference()) {
        ++idx;
    }
    panda_file::ProtoDataAccessor pda(*(pandaFile_),
                                      panda_file::MethodDataAccessor::GetProtoId(*(pandaFile_), fileId_));
    panda_file::File::EntityId classId = pda.GetReferenceType(idx);
    return pandaFile_->GetStringData(classId);
}

panda_file::File::StringData Method::GetName() const
{
    return panda_file::MethodDataAccessor::GetName(*(pandaFile_), fileId_);
}

PandaString Method::GetFullName(bool withSignature) const
{
    PandaOStringStream ss;
    int refIdx = 0;
    if (withSignature) {
        auto returnType = GetReturnType();
        if (returnType.IsReference()) {
            ss << ClassHelper::GetName(GetRefReturnType().data) << ' ';
        } else {
            ss << returnType << ' ';
        }
    }
    if (GetClass() != nullptr) {
        ss << PandaString(GetClass()->GetName());
    }
    ss << "::" << utf::Mutf8AsCString(Method::GetName().data);
    if (!withSignature) {
        return ss.str();
    }
    const char *sep = "";
    ss << '(';
    panda_file::ProtoDataAccessor pda(*(pandaFile_),
                                      panda_file::MethodDataAccessor::GetProtoId(*(pandaFile_), fileId_));
    for (size_t i = 0; i < GetNumArgs(); i++) {
        auto type = GetEffectiveArgType(i);
        if (type.IsReference()) {
            ss << sep << ClassHelper::GetName(GetRefArgType(refIdx++).data);
        } else {
            ss << sep << type;
        }
        sep = ", ";
    }
    ss << ')';
    return ss.str();
}

PandaString Method::GetLineNumberAndSourceFile(uint32_t bcOffset) const
{
    PandaOStringStream ss;
    auto *source = GetClassSourceFile().data;
    auto lineNum = GetLineNumFromBytecodeOffset(bcOffset);

    if (source == nullptr) {
        source = utf::CStringAsMutf8("<unknown>");
    }

    ss << source << ":" << lineNum;
    return ss.str();
}

panda_file::File::StringData Method::GetClassName() const
{
    return pandaFile_->GetStringData(panda_file::MethodDataAccessor::GetClassId(*(pandaFile_), fileId_));
}

Method::Proto Method::GetProto() const
{
    return Proto(*(pandaFile_), panda_file::MethodDataAccessor::GetProtoId(*(pandaFile_), fileId_));
}

Method::ProtoId Method::GetProtoId() const
{
    return ProtoId(*(pandaFile_), panda_file::MethodDataAccessor::GetProtoId(*(pandaFile_), fileId_));
}

uint32_t Method::GetNumericalAnnotation(AnnotationField fieldId) const
{
    panda_file::MethodDataAccessor mda(*(pandaFile_), fileId_);
    return mda.GetNumericalAnnotation(fieldId);
}

panda_file::File::StringData Method::GetStringDataAnnotation(AnnotationField fieldId) const
{
    ASSERT(fieldId >= AnnotationField::STRING_DATA_BEGIN && fieldId <= AnnotationField::STRING_DATA_END);
    panda_file::MethodDataAccessor mda(*(pandaFile_), fileId_);
    uint32_t strOffset = mda.GetNumericalAnnotation(fieldId);
    if (strOffset == 0) {
        return {0, nullptr};
    }
    return pandaFile_->GetStringData(panda_file::File::EntityId(strOffset));
}

int64_t Method::GetBranchTakenCounter(uint32_t pc)
{
    auto profilingData = GetProfilingData();
    if (profilingData == nullptr) {
        return 0;
    }
    return profilingData->GetBranchTakenCounter(pc);
}

int64_t Method::GetBranchNotTakenCounter(uint32_t pc)
{
    auto profilingData = GetProfilingData();
    if (profilingData == nullptr) {
        return 0;
    }
    return profilingData->GetBranchNotTakenCounter(pc);
}

int64_t Method::GetThrowTakenCounter(uint32_t pc)
{
    auto profilingData = GetProfilingData();
    if (profilingData == nullptr) {
        return 0;
    }
    return profilingData->GetThrowTakenCounter(pc);
}

uint32_t Method::FindCatchBlockInPandaFile(const Class *cls, uint32_t pc) const
{
    ASSERT(!IsAbstract());

    panda_file::MethodDataAccessor mda(*(pandaFile_), fileId_);
    panda_file::CodeDataAccessor cda(*(pandaFile_), mda.GetCodeId().value());

    uint32_t pcOffset = panda_file::INVALID_OFFSET;

    cda.EnumerateTryBlocks([&pcOffset, cls, pc, this](panda_file::CodeDataAccessor::TryBlock &tryBlock) {
        auto cb = [this, &pcOffset, &cls](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
            auto typeIdx = catchBlock.GetTypeIdx();
            if (typeIdx == panda_file::INVALID_INDEX) {
                pcOffset = catchBlock.GetHandlerPc();
                return false;
            }

            auto typeId = GetClass()->ResolveClassIndex(typeIdx);
            auto *handlerClass = Runtime::GetCurrent()->GetClassLinker()->GetClass(*this, typeId);
            if (cls->IsSubClassOf(handlerClass)) {
                pcOffset = catchBlock.GetHandlerPc();
                return false;
            }
            return true;
        };
        if ((tryBlock.GetStartPc() <= pc) && ((tryBlock.GetStartPc() + tryBlock.GetLength()) > pc)) {
            tryBlock.EnumerateCatchBlocks(cb);
        }
        return pcOffset == panda_file::INVALID_OFFSET;
    });
    return pcOffset;
}

uint32_t Method::FindCatchBlock(const Class *cls, uint32_t pc) const
{
    auto *thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> exception(thread, thread->GetException());
    thread->ClearException();

    auto pcOffset = FindCatchBlockInPandaFile(cls, pc);

    thread->SetException(exception.GetPtr());

    return pcOffset;
}

panda_file::Type Method::GetEffectiveArgType(size_t idx) const
{
    return panda_file::GetEffectiveType(GetArgType(idx));
}

panda_file::Type Method::GetEffectiveReturnType() const
{
    return panda_file::GetEffectiveType(GetReturnType());
}

size_t Method::GetLineNumFromBytecodeOffset(uint32_t bcOffset) const
{
    panda_file::MethodDataAccessor mda(*(pandaFile_), fileId_);
    return panda_file::debug_helpers::GetLineNumber(mda, bcOffset, pandaFile_);
}

panda_file::File::StringData Method::GetClassSourceFile() const
{
    panda_file::ClassDataAccessor cda(*(pandaFile_), GetClass()->GetFileId());
    auto sourceFileId = cda.GetSourceFileId();
    if (!sourceFileId) {
        return {0, nullptr};
    }

    return pandaFile_->GetStringData(sourceFileId.value());
}

bool Method::IsVerified() const
{
    if (IsIntrinsic()) {
        return true;
    }
    auto stage = GetVerificationStage();
    return stage == VerificationStage::VERIFIED_OK || stage == VerificationStage::VERIFIED_FAIL;
}

void Method::SetVerified(bool result)
{
    if (IsIntrinsic()) {
        return;
    }
    SetVerificationStage(result ? VerificationStage::VERIFIED_OK : VerificationStage::VERIFIED_FAIL);
}

bool Method::Verify()
{
    if (IsVerified()) {
        return true;
    }
    auto &&options = Runtime::GetCurrent()->GetOptions();
    auto mode = verifier::VerificationModeFromString(options.GetVerificationMode());
    auto service = Runtime::GetCurrent()->GetVerifierService();
    if (service == nullptr) {
        ASSERT(mode == verifier::VerificationMode::DISABLED);
        // set VERIFIED_OK status in order not to go into Method::Verify() anymore,
        // since the verifier cannot be enabled during the execution of the managed code
        if (!IsIntrinsic()) {
            SetVerificationStage(VerificationStage::VERIFIED_OK);
        }
        return true;
    }
    if (!options.IsVerifyRuntimeLibraries() && verifier::IsSystemClass(GetClass())) {
        LOG(DEBUG, VERIFIER) << "Skipping verification of system method " << GetFullName(true);
        return true;
    }
    if (verifier::Verify(service, this, mode) != verifier::Status::OK) {
        LOG(ERROR, VERIFIER) << "Method verification failed: " << GetFullName(true);
        return false;
    }
    return true;
}

inline void Method::FillVecsByInsts(BytecodeInstruction &inst, PandaVector<uint32_t> &vcalls,
                                    PandaVector<uint32_t> &branches, PandaVector<uint32_t> &throws) const
{
    if (inst.HasFlag(BytecodeInstruction::Flags::CALL_VIRT)) {
        vcalls.push_back(inst.GetAddress() - GetInstructions());
    }
    if (inst.HasFlag(BytecodeInstruction::Flags::JUMP)) {
        branches.push_back(inst.GetAddress() - GetInstructions());
    }
    if (inst.IsThrow(BytecodeInstruction::Exceptions::X_THROW)) {
        throws.push_back(inst.GetAddress() - GetInstructions());
    }
}

void Method::StartProfiling()
{
#ifdef PANDA_WITH_ECMASCRIPT
    // Object handles can be created during class initialization, so check lock state only after GC is started.
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() ||
           PandaVM::GetCurrent()->GetMutatorLock()->HasLock() ||
           ManagedThread::GetCurrent()->GetThreadLang() == ark::panda_file::SourceLang::ECMASCRIPT);
#else
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() ||
           PandaVM::GetCurrent()->GetMutatorLock()->HasLock());
#endif

    // Some thread already started profiling
    if (IsProfilingWithoutLock()) {
        return;
    }

    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    PandaVector<uint32_t> vcalls;
    PandaVector<uint32_t> branches;
    PandaVector<uint32_t> throws;

    Span<const uint8_t> instructions(GetInstructions(), GetCodeSize());
    for (BytecodeInstruction inst(instructions.begin()); inst.GetAddress() < instructions.end();
         inst = inst.GetNext()) {
        FillVecsByInsts(inst, vcalls, branches, throws);
    }
    if (vcalls.empty() && branches.empty() && throws.empty()) {
        return;
    }
    ASSERT(std::is_sorted(vcalls.begin(), vcalls.end()));

    // Set branch profiling enabled based on runtime options (auto-enabled when JIT is active)
    bool branchProfilingEnabled = Runtime::GetCurrent()->IsProfileBranches();

    auto profilingData = ProfilingData::Make(
        allocator, vcalls.size(), branches.size(), throws.size(),
        [&](void *data, void *vcallsMem, void *branchesMem, void *throwsMem) {
            return new (data)
                ProfilingData(CallSiteInlineCache::From(vcallsMem, vcalls), BranchData::From(branchesMem, branches),
                              ThrowData::From(throwsMem, throws), branchProfilingEnabled);
        });
    if (!InitProfilingData(profilingData)) {
        allocator->Free(profilingData);
        return;
    }

    EVENT_INTERP_PROFILING(events::InterpProfilingAction::START, GetFullName(), vcalls.size());
    Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->TryAddMethodToProfile(this);
}

void Method::StopProfiling()
{
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() ||
           PandaVM::GetCurrent()->GetMutatorLock()->HasLock());

    if (!IsProfilingWithoutLock()) {
        return;
    }

    EVENT_INTERP_PROFILING(events::InterpProfilingAction::STOP, GetFullName(),
                           GetProfilingData()->GetInlineCaches().size());

    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Free(GetProfilingData());
    // Atomic with release order reason: data race with profiling_data_ with dependecies on writes before the store
    // which should become visible acquire NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    pointer_.profilingData.store(nullptr, std::memory_order_release);
}

bool Method::IsProxy() const
{
    return GetClass()->IsProxy();
}

/* static */
int16_t Method::GetInitialHotnessCounter()
{
    if (!Runtime::GetCurrent()->IsProfilerEnabled()) {
        return std::numeric_limits<int16_t>::max();
    }

    auto &options = Runtime::GetOptions();
    uint32_t threshold = options.GetCompilerHotnessThreshold();
    uint32_t profThreshold = options.GetCompilerProfilingThreshold();
    return std::min(threshold, profThreshold);
}

void Method::ResetHotnessCounter()
{
    stor16Pair_.hotnessCounter = GetInitialHotnessCounter();
}

void Method::TryCreateSaverTask()
{
    DecrementSaverTryCounter();
    if (GetSaverTryCounter() < 0) {
        ResetSaverTryCounter();
        Runtime::GetCurrent()->TryCreateSaverTask();
    }
}

}  // namespace ark
