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

#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

// NOLINTBEGIN(readability-magic-numbers,cppcoreguidelines-pro-type-cstyle-cast)
namespace ark::ets::interop::js::ets_proxy::testing {

class SharedReferenceStorage1GTest : public js::testing::EtsInteropTest {
public:
    void SetUp() override
    {
        storage_ = SharedReferenceStorage::GetCurrent();
        ASSERT_NE(storage_, nullptr);

        memset_s(&objectArray_, sizeof(objectArray_), 0, sizeof(objectArray_));
        nextFreeIdx_ = 0;
    }

    EtsObject *NewEtsObject()
    {
        if (nextFreeIdx_ == MAX_OBJECTS) {
            std::cerr << "Out of memory" << std::endl;
            std::abort();
        }
        return EtsObject::FromCoreType(&objectArray_[nextFreeIdx_++]);
    }

    SharedReference *CreateReference(EtsObject *etsObject)
    {
        auto coro = EtsCoroutine::GetCurrent();
        [[maybe_unused]] EtsHandleScope s(coro);
        EtsHandle<EtsObject> objHandle(coro, etsObject);
        napi_value jsObj;

        NAPI_CHECK_FATAL(napi_create_object(InteropCtx::Current()->GetJSEnv(), &jsObj));
        SharedReference *ref = storage_->CreateETSObjectRef(InteropCtx::Current(), objHandle, jsObj);

        // Emulate wrappper usage
        ((uintptr_t *)ref)[0] = 0xcc00ff23deadbeef;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ((uintptr_t *)ref)[1] = 0xdd330047beefdead;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        return ref;
    }

    SharedReference *GetReference(void *data)
    {
        return storage_->GetReference(data);
    }

    void RemoveReference(SharedReference *ref)
    {
        return storage_->RemoveReference(ref);
    }

    bool HasReference(EtsObject *object) const
    {
        return SharedReference::HasReference(object);
    }

    bool CheckAlive(void *data)
    {
        return storage_->CheckAlive(data);
    }

protected:
    SharedReferenceStorage *storage_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    static constexpr size_t MAX_OBJECTS = 128;
    std::array<ObjectHeader, MAX_OBJECTS> objectArray_ {};
    size_t nextFreeIdx_ {};
};

TEST_F(SharedReferenceStorage1GTest, test_0)
{
    EtsObject *etsObject = NewEtsObject();

    ASSERT_EQ(HasReference(etsObject), false);
    SharedReference *ref = CreateReference(etsObject);
    ASSERT_EQ(HasReference(etsObject), true);

    SharedReference *refX = storage_->GetReference(etsObject);
    SharedReference *refY = GetReference((void *)ref);

    ASSERT_EQ(ref, refX);
    ASSERT_EQ(ref, refY);

    RemoveReference(ref);
}

TEST_F(SharedReferenceStorage1GTest, test_1)
{
    EtsObject *etsObject = NewEtsObject();

    SharedReference *ref = CreateReference(etsObject);

    // Check allocated ref
    SharedReference *ref0 = GetReference((void *)(uintptr_t(ref) + 0));
    ASSERT_EQ(ref0, ref);

    // Check unaligned address
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 1U)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 2U)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 3U)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 4U)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 5U)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 6U)), false);
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 7U)), false);

    // Check next unallocated reference
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + sizeof(SharedReference))), false);

    // Check virtually unmapped space
    ASSERT_EQ(CheckAlive((void *)(uintptr_t(ref) + 16U * 4096U)), false);

    RemoveReference(ref);
}

TEST_F(SharedReferenceStorage1GTest, test_2)
{
    EtsObject *etsObject = NewEtsObject();
    SharedReference *ref = CreateReference(etsObject);

    EtsObject *etsObject1 = NewEtsObject();
    SharedReference *ref1 = CreateReference(etsObject1);

    EtsObject *etsObject2 = NewEtsObject();
    SharedReference *ref2 = CreateReference(etsObject2);

    SharedReference *ref1a = GetReference((void *)ref1);
    ASSERT_EQ(ref1a, ref1);

    RemoveReference(ref1);

    // Check deallocated ref
    ASSERT_EQ(CheckAlive((void *)ref1), false);

    // Check
    EtsObject *etsObject4 = NewEtsObject();
    SharedReference *ref4 = CreateReference(etsObject4);
    ASSERT_EQ(ref4, ref1);

    RemoveReference(ref);
    RemoveReference(ref2);
    RemoveReference(ref4);

    ASSERT_EQ(CheckAlive((void *)ref), false);
    ASSERT_EQ(CheckAlive((void *)ref2), false);
    ASSERT_EQ(CheckAlive((void *)ref4), false);
}

}  // namespace ark::ets::interop::js::ets_proxy::testing
// NOLINTEND(readability-magic-numbers,cppcoreguidelines-pro-type-cstyle-cast)
