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

#include "ets_interop_js_gtest.h"

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage_verifier.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "ets_class_linker_extension.h"
#include "plugins/ets/assembler/extension/ets_meta.h"
#include "libarkbase/os/stacktrace.h"
#include "assembly-program.h"
#include "assembler/assembly-parser.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"

// NOLINTBEGIN(readability-magic-numbers)
namespace ark::ets::interop::js::ets_proxy::testing {

class SharedReferenceStorage1GTest : public js::testing::EtsInteropTest {
public:
    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
        storage_ = SharedReferenceStorage::GetCurrent();
        ASSERT_NE(storage_, nullptr);
        SetClassesPandasmSources();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    EtsClass *GetTestClass(std::string className)
    {
        std::unordered_map<std::string, const char *> sourcesMap = GetSources();
        pandasm::Parser p;

        auto res = p.Parse(sourcesMap[className]);
        auto pf = ark::pandasm::AsmEmitter::Emit(res.Value());

        auto classLinker = Runtime::GetCurrent()->GetClassLinker();
        classLinker->AddPandaFile(std::move(pf));

        className.insert(0, 1, 'L');
        className.push_back(';');

        EtsClass *klass = coroutine_->GetPandaVM()->GetClassLinker()->GetClass(className.c_str());
        ASSERT(klass);
        return klass;
    }

    void SetETSObject(SharedReference *ref, EtsObject *obj)
    {
        ref->SetETSObject(obj);
    }

    void SetClassesPandasmSources()
    {
        const char *source = R"(
              .language eTS
              .record Rectangle {
                  i32 Width
                  f32 Height
                  i64 Color <static>
              }
          )";
        sources_["Rectangle"] = source;

        source = R"(
              .language eTS
              .record Triangle {
                  i32 firSide
                  i32 secSide
                  i32 thirdSide
                  i64 Color <static>
              }
          )";
        sources_["Triangle"] = source;

        source = R"(
              .language eTS
              .record Foo {
                  i32 member
              }
              .record Bar {
                  Foo foo1
                  Foo foo2
              }
          )";
        sources_["Foo"] = source;
        sources_["Bar"] = source;
    }

    EtsObject *NewEtsObject()
    {
        EtsClass *barKlass = GetTestClass("Bar");
        EtsObject *obj = EtsObject::Create(barKlass);
        return obj;
    }

    SharedReference *CreateReference(EtsObject *etsObject, napi_value &jsObj)
    {
        auto coro = EtsCoroutine::GetCurrent();
        [[maybe_unused]] EtsHandleScope s(coro);
        EtsHandle<EtsObject> objHandle(coro, etsObject);

        NAPI_CHECK_FATAL(napi_create_object(InteropCtx::Current()->GetJSEnv(), &jsObj));
        SharedReference *ref = storage_->CreateJSObjectRef(InteropCtx::Current(), objHandle, jsObj);
        return ref;
    }

    SharedReference *CreateReferenceEts(EtsObject *etsObject, napi_value &jsObj)
    {
        auto coro = EtsCoroutine::GetCurrent();
        [[maybe_unused]] EtsHandleScope s(coro);
        EtsHandle<EtsObject> objHandle(coro, etsObject);

        NAPI_CHECK_FATAL(napi_create_object(InteropCtx::Current()->GetJSEnv(), &jsObj));
        SharedReference *ref = storage_->CreateETSObjectRef(InteropCtx::Current(), objHandle, jsObj);
        return ref;
    }

    void RemoveReference(SharedReference *ref)
    {
        return storage_->RemoveReference(ref);
    }

    const std::unordered_map<std::string, const char *> &GetSources()
    {
        return sources_;
    }

    size_t CheckObjectAddressValid(SharedReference *ref, size_t index)
    {
        return SharedReferenceStorageVerifier::CheckObjectAddressValid(ref, index);
    }

    size_t CheckObjectAlive(SharedReference *ref, size_t index)
    {
        return SharedReferenceStorageVerifier::CheckObjectAlive(ref, index);
    }

    size_t CheckObjectReindex([[maybe_unused]] const SharedReferenceStorage *const storage, SharedReference *ref,
                              size_t index)
    {
        return SharedReferenceStorageVerifier::CheckObjectReindex(storage_, ref, index);
    }

    size_t CheckJsObjectType(SharedReference *ref, size_t index)
    {
        return SharedReferenceStorageVerifier::CheckJsObjectType(ref, index);
    }

    size_t IsAllItemsMarked(SharedReference *ref, size_t index)
    {
        return SharedReferenceStorageVerifier::IsAllItemsMarked(ref, index);
    }

