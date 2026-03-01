//===-- Checksum.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_CHECKSUM_H
#define LLDB_UTILITY_CHECKSUM_H

#include "llvm/Support/MD5.h"

namespace lldb_private {
class Checksum {
public:
  static llvm::MD5::MD5Result g_sentinel;

  Checksum(llvm::MD5::MD5Result md5 = g_sentinel);
  Checksum(const Checksum &checksum);
  Checksum &operator=(const Checksum &checksum);

  explicit operator bool() const;
  bool operator==(const Checksum &checksum) const;
  bool operator!=(const Checksum &checksum) const;

  std::string digest() const;

private:
  void SetMD5(llvm::MD5::MD5Result);

  llvm::MD5::MD5Result m_checksum;
};
} // namespace lldb_private

#endif // LLDB_UTILITY_CHECKSUM_H
