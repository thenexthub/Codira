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

#include "runtime/mem/heap_manager.h"
#include "runtime/runtime_helpers.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_atomic_flag.h"
#include "plugins/ets/runtime/types/ets_atomic_int.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_typeapi_type.h"
#include "runtime/include/thread_scopes.h"

#include "runtime/include/stack_walker.h"
#include "runtime/include/thread.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"
#include "types/ets_primitives.h"

#ifndef UINT8_MAX
#define UINT8_MAX (255)
#endif  // UINT8_MAX

namespace ark::ets::intrinsics {

extern "C" EtsInt CountInstancesOfClass(EtsClass *cls)
{
    auto *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coro);
    if (cls == nullptr) {
        return EtsInt(0);
    }
    auto *heapMgr = coro->GetPandaVM()->GetHeapManager();
    return static_cast<EtsInt>(heapMgr->CountInstancesOfClass(cls->GetRuntimeClass()));
}

extern "C" EtsArray *StdCoreStackTraceLines()
{
    auto runtime = Runtime::GetCurrent();
    auto linker = runtime->GetClassLinker();
    auto ext = linker->GetExtension(panda_file::SourceLang::ETS);
    auto klass = ext->GetClassRoot(ClassRoot::ARRAY_STRING);

    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::ETS);

    auto thread = ManagedThread::GetCurrent();
    auto walker = StackWalker::Create(thread);

    std::vector<std::string> lines;

    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        Method *method = stack.GetMethod();
        auto *source = method->GetClassSourceFile().data;
        auto lineNum = method->GetLineNumFromBytecodeOffset(stack.GetBytecodePc());

        if (source == nullptr) {
            source = utf::CStringAsMutf8("<unknown>");
        }

        std::stringstream ss;
        ss << method->GetClass()->GetName() << "." << method->GetName().data << " at " << source << ":" << lineNum;
        lines.push_back(ss.str());
    }

    auto coroutine = Coroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    auto *arr = ark::coretypes::Array::Create(klass, lines.size());

    VMHandle<coretypes::Array> arrayHandle(coroutine, arr);

    for (ark::ArraySizeT i = 0; i < (ark::ArraySizeT)lines.size(); i++) {
        auto *str = coretypes::String::CreateFromMUtf8(utf::CStringAsMutf8(lines[i].data()), lines[i].length(), ctx,
                                                       thread->GetVM());
        arrayHandle.GetPtr()->Set(i, str);
    }

    return reinterpret_cast<EtsArray *>(arrayHandle.GetPtr());
}

extern "C" void StdCorePrintStackTrace()
{
    ark::PrintStackTrace();
}

static PandaString ResolveLibraryName(const PandaString &name)
{
#ifdef PANDA_TARGET_UNIX
    return PandaString("lib") + name + ".so";
#else
    // Unsupported on windows platform
    UNREACHABLE();
#endif  // PANDA_TARGET_UNIX
}

void LoadNativeLibraryHandler(ark::ets::EtsString *name, bool shouldVerifyPermission,
                              ark::ets::EtsString *fileName = nullptr)
{
    ASSERT(name->AsObject()->IsStringClass());
    auto coroutine = EtsCoroutine::GetCurrent();
    if (shouldVerifyPermission) {
        ASSERT(fileName != nullptr);
        ASSERT(fileName->AsObject()->IsStringClass());
    }

    if (name->IsUtf16()) {
        LOG(FATAL, RUNTIME) << "UTF-16 native library pathes are not supported";
        return;
    }

    auto nameStr = name->GetMutf8();
    if (nameStr.empty()) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::FILE_NOT_FOUND_ERROR,
                          "The native library path is empty");
        return;
    }
    PandaString fileNameStr = fileName != nullptr ? fileName->GetMutf8() : "";
    LOG(INFO, RUNTIME) << "load library name is " << nameStr.c_str()
                       << "; shouldVerifyPermission: " << shouldVerifyPermission
                       << "; fileName: " << fileNameStr.c_str();
    ScopedNativeCodeThread snct(coroutine);
    auto env = coroutine->GetEtsNapiEnv();
    if (!coroutine->GetPandaVM()->LoadNativeLibrary(env, ResolveLibraryName(nameStr), shouldVerifyPermission,
                                                    fileNameStr)) {
        ScopedManagedCodeThread smct(coroutine);

        PandaStringStream ss;
        ss << "Cannot load native library " << nameStr;

        ThrowEtsException(coroutine, panda_file_items::class_descriptors::EXCEPTION_IN_INITIALIZER_ERROR, ss.str());
    }
}