protected:
    SharedReferenceStorage *storage_ {nullptr};              // NOLINT(misc-non-private-member-variables-in-classes)
    EtsCoroutine *coroutine_ = nullptr;                      // NOLINT(misc-non-private-member-variables-in-classes)
    std::unordered_map<std::string, const char *> sources_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(SharedReferenceStorage1GTest, test_Js_Reindex)
{
    EtsObject *etsObject = NewEtsObject();
    ASSERT_NE(etsObject, nullptr);
    napi_value jsObject = nullptr;
    SharedReference *ref = CreateReferenceEts(etsObject, jsObject);
    EtsObject *etsObject1 = NewEtsObject();
    napi_value jsObject1 = nullptr;
    SharedReference *ref1 = CreateReferenceEts(etsObject1, jsObject1);
    ASSERT_NE(ref1, nullptr);
    SetETSObject(ref, etsObject1);
    size_t failCount = CheckObjectReindex(storage_, ref, 1);
    ASSERT_NE(failCount, 0);
    RemoveReference(ref);
    RemoveReference(ref1);
}

TEST_F(SharedReferenceStorage1GTest, test_Ets_InHeap)
{
    EtsObject *etsObject = NewEtsObject();
    ASSERT_NE(etsObject, nullptr);
    napi_value jsObject = nullptr;
    SharedReference *ref = CreateReferenceEts(etsObject, jsObject);
    ASSERT_NE(ref, nullptr);
    ASSERT_NE(jsObject, nullptr);
    size_t failCount = CheckObjectAddressValid(ref, 1);
    ASSERT_EQ(failCount, 0);
    RemoveReference(ref);
}

TEST_F(SharedReferenceStorage1GTest, test_mark)
{
    EtsObject *etsObject = NewEtsObject();
    ASSERT_NE(etsObject, nullptr);
    napi_value jsObject = nullptr;
    SharedReference *ref = CreateReference(etsObject, jsObject);
    size_t failCount = IsAllItemsMarked(ref, 1);
    ASSERT_NE(failCount, 0);
    RemoveReference(ref);
}

TEST_F(SharedReferenceStorage1GTest, test_Js_Type)
{
    EtsObject *etsObject = NewEtsObject();
    ASSERT_NE(etsObject, nullptr);
    napi_value jsObject = nullptr;
    SharedReference *ref = CreateReference(etsObject, jsObject);
    size_t failCount = CheckJsObjectType(ref, 1);
    ASSERT_NE(failCount, 0);
    RemoveReference(ref);
}

TEST_F(SharedReferenceStorage1GTest, test_Js_Alive)
{
    EtsObject *etsObject = NewEtsObject();
    ASSERT_NE(etsObject, nullptr);
    napi_value jsObject = nullptr;
    SharedReference *ref = CreateReference(etsObject, jsObject);
    ASSERT_NE(ref, nullptr);
    ASSERT_NE(jsObject, nullptr);
    Runtime *runtime = Runtime::GetCurrent();
    auto gc = runtime->GetPandaVM()->GetGC();
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    EtsObject *etsObject1 = NewEtsObject();
    napi_value jsObject1 = nullptr;
    SharedReference *ref1 = CreateReference(etsObject1, jsObject1);
    ASSERT_NE(ref1, nullptr);
    SetETSObject(ref, etsObject);
    size_t failCount = CheckObjectAlive(ref, 1);
    ASSERT_NE(failCount, 0);
    RemoveReference(ref);
    RemoveReference(ref1);
}

TEST_F(SharedReferenceStorage1GTest, test_Ets_Alive)
{
    EtsObject *etsObject = NewEtsObject();
    ASSERT_NE(etsObject, nullptr);
    napi_value jsObject = nullptr;
    SharedReference *ref = CreateReferenceEts(etsObject, jsObject);
    ASSERT_NE(ref, nullptr);
    ASSERT_NE(jsObject, nullptr);
    Runtime *runtime = Runtime::GetCurrent();
    auto gc = runtime->GetPandaVM()->GetGC();
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    EtsObject *etsObject1 = NewEtsObject();
    napi_value jsObject1 = nullptr;
    SharedReference *ref1 = CreateReferenceEts(etsObject1, jsObject1);
    ASSERT_NE(ref1, nullptr);
    SetETSObject(ref, etsObject);
    size_t failCount = CheckObjectAlive(ref, 1);
    ASSERT_NE(failCount, 0);
    RemoveReference(ref);
    RemoveReference(ref1);
}
}  // namespace ark::ets::interop::js::ets_proxy::testing
// NOLINTEND(readability-magic-numbers,cppcoreguidelines-pro-type-cstyle-cast)
