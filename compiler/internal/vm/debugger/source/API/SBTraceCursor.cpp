//===-- SBTraceCursor.cpp
//-------------------------------------------------------===//
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

#include "lldb/API/SBTraceCursor.h"
#include "Utils.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/Target/TraceCursor.h"

using namespace lldb;
using namespace lldb_private;

SBTraceCursor::SBTraceCursor() { LLDB_INSTRUMENT_VA(this); }

SBTraceCursor::SBTraceCursor(TraceCursorSP trace_cursor_sp)
    : m_opaque_sp{std::move(trace_cursor_sp)} {
  LLDB_INSTRUMENT_VA(this, trace_cursor_sp);
}

void SBTraceCursor::SetForwards(bool forwards) {
  LLDB_INSTRUMENT_VA(this, forwards);

  m_opaque_sp->SetForwards(forwards);
}

bool SBTraceCursor::IsForwards() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->IsForwards();
}

void SBTraceCursor::Next() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->Next();
}

bool SBTraceCursor::HasValue() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->HasValue();
}

bool SBTraceCursor::GoToId(lldb::user_id_t id) {
  LLDB_INSTRUMENT_VA(this, id);

  return m_opaque_sp->GoToId(id);
}

bool SBTraceCursor::HasId(lldb::user_id_t id) const {
  LLDB_INSTRUMENT_VA(this, id);

  return m_opaque_sp->HasId(id);
}

lldb::user_id_t SBTraceCursor::GetId() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->GetId();
}

bool SBTraceCursor::Seek(int64_t offset, lldb::TraceCursorSeekType origin) {
  LLDB_INSTRUMENT_VA(this, offset);

  return m_opaque_sp->Seek(offset, origin);
}

lldb::TraceItemKind SBTraceCursor::GetItemKind() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->GetItemKind();
}

bool SBTraceCursor::IsError() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->IsError();
}

const char *SBTraceCursor::GetError() const {
  LLDB_INSTRUMENT_VA(this);

  return ConstString(m_opaque_sp->GetError()).GetCString();
}

bool SBTraceCursor::IsEvent() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->IsEvent();
}

lldb::TraceEvent SBTraceCursor::GetEventType() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->GetEventType();
}

const char *SBTraceCursor::GetEventTypeAsString() const {
  LLDB_INSTRUMENT_VA(this);

  return ConstString(m_opaque_sp->GetEventTypeAsString()).GetCString();
}

bool SBTraceCursor::IsInstruction() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->IsInstruction();
}

lldb::addr_t SBTraceCursor::GetLoadAddress() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->GetLoadAddress();
}

lldb::cpu_id_t SBTraceCursor::GetCPU() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp->GetCPU();
}

bool SBTraceCursor::IsValid() const {
  LLDB_INSTRUMENT_VA(this);

  return this->operator bool();
}

SBTraceCursor::operator bool() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_sp.get() != nullptr;
}
