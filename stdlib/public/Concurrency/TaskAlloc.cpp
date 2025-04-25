//===--- TaskAlloc.cpp - Task-local stack allocator -----------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//
//
// A task-local allocator that obeys a stack discipline.
//
// Because allocation is task-local, and there's at most one thread
// running a task at once, no synchronization is required.
//
//===----------------------------------------------------------------------===//

#include "TaskPrivate.h"
#include "language/ABI/Task.h"
#include "language/Runtime/Concurrency.h"

#include <stdlib.h>

using namespace language;

namespace {

struct GlobalAllocator {
  TaskAllocator allocator;
  void *spaceForFirstSlab[64];

  GlobalAllocator() : allocator(spaceForFirstSlab, sizeof(spaceForFirstSlab)) {}
};

} // end anonymous namespace

static TaskAllocator &allocator(AsyncTask *task) {
  if (task)
    return task->Private.get().Allocator;

#if !SWIFT_CONCURRENCY_EMBEDDED
  // FIXME: this fall-back shouldn't be necessary, but it's useful
  // for now, since the current execution tests aren't setting up a task
  // properly.
  static GlobalAllocator global;
  return global.allocator;
#else
  puts("global allocator fallback not available\n");
  abort();
#endif
}

void *swift::swift_task_alloc(size_t size) {
  return allocator(swift_task_getCurrent()).alloc(size);
}

void *swift::_swift_task_alloc_specific(AsyncTask *task, size_t size) {
  return allocator(task).alloc(size);
}

void swift::swift_task_dealloc(void *ptr) {
  allocator(swift_task_getCurrent()).dealloc(ptr);
}

void swift::_swift_task_dealloc_specific(AsyncTask *task, void *ptr) {
  allocator(task).dealloc(ptr);
}

void swift::swift_task_dealloc_through(void *ptr) {
  allocator(swift_task_getCurrent()).deallocThrough(ptr);
}
void *swift::swift_job_allocate(Job *job, size_t size) {
  if (!job->isAsyncTask())
    return nullptr;

  return allocator(static_cast<AsyncTask *>(job)).alloc(size);
}

void swift::swift_job_deallocate(Job *job, void *ptr) {
  if (!job->isAsyncTask())
    return;

  allocator(static_cast<AsyncTask *>(job)).dealloc(ptr);
}
