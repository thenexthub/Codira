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

#include "runtime/tests/pygote_space_allocator_test_base.h"

namespace ark::mem {

void PygoteSpaceAllocatorTest::InitAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();

    auto nonMovableHeader = ark::ObjectHeader::CreateNonMovable(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));

    auto movableHeader = ark::ObjectHeader::Create(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movableHeader)));

    pygoteSpaceAllocator->Free(nonMovableHeader);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
}

void PygoteSpaceAllocatorTest::ForkedAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();

    PygoteFork();

    auto nonMovableHeader = ark::ObjectHeader::CreateNonMovable(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));

    auto movableHeader = ark::ObjectHeader::Create(cls);
    ASSERT_NE(movableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movableHeader)));
}

void PygoteSpaceAllocatorTest::NonMovableLiveObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    auto nonMovableHeader = ark::ObjectHeader::CreateNonMovable(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
    [[maybe_unused]] auto *ref = globalObjectStorage->Add(nonMovableHeader, ark::mem::Reference::ObjectType::GLOBAL);

    PygoteFork();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));

    TriggerGc();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));

    pygoteSpaceAllocator->Free(nonMovableHeader);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
}

void PygoteSpaceAllocatorTest::NonMovableUnliveObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    auto nonMovableHeader = ark::ObjectHeader::CreateNonMovable(cls);
    ASSERT_NE(nonMovableHeader, nullptr);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
    [[maybe_unused]] auto *ref = globalObjectStorage->Add(nonMovableHeader, ark::mem::Reference::ObjectType::GLOBAL);

    PygoteFork();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
    globalObjectStorage->Remove(ref);

    TriggerGc();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovableHeader)));
    ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovableHeader)));
}

void PygoteSpaceAllocatorTest::MovableLiveObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    auto movableHeader = ark::ObjectHeader::Create(cls);
    ASSERT_NE(movableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movableHeader)));
    [[maybe_unused]] auto *ref = globalObjectStorage->Add(movableHeader, ark::mem::Reference::ObjectType::GLOBAL);

    PygoteFork();

    auto obj = globalObjectStorage->Get(ref);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));

    TriggerGc();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
}

void PygoteSpaceAllocatorTest::MovableUnliveObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    auto movableHeader = ark::ObjectHeader::Create(cls);
    ASSERT_NE(movableHeader, nullptr);
    ASSERT_FALSE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movableHeader)));
    [[maybe_unused]] auto *ref = globalObjectStorage->Add(movableHeader, ark::mem::Reference::ObjectType::GLOBAL);

    PygoteFork();

    auto obj = globalObjectStorage->Get(ref);
    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
    ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
    globalObjectStorage->Remove(ref);

    TriggerGc();

    ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
    ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
}

void PygoteSpaceAllocatorTest::MuchObjectAllocTest()
{
    [[maybe_unused]] auto pygoteSpaceAllocator = GetPygoteSpaceAllocator();
    auto cls = GetObjectClass();
    auto globalObjectStorage = thread_->GetVM()->GetGlobalObjectStorage();

    static constexpr size_t OBJ_NUM = 1024;

    PandaVector<Reference *> movableRefs;
    PandaVector<Reference *> nonMovableRefs;
    for (size_t i = 0; i < OBJ_NUM; i++) {
        auto movable = ark::ObjectHeader::Create(cls);
        movableRefs.push_back(globalObjectStorage->Add(movable, ark::mem::Reference::ObjectType::GLOBAL));
        auto nonMovable = ark::ObjectHeader::CreateNonMovable(cls);
        nonMovableRefs.push_back(globalObjectStorage->Add(nonMovable, ark::mem::Reference::ObjectType::GLOBAL));
    }

    PygoteFork();

    PandaVector<ObjectHeader *> movableObjs;
    PandaVector<ObjectHeader *> nonMovableObjs;
    for (auto movalbeRef : movableRefs) {
        auto obj = globalObjectStorage->Get(movalbeRef);
        ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
        ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
        globalObjectStorage->Remove(movalbeRef);
        movableObjs.push_back(obj);
    }

    for (auto nonMovalbeRef : nonMovableRefs) {
        auto obj = globalObjectStorage->Get(nonMovalbeRef);
        ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(obj)));
        ASSERT_TRUE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(obj)));
        globalObjectStorage->Remove(nonMovalbeRef);
        nonMovableObjs.push_back(obj);
    }

    TriggerGc();

    for (auto movalbeObj : movableObjs) {
        ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(movalbeObj)));
        ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(movalbeObj)));
    }

    for (auto nonMovalbeObj : nonMovableObjs) {
        ASSERT_TRUE(pygoteSpaceAllocator->ContainObject(static_cast<ObjectHeader *>(nonMovalbeObj)));
        ASSERT_FALSE(pygoteSpaceAllocator->IsLive(static_cast<ObjectHeader *>(nonMovalbeObj)));
    }
}
}  // namespace ark::mem
