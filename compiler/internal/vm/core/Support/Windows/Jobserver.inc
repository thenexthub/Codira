//==- toolchain/Support/Windows/Jobserver.inc - Windows Jobserver Impl -*- C++ -*-=//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Windows-specific parts of the JobserverClient class.
// On Windows, the jobserver is implemented using a named semaphore.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/Windows/WindowsSupport.h"
#include <atomic>
#include <cassert>

namespace vm::core {
/// The constructor for the Windows jobserver client. It attempts to open a
/// handle to an existing named semaphore, the name of which is provided by
/// GNU make in the --jobserver-auth argument. If the semaphore is opened
/// successfully, the client is marked as initialized.
JobserverClientImpl::JobserverClientImpl(const JobserverConfig &Config) {
  Semaphore = (void *)::OpenSemaphoreA(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE,
                                       FALSE, Config.Path.c_str());
  if (Semaphore != nullptr)
    IsInitialized = true;
}

/// The destructor closes the handle to the semaphore, releasing the resource.
JobserverClientImpl::~JobserverClientImpl() {
  if (Semaphore != nullptr)
    ::CloseHandle((HANDLE)Semaphore);
}

/// Tries to acquire a job slot. The first call always returns the implicit
/// slot. Subsequent calls use a non-blocking wait on the semaphore
/// (`WaitForSingleObject` with a timeout of 0). If the wait succeeds, the
/// semaphore's count is decremented, and an explicit job slot is acquired.
/// If the wait times out, it means no slots are available, and an invalid
/// slot is returned.
JobSlot JobserverClientImpl::tryAcquire() {
  if (!IsInitialized)
    return JobSlot();

  // First, grant the implicit slot.
  if (HasImplicitSlot.exchange(false, std::memory_order_acquire)) {
    return JobSlot::createImplicit();
  }

  // Try to acquire a slot from the semaphore without blocking.
  if (::WaitForSingleObject((HANDLE)Semaphore, 0) == WAIT_OBJECT_0) {
    // The explicit token value is arbitrary on Windows, as the semaphore
    // count is the real resource.
    return JobSlot::createExplicit(1);
  }

  return JobSlot(); // Invalid slot
}

/// Releases a job slot back to the pool. If the slot is implicit, it simply
/// resets a flag. For an explicit slot, it increments the semaphore's count
/// by one using `ReleaseSemaphore`, making the slot available to other
/// processes.
void JobserverClientImpl::release(JobSlot Slot) {
  if (!IsInitialized || !Slot.isValid())
    return;

  if (Slot.isImplicit()) {
    [[maybe_unused]] bool was_already_released =
        HasImplicitSlot.exchange(true, std::memory_order_release);
    assert(!was_already_released && "Implicit slot released twice");
    return;
  }

  // Release the slot by incrementing the semaphore count.
  (void)::ReleaseSemaphore((HANDLE)Semaphore, 1, NULL);
}
} // namespace vm::core
