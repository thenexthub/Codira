//===- Mutex.cpp - Mutual Exclusion Lock ------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file implements the toolchain::sys::Mutex class.
//
//===----------------------------------------------------------------------===//

#include <IndexStoreDB_LLVMSupport/toolchain_Support_Mutex.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Config_config.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_ErrorHandling.h>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#if !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled
namespace toolchain {
using namespace sys;
MutexImpl::MutexImpl( bool recursive) { }
MutexImpl::~MutexImpl() { }
bool MutexImpl::acquire() { return true; }
bool MutexImpl::release() { return true; }
bool MutexImpl::tryacquire() { return true; }
}
#else

#if defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_MUTEX_LOCK)

#include <cassert>
#include <pthread.h>
#include <stdlib.h>

namespace toolchain {
using namespace sys;

// Construct a Mutex using pthread calls
MutexImpl::MutexImpl( bool recursive)
  : data_(nullptr)
{
  // Declare the pthread_mutex data structures
  pthread_mutex_t* mutex =
    static_cast<pthread_mutex_t*>(safe_malloc(sizeof(pthread_mutex_t)));

  pthread_mutexattr_t attr;

  // Initialize the mutex attributes
  int errorcode = pthread_mutexattr_init(&attr);
  assert(errorcode == 0); (void)errorcode;

  // Initialize the mutex as a recursive mutex, if requested, or normal
  // otherwise.
  int kind = ( recursive  ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL );
  errorcode = pthread_mutexattr_settype(&attr, kind);
  assert(errorcode == 0);

  // Initialize the mutex
  errorcode = pthread_mutex_init(mutex, &attr);
  assert(errorcode == 0);

  // Destroy the attributes
  errorcode = pthread_mutexattr_destroy(&attr);
  assert(errorcode == 0);

  // Assign the data member
  data_ = mutex;
}

// Destruct a Mutex
MutexImpl::~MutexImpl()
{
  pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(data_);
  assert(mutex != nullptr);
  pthread_mutex_destroy(mutex);
  free(mutex);
}

bool
MutexImpl::acquire()
{
  pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(data_);
  assert(mutex != nullptr);

  int errorcode = pthread_mutex_lock(mutex);
  return errorcode == 0;
}

bool
MutexImpl::release()
{
  pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(data_);
  assert(mutex != nullptr);

  int errorcode = pthread_mutex_unlock(mutex);
  return errorcode == 0;
}

bool
MutexImpl::tryacquire()
{
  pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(data_);
  assert(mutex != nullptr);

  int errorcode = pthread_mutex_trylock(mutex);
  return errorcode == 0;
}

}

#elif defined(LLVM_ON_UNIX)
#include "Unix/Mutex.inc"
#elif defined( _WIN32)
#include "Windows/Mutex.inc"
#else
#warning Neither LLVM_ON_UNIX nor _WIN32 was set in Support/Mutex.cpp
#endif
#endif
