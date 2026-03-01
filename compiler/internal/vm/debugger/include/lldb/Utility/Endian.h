//===-- Endian.h ------------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_ENDIAN_H
#define LLDB_UTILITY_ENDIAN_H

#include "lldb/lldb-enumerations.h"

#include <cstdint>

namespace lldb_private {

namespace endian {

static union EndianTest {
  uint32_t num;
  uint8_t bytes[sizeof(uint32_t)];
} const endianTest = {0x01020304};

inline lldb::ByteOrder InlHostByteOrder() {
  return static_cast<lldb::ByteOrder>(endianTest.bytes[0]);
}

//    ByteOrder const InlHostByteOrder = (ByteOrder)endianTest.bytes[0];
}
}

#endif // LLDB_UTILITY_ENDIAN_H
