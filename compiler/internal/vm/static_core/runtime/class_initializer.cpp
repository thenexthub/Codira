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

#include "runtime/class_initializer.h"

#include "include/object_header.h"
#include "libarkfile/file_items.h"
#include "libarkbase/macros.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/monitor.h"
#include "runtime/monitor_object_lock.h"
#include "runtime/class_lock.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "verification/util/is_system.h"
#include "verify_app_install.h"

namespace ark {
template <MTModeT MODE>
class ObjectLockConfig {
};

template <>
class ObjectLockConfig<MT_MODE_MULTI> {
public:
    using LockT = ObjectLock;
};

/// The "TASK" multithreading mode implies that M coroutines are running on N worker threads
template <>
class ObjectLockConfig<MT_MODE_TASK> {
public:
    /**
     * Note(shemetov.philip)
     * At the moment, we are using per-class locks. We imply that there will be no
     * coroutine switch during the class initialization and that assumption includes static constructor bodies too.
     * With a per-class lock, we have some global synchronization for accessing the mutex table, so coroutine switch
     * during a class initialiation sequence will possibly lead to deadlocks and other failures, so expect various
     * checkers to fire and warn you in case a coroutine switch is detected.
     *
     */
    using LockT = ClassLock;
};

template <>
class ObjectLockConfig<MT_MODE_SINGLE> {
public:
    class DummyObjectLock {
    public:
        explicit DummyObjectLock(ObjectHeader *header [[maybe_unused]]) {}
        ~DummyObjectLock() = default;
        bool Wait([[maybe_unused]] bool ignoreInterruption = false)
        {
            return true;
        }
        bool TimedWait([[maybe_unused]] uint64_t timeout)
        {
            return true;
        }
        void Notify() {}
        void NotifyAll() {}
        NO_COPY_SEMANTIC(DummyObjectLock);
        NO_MOVE_SEMANTIC(DummyObjectLock);
    };

    using LockT = DummyObjectLock;
};

/// Does nothing in MT_MODE_SINGLE and MT_MODE_MULTI
template <MTModeT MODE>
class ClassInitGuard {
public:
    using Guard = struct Dummy {
        explicit Dummy(ThreadManager *tm)
        {
            // GCC 8 and below has a strange bug: it reports a false syntax error in case
            // when [[maybe_unused]] is set for the first constructor argument.
            // NOTE(konstanting): revert to [[maybe_unused]] when we do not use GCC<=8
            UNUSED_VAR(tm);
        }
    };
};

/// Disables coroutine switch in MT_MODE_TASK (required by the current synchronization scheme)
template <>
class ClassInitGuard<MT_MODE_TASK> {
public:
    using Guard = class Adapter {
    public:
        explicit Adapter(ThreadManager *tm) : s_(static_cast<CoroutineManager *>(tm)) {}

    private:
        ScopedDisableCoroutineSwitch s_;
    };
};

static void WrapException(ClassLinker *classLinker, ManagedThread *thread)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*thread->GetException()->ClassAddr<Class>());
    ctx.WrapClassInitializerException(classLinker, thread);
}

static void ThrowNoClassDefFoundError(ManagedThread *thread, const Class *klass)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto name = klass->GetName();
    ThrowException(ctx, thread, ctx.GetNoClassDefFoundErrorDescriptor(), utf::CStringAsMutf8(name.c_str()));
}

static void ThrowEarlierInitializationException(ManagedThread *thread, const Class *klass)
{
    ASSERT(klass->IsErroneous());

    ThrowNoClassDefFoundError(thread, klass);
}

static void ThrowIncompatibleClassChangeError(ManagedThread *thread, const Class *klass)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto name = klass->GetName();
    ThrowException(ctx, thread, ctx.GetIncompatibleClassChangeErrorDescriptor(), utf::CStringAsMutf8(name.c_str()));
}

