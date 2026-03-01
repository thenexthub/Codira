//===-- SBEnvironment.cpp -------------------------------------------------===//
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

#include "lldb/API/SBEnvironment.h"
#include "Utils.h"
#include "lldb/API/SBStringList.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/Environment.h"
#include "lldb/Utility/Instrumentation.h"

using namespace lldb;
using namespace lldb_private;

SBEnvironment::SBEnvironment() : m_opaque_up(new Environment()) {
  LLDB_INSTRUMENT_VA(this);
}

SBEnvironment::SBEnvironment(const SBEnvironment &rhs)
    : m_opaque_up(clone(rhs.m_opaque_up)) {
  LLDB_INSTRUMENT_VA(this, rhs);
}

SBEnvironment::SBEnvironment(Environment rhs)
    : m_opaque_up(new Environment(std::move(rhs))) {}

SBEnvironment::~SBEnvironment() = default;

const SBEnvironment &SBEnvironment::operator=(const SBEnvironment &rhs) {
  LLDB_INSTRUMENT_VA(this, rhs);

  if (this != &rhs)
    m_opaque_up = clone(rhs.m_opaque_up);
  return *this;
}

size_t SBEnvironment::GetNumValues() {
  LLDB_INSTRUMENT_VA(this);

  return m_opaque_up->size();
}

const char *SBEnvironment::Get(const char *name) {
  LLDB_INSTRUMENT_VA(this, name);

  auto entry = m_opaque_up->find(name);
  if (entry == m_opaque_up->end()) {
    return nullptr;
  }
  return ConstString(entry->second).AsCString("");
}

const char *SBEnvironment::GetNameAtIndex(size_t index) {
  LLDB_INSTRUMENT_VA(this, index);

  if (index >= GetNumValues())
    return nullptr;
  return ConstString(std::next(m_opaque_up->begin(), index)->first())
      .AsCString("");
}

const char *SBEnvironment::GetValueAtIndex(size_t index) {
  LLDB_INSTRUMENT_VA(this, index);

  if (index >= GetNumValues())
    return nullptr;
  return ConstString(std::next(m_opaque_up->begin(), index)->second)
      .AsCString("");
}

bool SBEnvironment::Set(const char *name, const char *value, bool overwrite) {
  LLDB_INSTRUMENT_VA(this, name, value, overwrite);

  if (overwrite) {
    m_opaque_up->insert_or_assign(name, std::string(value));
    return true;
  }
  return m_opaque_up->try_emplace(name, std::string(value)).second;
}

bool SBEnvironment::Unset(const char *name) {
  LLDB_INSTRUMENT_VA(this, name);

  return m_opaque_up->erase(name);
}

SBStringList SBEnvironment::GetEntries() {
  LLDB_INSTRUMENT_VA(this);

  SBStringList entries;
  for (const auto &KV : *m_opaque_up) {
    entries.AppendString(Environment::compose(KV).c_str());
  }
  return entries;
}

void SBEnvironment::PutEntry(const char *name_and_value) {
  LLDB_INSTRUMENT_VA(this, name_and_value);

  auto split = llvm::StringRef(name_and_value).split('=');
  m_opaque_up->insert_or_assign(split.first.str(), split.second.str());
}

void SBEnvironment::SetEntries(const SBStringList &entries, bool append) {
  LLDB_INSTRUMENT_VA(this, entries, append);

  if (!append)
    m_opaque_up->clear();
  for (size_t i = 0; i < entries.GetSize(); i++) {
    PutEntry(entries.GetStringAtIndex(i));
  }
}

void SBEnvironment::Clear() {
  LLDB_INSTRUMENT_VA(this);

  m_opaque_up->clear();
}

Environment &SBEnvironment::ref() const { return *m_opaque_up; }
