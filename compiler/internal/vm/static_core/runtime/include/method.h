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
#ifndef PANDA_RUNTIME_METHOD_H_
#define PANDA_RUNTIME_METHOD_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <string_view>

#include "intrinsics_enum.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/logger.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/modifiers.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/interpreter/frame.h"
#include "value.h"

namespace ark {

class Class;
class ManagedThread;
class ProfilingData;

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
namespace interpreter {
class AccVRegisterT;
}  // namespace interpreter
using interpreter::AccVRegisterT;
#else
namespace interpreter {
using AccVRegisterT = AccVRegister;
}  // namespace interpreter
#endif

class FrameDeleter {
public:
    explicit FrameDeleter(ManagedThread *thread) : thread_(thread) {}

    void operator()(Frame *frame) const;

private:
    ManagedThread *thread_;
};

class Method {
public:
    using UniqId = uint64_t;

    enum CompilationStage {
        NOT_COMPILED,
        WAITING,
        COMPILATION,
        COMPILED,
        FAILED,
    };

    enum class VerificationStage { NOT_VERIFIED = 0, VERIFIED_FAIL = 1, VERIFIED_OK = 2, LAST = VERIFIED_OK };

    static_assert(MinimumBitsToStore(VerificationStage::LAST) <= VERIFICATION_STATUS_WIDTH);

    using AnnotationField = panda_file::MethodDataAccessor::AnnotationField;

    class Proto {
    public:
        using ShortyVector = PandaSmallVector<panda_file::Type>;
        using RefTypeVector = PandaSmallVector<std::string_view>;
        Proto() = default;

        Proto(const panda_file::File &pf, panda_file::File::EntityId protoId);

        Proto(ShortyVector shorty, RefTypeVector refTypes) : shorty_(std::move(shorty)), refTypes_(std::move(refTypes))
        {
        }

        bool operator==(const Proto &other) const
        {
            return shorty_ == other.shorty_ && refTypes_ == other.refTypes_;
        }

        panda_file::Type GetReturnType() const
        {
            return shorty_[0];
        }

        PANDA_PUBLIC_API std::string_view GetReturnTypeDescriptor() const;
        PandaString GetSignature(bool includeReturnType = true);

        ShortyVector &GetShorty()
        {
            return shorty_;
        }

        const ShortyVector &GetShorty() const
        {
            return shorty_;
        }

        RefTypeVector &GetRefTypes()
        {
            return refTypes_;
        }

        const RefTypeVector &GetRefTypes() const
        {
            return refTypes_;
        }

        ~Proto() = default;

        DEFAULT_COPY_SEMANTIC(Proto);
        DEFAULT_MOVE_SEMANTIC(Proto);

    private:
        ShortyVector shorty_;
        RefTypeVector refTypes_;
    };

    class PANDA_PUBLIC_API ProtoId {
    public:
        ProtoId(const panda_file::File &pf, panda_file::File::EntityId protoId) : pf_(pf), protoId_(protoId) {}
        bool operator==(const ProtoId &other) const;
        bool operator==(const Proto &other) const;
        bool operator!=(const ProtoId &other) const
        {
            return !operator==(other);
        }
        bool operator!=(const Proto &other) const
        {
            return !operator==(other);
        }

        const panda_file::File &GetPandaFile() const
        {
            return pf_;
        }

        const panda_file::File::EntityId &GetEntityId() const
        {
            return protoId_;
        }

        ~ProtoId() = default;

        DEFAULT_COPY_CTOR(ProtoId);
        NO_COPY_OPERATOR(ProtoId);
        NO_MOVE_SEMANTIC(ProtoId);

