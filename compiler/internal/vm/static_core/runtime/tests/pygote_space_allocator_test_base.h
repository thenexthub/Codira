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
#ifndef PANDA_RUNTIME_TESTS_PYGOTE_SPACE_ALLOCATOR_TEST_H_
#define PANDA_RUNTIME_TESTS_PYGOTE_SPACE_ALLOCATOR_TEST_H_

#include <sys/mman.h>
#include <gtest/gtest.h>

#include "libarkbase/os/mem.h"
#include "libarkbase/utils/logger.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/refstorage/global_object_storage.h"

namespace ark::mem {

class PygoteSpaceAllocatorTest : public testing::Test {
public:
    NO_COPY_SEMANTIC(PygoteSpaceAllocatorTest);
    NO_MOVE_SEMANTIC(PygoteSpaceAllocatorTest);

    using PygoteAllocator = PygoteSpaceAllocator<ObjectAllocConfig>;

    PygoteSpaceAllocatorTest() = default;
    ~PygoteSpaceAllocatorTest() override = default;

protected:
    PygoteAllocator *GetPygoteSpaceAllocator()
    {
        return thread_->GetVM()->GetGC()->GetObjectAllocator()->GetPygoteSpaceAllocator();
    }

    Class *GetObjectClass()
    {
        auto runtime = ark::Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        return runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::OBJECT);
    }

    void PygoteFork()
    {
        thread_->ManagedCodeEnd();
        auto runtime = ark::Runtime::GetCurrent();
        runtime->PreZygoteFork();
        runtime->PostZygoteFork();
        thread_->ManagedCodeBegin();
    }

    void TriggerGc()
    {
        auto gc = thread_->GetVM()->GetGC();
        auto task = GCTask(GCTaskCause::EXPLICIT_CAUSE);
        // trigger tenured gc
        gc->WaitForGCInManaged(task);
        gc->WaitForGCInManaged(task);
        gc->WaitForGCInManaged(task);
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    RuntimeOptions options_;

    void InitAllocTest();

    void ForkedAllocTest();

    void NonMovableLiveObjectAllocTest();

    void NonMovableUnliveObjectAllocTest();

    void MovableLiveObjectAllocTest();

    void MovableUnliveObjectAllocTest();

    void MuchObjectAllocTest();

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_ {nullptr};
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_TESTS_PYGOTE_SPACE_ALLOCATOR_TEST_H_