extern "C" void LoadLibrary(ark::ets::EtsString *name)
{
    LoadNativeLibraryHandler(name, false);
}

extern "C" void LoadLibraryWithPermissionCheck(ark::ets::EtsString *name, ark::ets::EtsString *fileName)
{
    LoadNativeLibraryHandler(name, true, fileName);
}

extern "C" void StdSystemScheduleCoroutine()
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    if (coro->GetCoroutineManager()->IsCoroutineSwitchDisabled()) {
        ThrowEtsException(coro, panda_file_items::class_descriptors::INVALID_COROUTINE_OPERATION_ERROR,
                          "Cannot switch coroutines in the current context!");
        return;
    }

    ScopedNativeCodeThread nativeScope(coro);
    coro->GetCoroutineManager()->Schedule();
}

extern "C" void StdSystemSetCoroutineSchedulingPolicy(int32_t policy)
{
    constexpr auto POLICIES_MAPPING =
        std::array {CoroutineSchedulingPolicy::ANY_WORKER, CoroutineSchedulingPolicy::NON_MAIN_WORKER};
    ASSERT((policy >= 0) && (static_cast<size_t>(policy) < POLICIES_MAPPING.size()));
    CoroutineSchedulingPolicy newPolicy = POLICIES_MAPPING[policy];

    Coroutine *coro = Coroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *cm = static_cast<CoroutineManager *>(coro->GetVM()->GetThreadManager());
    cm->SetSchedulingPolicy(newPolicy);
}

extern "C" int32_t StdSystemGetCoroutineId()
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    return coro->GetCoroutineId();
}

extern "C" EtsBoolean StdSystemIsMainWorker()
{
    return static_cast<EtsBoolean>(EtsCoroutine::GetCurrent()->GetWorker()->IsMainWorker());
}

extern "C" EtsBoolean StdSystemWorkerHasExternalScheduler()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    return static_cast<EtsBoolean>(coro->GetWorker()->IsExternalSchedulingEnabled());
}

extern "C" void StdSystemScaleWorkersPool(int32_t scaler)
{
    if (UNLIKELY(scaler == 0)) {
        return;
    }
    auto *coro = EtsCoroutine::GetCurrent();
    auto *vm = coro->GetPandaVM();
    auto *runtime = vm->GetRuntime();
    if (scaler > 0) {
        coro->GetManager()->CreateWorkers(scaler, runtime, vm);
        return;
    }
    ScopedNativeCodeThread nativeScope(coro);
    auto *coroutine = Coroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    coroutine->GetManager()->FinalizeWorkers(std::abs(scaler), runtime, vm);
}

static Class *GetTaskPoolClass()
{
    auto *runtime = Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    ClassLinkerExtension *cle = classLinker->GetExtension(SourceLanguage::ETS);
    auto mutf8Name = reinterpret_cast<const uint8_t *>("Lstd/concurrency/taskpool;");
    return cle->GetClass(mutf8Name);
}

extern "C" void StdSystemStopTaskpool()
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("stopAllWorkers", ":V");
    ASSERT(method != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    method->GetPandaMethod()->Invoke(coro, nullptr);
}

extern "C" void StdSystemIncreaseTaskpoolWorkersToN(int32_t workersNum)
{
    if (UNLIKELY(workersNum == 0)) {
        return;
    }
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("increaseWorkersToN", "I:V");
    ASSERT(method != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    std::array args = {Value(workersNum)};
    method->GetPandaMethod()->Invoke(coro, args.data());
}

extern "C" void StdSystemSetTaskPoolTriggerShrinkInterval(int32_t interval)
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("setTaskPoolTriggerShrinkInterval", "I:V");
    ASSERT(method != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    std::array args = {Value(interval)};
    method->GetPandaMethod()->Invoke(coro, args.data());
}

extern "C" void StdSystemSetTaskPoolIdleThreshold(int32_t threshold)
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("setTaskPoolIdleThreshold", "I:V");
    ASSERT(method != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    std::array args = {Value {threshold}};
    method->GetPandaMethod()->Invoke(coro, args.data());
}

extern "C" EtsInt StdSystemGetTaskPoolWorkersNum()
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return EtsInt(-1);
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("getTaskPoolWorkersNum", ":I");
    ASSERT(method != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    ark::Value res = method->GetPandaMethod()->Invoke(coro, nullptr);
    return res.GetAs<int>();
}

extern "C" void StdSystemSetTaskPoolWorkersLimit(int32_t threshold)
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("setTaskPoolWorkersLimit", "I:V");
    ASSERT(method != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    std::array args = {Value {threshold}};
    method->GetPandaMethod()->Invoke(coro, args.data());
}

extern "C" void StdSystemRetriggerTaskPoolShrink()
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("retriggerTaskPoolShrink", ":V");
    ASSERT(method != nullptr);
    auto *coro = EtsCoroutine::GetCurrent();
    method->GetPandaMethod()->Invoke(coro, nullptr);
}