    private:
        const panda_file::File &pf_;
        panda_file::File::EntityId protoId_;
    };
    // CC-OFFNXT(G.FUN.01) solid logic
    PANDA_PUBLIC_API Method(Class *klass, const panda_file::File *pf, panda_file::File::EntityId fileId,
                            panda_file::File::EntityId codeId, uint32_t accessFlags, uint32_t numArgs,
                            const uint16_t *shorty);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit Method(const Method *method)
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        : accessFlags_(method->accessFlags_.load(std::memory_order_acquire)),
          numArgs_(method->numArgs_),
          stor16Pair_(method->stor16Pair_),
          classWord_(method->classWord_),
          pandaFile_(method->pandaFile_),
          fileId_(method->fileId_),
          codeId_(method->codeId_),
          shorty_(method->shorty_),
          saverTryCounter_(method->saverTryCounter_)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        pointer_.nativePointer.store(
            // Atomic with relaxed order reason: data race with native_pointer_ with no synchronization or ordering
            // constraints imposed on other reads or writes NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            method->pointer_.nativePointer.load(std::memory_order_relaxed), std::memory_order_relaxed);

        // Atomic with release order reason: data race with compiled_entry_point_ with dependecies on writes before the
        // store which should become visible acquire
        compiledEntryPoint_.store(method->IsNative() ? method->GetCompiledEntryPoint()
                                                     : GetCompiledCodeToInterpreterBridge(method),
                                  std::memory_order_release);
        // Atomic with release order reason: exclude data races with GetNumVRegs
        numVregs_.store(method->numVregs_, std::memory_order_release);
        // Atomic with release order reason: exclude data races with GetInstructions
        instructions_.store(method->instructions_, std::memory_order_release);
        SetCompilationStatus(CompilationStage::NOT_COMPILED);
    }

    Method() = delete;
    Method(const Method &) = delete;
    Method(Method &&) = delete;
    Method &operator=(const Method &) = delete;
    Method &operator=(Method &&) = delete;
    ~Method() = default;

    uint32_t GetNumArgs() const
    {
        return numArgs_;
    }

    uint32_t GetNumVregs() const
    {
        if (!codeId_.IsValid()) {
            return 0;
        }
        // Atomic with acquire order reason: exclude data races in parallel execution
        uint32_t numVregs = numVregs_.load(std::memory_order_acquire);
        if (numVregs == NUM_VREGS_UNKNOWN) {
            numVregs = panda_file::CodeDataAccessor::GetNumVregs(*(pandaFile_), codeId_);
            // Atomic with release order reason: exclude data races in parallel execution
            numVregs_.store(numVregs, std::memory_order_release);
        }
        return numVregs;
    }

    uint32_t GetCodeSize() const
    {
        if (!codeId_.IsValid()) {
            return 0;
        }
        panda_file::CodeDataAccessor cda(*(pandaFile_), codeId_);
        return cda.GetCodeSize();
    }

    const uint8_t *GetInstructions() const
    {
        if (!codeId_.IsValid()) {
            return nullptr;
        }
        // Atomic with acquire order reason: exclude data races in parallel execution
        const uint8_t *instructions = instructions_.load(std::memory_order_acquire);
        if (instructions == nullptr) {
            instructions = panda_file::CodeDataAccessor::GetInstructions(*(pandaFile_), codeId_);
            // Atomic with release order reason: exclude data races in parallel execution
            instructions_.store(instructions, std::memory_order_release);
        }
        return instructions;
    }

    /*
     * Invoke the method as a static method.
     * Number of arguments and their types must match the method's signature
     */
    PANDA_PUBLIC_API Value Invoke(ManagedThread *thread, Value *args, bool proxyCall = false);

    void InvokeVoid(ManagedThread *thread, Value *args)
    {
        Invoke(thread, args);
    }

    /*
     * Invoke the method as a dynamic function.
     * Number of arguments may vary, all arguments must be of type coretypes::TaggedValue.
     * args - array of arguments. The first value must be the callee function object
     * num_args - length of args array
     * data - ark::ExtFrame language-related extension data
     */
    coretypes::TaggedValue InvokeDyn(ManagedThread *thread, uint32_t numArgs, coretypes::TaggedValue *args);

    template <class InvokeHelper>
    coretypes::TaggedValue InvokeDyn(ManagedThread *thread, uint32_t numArgs, coretypes::TaggedValue *args);

