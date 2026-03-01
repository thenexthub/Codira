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

#include "plugins/ets/runtime/ets_itable_builder.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "plugins/ets/runtime/ets_vtable_builder.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"

#include <array>

namespace ark::ets {

void EtsLanguageContext::ThrowException(ManagedThread *thread, const uint8_t *mutf8Name, const uint8_t *mutf8Msg) const
{
    EtsCoroutine *coroutine = EtsCoroutine::CastFromThread(thread);
    ThrowEtsException(coroutine, utf::Mutf8AsCString(mutf8Name), utf::Mutf8AsCString(mutf8Msg));
}

PandaUniquePtr<ITableBuilder> EtsLanguageContext::CreateITableBuilder(ClassLinkerErrorHandler *errHandler) const
{
    return MakePandaUnique<EtsITableBuilder>(errHandler);
}

PandaUniquePtr<VTableBuilder> EtsLanguageContext::CreateVTableBuilder(ClassLinkerErrorHandler *errHandler) const
{
    return MakePandaUnique<EtsVTableBuilder>(errHandler);
}

ets::PandaEtsVM *EtsLanguageContext::CreateVM(Runtime *runtime, const RuntimeOptions &options) const
{
    ASSERT(runtime != nullptr);

    auto vm = ets::PandaEtsVM::Create(runtime, options);
    if (!vm) {
        LOG(ERROR, RUNTIME) << vm.Error();
        return nullptr;
    }

    return vm.Value();
}

mem::GC *EtsLanguageContext::CreateGC(mem::GCType gcType, mem::ObjectAllocatorBase *objectAllocator,
                                      const mem::GCSettings &settings) const
{
    return mem::CreateGC<EtsLanguageConfig>(gcType, objectAllocator, settings);
}

void EtsLanguageContext::ThrowStackOverflowException(ManagedThread *thread) const
{
    ASSERT(thread != nullptr);
    EtsCoroutine *coroutine = EtsCoroutine::CastFromThread(thread);
    EtsClassLinker *classLinker = coroutine->GetPandaVM()->GetClassLinker();
    const char *classDescriptor = utf::Mutf8AsCString(GetStackOverflowErrorClassDescriptor());
    EtsClass *cls = classLinker->GetClass(classDescriptor, true);
    ASSERT(cls != nullptr);

    EtsHandleScope scope(coroutine);
    EtsHandle<EtsObject> exc(coroutine, EtsObject::Create(cls));
    ASSERT(exc.GetPtr() != nullptr);
    coroutine->SetException(exc.GetPtr()->GetCoreType());
}

VerificationInitAPI EtsLanguageContext::GetVerificationInitAPI() const
{
    VerificationInitAPI vApi;
    vApi.primitiveRootsForVerification = {
        panda_file::Type::TypeId::TAGGED, panda_file::Type::TypeId::VOID, panda_file::Type::TypeId::U1,
        panda_file::Type::TypeId::U8,     panda_file::Type::TypeId::U16,  panda_file::Type::TypeId::U32,
        panda_file::Type::TypeId::U64,    panda_file::Type::TypeId::I8,   panda_file::Type::TypeId::I16,
        panda_file::Type::TypeId::I32,    panda_file::Type::TypeId::I64,  panda_file::Type::TypeId::F32,
        panda_file::Type::TypeId::F64};

    vApi.arrayElementsForVerification = {reinterpret_cast<const uint8_t *>("[Z"),
                                         reinterpret_cast<const uint8_t *>("[B"),
                                         reinterpret_cast<const uint8_t *>("[S"),
                                         reinterpret_cast<const uint8_t *>("[C"),
                                         reinterpret_cast<const uint8_t *>("[I"),
                                         reinterpret_cast<const uint8_t *>("[J"),
                                         reinterpret_cast<const uint8_t *>("[F"),
                                         reinterpret_cast<const uint8_t *>("[D")

    };

    vApi.isNeedClassSyntheticClass = true;
    vApi.isNeedObjectSyntheticClass = false;
    vApi.isNeedStringSyntheticClass = false;

    return vApi;
}

}  // namespace ark::ets
