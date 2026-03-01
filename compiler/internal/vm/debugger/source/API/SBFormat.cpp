//===-- SBFormat.cpp ------------------------------------------------------===//
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

#include "lldb/API/SBFormat.h"
#include "Utils.h"
#include "lldb/Core/FormatEntity.h"
#include "lldb/lldb-types.h"
#include <lldb/API/SBError.h>
#include <lldb/Utility/Status.h>

using namespace lldb;
using namespace lldb_private;

SBFormat::SBFormat() : m_opaque_sp() {}

SBFormat::SBFormat(const SBFormat &rhs) {
  m_opaque_sp = clone(rhs.m_opaque_sp);
}

SBFormat::~SBFormat() = default;

SBFormat &SBFormat::operator=(const SBFormat &rhs) {
  if (this != &rhs)
    m_opaque_sp = clone(rhs.m_opaque_sp);
  return *this;
}

SBFormat::operator bool() const { return (bool)m_opaque_sp; }

SBFormat::SBFormat(const char *format, lldb::SBError &error) {
  FormatEntrySP format_entry_sp = std::make_shared<FormatEntity::Entry>();
  Status status = FormatEntity::Parse(format, *format_entry_sp);

  error.SetError(std::move(status));
  if (error.Success())
    m_opaque_sp = format_entry_sp;
}

lldb::FormatEntrySP SBFormat::GetFormatEntrySP() const { return m_opaque_sp; }
