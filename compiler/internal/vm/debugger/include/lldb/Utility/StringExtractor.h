//===-- StringExtractor.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_STRINGEXTRACTOR_H
#define LLDB_UTILITY_STRINGEXTRACTOR_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

#include <cstddef>
#include <cstdint>
#include <string>

class StringExtractor {
public:
  enum { BigEndian = 0, LittleEndian = 1 };
  // Constructors and Destructors
  StringExtractor();
  StringExtractor(llvm::StringRef packet_str);
  StringExtractor(const char *packet_cstr);
  virtual ~StringExtractor();

  void Reset(llvm::StringRef str) {
    m_packet = std::string(str);
    m_index = 0;
  }

  // Returns true if the file position is still valid for the data contained in
  // this string extractor object.
  bool IsGood() const { return m_index != UINT64_MAX; }

  uint64_t GetFilePos() const { return m_index; }

  void SetFilePos(uint32_t idx) { m_index = idx; }

  void Clear() {
    m_packet.clear();
    m_index = 0;
  }

  void SkipSpaces();

  llvm::StringRef GetStringRef() const { return m_packet; }

  bool Empty() { return m_packet.empty(); }

  size_t GetBytesLeft() {
    if (m_index < m_packet.size())
      return m_packet.size() - m_index;
    return 0;
  }

  char GetChar(char fail_value = '\0');

  char PeekChar(char fail_value = '\0') {
    const char *cstr = Peek();
    if (cstr)
      return cstr[0];
    return fail_value;
  }

  int DecodeHexU8();

  uint8_t GetHexU8(uint8_t fail_value = 0, bool set_eof_on_fail = true);

  bool GetHexU8Ex(uint8_t &ch, bool set_eof_on_fail = true);

  bool GetNameColonValue(llvm::StringRef &name, llvm::StringRef &value);

  int32_t GetS32(int32_t fail_value, int base = 0);

  uint32_t GetU32(uint32_t fail_value, int base = 0);

  int64_t GetS64(int64_t fail_value, int base = 0);

  uint64_t GetU64(uint64_t fail_value, int base = 0);

  uint32_t GetHexMaxU32(bool little_endian, uint32_t fail_value);

  uint64_t GetHexMaxU64(bool little_endian, uint64_t fail_value);

  size_t GetHexBytes(llvm::MutableArrayRef<uint8_t> dest,
                     uint8_t fail_fill_value);

  size_t GetHexBytesAvail(llvm::MutableArrayRef<uint8_t> dest);

  size_t GetHexByteString(std::string &str);

  size_t GetHexByteStringFixedLength(std::string &str, uint32_t nibble_length);

  size_t GetHexByteStringTerminatedBy(std::string &str, char terminator);

  bool ConsumeFront(const llvm::StringRef &str);

  const char *Peek() {
    if (m_index < m_packet.size())
      return m_packet.c_str() + m_index;
    return nullptr;
  }

protected:
  bool fail() {
    m_index = UINT64_MAX;
    return false;
  }

  /// The string in which to extract data.
  std::string m_packet;

  /// When extracting data from a packet, this index will march along as things
  /// get extracted. If set to UINT64_MAX the end of the packet data was
  /// reached when decoding information.
  uint64_t m_index = 0;
};

#endif // LLDB_UTILITY_STRINGEXTRACTOR_H