extern "C" void StdSystemAtomicFlagSet(EtsAtomicFlag *instance, EtsBoolean v)
{
    instance->SetValue(v);
}

extern "C" EtsBoolean StdSystemAtomicFlagGet(EtsAtomicFlag *instance)
{
    return instance->GetValue();
}

extern "C" EtsInt EtsEscompatUint8ClampedArrayToUint8Clamped(EtsDouble val)
{
    // Convert the double value to uint8_t with clamping
    if (val <= 0 || std::isnan(val)) {
        return 0;
    }
    if (val > UINT8_MAX) {
        return UINT8_MAX;
    }
    return std::lrint(val);
}

extern "C" void EtsAtomicIntSetValue(EtsObject *atomicInt, EtsInt value)
{
    EtsAtomicInt::FromEtsObject(atomicInt)->SetValue(value);
}

extern "C" EtsInt EtsAtomicIntGetValue(EtsObject *atomicInt)
{
    return EtsAtomicInt::FromEtsObject(atomicInt)->GetValue();
}

extern "C" EtsInt EtsAtomicIntGetAndSet(EtsObject *atomicInt, EtsInt value)
{
    return EtsAtomicInt::FromEtsObject(atomicInt)->GetAndSet(value);
}

extern "C" EtsInt EtsAtomicIntCompareAndSet(EtsObject *atomicInt, EtsInt expected, EtsInt newValue)
{
    return EtsAtomicInt::FromEtsObject(atomicInt)->CompareAndSet(expected, newValue);
}

extern "C" EtsInt EtsAtomicIntFetchAndAdd(EtsObject *atomicInt, EtsInt value)
{
    return EtsAtomicInt::FromEtsObject(atomicInt)->FetchAndAdd(value);
}

extern "C" EtsInt EtsAtomicIntFetchAndSub(EtsObject *atomicInt, EtsInt value)
{
    return EtsAtomicInt::FromEtsObject(atomicInt)->FetchAndSub(value);
}

extern "C" EtsBoolean StdSystemIsExternalTimerEnabled()
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto extTimerOption = ark::ets::ToEtsBoolean(coro->GetManager()->GetConfig().enableExternalTimer);
    return extTimerOption && coro->GetWorker()->IsExternalSchedulingEnabled();
}

extern "C" void StdSystemDumpUhandledFailedJobs()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    ScopedNativeCodeThread snct(coro);
    coro->ProcessUnhandledFailedJobs();
}

static void SetPrimitiveFieldInClass(const char *name, ClassRoot root)
{
    auto *ext = Runtime::GetCurrent()->GetClassLinker()->GetExtension(panda_file::SourceLang::ETS);
    auto *klass = EtsClass::FromRuntimeClass(ext->GetClassRoot(ClassRoot::CLASS));
    auto *primCls = EtsClass::FromRuntimeClass(ext->GetClassRoot(root));
    EtsField *primField = klass->GetStaticFieldIDByName(name, nullptr);
    ASSERT(primField != nullptr);
    klass->SetStaticFieldObject(primField, reinterpret_cast<EtsObject *>(primCls));
}

extern "C" void InitializePrimitivesInClass()
{
    SetPrimitiveFieldInClass("PRIMITIVE_BOOLEAN", ClassRoot::U1);
    SetPrimitiveFieldInClass("PRIMITIVE_BYTE", ClassRoot::I8);
    SetPrimitiveFieldInClass("PRIMITIVE_CHAR", ClassRoot::U16);
    SetPrimitiveFieldInClass("PRIMITIVE_SHORT", ClassRoot::I16);
    SetPrimitiveFieldInClass("PRIMITIVE_INT", ClassRoot::I32);
    SetPrimitiveFieldInClass("PRIMITIVE_LONG", ClassRoot::I64);
    SetPrimitiveFieldInClass("PRIMITIVE_FLOAT", ClassRoot::F32);
    SetPrimitiveFieldInClass("PRIMITIVE_DOUBLE", ClassRoot::F64);
    SetPrimitiveFieldInClass("PRIMITIVE_NUMBER", ClassRoot::F64);
    SetPrimitiveFieldInClass("PRIMITIVE_VOID", ClassRoot::V);
}

}  // namespace ark::ets::intrinsics