    template <class InvokeHelper>
    void InvokeEntry(ManagedThread *thread, Frame *currentFrame, Frame *frame, const uint8_t *pc);

    /*
     * Enter execution context (ECMAScript generators)
     * pc - pc of context
     * acc - accumulator of context
     * nregs - number of registers in context
     * regs - registers of context
     * data - ark::ExtFrame language-related extension data
     */
    coretypes::TaggedValue InvokeContext(ManagedThread *thread, const uint8_t *pc, coretypes::TaggedValue *acc,
                                         uint32_t nregs, coretypes::TaggedValue *regs);

    template <class InvokeHelper>
    coretypes::TaggedValue InvokeContext(ManagedThread *thread, const uint8_t *pc, coretypes::TaggedValue *acc,
                                         uint32_t nregs, coretypes::TaggedValue *regs);

    /*
     * Create new frame for native method, but don't start execution
     * Number of arguments may vary, all arguments must be of type coretypes::TaggedValue.
     * args - array of arguments. The first value must be the callee function object
     * num_vregs - number of registers in frame
     * num_args - length of args array
     * data - ark::ExtFrame language-related extension data
     */
    template <class InvokeHelper, class ValueT>
    Frame *EnterNativeMethodFrame(ManagedThread *thread, uint32_t numVregs, uint32_t numArgs, ValueT *args);

    /*
     * Pop native method frame
     */
    // CC-OFFNXT(G.INC.10) false positive: static method
    static void ExitNativeMethodFrame(ManagedThread *thread);

    Class *GetClass() const
    {
        return reinterpret_cast<Class *>(classWord_);
    }

    void SetClass(Class *cls)
    {
        classWord_ = static_cast<ClassHelper::ClassWordSize>(ToObjPtrType(cls));
    }

    void SetPandaFile(const panda_file::File *file)
    {
        pandaFile_ = file;
    }

    const panda_file::File *GetPandaFile() const
    {
        return pandaFile_;
    }

    panda_file::File::EntityId GetFileId() const
    {
        return fileId_;
    }

    panda_file::File::EntityId GetCodeId() const
    {
        return codeId_;
    }

    inline int16_t GetHotnessCounter() const
    {
        return stor16Pair_.hotnessCounter;
    }

    inline NO_THREAD_SANITIZE void DecrementHotnessCounter()
    {
        --stor16Pair_.hotnessCounter;
    }

    inline int16_t GetSaverTryCounter() const
    {
        return saverTryCounter_;
    }

    inline NO_THREAD_SANITIZE void DecrementSaverTryCounter()
    {
        --saverTryCounter_;
    }

    inline NO_THREAD_SANITIZE void ResetSaverTryCounter()
    {
        // To induce call of Runtime::TryCreateSaverTask since it may cost 70% of execution time
        // change 70% to (70 / 4096) / 30 * 100% = 0.05%
        const int16_t initialCounter = 4096;
        saverTryCounter_ = initialCounter;
    }

    void TryCreateSaverTask();

    // CC-OFFNXT(G.INC.10) false positive: static method
    static NO_THREAD_SANITIZE int16_t GetInitialHotnessCounter();

    NO_THREAD_SANITIZE void ResetHotnessCounter();

    template <class AccVRegisterPtrT>
    NO_THREAD_SANITIZE void SetAcc([[maybe_unused]] AccVRegisterPtrT acc);
    template <class AccVRegisterPtrT>
    NO_THREAD_SANITIZE void SetAcc([[maybe_unused]] ManagedThread *thread, [[maybe_unused]] AccVRegisterPtrT acc);

    // NO_THREAD_SANITIZE because of perfomance degradation (see commit 7c913cb1 and MR 997#note_113500)
    template <bool IS_CALL, class AccVRegisterPtrT>
    NO_THREAD_SANITIZE bool DecrementHotnessCounter(uintptr_t bytecodeOffset, [[maybe_unused]] AccVRegisterPtrT cc,
                                                    bool osr = false,
                                                    coretypes::TaggedValue func = coretypes::TaggedValue::Hole());