static void ThrowVerifyError(ManagedThread *thread, const Class *klass)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto name = klass->GetName();
    ThrowException(ctx, thread, ctx.GetVerifyErrorClassDescriptor(), utf::CStringAsMutf8(name.c_str()));
}

static bool IsBadSuperClass(const Class *base, ManagedThread *thread, const Class *klass)
{
    if (base->IsInterface()) {
        ThrowIncompatibleClassChangeError(thread, klass);
        return true;
    }

    if (!base->IsExtensible() && !klass->IsStringClass()) {
        ThrowVerifyError(thread, klass);
        return true;
    }

    return false;
}

template <class ObjectLockT>
static bool WaitInitialization(ObjectLockT *lock, ClassLinker *classLinker, ManagedThread *thread, Class *klass)
{
    while (true) {
        // Should be possible to interrupt Wait for termination
        auto state = lock->Wait(false);
        if (!state && thread->IsRuntimeTerminated()) {
            // If daemon threads are terminated then wait may be interrupted,
            // otherwise continue waiting
            ThrowNoClassDefFoundError(thread, klass);
            klass->SetState(Class::State::ERRONEOUS);
            return false;
        }

        if (thread->HasPendingException()) {
            WrapException(classLinker, thread);
            klass->SetState(Class::State::ERRONEOUS);
            return false;
        }

        if (klass->IsInitializing()) {
            continue;
        }

        if (klass->IsErroneous()) {
            ThrowNoClassDefFoundError(thread, klass);
            return false;
        }

        if (klass->IsInitialized()) {
            return true;
        }

        UNREACHABLE();
    }
}

/* static */
template <MTModeT MODE>
bool ClassInitializer<MODE>::Initialize(ClassLinker *classLinker, ManagedThread *thread, Class *klass)
{
    if (klass->IsInitialized()) {
        return true;
    }

    // embraces the class init sequence with some checkers in the MT_MODEs where it is required
    [[maybe_unused]] typename ClassInitGuard<MODE>::Guard guard(thread->GetVM()->GetThreadManager());

    using ObjectLockT = typename ObjectLockConfig<MODE>::LockT;

    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> managedClassObjHandle(thread, klass->GetManagedObject());
    {
        ObjectLockT lock(managedClassObjHandle.GetPtr());

        if (klass->IsInitialized()) {
            return true;
        }

        if (klass->IsErroneous()) {
            ThrowEarlierInitializationException(thread, klass);
            return false;
        }

        if (!InitClassVerificationMode(klass)) {
            return false;
        }

        if (klass->IsInitializing()) {
            if constexpr (MODE == MT_MODE_TASK) {
                if (klass->GetInitTid() == Coroutine::CastFromThread(thread)->GetCoroutineId()) {
                    return true;
                }
            } else {
                if (klass->GetInitTid() == thread->GetId()) {
                    return true;
                }
            }

            if constexpr ((MODE == MT_MODE_MULTI) || (MODE == MT_MODE_TASK)) {
                return WaitInitialization(&lock, classLinker, thread, klass);
            }

            UNREACHABLE();
        }

        if constexpr (MODE == MT_MODE_TASK) {
            klass->SetInitTid(Coroutine::CastFromThread(thread)->GetCoroutineId());
        } else {
            klass->SetInitTid(thread->GetId());
        }
        klass->SetState(Class::State::INITIALIZING);
        if (!ClassInitializer::InitializeFields(klass)) {
            LOG(ERROR, CLASS_LINKER) << "Cannot initialize fields of class '" << klass->GetName() << "'";
            return false;
        }
    }

    LOG(DEBUG, CLASS_LINKER) << "Initializing class " << klass->GetName();

    return InitializeClass(classLinker, thread, klass, managedClassObjHandle);
}

