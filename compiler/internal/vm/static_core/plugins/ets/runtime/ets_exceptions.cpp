/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/ets_exceptions.h"

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_error_options.h"

namespace ark::ets {

static EtsClass *GetExceptionClass(EtsCoroutine *coro, const char *classDescriptor)
{
    EtsClassLinker *classLinker = coro->GetPandaVM()->GetClassLinker();
    EtsClass *cls = classLinker->GetClass(classDescriptor, true);
    if (cls == nullptr) {
        LOG(ERROR, CLASS_LINKER) << "Class " << classDescriptor << " not found";
        return nullptr;
    }

    if (!classLinker->InitializeClass(coro, cls)) {
        LOG(ERROR, CLASS_LINKER) << "Class " << classDescriptor << " cannot be initialized";
        return nullptr;
    }
    return cls;
}

static EtsObject *CreateErrorInstance(EtsCoroutine *coro, EtsClass *cls, EtsHandle<EtsString> msg,
                                      EtsHandle<EtsObject> pending)
{
    EtsHandle<EtsErrorOptions> errOptions(coro, EtsErrorOptions::Create(coro));
    if (UNLIKELY(errOptions.GetPtr() == nullptr)) {
        return nullptr;
    }
    errOptions->SetCause(pending.GetPtr());

    EtsHandle<EtsObject> error(coro, EtsObject::Create(cls));
    if (UNLIKELY(error.GetPtr() == nullptr)) {
        return nullptr;
    }

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {panda_file_items::class_descriptors::STRING,
                                                      panda_file_items::class_descriptors::ERROR_OPTIONS});

    EtsMethod *ctor = cls->GetDirectMethod(panda_file_items::CTOR.data(), proto);
    if (ctor == nullptr) {
        LOG(FATAL, RUNTIME) << "No method " << panda_file_items::CTOR << " in class " << cls->GetDescriptor();
        return nullptr;
    }

    std::array args {Value(error.GetPtr()->GetCoreType()), Value(msg.GetPtr()->GetCoreType()),
                     Value(errOptions.GetPtr()->AsObject()->GetCoreType())};
    EtsMethod::ToRuntimeMethod(ctor)->InvokeVoid(coro, args.data());

    if (UNLIKELY(coro->HasPendingException())) {
        return nullptr;
    }
    return error.GetPtr();
}

EtsObject *SetupEtsException(EtsCoroutine *coro, const char *classDescriptor, const char *msg)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObject> pending(coro, EtsObject::FromCoreType(coro->GetException()));
    coro->ClearException();

    // NOTE(mgroshev): In case of usage of mock_stdlib.pa we do not support creation of exceptions
    ASSERT(Runtime::GetOptions().ShouldInitializeIntrinsics());

    EtsHandle<EtsString> etsMsg(coro,
                                msg == nullptr ? EtsString::CreateNewEmptyString() : EtsString::CreateFromMUtf8(msg));
    if (UNLIKELY(etsMsg.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    EtsClass *cls = GetExceptionClass(coro, classDescriptor);
    if (UNLIKELY(cls == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    if (PlatformTypes(coro)->escompatError->IsAssignableFrom(cls)) {
        return CreateErrorInstance(coro, cls, etsMsg, pending);
    }
    LOG(FATAL, RUNTIME) << "Class " << cls->GetDescriptor() << " is not compatible with Error";
    return nullptr;
}

void ThrowEtsException(EtsCoroutine *coroutine, const char *classDescriptor, const char *msg)
{
    ASSERT(coroutine != nullptr);
    ASSERT(coroutine == EtsCoroutine::GetCurrent());

    if (coroutine->IsUsePreAllocObj()) {
        coroutine->SetUsePreAllocObj(false);
        coroutine->SetException(coroutine->GetVM()->GetOOMErrorObject());
        return;
    }

    EtsObject *exc = SetupEtsException(coroutine, classDescriptor, msg);
    if (LIKELY(exc != nullptr)) {
        coroutine->SetException(exc->GetCoreType());
    }
    ASSERT(coroutine->HasPendingException());
}

}  // namespace ark::ets
