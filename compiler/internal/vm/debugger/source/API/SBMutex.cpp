//===-- SBMutex.cpp -------------------------------------------------------===//
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

#include "lldb/API/SBMutex.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/lldb-forward.h"
#include <memory>
#include <mutex>

using namespace lldb;
using namespace lldb_private;

SBMutex::SBMutex() : m_opaque_sp(std::make_shared<std::recursive_mutex>()) {
  LLDB_INSTRUMENT_VA(this);
}

SBMutex::SBMutex(const SBMutex &rhs) : m_opaque_sp(rhs.m_opaque_sp) {
  LLDB_INSTRUMENT_VA(this);
}

const SBMutex &SBMutex::operator=(const SBMutex &rhs) {
  LLDB_INSTRUMENT_VA(this);

  m_opaque_sp = rhs.m_opaque_sp;
  return *this;
}

SBMutex::SBMutex(lldb::TargetSP target_sp)
    : m_opaque_sp(std::shared_ptr<std::recursive_mutex>(
          target_sp, &target_sp->GetAPIMutex())) {
  LLDB_INSTRUMENT_VA(this, target_sp);
}

SBMutex::~SBMutex() { LLDB_INSTRUMENT_VA(this); }

bool SBMutex::IsValid() const {
  LLDB_INSTRUMENT_VA(this);

  return static_cast<bool>(m_opaque_sp);
}

void SBMutex::lock() const {
  LLDB_INSTRUMENT_VA(this);

  if (m_opaque_sp)
    m_opaque_sp->lock();
}

void SBMutex::unlock() const {
  LLDB_INSTRUMENT_VA(this);

  if (m_opaque_sp)
    m_opaque_sp->unlock();
}

bool SBMutex::try_lock() const {
  LLDB_INSTRUMENT_VA(this);

  if (m_opaque_sp)
    return m_opaque_sp->try_lock();

  return false;
}