template <MTModeT MODE>
bool ClassInitializer<MODE>::InitClassVerificationMode(Class *klass)
{
    const auto &options = Runtime::GetCurrent()->GetOptions();
    switch (verifier::VerificationModeFromString(options.GetVerificationMode())) {
        case verifier::VerificationMode::DISABLED:
            if (!klass->IsVerified()) {
                klass->SetState(Class::State::VERIFIED);
            }
            return true;
        case verifier::VerificationMode::AHEAD_OF_TIME:
            if (!klass->IsVerified()) {
                if (!VerifyClass(klass)) {
                    klass->SetState(Class::State::ERRONEOUS);
                    ark::ThrowVerificationException(utf::Mutf8AsCString(klass->GetDescriptor()));
                    return false;
                }
            }
            return true;
        case verifier::VerificationMode::ON_THE_FLY:
            if (options.IsArkAot()) {
                LOG(FATAL, VERIFIER) << "On the fly verification mode is not compatible with ark_aot";
            }
            return true;
        default:
            UNREACHABLE();
    }
}

/* static */
template <MTModeT MODE>
bool ClassInitializer<MODE>::InitializeClass(ClassLinker *classLinker, ManagedThread *thread, Class *klass,
                                             const VMHandle<ObjectHeader> &managedClassObjHandle)
{
    using ObjectLockT = typename ObjectLockConfig<MODE>::LockT;

    if (!klass->IsInterface()) {
        auto *base = klass->GetBase();

        if (base != nullptr) {
            if (IsBadSuperClass(base, thread, klass)) {
                return false;
            }

            if (!Initialize(classLinker, thread, base)) {
                ObjectLockT lock(managedClassObjHandle.GetPtr());
                klass->SetState(Class::State::ERRONEOUS);
                lock.NotifyAll();
                return false;
            }
        }

        for (auto *iface : klass->GetInterfaces()) {
            if (iface->IsInitialized()) {
                continue;
            }

            if (!InitializeInterface(classLinker, thread, iface, klass)) {
                ObjectLockT lock(managedClassObjHandle.GetPtr());
                klass->SetState(Class::State::ERRONEOUS);
                lock.NotifyAll();
                return false;
            }
        }
    }

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID)},
                        Method::Proto::RefTypeVector {});
    auto *cctorName = ctx.GetCctorName();
    auto *cctor = klass->GetDirectMethod(cctorName, proto);
    if (cctor != nullptr) {
        cctor->InvokeVoid(thread, nullptr);
    }

    {
        ObjectLockT lock(managedClassObjHandle.GetPtr());

        if (thread->HasPendingException()) {
            LOG(WARNING, CLASS_LINKER) << "There was an exception "
                                       << thread->GetException()->ClassAddr<Class>()->GetName()
                                       << " when initializing class '" << klass->GetName() << "'";
            WrapException(classLinker, thread);
            klass->SetState(Class::State::ERRONEOUS);
            lock.NotifyAll();
            return false;
        }

        klass->SetState(Class::State::INITIALIZED);

        lock.NotifyAll();
    }

    return true;
}

/* static */
template <MTModeT MODE>
bool ClassInitializer<MODE>::InitializeInterface(ClassLinker *classLinker, ManagedThread *thread, Class *iface,
                                                 Class *klass)
{
    if (!iface->IsInterface()) {
        ThrowIncompatibleClassChangeError(thread, klass);
        return false;
    }

    for (auto *baseIface : iface->GetInterfaces()) {
        if (baseIface->IsInitialized()) {
            continue;
        }

        if (!InitializeInterface(classLinker, thread, baseIface, klass)) {
            return false;
        }
    }

    if (!iface->HasDefaultMethods()) {
        return true;
    }

    return Initialize(classLinker, thread, iface);
}

