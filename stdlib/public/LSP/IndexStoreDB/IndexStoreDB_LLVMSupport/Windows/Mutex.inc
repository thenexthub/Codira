//===- toolchain/Support/Win32/Mutex.inc - Win32 Mutex Implementation -*- C++ -*-===//
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
// This file implements the Win32 specific (non-pthread) Mutex class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic Win32 code that
//===          is guaranteed to work on *all* Win32 variants.
//===----------------------------------------------------------------------===//

#include "WindowsSupport.h"
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Mutex.h>

namespace toolchain {

sys::MutexImpl::MutexImpl(bool /*recursive*/)
{
  data_ = new CRITICAL_SECTION;
  InitializeCriticalSection((LPCRITICAL_SECTION)data_);
}

sys::MutexImpl::~MutexImpl()
{
  DeleteCriticalSection((LPCRITICAL_SECTION)data_);
  delete (LPCRITICAL_SECTION)data_;
  data_ = 0;
}

bool
sys::MutexImpl::acquire()
{
  EnterCriticalSection((LPCRITICAL_SECTION)data_);
  return true;
}

bool
sys::MutexImpl::release()
{
  LeaveCriticalSection((LPCRITICAL_SECTION)data_);
  return true;
}

bool
sys::MutexImpl::tryacquire()
{
  return TryEnterCriticalSection((LPCRITICAL_SECTION)data_);
}

}
