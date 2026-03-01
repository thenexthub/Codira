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

#include <sys/mman.h>

#include "libarkbase/mem/mem.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/utils/asan_interface.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/math_helpers.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/alloc_config.h"
#include "runtime/mem/region_allocator-inl.h"
#include "runtime/mem/tlab.h"
#include "runtime/tests/allocator_test_base.h"

#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/region_space.h"
#include "runtime/mem/gc/gc.h"

namespace ark::mem::test {

using NonObjectRegionAllocator = RegionAllocator<EmptyAllocConfigWithCrossingMap>;
using RemSetWithCommonLock = RemSet<RemSetLockConfig::CommonLock>;

class RemSetTest : public testing::Test {
public:
    RemSetTest()
    {
        options_.SetShouldLoadBootPandaFiles(false);
        options_.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options_);
        // For tests we don't limit spaces
        size_t spaceSize = options_.GetHeapSizeLimit();
        spaces_.youngSpace_.Initialize(spaceSize, spaceSize);
        spaces_.memSpace_.Initialize(spaceSize, spaceSize);
        // NOLINTNEXTLINE(readability-magic-numbers)
        spaces_.InitializePercentages(0, 100U);
        spaces_.isInitialized_ = true;
        thread_ = ark::ManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
        Init();
    }

    ~RemSetTest() override
    {
        thread_->ManagedCodeEnd();
        cardTable_ = nullptr;
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(RemSetTest);
    NO_MOVE_SEMANTIC(RemSetTest);

    void Init()
    {
        auto lang = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        ext_ = Runtime::GetCurrent()->GetClassLinker()->GetExtension(lang);
        cardTable_ = MakePandaUnique<CardTable>(Runtime::GetCurrent()->GetInternalAllocator(),
                                                PoolManager::GetMmapMemPool()->GetMinObjectAddress(),
                                                PoolManager::GetMmapMemPool()->GetTotalObjectSize());
        cardTable_->Initialize();
    }

private:
    ark::ManagedThread *thread_ {};
    RuntimeOptions options_;
    PandaUniquePtr<CardTable> cardTable_ {nullptr};

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ClassLinkerExtension *ext_ {};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    GenerationalSpaces spaces_;
};

TEST_F(RemSetTest, AddRefTest)
{
    auto *memStats = new mem::MemStatsType();
    NonObjectRegionAllocator allocator(memStats, &spaces_);
    auto cls = ext_->CreateClass(nullptr, 0, 0, sizeof(ark::Class));
    cls->SetObjectSize(allocator.GetMaxRegularObjectSize());

    auto obj1 = static_cast<ObjectHeader *>(allocator.Alloc(allocator.GetMaxRegularObjectSize()));
    obj1->SetClass(cls);
    auto region1 = ObjectToRegion(obj1);

    auto obj2 = static_cast<ObjectHeader *>(allocator.Alloc(allocator.GetMaxRegularObjectSize()));
    obj2->SetClass(cls);
    auto region2 = ObjectToRegion(obj2);

    // simulate gc process: mark obj2 and update live bitmap with mark bitmap
    region2->GetMarkBitmap()->Set(obj2);
    region2->CreateLiveBitmap();
    region2->SwapMarkBitmap();

    auto remset1 = region1->GetRemSet();
    remset1->AddRef(obj2, 0);

    PandaVector<void *> testList;
    auto visitor = [&testList](ObjectHeader *obj) { testList.push_back(obj); };
    remset1->IterateOverObjects(visitor);
    ASSERT_EQ(testList.size(), 1);
    auto first = testList.front();
    ASSERT_EQ(first, obj2);

    ext_->FreeClass(cls);
    delete memStats;
}

TEST_F(RemSetTest, AddRefWithAddrTest)
{
    auto *memStats = new mem::MemStatsType();
    NonObjectRegionAllocator allocator(memStats, &spaces_);
    auto cls = ext_->CreateClass(nullptr, 0, 0, sizeof(ark::Class));
    cls->SetObjectSize(allocator.GetMaxRegularObjectSize());

    auto obj1 = static_cast<ObjectHeader *>(allocator.Alloc(allocator.GetMaxRegularObjectSize()));
    obj1->SetClass(cls);
    auto region1 = ObjectToRegion(obj1);

    auto obj2 = static_cast<ObjectHeader *>(allocator.Alloc(allocator.GetMaxRegularObjectSize()));
    obj2->SetClass(cls);
    auto region2 = ObjectToRegion(obj2);

    region1->CreateLiveBitmap()->Set(obj1);

    RemSetWithCommonLock::AddRefWithAddr(obj1, 0, obj2);
    auto remset2 = region2->GetRemSet();

    PandaVector<void *> testList;
    auto visitor = [&testList](void *obj) { testList.push_back(obj); };
    remset2->IterateOverObjects(visitor);
    ASSERT_EQ(1, testList.size());

    auto first = testList.front();
    ASSERT_EQ(first, obj1);

    ext_->FreeClass(cls);
    delete memStats;
}

TEST_F(RemSetTest, TravelObjectToAddRefTest)
{
    auto *memStats = new mem::MemStatsType();
    NonObjectRegionAllocator allocator(memStats, &spaces_);
    auto cls = ext_->CreateClass(nullptr, 0, 0, sizeof(ark::Class));
    cls->SetObjectSize(allocator.GetMaxRegularObjectSize());
    cls->SetRefFieldsNum(1, false);
    auto offset = ObjectHeader::ObjectHeaderSize();
    cls->SetRefFieldsOffset(offset, false);
    // As cls is a fake class we have to calculate HaveNoRefsInParents flag manually
    cls->CalcHaveNoRefsInParents();

    auto obj1 = static_cast<ObjectHeader *>(allocator.Alloc(allocator.GetMaxRegularObjectSize()));
    obj1->SetClass(cls);
    auto region1 = ObjectToRegion(obj1);

    auto obj2 = static_cast<ObjectHeader *>(allocator.Alloc(allocator.GetMaxRegularObjectSize()));
    obj2->SetClass(cls);
    auto region2 = ObjectToRegion(obj2);

    // simulate gc process: mark obj2 and update live bitmap with mark bitmap
    region1->CreateLiveBitmap()->Set(obj1);

    static_cast<ObjectHeader *>(obj1)->SetFieldObject<false, false>(offset, static_cast<ObjectHeader *>(obj2));

    auto traverseObjectVisitor = [](ObjectHeader *fromObject, ObjectHeader *objectToTraverse) {
        RemSetWithCommonLock::AddRefWithAddr(fromObject, 0, objectToTraverse);
    };
    GCStaticObjectHelpers::TraverseAllObjects(obj1, traverseObjectVisitor);
    auto remset2 = region2->GetRemSet();

    PandaVector<void *> testList;
    auto visitor = [&testList](void *obj) { testList.push_back(obj); };
    remset2->IterateOverObjects(visitor);
    ASSERT_EQ(1, testList.size());

    auto first = testList.front();
    ASSERT_EQ(first, obj1);

    ext_->FreeClass(cls);
    delete memStats;
}

}  // namespace ark::mem::test
