//===- FileWriter.cpp -------------------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/GSYM/FileWriter.h"
#include "vm/core/Support/LEB128.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>

using namespace vm::core;
using namespace gsym;

FileWriter::~FileWriter() { OS.flush(); }

void FileWriter::writeSLEB(int64_t S) {
  uint8_t Bytes[32];
  auto Length = encodeSLEB128(S, Bytes);
  assert(Length < sizeof(Bytes));
  OS.write(reinterpret_cast<const char *>(Bytes), Length);
}

void FileWriter::writeULEB(uint64_t U) {
  uint8_t Bytes[32];
  auto Length = encodeULEB128(U, Bytes);
  assert(Length < sizeof(Bytes));
  OS.write(reinterpret_cast<const char *>(Bytes), Length);
}

void FileWriter::writeU8(uint8_t U) {
  OS.write(reinterpret_cast<const char *>(&U), sizeof(U));
}

void FileWriter::writeU16(uint16_t U) {
  const uint16_t Swapped = support::endian::byte_swap(U, ByteOrder);
  OS.write(reinterpret_cast<const char *>(&Swapped), sizeof(Swapped));
}

void FileWriter::writeU32(uint32_t U) {
  const uint32_t Swapped = support::endian::byte_swap(U, ByteOrder);
  OS.write(reinterpret_cast<const char *>(&Swapped), sizeof(Swapped));
}

void FileWriter::writeU64(uint64_t U) {
  const uint64_t Swapped = support::endian::byte_swap(U, ByteOrder);
  OS.write(reinterpret_cast<const char *>(&Swapped), sizeof(Swapped));
}

void FileWriter::fixup32(uint32_t U, uint64_t Offset) {
  const uint32_t Swapped = support::endian::byte_swap(U, ByteOrder);
  OS.pwrite(reinterpret_cast<const char *>(&Swapped), sizeof(Swapped),
            Offset);
}

void FileWriter::writeData(toolchain::ArrayRef<uint8_t> Data) {
  OS.write(reinterpret_cast<const char *>(Data.data()), Data.size());
}

void FileWriter::writeNullTerminated(toolchain::StringRef Str) {
  OS << Str << '\0';
}

uint64_t FileWriter::tell() {
  return OS.tell();
}

void FileWriter::alignTo(size_t Align) {
  off_t Offset = OS.tell();
  off_t AlignedOffset = (Offset + Align - 1) / Align * Align;
  if (AlignedOffset == Offset)
    return;
  off_t PadCount = AlignedOffset - Offset;
  OS.write_zeros(PadCount);
}
