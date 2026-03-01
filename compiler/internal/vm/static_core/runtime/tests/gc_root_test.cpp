/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <array>

#include "libarkfile/bytecode_emitter.h"
#include "runtime/include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/tests/interpreter_test_utils.h"

namespace ark::mem {

class GCRootTest : public testing::Test {
public:
    explicit GCRootTest() : GCRootTest(CreateDefaultOptions()) {}

    explicit GCRootTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
        runtime_ = Runtime::GetCurrent();
        vm_ = runtime_->GetPandaVM();
    }

    ~GCRootTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(GCRootTest);
    NO_MOVE_SEMANTIC(GCRootTest);

    static RuntimeOptions CreateDefaultOptions()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("g1-gc");
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        options.SetAdaptiveTlabSize(false);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(false);
        return options;
    }

    ObjectHeader *NewObject()
    {
        LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *klass = runtime_->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
        return ObjectHeader::Create(klass);
    }

    coretypes::String *NewString()
    {
        LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return coretypes::String::CreateEmptyString(ctx, vm_);
    }

    void TriggerGC()
    {
        GC *gc = vm_->GetGC();
        gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
    }

protected:
    Runtime *runtime_;  // NOLINT(misc-non-private-member-variables-in-classes)
    PandaVM *vm_;       // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(GCRootTest, TestVregRoot)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    BytecodeEmitter emitter;
    emitter.ReturnVoid();
    std::vector<uint8_t> bytecode;
    ASSERT_EQ(emitter.Build(&bytecode), BytecodeEmitter::ErrorCode::SUCCESS);
    auto frame = interpreter::test::CreateFrame(1U, nullptr, nullptr);
    thread->SetCurrentFrame(frame.get());
    StaticFrameHandler handler(frame.get());
    ObjectHeader *obj = NewObject();
    handler.GetVReg(0).SetReference(obj);
    auto cls = interpreter::test::CreateClass(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto methodData = interpreter::test::CreateMethod(cls, ACC_STATIC, 1, 1, nullptr, bytecode);
    auto method = std::move(methodData.first);
    frame->SetMethod(method.get());

    TriggerGC();
    ASSERT_NE(obj, handler.GetVReg(0).GetReference());
}

TEST_F(GCRootTest, TestClassRoot)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);

    const std::string className("Foo");
    auto runtime = Runtime::GetCurrent();
    auto etx =
        runtime->GetClassLinker()->GetExtension(runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY));
    auto klass = etx->CreateClass(reinterpret_cast<const uint8_t *>(className.data()), 0, 0,
                                  AlignUp(sizeof(Class) + OBJECT_POINTER_SIZE, OBJECT_POINTER_SIZE));
    klass->SetRefFieldsNum(1, true);
    klass->SetState(Class::State::INITIALIZED);
    Field field(klass, static_cast<panda_file::File::EntityId>(0), 0,
                panda_file::Type(panda_file::Type::TypeId::REFERENCE));
    klass->SetFields(Span<Field>(&field, 1), 1);
    field.SetOffset(klass->GetRefFieldsOffset<true>() + sizeof(ObjectHeader));
    ObjectHeader *obj = NewObject();
    ObjectAccessor::SetFieldObject(klass->GetManagedObject(), field, obj);

    TriggerGC();
    ASSERT_NE(obj, ObjectAccessor::GetFieldObject(klass->GetManagedObject(), field));
}

TEST_F(GCRootTest, TestExceptionRoot)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ObjectHeader *exception = NewObject();
    thread->SetException(exception);
    TriggerGC();
    ASSERT_NE(exception, thread->GetException());
}

TEST_F(GCRootTest, TestVmHandleRoot)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> handleScope(thread);
    ObjectHeader *obj = NewObject();
    VMHandle<ObjectHeader> handle(thread, obj);
    TriggerGC();
    ASSERT_NE(obj, handle.GetPtr());
}

TEST_F(GCRootTest, TestClassLinkerContextRoot)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    LanguageContext langCtx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    ObjectHeader *obj = NewObject();
    ClassLinkerContext *ctx = runtime_->GetClassLinker()->GetExtension(langCtx)->GetBootContext();
    ctx->AddGCRoot(obj);
    bool found = false;
    ctx->VisitGCRoots([obj, &found](GCRoot root) {
        if (root.GetObjectHeader() == obj) {
            found = true;
        }
    });
    ASSERT_TRUE(found);
    TriggerGC();
    found = false;
    ctx->VisitGCRoots([obj, &found](GCRoot root) {
        if (root.GetObjectHeader() == obj) {
            found = true;
        }
    });
    ASSERT_FALSE(found);
}

// There is no a test for StringTable::InternalTable because all strings
// in this table are non-movable.

TEST_F(GCRootTest, TestAotStringRoot)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    AotManager *aotManager = runtime_->GetClassLinker()->GetAotManager();
    ObjectHeader *obj = NewObject();
    ObjectHeader *oldObjAddr = obj;
    aotManager->RegisterAotStringRoot(&obj, false);
    bool found = false;
    aotManager->VisitAotStringRoots(
        [obj, &found](ObjectHeader **root) {
            if (*root == obj) {
                found = true;
            }
        },
        false);
    ASSERT_TRUE(found);
    TriggerGC();
    found = false;
    aotManager->VisitAotStringRoots(
        [oldObjAddr, &found](ObjectHeader **root) {
            if (*root == oldObjAddr) {
                found = true;
            }
        },
        false);
    ASSERT_FALSE(found);
}

