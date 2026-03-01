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

#include "runtime/include/class_linker_extension.h"

#include "libarkbase/utils/utf.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"

namespace ark {

ClassLinkerExtension::~ClassLinkerExtension()
{
    os::memory::LockHolder lock(contextsLock_);
    for (auto *ctx : contexts_) {
        classLinker_->GetAllocator()->Delete(ctx);
    }
}

Class *ClassLinkerExtension::BootContext::LoadClass(const uint8_t *descriptor, bool needCopyDescriptor,
                                                    ClassLinkerErrorHandler *errorHandler)
{
    ASSERT(extension_->IsInitialized());

    return extension_->GetClassLinker()->GetClass(descriptor, needCopyDescriptor, this, errorHandler);
}

void ClassLinkerExtension::BootContext::EnumeratePandaFiles(
    const std::function<bool(const panda_file::File &)> &cb) const
{
    extension_->GetClassLinker()->EnumerateBootPandaFiles(cb);
}

class SuppressErrorHandler : public ClassLinkerErrorHandler {
    void OnError([[maybe_unused]] ClassLinker::Error error, [[maybe_unused]] const PandaString &message) override {}
};

Class *ClassLinkerExtension::AppContext::LoadClass(const uint8_t *descriptor, bool needCopyDescriptor,
                                                   ClassLinkerErrorHandler *errorHandler)
{
    ASSERT(extension_->IsInitialized());

    SuppressErrorHandler handler;
    auto *cls = extension_->GetClass(descriptor, needCopyDescriptor, nullptr, &handler);
    if (cls != nullptr) {
        return cls;
    }

    for (auto &pf : pfs_) {
        auto classId = pf->GetClassId(descriptor);
        if (!classId.IsValid() || pf->IsExternal(classId)) {
            continue;
        }
        return extension_->GetClassLinker()->LoadClass(*pf, classId, this, errorHandler);
    }

    if (errorHandler != nullptr) {
        PandaStringStream ss;
        ss << "Cannot find class " << descriptor << " in all app panda files";
        errorHandler->OnError(ClassLinker::Error::CLASS_NOT_FOUND, ss.str());
    }
    return nullptr;
}

void ClassLinkerExtension::InitializeArrayClassRoot(ClassRoot root, ClassRoot componentRoot, const char *descriptor)
{
    ASSERT(IsInitialized());

    auto *arrayClass = CreateClass(utf::CStringAsMutf8(descriptor), GetClassVTableSize(root), GetClassIMTSize(root),
                                   GetClassSize(root));
    ASSERT(arrayClass != nullptr);
    arrayClass->SetLoadContext(&bootContext_);
    auto *componentClass = GetClassRoot(componentRoot);
    if (!InitializeArrayClass(arrayClass, componentClass)) {
        LOG(FATAL, CLASS_LINKER) << "Failed to initialize array class root '" << arrayClass->GetName() << "'";
        return;
    }

    AddClass(arrayClass);
    SetClassRoot(root, arrayClass);
}

void ClassLinkerExtension::InitializePrimitiveClassRoot(ClassRoot root, panda_file::Type::TypeId typeId,
                                                        const char *descriptor)
{
    ASSERT(IsInitialized());

    auto *primitiveClass = CreateClass(utf::CStringAsMutf8(descriptor), GetClassVTableSize(root), GetClassIMTSize(root),
                                       GetClassSize(root));
    ASSERT(primitiveClass != nullptr);
    primitiveClass->SetType(panda_file::Type(typeId));
    primitiveClass->SetLoadContext(&bootContext_);
    InitializePrimitiveClass(primitiveClass);
    AddClass(primitiveClass);
    SetClassRoot(root, primitiveClass);
}

void ClassLinkerExtension::InitializeSyntheticClassRoot(ClassRoot root, const char *descriptor)
{
    ASSERT(IsInitialized());

    auto *synClass = CreateClass(utf::CStringAsMutf8(descriptor), 0, 0, GetClassSize(root));
    ASSERT(synClass != nullptr);
    synClass->SetType(panda_file::Type(panda_file::Type::TypeId::REFERENCE));
    synClass->SetLoadContext(&bootContext_);
    InitializeSyntheticClass(synClass);
    AddClass(synClass);
    SetClassRoot(root, synClass);
}

bool ClassLinkerExtension::Initialize(ClassLinker *classLinker, bool compressedStringEnabled)
{
    classLinker_ = classLinker;
    InitializeImpl(compressedStringEnabled);

    canInitializeClasses_ = true;
    // Copy classes to separate container as ClassLinkerExtension::InitializeClass
    // can load more classes and modify boot context
    PandaVector<Class *> klasses;
    bootContext_.EnumerateClasses([&klasses](Class *klass) {
        if (!klass->IsLoaded()) {
            klasses.push_back(klass);
        }
        return true;
    });

    for (auto *klass : klasses) {
        if (klass->IsLoaded()) {
            continue;
        }

        InitializeClass(klass);
        klass->SetState(Class::State::LOADED);
    }
    return true;
}

bool ClassLinkerExtension::InitializeRoots(ManagedThread *thread)
{
    ASSERT(IsInitialized());

    for (auto *klass : classRoots_) {
        if (klass == nullptr) {
            continue;
        }

        if (!classLinker_->InitializeClass(thread, klass)) {
            LOG(FATAL, CLASS_LINKER) << "Failed to initialize class '" << klass->GetName() << "'";
            return false;
        }
    }

    return true;
}

Class *ClassLinkerExtension::FindLoadedClass(const uint8_t *descriptor, ClassLinkerContext *context /* = nullptr */)
{
    return classLinker_->FindLoadedClass(descriptor, ResolveContext(context));
}

Class *ClassLinkerExtension::GetClass(const uint8_t *descriptor, bool needCopyDescriptor /* = true */,
                                      ClassLinkerContext *context /* = nullptr */,
                                      ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    ASSERT(IsInitialized());

    return classLinker_->GetClass(descriptor, needCopyDescriptor, ResolveContext(context),
                                  ResolveErrorHandler(errorHandler));
}

static void WrapClassNotFoundExceptionIfNeeded(ClassLinker *classLinker, const uint8_t *descriptor,
                                               const LanguageContext &ctx)
{
    auto *thread = ManagedThread::GetCurrent();
    if (thread == nullptr || !thread->HasPendingException()) {
        return;
    }

    auto *classNotFoundExceptionClass =
        classLinker->GetExtension(ctx)->GetClass(ctx.GetClassNotFoundExceptionDescriptor());
    if (classNotFoundExceptionClass == nullptr) {
        // We've got OOM
        ASSERT(thread->GetVM()->GetOOMErrorObject() == nullptr ||
               thread->GetException()->ClassAddr<Class>() == thread->GetVM()->GetOOMErrorObject()->ClassAddr<Class>());
        return;
    }
    ASSERT(classNotFoundExceptionClass != nullptr);

    auto *cause = thread->GetException();
    if (cause->IsInstanceOf(classNotFoundExceptionClass)) {
        auto name = ClassHelper::GetName(descriptor);
        ark::ThrowException(ctx, thread, ctx.GetNoClassDefFoundErrorDescriptor(), utf::CStringAsMutf8(name.c_str()));
    }
}

Class *ClassLinkerExtension::GetClass(const panda_file::File &pf, panda_file::File::EntityId id,
                                      ClassLinkerContext *context /* = nullptr */,
                                      ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    ASSERT(IsInitialized());

    auto *cls = classLinker_->GetClass(pf, id, ResolveContext(context), ResolveErrorHandler(errorHandler));

    if (UNLIKELY(cls == nullptr)) {
        auto *descriptor = pf.GetStringData(id).data;
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(GetLanguage());
        WrapClassNotFoundExceptionIfNeeded(classLinker_, descriptor, ctx);
    }

    return cls;
}

Class *ClassLinkerExtension::AddClass(Class *klass)
{
    ASSERT(IsInitialized());

    auto *context = klass->GetLoadContext();

    auto *otherKlass = ResolveContext(context)->InsertClass(klass);
    if (otherKlass != nullptr) {
        classLinker_->FreeClass(klass);
        return otherKlass;
    }
    OnClassPrepared(klass);

    return klass;
}

size_t ClassLinkerExtension::NumLoadedClasses()
{
    ASSERT(IsInitialized());

    size_t sum = bootContext_.NumLoadedClasses();

    {
        os::memory::LockHolder lock(contextsLock_);
        for (auto *ctx : contexts_) {
            sum += ctx->NumLoadedClasses();
        }
    }
    return sum;
}

void ClassLinkerExtension::VisitLoadedClasses(size_t flag)
{
    bootContext_.VisitLoadedClasses(flag);
    {
        os::memory::LockHolder lock(contextsLock_);
        for (auto *ctx : contexts_) {
            ctx->VisitLoadedClasses(flag);
        }
    }
}

void ClassLinkerExtension::FreeLoadedClasses()
{
    ASSERT(IsInitialized());

    bootContext_.EnumerateClasses([this](Class *klass) {
        FreeClass(klass);
        classLinker_->FreeClassData(klass);
        return true;
    });
    {
        os::memory::LockHolder lock(contextsLock_);
        for (auto *ctx : contexts_) {
            ctx->EnumerateClasses([this](Class *klass) {
                FreeClass(klass);
                classLinker_->FreeClassData(klass);
                return true;
            });
        }
    }
}

ClassLinkerContext *ClassLinkerExtension::CreateApplicationClassLinkerContext(const PandaVector<PandaString> &path)
{
    PandaVector<PandaFilePtr> appFiles;
    for (auto &pfPath : path) {
        auto pf = panda_file::OpenPandaFileOrZip(pfPath);
        if (pf == nullptr) {
            return nullptr;
        }
        appFiles.push_back(std::move(pf));
    }
    ClassLinkerContext *ctx = CreateApplicationClassLinkerContext(std::move(appFiles));
    return ctx;
}

ClassLinkerContext *ClassLinkerExtension::CreateApplicationClassLinkerContext(PandaVector<PandaFilePtr> &&appFiles)
{
    PandaVector<const panda_file::File *> appFilePtrs;
    appFilePtrs.reserve(appFiles.size());
    for (auto &pf : appFiles) {
        appFilePtrs.emplace_back(pf.get());
    }

    auto allocator = classLinker_->GetAllocator();
    auto *ctx = allocator->New<AppContext>(this, std::move(appFilePtrs));
    RegisterContext([ctx]() { return ctx; });
    for (auto &pf : appFiles) {
        classLinker_->AddPandaFile(std::move(pf), ctx);
    }
    return ctx;
}

void ClassLinkerExtension::AddCreatedClass(Class *klass)
{
    os::memory::LockHolder lock(createdClassesLock_);
    createdClasses_.push_back(klass);
}

void ClassLinkerExtension::RemoveCreatedClass(Class *klass)
{
    os::memory::LockHolder lock(createdClassesLock_);
    auto it = find(createdClasses_.begin(), createdClasses_.end(), klass);
    if (it != createdClasses_.end()) {
        createdClasses_.erase(it);
    }
}

void ClassLinkerExtension::OnClassPrepared(Class *klass)
{
    // Atomic with seq_cst order reason: data race with record_new_class_ with requirement for sequentially
    // consistent order where threads observe all modifications in the same order
    if (recordNewClass_.load(std::memory_order_seq_cst)) {
        os::memory::LockHolder newClassesLock(newClassesLock_);
        newClasses_.push_back(klass);
    }

    RemoveCreatedClass(klass);
}

Class *ClassLinkerExtension::FromClassObject(ObjectHeader *obj)
{
    return (obj != nullptr) ? ((reinterpret_cast<ark::coretypes::Class *>(obj))->GetRuntimeClass()) : nullptr;
}

size_t ClassLinkerExtension::GetClassObjectSizeFromClassSize(uint32_t size)
{
    return ark::coretypes::Class::GetSize(size);
}

void ClassLinkerExtension::FreeObsoleteData()
{
    os::memory::LockHolder lock(obsoleteClassesLock_);
    for (auto &cls : obsoleteClasses_) {
        ASSERT(cls != nullptr);
        GetClassLinker()->FreeClass(cls);
    }
}

void ClassLinkerExtension::AddObsoleteClass(const PandaVector<Class *> &classes)
{
    if (classes.empty()) {
        return;
    }
    os::memory::LockHolder lock(obsoleteClassesLock_);
    obsoleteClasses_.insert(obsoleteClasses_.end(), classes.begin(), classes.end());
}

}  // namespace ark
