//===- RWMutex.cpp - Reader/Writer Mutual Exclusion Lock --------*- C++ -*-===//
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
// This file implements the toolchain::sys::RWMutex class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/RWMutex.h"
#include "vm/core/Config/config.h"
#include "vm/core/Config/toolchain-config.h" // for LLVM_ENABLE_THREADS
#include "vm/core/Support/Allocator.h"

#if defined(LLVM_USE_RW_MUTEX_IMPL)
using namespace vm::core;
using namespace sys;

#if !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled

RWMutexImpl::RWMutexImpl() = default;
RWMutexImpl::~RWMutexImpl() = default;

bool RWMutexImpl::lock_shared() { return true; }
bool RWMutexImpl::unlock_shared() { return true; }
bool RWMutexImpl::try_lock_shared() { return true; }
bool RWMutexImpl::lock() { return true; }
bool RWMutexImpl::unlock() { return true; }
bool RWMutexImpl::try_lock() { return true; }

#else

#if defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#include <cassert>
#include <cstdlib>
#include <pthread.h>

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(safe_malloc(sizeof(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, sizeof(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  int errorcode = pthread_rwlock_init(rwlock, nullptr);
  (void)errorcode;
  assert(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);
  pthread_rwlock_destroy(rwlock);
  free(rwlock);
}

bool
RWMutexImpl::lock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);

  int errorcode = pthread_rwlock_rdlock(rwlock);
  return errorcode == 0;
}

bool
RWMutexImpl::unlock_shared()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);

  int errorcode = pthread_rwlock_unlock(rwlock);
  return errorcode == 0;
}

bool RWMutexImpl::try_lock_shared() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  assert(rwlock != nullptr);

  int errorcode = pthread_rwlock_tryrdlock(rwlock);
  return errorcode == 0;
}

bool
RWMutexImpl::lock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);

  int errorcode = pthread_rwlock_wrlock(rwlock);
  return errorcode == 0;
}

bool
RWMutexImpl::unlock()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != nullptr);

  int errorcode = pthread_rwlock_unlock(rwlock);
  return errorcode == 0;
}

bool RWMutexImpl::try_lock() {
  pthread_rwlock_t *rwlock = static_cast<pthread_rwlock_t *>(data_);
  assert(rwlock != nullptr);

  int errorcode = pthread_rwlock_trywrlock(rwlock);
  return errorcode == 0;
}

#else

RWMutexImpl::RWMutexImpl() : data_(new MutexImpl(false)) { }

RWMutexImpl::~RWMutexImpl() {
  delete static_cast<MutexImpl *>(data_);
}

bool RWMutexImpl::lock_shared() {
  return static_cast<MutexImpl *>(data_)->acquire();
}

bool RWMutexImpl::unlock_shared() {
  return static_cast<MutexImpl *>(data_)->release();
}

bool RWMutexImpl::try_lock_shared() {
  return static_cast<MutexImpl *>(data_)->tryacquire();
}

bool RWMutexImpl::lock() {
  return static_cast<MutexImpl *>(data_)->acquire();
}

bool RWMutexImpl::unlock() {
  return static_cast<MutexImpl *>(data_)->release();
}

bool RWMutexImpl::try_lock() {
  return static_cast<MutexImpl *>(data_)->tryacquire();
}

#endif
#endif
#endif