    template <bool IS_CALL, class AccVRegisterPtrT>
    NO_THREAD_SANITIZE bool DecrementHotnessCounter(ManagedThread *thread, uintptr_t bcOffset,
                                                    [[maybe_unused]] AccVRegisterPtrT cc, bool osr = false,
                                                    coretypes::TaggedValue func = coretypes::TaggedValue::Hole());

    // NOTE(xucheng): change the input type to uint16_t when we don't input the max num of int32_t
    inline NO_THREAD_SANITIZE void SetHotnessCounter(uint32_t counter)
    {
        stor16Pair_.hotnessCounter = static_cast<uint16_t>(counter);
    }

    PANDA_PUBLIC_API int64_t GetBranchTakenCounter(uint32_t pc);
    PANDA_PUBLIC_API int64_t GetBranchNotTakenCounter(uint32_t pc);

    int64_t GetThrowTakenCounter(uint32_t pc);

    const void *GetCompiledEntryPoint()
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return compiledEntryPoint_.load(std::memory_order_acquire);
    }

    const void *GetCompiledEntryPoint() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return compiledEntryPoint_.load(std::memory_order_acquire);
    }

    void SetCompiledEntryPoint(const void *entryPoint)
    {
        // Atomic with release order reason: data race with compiled_entry_point_ with dependecies on writes before the
        // store which should become visible acquire
        compiledEntryPoint_.store(entryPoint, std::memory_order_release);
    }

    void SetInterpreterEntryPoint()
    {
        if (!IsNative()) {
            SetCompiledEntryPoint(GetCompiledCodeToInterpreterBridge(this));
        }
    }

    bool HasCompiledCode() const
    {
        auto entryPoint = GetCompiledEntryPoint();
        return entryPoint != GetCompiledCodeToInterpreterBridge() &&
               entryPoint != GetCompiledCodeToInterpreterBridgeDyn();
    }

    inline CompilationStage GetCompilationStatus() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return static_cast<CompilationStage>((accessFlags_.load(std::memory_order_acquire) & COMPILATION_STATUS_MASK) >>
                                             COMPILATION_STATUS_SHIFT);
    }

    inline CompilationStage GetCompilationStatus(uint32_t value)
    {
        return static_cast<CompilationStage>((value & COMPILATION_STATUS_MASK) >> COMPILATION_STATUS_SHIFT);
    }

    inline void SetCompilationStatus(enum CompilationStage newStatus)
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        auto result = (accessFlags_.load(std::memory_order_acquire) & ~COMPILATION_STATUS_MASK) |
                      static_cast<uint32_t>(newStatus) << COMPILATION_STATUS_SHIFT;
        // Atomic with release order reason: data race with access_flags_ with dependecies on writes before the store
        // which should become visible acquire
        accessFlags_.store(result, std::memory_order_release);
    }

    inline bool AtomicSetCompilationStatus(enum CompilationStage oldStatus, enum CompilationStage newStatus)
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        uint32_t oldValue = accessFlags_.load(std::memory_order_acquire);
        while (GetCompilationStatus(oldValue) == oldStatus) {
            uint32_t newValue = MakeCompilationStatusValue(oldValue, newStatus);
            if (accessFlags_.compare_exchange_strong(oldValue, newValue)) {
                return true;
            }
        }
        return false;
    }

    panda_file::Type GetReturnType() const;

    panda_file::File::StringData GetRefReturnType() const;

    // idx - index number of the argument in the signature
    PANDA_PUBLIC_API panda_file::Type GetArgType(size_t idx) const;

    PANDA_PUBLIC_API panda_file::File::StringData GetRefArgType(size_t idx) const;

    template <typename Callback>
    void EnumerateTypes(Callback handler) const;

    PANDA_PUBLIC_API panda_file::File::StringData GetName() const;

    PANDA_PUBLIC_API panda_file::File::StringData GetClassName() const;

    PANDA_PUBLIC_API PandaString GetFullName(bool withSignature = false) const;
    PANDA_PUBLIC_API PandaString GetLineNumberAndSourceFile(uint32_t bcOffset) const;

    // CC-OFFNXT(G.INC.10) false positive: static method
    static uint32_t GetFullNameHashFromString(const PandaString &str);
    // CC-OFFNXT(G.INC.10) false positive: static method
    static uint32_t GetClassNameHashFromString(const PandaString &str);

    PANDA_PUBLIC_API Proto GetProto() const;

    PANDA_PUBLIC_API ProtoId GetProtoId() const;

    size_t GetFrameSize() const
    {
        return Frame::GetAllocSize(GetNumArgs() + GetNumVregs(), EMPTY_EXT_FRAME_DATA_SIZE);
    }

    uint32_t GetNumericalAnnotation(AnnotationField fieldId) const;
    panda_file::File::StringData GetStringDataAnnotation(AnnotationField fieldId) const;

    uint32_t GetAccessFlags() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return accessFlags_.load(std::memory_order_acquire);
    }

    void SetAccessFlags(uint32_t accessFlags)
    {
        // Atomic with release order reason: data race with access_flags_ with dependecies on writes before the store
        // which should become visible acquire
        accessFlags_.store(accessFlags, std::memory_order_release);
    }

    bool IsStatic() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_STATIC) != 0;
    }

    bool IsNative() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_NATIVE) != 0;
    }

    bool IsPublic() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_PUBLIC) != 0;
    }

    bool IsPrivate() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_PRIVATE) != 0;
    }

    bool IsProtected() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_PROTECTED) != 0;
    }

    bool IsIntrinsic() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_INTRINSIC) != 0;
    }

    bool IsSynthetic() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_SYNTHETIC) != 0;
    }

    bool IsAbstract() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_ABSTRACT) != 0;
    }

    bool IsFinal() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_FINAL) != 0;
    }

    bool IsSynchronized() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_SYNCHRONIZED) != 0;
    }

    bool IsCriticalNative() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_CRITICAL_NATIVE) != 0;
    }

    bool HasVarArgs() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_VARARGS) != 0;
    }

    bool HasSingleImplementation() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_SINGLE_IMPL) != 0;
    }

    bool IsProfiled() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_PROFILING) != 0;
    }

    bool IsDestroyed() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_DESTROYED) != 0;
    }

    void SetHasSingleImplementation(bool v)
    {
        if (v) {
            // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load
            // and on writes before the store
            accessFlags_.fetch_or(ACC_SINGLE_IMPL, std::memory_order_acq_rel);
        } else {
            // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load
            // and on writes before the store
            accessFlags_.fetch_and(~ACC_SINGLE_IMPL, std::memory_order_acq_rel);
        }
    }

    void SetProfiled()
    {
        ASSERT(!IsIntrinsic());
        // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load
        // and on writes before the store
        accessFlags_.fetch_or(ACC_PROFILING, std::memory_order_acq_rel);
    }

    void SetDestroyed()
    {
        ASSERT(!IsIntrinsic());
        // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load
        // and on writes before the store
        accessFlags_.fetch_or(ACC_DESTROYED, std::memory_order_acq_rel);
    }

    Method *GetSingleImplementation()
    {
        return HasSingleImplementation() ? this : nullptr;
    }

    void SetIntrinsic(intrinsics::Intrinsic intrinsic)
    {
        ASSERT(!IsIntrinsic());

        // Atomic with relaxed order reason: there is a release store below, which will make this store visible
        // for other threads
        intrinsicId_.store(static_cast<uint32_t>(intrinsic), std::memory_order_relaxed);
        // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load and
        // on writes before the store
        accessFlags_.fetch_or(ACC_INTRINSIC, std::memory_order_acq_rel);
    }

    intrinsics::Intrinsic GetIntrinsic() const
    {
        ASSERT(IsIntrinsic());
        // Atomic with acquire order reason: data race with intrinsicId_ with dependecies on reads after the load which
        // should become visible
        return static_cast<intrinsics::Intrinsic>(intrinsicId_.load(std::memory_order_acquire));
    }

    void SetVTableIndex(uint16_t vtableIndex)
    {
        stor16Pair_.vtableIndex = vtableIndex;
    }

    uint16_t GetVTableIndex() const
    {
        return stor16Pair_.vtableIndex;
    }

    void SetNativePointer(void *nativePointer)
    {
        ASSERT((IsNative() || IsProxy()));
        // Atomic with relaxed order reason: data race with native_pointer_ with no synchronization or ordering
        // constraints imposed on other reads or writes NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        pointer_.nativePointer.store(nativePointer, std::memory_order_relaxed);
    }

    void *GetNativePointer() const
    {
        ASSERT((IsNative() || IsProxy()));
        // Atomic with relaxed order reason: data race with native_pointer_ with no synchronization or ordering
        // constraints imposed on other reads or writes NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.nativePointer.load(std::memory_order_relaxed);
    }

    const uint16_t *GetShorty() const
    {
        return shorty_;
    }

    uint32_t FindCatchBlockInPandaFile(const Class *cls, uint32_t pc) const;
    uint32_t FindCatchBlock(const Class *cls, uint32_t pc) const;

    PANDA_PUBLIC_API panda_file::Type GetEffectiveArgType(size_t idx) const;

    PANDA_PUBLIC_API panda_file::Type GetEffectiveReturnType() const;

    void SetIsDefaultInterfaceMethod()
    {
        // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load and
        // on writes before the store
        accessFlags_.fetch_or(ACC_DEFAULT_INTERFACE_METHOD, std::memory_order_acq_rel);
    }

    bool IsDefaultInterfaceMethod() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_DEFAULT_INTERFACE_METHOD) != 0;
    }

    bool IsConstructor() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (accessFlags_.load(std::memory_order_acquire) & ACC_CONSTRUCTOR) != 0;
    }

    bool IsInstanceConstructor() const
    {
        return IsConstructor() && !IsStatic();
    }

    bool IsStaticConstructor() const
    {
        return IsConstructor() && IsStatic();
    }

    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetAccessFlagsOffset()
    {
        return MEMBER_OFFSET(Method, accessFlags_);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetNumArgsOffset()
    {
        return MEMBER_OFFSET(Method, numArgs_);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetVTableIndexOffset()
    {
        return MEMBER_OFFSET(Method, stor16Pair_) + MEMBER_OFFSET(Storage16Pair, vtableIndex);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetHotnessCounterOffset()
    {
        return MEMBER_OFFSET(Method, stor16Pair_) + MEMBER_OFFSET(Storage16Pair, hotnessCounter);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetClassOffset()
    {
        return MEMBER_OFFSET(Method, classWord_);
    }

    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetCompiledEntryPointOffset()
    {
        return MEMBER_OFFSET(Method, compiledEntryPoint_);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetPandaFileOffset()
    {
        return MEMBER_OFFSET(Method, pandaFile_);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetCodeIdOffset()
    {
        return MEMBER_OFFSET(Method, codeId_);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetNativePointerOffset()
    {
        return MEMBER_OFFSET(Method, pointer_);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetShortyOffset()
    {
        return MEMBER_OFFSET(Method, shorty_);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetNumVregsOffset()
    {
        return MEMBER_OFFSET(Method, numVregs_);
    }
    // CC-OFFNXT(G.INC.10) false positive: static method
    static constexpr uint32_t GetInstructionsOffset()
    {
        return MEMBER_OFFSET(Method, instructions_);
    }

    template <typename Callback>
    void EnumerateTryBlocks(Callback callback) const;

    template <typename Callback>
    void EnumerateCatchBlocks(Callback callback) const;

    template <typename Callback>
    void EnumerateExceptionHandlers(Callback callback) const;

    // CC-OFFNXT(G.INC.10) false positive: static method
    static inline UniqId CalcUniqId(const panda_file::File *file, panda_file::File::EntityId fileId)
    {
        constexpr uint64_t HALF = 32ULL;
        uint64_t uid = file->GetUniqId();
        uid <<= HALF;
        uid |= fileId.GetOffset();
        return uid;
    }

    // for synthetic methods, like array .ctor
    // CC-OFFNXT(G.INC.10) false positive: static method
    static UniqId CalcUniqId(const uint8_t *classDescr, const uint8_t *name);

    UniqId GetUniqId() const
    {
        return CalcUniqId(pandaFile_, fileId_);
    }

    size_t GetLineNumFromBytecodeOffset(uint32_t bcOffset) const;

    panda_file::File::StringData GetClassSourceFile() const;

    inline bool InitProfilingData(ProfilingData *profilingData);

    PANDA_PUBLIC_API void StartProfiling();
    PANDA_PUBLIC_API void StopProfiling();

    PANDA_PUBLIC_API bool IsProxy() const;

    ProfilingData *GetProfilingData()
    {
        if (UNLIKELY(IsNative() || IsProxy())) {
            return nullptr;
        }
        // Atomic with acquire order reason: data race with profiling_data_ with dependecies on reads after the load
        // which should become visible NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.profilingData.load(std::memory_order_acquire);
    }

    ProfilingData *GetProfilingDataWithoutCheck()
    {
        // Atomic with acquire order reason: data race with profiling_data_ with dependecies on reads after the load
        // which should become visible NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.profilingData.load(std::memory_order_acquire);
    }

    const ProfilingData *GetProfilingData() const
    {
        if (UNLIKELY(IsNative() || IsProxy())) {
            return nullptr;
        }
        // Atomic with acquire order reason: data race with profiling_data_ with dependecies on reads after the load
        // which should become visible NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.profilingData.load(std::memory_order_acquire);
    }

    bool IsProfiling() const
    {
        return GetProfilingData() != nullptr;
    }

    bool IsProfilingWithoutLock() const
    {
        if (UNLIKELY(IsNative() || IsProxy())) {
            return false;
        }
        // Atomic with acquire order reason: data race with profiling_data_ with dependecies on reads after the load
        // which should become visible NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.profilingData.load(std::memory_order_acquire) != nullptr;
    }

    void SetVerified(bool result);
    bool IsVerified() const;
    PANDA_PUBLIC_API bool Verify();
    template <bool IS_CALL>
    bool TryVerify();

    // CC-OFFNXT(G.INC.10) false positive: static method
    inline static VerificationStage GetVerificationStage(uint32_t value)
    {
        return static_cast<VerificationStage>((value & VERIFICATION_STATUS_MASK) >> VERIFICATION_STATUS_SHIFT);
    }

    inline VerificationStage GetVerificationStage() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return GetVerificationStage(accessFlags_.load(std::memory_order_acquire));
    }

    inline void SetVerificationStage(enum VerificationStage newStage)
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        uint32_t oldValue = accessFlags_.load(std::memory_order_acquire);
        uint32_t newValue = MakeVerificationStageValue(oldValue, newStage);
        while (!accessFlags_.compare_exchange_weak(oldValue, newValue, std::memory_order_acq_rel)) {
            newValue = MakeVerificationStageValue(oldValue, newStage);
        }
    }

    static constexpr uint16_t NUM_VREGS_UNKNOWN = UINT16_MAX;

private:
    inline void FillVecsByInsts(BytecodeInstruction &inst, PandaVector<uint32_t> &vcalls,
                                PandaVector<uint32_t> &branches, PandaVector<uint32_t> &throws) const;

    Value InvokeCompiledCode(ManagedThread *thread, uint32_t numArgs, Value *args);

    Value GetReturnValueFromTaggedValue(uint64_t retValue)
    {
        panda_file::Type retType = GetReturnType();
        if (retType.GetId() == panda_file::Type::TypeId::VOID) {
            return Value(static_cast<int64_t>(0));
        }
        if (retType.GetId() == panda_file::Type::TypeId::REFERENCE) {
            return Value(reinterpret_cast<ObjectHeader *>(retValue));
        }
        return Value(retValue);
    }

    // CC-OFFNXT(G.INC.10) false positive: static method
    inline static uint32_t MakeCompilationStatusValue(uint32_t value, CompilationStage newStatus)
    {
        value &= ~COMPILATION_STATUS_MASK;
        value |= static_cast<uint32_t>(newStatus) << COMPILATION_STATUS_SHIFT;
        return value;
    }

    // CC-OFFNXT(G.INC.10) false positive: static method
    inline static uint32_t MakeVerificationStageValue(uint32_t value, VerificationStage newStage)
    {
        value &= ~VERIFICATION_STATUS_MASK;
        value |= static_cast<uint32_t>(newStage) << VERIFICATION_STATUS_SHIFT;
        return value;
    }

    template <class InvokeHelper, class ValueT>
    ValueT InvokeInterpretedCode(ManagedThread *thread, uint32_t numActualArgs, ValueT *args);

    template <class InvokeHelper, class ValueT>
    PandaUniquePtr<Frame, FrameDeleter> InitFrame(ManagedThread *thread, uint32_t numActualArgs, ValueT *args,
                                                  Frame *currentFrame);

    template <class InvokeHelper, class ValueT, bool IS_NATIVE_METHOD>
    PandaUniquePtr<Frame, FrameDeleter> InitFrameWithNumVRegs(ManagedThread *thread, uint32_t numVregs,
                                                              uint32_t numActualArgs, ValueT *args,
                                                              Frame *currentFrame);

    template <class InvokeHelper, class ValueT>
    ValueT GetReturnValueFromException();

    template <class InvokeHelper, class ValueT>
    ValueT GetReturnValueFromAcc(interpreter::AccVRegister &aacVreg);

    template <class InvokeHelper, class ValueT>
    ValueT InvokeImpl(ManagedThread *thread, uint32_t numActualArgs, ValueT *args, bool proxyCall);

    template <bool IS_CALL>
    inline bool DecrementHotnessCounterForTaggedFunction(ManagedThread *thread, uintptr_t bcOffset, bool osr,
                                                         coretypes::TaggedValue func);

private:
    using IntrinsicIdType = uint32_t;
    static_assert(std::numeric_limits<IntrinsicIdType>::max() == MAX_INTRINSIC_NUMBER);

    union PointerInMethod {
        // It's native pointer when the method is native or proxy method.
        std::atomic<void *> nativePointer;
        // It's profiling data when the method isn't native or proxy method.
        std::atomic<ProfilingData *> profilingData;
    };

    struct Storage16Pair {
        uint16_t vtableIndex;
        int16_t hotnessCounter;
    };

    std::atomic_uint32_t accessFlags_;
    uint32_t numArgs_;
    Storage16Pair stor16Pair_;
    ClassHelper::ClassWordSize classWord_;

    std::atomic<const void *> compiledEntryPoint_ {nullptr};
    const panda_file::File *pandaFile_;
    union PointerInMethod pointer_ {
    };

    panda_file::File::EntityId fileId_;
    panda_file::File::EntityId codeId_;
    const uint16_t *shorty_;
    int16_t saverTryCounter_ {0};

    std::atomic<IntrinsicIdType> intrinsicId_ {0U};

    // The instructions_ and numVregs_ fields are used as caches for fast access to corresponding data.
    // Declared as mutable because they employ lazy (on-demand) initialization,
    // which requires modification even in const member functions.
    // The atomic types ensure thread-safe access during concurrent operations.
    mutable std::atomic<const uint8_t *> instructions_;
    mutable std::atomic_uint32_t numVregs_;
};

static_assert(!std::is_polymorphic_v<Method>);

}  // namespace ark

#endif  // PANDA_RUNTIME_METHOD_H_