TEST_F(GCRootTest, TestGlobalObjectStorageRoot)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    auto *storage = vm_->GetGlobalObjectStorage();
    ASSERT_NE(storage, nullptr);
    ObjectHeader *obj = NewObject();
    auto *ref = storage->Add(obj, Reference::ObjectType::GLOBAL);
    TriggerGC();
    ASSERT_NE(obj, storage->Get(ref));
}

TEST_F(GCRootTest, TestPtReferenceStorageRoot)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    auto *storage = thread->GetPtReferenceStorage();
    ASSERT_NE(storage, nullptr);
    ObjectHeader *obj = NewObject();
    auto *ref = storage->NewRef(obj, Reference::ObjectType::LOCAL);
    TriggerGC();
    ASSERT_NE(obj, storage->GetObject(ref));
}

TEST_F(GCRootTest, TestSweepFlattenedStringCacheRoot)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ObjectHeader *obj = NewObject();
    thread->SetFlattenedStringCache(obj);
    TriggerGC();
    ASSERT_NE(obj, thread->GetFlattenedStringCache());
}

TEST_F(GCRootTest, TestUpdateAndSweepMonitor)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    auto *monitorPool = vm_->GetMonitorPool();
    [[maybe_unused]] HandleScope<ObjectHeader *> handleScope(thread);
    ObjectHeader *liveObj = NewObject();
    ObjectHeader *deadObj = NewObject();
    VMHandle<ObjectHeader> objHandle(thread, liveObj);

    ASSERT_TRUE(Monitor::Inflate(liveObj, thread));
    ASSERT_EQ(thread->GetMonitorCount(), 1);
    Monitor *liveMonitor = nullptr;
    monitorPool->EnumerateMonitors([&liveMonitor](Monitor *m) {
        liveMonitor = m;
        return false;
    });

    Monitor *deadMonitor = monitorPool->CreateMonitor(deadObj);
    Monitor::MonitorId deadId = deadMonitor->GetId();
    TriggerGC();
    ASSERT_NE(liveObj, liveMonitor->GetObject());
    ASSERT_EQ(nullptr, monitorPool->LookupMonitor(deadId));
}

TEST_F(GCRootTest, TestUpdateFlattenedStringCache)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> handleScope(thread);
    ObjectHeader *obj = NewObject();
    VMHandle<ObjectHeader> handle(thread, obj);
    thread->SetFlattenedStringCache(obj);
    TriggerGC();
    ASSERT_NE(obj, thread->GetFlattenedStringCache());
}

TEST_F(GCRootTest, TestUpdateStringTable)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> handleScope(thread);
    StringTable *strTable = vm_->GetStringTable();
    LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    coretypes::String *str = strTable->GetOrInternString(reinterpret_cast<const uint8_t *>("abc"), 3U, ctx);
    VMHandle<ObjectHeader> handle(thread, str);
    size_t oldCount = 0;
    bool found = false;
    strTable->VisitStrings([&oldCount, &found, str](ObjectHeader *obj) {
        ++oldCount;
        if (obj == str) {
            found = true;
        }
    });
    ASSERT_GE(oldCount, 0);
    ASSERT_TRUE(found);
    TriggerGC();
    size_t newCount = 0;
    found = false;
    strTable->VisitStrings([&newCount, &found, handle](ObjectHeader *obj) {
        ++newCount;
        if (obj == handle.GetPtr()) {
            found = true;
        }
    });
    ASSERT_EQ(oldCount, newCount);
    ASSERT_TRUE(found);
}

TEST_F(GCRootTest, TestSweepStringTable)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    StringTable *strTable = vm_->GetStringTable();
    LanguageContext ctx = runtime_->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    coretypes::String *str = strTable->GetOrInternString(reinterpret_cast<const uint8_t *>("abc"), 3U, ctx);
    size_t oldCount = 0;
    bool found = false;
    strTable->VisitStrings([&oldCount, &found, str](ObjectHeader *obj) {
        ++oldCount;
        if (obj == str) {
            found = true;
        }
    });
    ASSERT_GE(oldCount, 0);
    ASSERT_TRUE(found);
    TriggerGC();
    size_t newCount = 0;
    found = false;
    strTable->VisitStrings([&newCount, &found, str](ObjectHeader *obj) {
        ++newCount;
        if (obj == str) {
            found = true;
        }
    });
    ASSERT_GT(oldCount, newCount);
    ASSERT_FALSE(found);
}

TEST_F(GCRootTest, TestUpdateGlobalObjectStorage)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> handleScope(thread);
    auto *storage = vm_->GetGlobalObjectStorage();
    ASSERT_NE(storage, nullptr);
    ObjectHeader *obj = NewObject();
    VMHandle<ObjectHeader> handle(thread, obj);
    auto *ref = storage->Add(obj, Reference::ObjectType::WEAK);
    TriggerGC();
    ASSERT_NE(obj, handle.GetPtr());
    ASSERT_NE(nullptr, storage->Get(ref));
    ASSERT_NE(obj, storage->Get(ref));
}

TEST_F(GCRootTest, TestSweepGlobalObjectStorage)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    auto *storage = vm_->GetGlobalObjectStorage();
    ASSERT_NE(storage, nullptr);
    ObjectHeader *obj = NewObject();
    auto *ref = storage->Add(obj, Reference::ObjectType::WEAK);
    TriggerGC();
    ASSERT_EQ(nullptr, storage->Get(ref));
}
}  // namespace ark::mem
