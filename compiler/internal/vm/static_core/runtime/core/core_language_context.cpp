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

#include "runtime/core/core_language_context.h"

#include "runtime/core/core_itable_builder.h"
#include "runtime/core/core_vtable_builder.h"
#include "runtime/include/vtable_builder_standard-inl.h"
#include "runtime/handle_scope-inl.h"

namespace ark {

static Class *GetExceptionClass(const uint8_t *mutf8Name, ManagedThread *thread, ClassLinker *classLinker)
{
    auto runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *extension = classLinker->GetExtension(ctx);
    auto *cls = classLinker->GetClass(mutf8Name, true, extension->GetBootContext());
    if (cls == nullptr) {
        LOG(ERROR, CORE) << "Class " << utf::Mutf8AsCString(mutf8Name) << " not found";
        return nullptr;
    }

    if (!classLinker->InitializeClass(thread, cls)) {
        LOG(ERROR, CORE) << "Class " << utf::Mutf8AsCString(mutf8Name) << " cannot be initialized";
        return nullptr;
    }
    return cls;
}

void CoreLanguageContext::ThrowException(ManagedThread *thread, const uint8_t *mutf8Name, const uint8_t *mutf8Msg) const
{
    ASSERT(thread == ManagedThread::GetCurrent());

    if (thread->IsUsePreAllocObj()) {
        thread->SetUsePreAllocObj(false);
        auto *oom = thread->GetVM()->GetOOMErrorObject();
        thread->SetException(oom);
        return;
    }

    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> cause(thread, thread->GetException());
    thread->ClearException();

    auto runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *classLinker = runtime->GetClassLinker();
    auto *cls = GetExceptionClass(mutf8Name, thread, classLinker);
    if (cls == nullptr) {
        return;
    }

    VMHandle<ObjectHeader> excHandle(thread, ObjectHeader::Create(cls));

    coretypes::String *msg;
    if (mutf8Msg != nullptr) {
        msg = coretypes::String::CreateFromMUtf8(mutf8Msg, ctx, Runtime::GetCurrent()->GetPandaVM());
        if (UNLIKELY(msg == nullptr)) {
            // OOM happened during msg allocation
            ASSERT(thread->HasPendingException());
            return;
        }
    } else {
        msg = nullptr;
    }
    VMHandle<ObjectHeader> msgHandle(thread, msg);

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {utf::Mutf8AsCString(ctx.GetStringClassDescriptor()),
                                                      utf::Mutf8AsCString(ctx.GetObjectClassDescriptor())});
    auto *ctorName = ctx.GetCtorName();
    auto *ctor = cls->GetDirectMethod(ctorName, proto);
    if (ctor == nullptr) {
        LOG(ERROR, CORE) << "No method " << utf::Mutf8AsCString(ctorName) << " in class "
                         << utf::Mutf8AsCString(mutf8Name);
        return;
    }

    constexpr size_t NARGS = 3;
    std::array<Value, NARGS> args {Value(excHandle.GetPtr()), Value(msgHandle.GetPtr()), Value(cause.GetPtr())};
    ctor->InvokeVoid(thread, args.data());
    if (LIKELY(!thread->HasPendingException())) {
        thread->SetException(excHandle.GetPtr());
    }
}

PandaUniquePtr<ITableBuilder> CoreLanguageContext::CreateITableBuilder(
    [[maybe_unused]] ClassLinkerErrorHandler *errHandler) const  // CC-OFF(G.FMT.06) false positive
{
    return MakePandaUnique<CoreITableBuilder>();
}

PandaUniquePtr<VTableBuilder> CoreLanguageContext::CreateVTableBuilder(ClassLinkerErrorHandler *errHandler) const
{
    return MakePandaUnique<CoreVTableBuilder>(errHandler);
}

PandaVM *CoreLanguageContext::CreateVM(Runtime *runtime, const RuntimeOptions &options) const
{
    auto coreVm = core::PandaCoreVM::Create(runtime, options);
    if (!coreVm) {
        LOG(ERROR, CORE) << coreVm.Error();
        return nullptr;
    }
    return coreVm.Value();
}

mem::GC *CoreLanguageContext::CreateGC(mem::GCType gcType, mem::ObjectAllocatorBase *objectAllocator,
                                       const mem::GCSettings &settings) const
{
    return mem::CreateGC<PandaAssemblyLanguageConfig>(gcType, objectAllocator, settings);
}

void CoreLanguageContext::ThrowStackOverflowException(ManagedThread *thread) const
{
    auto runtime = Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *extension = classLinker->GetExtension(ctx);
    auto *cls = classLinker->GetClass(ctx.GetStackOverflowErrorClassDescriptor(), true, extension->GetBootContext());

    HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> exc(thread, ObjectHeader::Create(cls));
    thread->SetException(exc.GetPtr());
}

VerificationInitAPI CoreLanguageContext::GetVerificationInitAPI() const
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
    vApi.isNeedObjectSyntheticClass = true;
    vApi.isNeedStringSyntheticClass = true;

    return vApi;
}

}  // namespace ark
