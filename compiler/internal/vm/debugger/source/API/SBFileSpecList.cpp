//===-- SBFileSpecList.cpp ------------------------------------------------===//
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

#include "lldb/API/SBFileSpecList.h"
#include "Utils.h"
#include "lldb/API/SBFileSpec.h"
#include "lldb/API/SBStream.h"
#include "lldb/Host/PosixApi.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/FileSpecList.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/Utility/Stream.h"

#include <climits>

using namespace lldb;
using namespace lldb_private;

SBFileSpecList::SBFileSpecList() : m_opaque_up(new FileSpecList()) {
  LLDB_INSTRUMENT_VA(this);
}

SBFileSpecList::SBFileSpecList(const SBFileSpecList &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  m_opaque_up = clone(rhs.m_opaque_up);
}

SBFileSpecList::~SBFileSpecList() = default;

const SBFileSpecList &SBFileSpecList::operator=(const SBFileSpecList &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_up = clone(rhs.m_opaque_up);
  return *this;
}

uint32_t SBFileSpecList::GetSize() const {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->GetSize();
}

void SBFileSpecList::Append(const SBFileSpec &sb_file) {
  LLDB_INSTRUMENT_VA(this, sb_file);

  m_opaque_up->Append(sb_file.ref());
}

bool SBFileSpecList::AppendIfUnique(const SBFileSpec &sb_file) {
  LLDB_INSTRUMENT_VA(this, sb_file);

  return m_opaque_up->AppendIfUnique(sb_file.ref());
}

void SBFileSpecList::Clear() {
  LLDB_INSTRUMENT_VA(this);

  m_opaque_up->Clear();
}

uint32_t SBFileSpecList::FindFileIndex(uint32_t idx, const SBFileSpec &sb_file,
                                       bool full) {
  LLDB_INSTRUMENT_VA(this, idx, sb_file, full);

  return m_opaque_up->FindFileIndex(idx, sb_file.ref(), full);
}

const SBFileSpec SBFileSpecList::GetFileSpecAtIndex(uint32_t idx) const {
  LLDB_INSTRUMENT_VA(this, idx);

  SBFileSpec new_spec;
  new_spec.SetFileSpec(m_opaque_up->GetFileSpecAtIndex(idx));
  return new_spec;
}

const lldb_private::FileSpecList *SBFileSpecList::operator->() const {
  return m_opaque_up.get();
}

const lldb_private::FileSpecList *SBFileSpecList::get() const {
  return m_opaque_up.get();
}

const lldb_private::FileSpecList &SBFileSpecList::operator*() const {
  return *m_opaque_up;
}

const lldb_private::FileSpecList &SBFileSpecList::ref() const {
  return *m_opaque_up;
}

bool SBFileSpecList::GetDescription(SBStream &description) const {
  LLDB_INSTRUMENT_VA(this, description);

  Stream &strm = description.ref();

  if (m_opaque_up) {
    uint32_t num_files = m_opaque_up->GetSize();
    strm.Printf("%d files: ", num_files);
    for (uint32_t i = 0; i < num_files; i++) {
      char path[PATH_MAX];
      if (m_opaque_up->GetFileSpecAtIndex(i).GetPath(path, sizeof(path)))
        strm.Printf("\n    %s", path);
    }
  } else
    strm.PutCString("No value");

  return true;
}
