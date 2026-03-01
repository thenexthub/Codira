//===-- StreamString.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_STREAMSTRING_H
#define LLDB_UTILITY_STREAMSTRING_H

#include "lldb/Utility/Stream.h"
#include "lldb/lldb-enumerations.h"
#include "llvm/ADT/StringRef.h"

#include <string>

#include <cstddef>
#include <cstdint>

namespace lldb_private {

class ScriptInterpreter;

class StreamString : public Stream {
public:
  StreamString(bool colors = false);

  StreamString(uint32_t flags, uint32_t addr_size, lldb::ByteOrder byte_order);

  ~StreamString() override;

  void Flush() override;

  void Clear();

  bool Empty() const;

  size_t GetSize() const;

  size_t GetSizeOfLastLine() const;

  llvm::StringRef GetString() const;

  const char *GetData() const { return m_packet.c_str(); }

  void FillLastLineToColumn(uint32_t column, char fill_char);

protected:
  friend class ScriptInterpreter;

  std::string m_packet;
  size_t WriteImpl(const void *s, size_t length) override;
};

} // namespace lldb_private

#endif // LLDB_UTILITY_STREAMSTRING_H
