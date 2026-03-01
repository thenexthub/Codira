//===-- Checksum.cpp ------------------------------------------------------===//
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

#include "lldb/Utility/Checksum.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"

using namespace lldb_private;

Checksum::Checksum(llvm::MD5::MD5Result md5) { SetMD5(md5); }

Checksum::Checksum(const Checksum &checksum) { SetMD5(checksum.m_checksum); }

Checksum &Checksum::operator=(const Checksum &checksum) {
  SetMD5(checksum.m_checksum);
  return *this;
}

void Checksum::SetMD5(llvm::MD5::MD5Result md5) { m_checksum = md5; }

Checksum::operator bool() const { return !llvm::equal(m_checksum, g_sentinel); }

bool Checksum::operator==(const Checksum &checksum) const {
  return llvm::equal(m_checksum, checksum.m_checksum);
}

bool Checksum::operator!=(const Checksum &checksum) const {
  return !(*this == checksum);
}

std::string Checksum::digest() const {
  return std::string(m_checksum.digest());
}

llvm::MD5::MD5Result Checksum::g_sentinel = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
