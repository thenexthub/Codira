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

#ifndef PANDA_RUNTIME_THREAD_SCOPES_H_
#define PANDA_RUNTIME_THREAD_SCOPES_H_

#include "mtmanaged_thread.h"

namespace ark {

#ifndef NDEBUG
class ScopedAssertManagedCode {
public:
    explicit ScopedAssertManagedCode()
    {
        ASSERT_MANAGED_CODE();
    }

    ~ScopedAssertManagedCode()
    {
        ASSERT_MANAGED_CODE();
    }

private:
    NO_COPY_SEMANTIC(ScopedAssertManagedCode);
    NO_MOVE_SEMANTIC(ScopedAssertManagedCode);
};

class ScopedAssertNativeCode {
public:
    explicit ScopedAssertNativeCode()
    {
        ASSERT_NATIVE_CODE();
    }

    ~ScopedAssertNativeCode()
    {
        ASSERT_NATIVE_CODE();
    }

private:
    NO_COPY_SEMANTIC(ScopedAssertNativeCode);
    NO_MOVE_SEMANTIC(ScopedAssertNativeCode);
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define ASSERT_SCOPED_MANAGED_CODE() ::ark::ScopedAssertManagedCode MERGE_WORDS(managed_scope_, __LINE__)
#define ASSERT_SCOPED_NATIVE_CODE() ::ark::ScopedAssertNativeCode MERGE_WORDS(native_scope_, __LINE__)
#else  // NDEBUG
#define ASSERT_SCOPED_MANAGED_CODE()
#define ASSERT_SCOPED_NATIVE_CODE()
#endif  // NDEBUG
// NOLINTEND(cppcoreguidelines-macro-usage)

class PANDA_PUBLIC_API ScopedNativeCodeThread {
public:
    explicit ScopedNativeCodeThread(ManagedThread *thread) : thread_(thread)
    {
        ASSERT(thread_ != nullptr);
        ASSERT(thread_ == ManagedThread::GetCurrent());
        thread_->NativeCodeBegin();
    }

    ~ScopedNativeCodeThread()
    {
        thread_->NativeCodeEnd();
    }

private:
    ManagedThread *thread_;

    NO_COPY_SEMANTIC(ScopedNativeCodeThread);
    NO_MOVE_SEMANTIC(ScopedNativeCodeThread);
};

class PANDA_PUBLIC_API ScopedManagedCodeThread {
public:
    explicit ScopedManagedCodeThread(ManagedThread *thread) : thread_(thread)
    {
        ASSERT(thread_ != nullptr);
        ASSERT(thread_ == ManagedThread::GetCurrent());
        thread_->ManagedCodeBegin();
    }

    ~ScopedManagedCodeThread()
    {
        thread_->ManagedCodeEnd();
    }

private:
    ManagedThread *thread_;

    NO_COPY_SEMANTIC(ScopedManagedCodeThread);
    NO_MOVE_SEMANTIC(ScopedManagedCodeThread);
};

class PANDA_PUBLIC_API ScopedChangeThreadStatus {
public:
    explicit ScopedChangeThreadStatus(ManagedThread *thread, ThreadStatus newStatus) : thread_(thread)
    {
        ASSERT(thread_ != nullptr);
        oldStatus_ = thread_->GetStatus();
        thread_->UpdateStatus(newStatus);
    }

    ~ScopedChangeThreadStatus()
    {
        ASSERT(thread_ != nullptr);
        thread_->UpdateStatus(oldStatus_);
    }

private:
    ManagedThread *thread_;
    ThreadStatus oldStatus_;

    NO_COPY_SEMANTIC(ScopedChangeThreadStatus);
    NO_MOVE_SEMANTIC(ScopedChangeThreadStatus);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_SCOPES_H_