/* static */
template <MTModeT MODE>
bool ClassInitializer<MODE>::VerifyClass(Class *klass)
{
    ASSERT(!klass->IsVerified());

    auto &opts = Runtime::GetCurrent()->GetOptions();

    if (!IsVerifySuccInAppInstall(klass->GetPandaFile())) {
        LOG(ERROR, VERIFIER) << "Class verification failed: '" << klass->GetName() << "'";
        return false;
    }

    bool skipVerification = !opts.IsVerifyRuntimeLibraries() && verifier::IsSystemOrSyntheticClass(klass);
    if (skipVerification) {
        for (auto &method : klass->GetMethods()) {
            method.SetVerified(true);
        }
        klass->SetState(Class::State::VERIFIED);
        return true;
    }

    LOG(INFO, VERIFIER) << "Start verification of class '" << klass->GetName() << "'";

    for (auto &method : klass->GetMethods()) {
        if (!method.Verify()) {
            LOG(ERROR, VERIFIER) << "Class verification failed: '" << klass->GetName()
                                 << "' method: " << method.GetFullName(true);
            return false;
        }
    }

    klass->SetState(Class::State::VERIFIED);
    return true;
}

template <class T>
static void InitializePrimitiveField(Class *klass, const Field &field)
{
    panda_file::FieldDataAccessor fda(*field.GetPandaFile(), field.GetFileId());
    auto value = fda.GetValue<T>();
    klass->SetFieldPrimitive<T>(field, value ? value.value() : 0);
}

static void InitializeTaggedField(Class *klass, const Field &field)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    klass->SetFieldPrimitive<coretypes::TaggedValue>(field, ctx.GetInitialTaggedValue());
}

static void InitializeStringField(Class *klass, const Field &field)
{
    panda_file::FieldDataAccessor fda(*field.GetPandaFile(), field.GetFileId());
    auto value = fda.GetValue<uint32_t>();
    if (value) {
        panda_file::File::EntityId id(value.value());
        coretypes::String *str = Runtime::GetCurrent()->GetPandaVM()->ResolveString(*klass->GetPandaFile(), id);
        if (LIKELY(str != nullptr)) {
            klass->SetFieldObject(field, str);
            return;
        }
    }
    // Should nullptr be set?
    klass->SetFieldObject(field, nullptr);
}

/* static */
template <MTModeT MODE>
bool ClassInitializer<MODE>::InitializeFields(Class *klass)
{
    using Type = panda_file::Type;

    for (const auto &field : klass->GetStaticFields()) {
        switch (field.GetTypeId()) {
            case Type::TypeId::U1:
            case Type::TypeId::U8:
                InitializePrimitiveField<uint8_t>(klass, field);
                break;
            case Type::TypeId::I8:
                InitializePrimitiveField<int8_t>(klass, field);
                break;
            case Type::TypeId::I16:
                InitializePrimitiveField<int16_t>(klass, field);
                break;
            case Type::TypeId::U16:
                InitializePrimitiveField<uint16_t>(klass, field);
                break;
            case Type::TypeId::I32:
                InitializePrimitiveField<int32_t>(klass, field);
                break;
            case Type::TypeId::U32:
                InitializePrimitiveField<uint32_t>(klass, field);
                break;
            case Type::TypeId::I64:
                InitializePrimitiveField<int64_t>(klass, field);
                break;
            case Type::TypeId::U64:
                InitializePrimitiveField<uint64_t>(klass, field);
                break;
            case Type::TypeId::F32:
                InitializePrimitiveField<float>(klass, field);
                break;
            case Type::TypeId::F64:
                InitializePrimitiveField<double>(klass, field);
                break;
            case Type::TypeId::TAGGED:
                InitializeTaggedField(klass, field);
                break;
            case Type::TypeId::REFERENCE:
                InitializeStringField(klass, field);
                break;
            default: {
                UNREACHABLE();
                break;
            }
        }
    }

    return true;
}

template class ClassInitializer<MT_MODE_SINGLE>;
template class ClassInitializer<MT_MODE_MULTI>;
template class ClassInitializer<MT_MODE_TASK>;
}  // namespace ark
